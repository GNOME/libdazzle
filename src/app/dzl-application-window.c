/* dzl-application-window.c
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define G_LOG_DOMAIN "dzl-application-window"

#include "config.h"

#include "app/dzl-application-window.h"
#include "shortcuts/dzl-shortcut-manager.h"
#include "util/dzl-gtk.h"
#include "util/dzl-macros.h"

#define DEFAULT_DISMISSAL_SECONDS   3
#define SHOW_HEADER_WITHIN_DISTANCE 5

/**
 * SECTION:dzl-application-window
 * @title: DzlApplicationWindow
 * @short_description: An base application window for applications
 *
 * The #DzlApplicationWindow class provides a #GtkApplicationWindow subclass
 * that integrates well with #DzlApplication. It provides features such as:
 *
 *  - Integration with the #DzlShortcutManager for capture/bubble keyboard
 *    input events.
 *  - Native support for fullscreen state by re-parenting the #GtkHeaderBar as
 *    necessary. #DzlApplicationWindow does expect you to use GtkHeaderBar.
 *
 * Since: 3.26
 */

typedef struct
{
  GtkStack    *titlebar_container;
  GtkRevealer *titlebar_revealer;
  GtkEventBox *event_box;
  GtkOverlay  *overlay;

  gulong       motion_notify_handler;

  guint        fullscreen_source;
  guint        fullscreen_reveal_source;
  guint        fullscreen : 1;
  guint        in_key_press : 1;
} DzlApplicationWindowPrivate;

enum {
  PROP_0,
  PROP_FULLSCREEN,
  N_PROPS
};

static void buildable_iface_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (DzlApplicationWindow, dzl_application_window, GTK_TYPE_APPLICATION_WINDOW,
                         G_ADD_PRIVATE (DzlApplicationWindow)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, buildable_iface_init))

static GParamSpec *properties [N_PROPS];
static GtkBuildableIface *parent_buildable;

static gboolean
dzl_application_window_dismissal (DzlApplicationWindow *self)
{
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);

  g_assert (DZL_IS_APPLICATION_WINDOW (self));

  priv->fullscreen_reveal_source = 0;

  if (dzl_application_window_get_fullscreen (self))
    gtk_revealer_set_reveal_child (priv->titlebar_revealer, FALSE);

  return G_SOURCE_REMOVE;
}

static void
dzl_application_window_queue_dismissal (DzlApplicationWindow *self)
{
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);

  g_assert (DZL_IS_APPLICATION_WINDOW (self));

  if (priv->fullscreen_reveal_source != 0)
    g_source_remove (priv->fullscreen_reveal_source);

  priv->fullscreen_reveal_source =
    gdk_threads_add_timeout_seconds_full (G_PRIORITY_LOW,
                                          DEFAULT_DISMISSAL_SECONDS,
                                          (GSourceFunc) dzl_application_window_dismissal,
                                          self, NULL);
}

static gboolean
dzl_application_window_real_get_fullscreen (DzlApplicationWindow *self)
{
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);

  g_assert (DZL_IS_APPLICATION_WINDOW (self));

  return priv->fullscreen;
}

static void
revealer_set_reveal_child_now (GtkRevealer *revealer,
                               gboolean     reveal_child)
{

  g_assert (GTK_IS_REVEALER (revealer));

  gtk_revealer_set_transition_type (revealer, GTK_REVEALER_TRANSITION_TYPE_NONE);
  gtk_revealer_set_reveal_child (revealer, reveal_child);
  gtk_revealer_set_transition_type (revealer, GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
}

static gboolean
dzl_application_window_event_box_motion (DzlApplicationWindow *self,
                                         GdkEventMotion       *motion,
                                         GtkEventBox          *event_box)
{
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);
  GtkWidget *widget = NULL;
  GtkWidget *focus;
  gint x = 0;
  gint y = 0;

  g_assert (DZL_IS_APPLICATION_WINDOW (self));
  g_assert (motion != NULL);
  g_assert (GTK_IS_EVENT_BOX (event_box));

  /*
   * If we are focused in the revealer, ignore this.  We will have already
   * killed the GSource in set_focus().
   */
  focus = gtk_window_get_focus (GTK_WINDOW (self));
  if (focus != NULL &&
      dzl_gtk_widget_is_ancestor_or_relative (focus, GTK_WIDGET (priv->titlebar_revealer)))
    return GDK_EVENT_PROPAGATE;

  /* The widget is stored in GdkWindow user_data */
  gdk_window_get_user_data (motion->window, (gpointer *)&widget);
  if (widget == NULL)
    return GDK_EVENT_PROPAGATE;

  /* If the headerbar is underneath the pointer or we are within a
   * small distance of the edge of the window (and monitor), ensure
   * that the titlebar is displayed and queue our next dismissal.
   */
  if (dzl_gtk_widget_is_ancestor_or_relative (widget, GTK_WIDGET (priv->titlebar_revealer)) ||
      gtk_widget_translate_coordinates (widget, GTK_WIDGET (self), motion->x, motion->y, &x, &y))
    {
      if (y < SHOW_HEADER_WITHIN_DISTANCE)
        {
          gtk_revealer_set_reveal_child (priv->titlebar_revealer, TRUE);
          dzl_application_window_queue_dismissal (self);
        }
    }

  return GDK_EVENT_PROPAGATE;
}

static gboolean
dzl_application_window_complete_fullscreen (DzlApplicationWindow *self)
{
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);
  GtkWidget *titlebar;

  g_assert (DZL_IS_APPLICATION_WINDOW (self));

  priv->fullscreen_source = 0;

  titlebar = dzl_application_window_get_titlebar (self);

  if (titlebar == NULL)
    {
      g_warning ("Attempt to alter fullscreen state without a titlebar set!");
      return G_SOURCE_REMOVE;
    }

  /*
   * This is where we apply the fullscreen widget transitions.
   *
   * If we were toggled really fast, we could have skipped oure previous
   * operation. So make sure that the revealer is in the state we expect
   * it before performing further (destructive) work.
   */

  g_object_ref (titlebar);

  if (priv->fullscreen)
    {
      /* Only listen for motion notify events while in fullscreen mode. */
      g_signal_handler_unblock (priv->event_box, priv->motion_notify_handler);

      if (titlebar && gtk_widget_is_ancestor (titlebar, GTK_WIDGET (priv->titlebar_container)))
        {
          revealer_set_reveal_child_now (priv->titlebar_revealer, FALSE);
          gtk_container_remove (GTK_CONTAINER (priv->titlebar_container), titlebar);
          gtk_container_add (GTK_CONTAINER (priv->titlebar_revealer), titlebar);
          gtk_revealer_set_reveal_child (priv->titlebar_revealer, TRUE);
          dzl_application_window_queue_dismissal (self);
        }
    }
  else
    {
      /* Motion events are no longer needed */
      g_signal_handler_block (priv->event_box, priv->motion_notify_handler);

      if (gtk_widget_is_ancestor (titlebar, GTK_WIDGET (priv->titlebar_revealer)))
        {
          gtk_container_remove (GTK_CONTAINER (priv->titlebar_revealer), titlebar);
          gtk_container_add (GTK_CONTAINER (priv->titlebar_container), titlebar);
          revealer_set_reveal_child_now (priv->titlebar_revealer, FALSE);
        }
    }

  g_object_unref (titlebar);

  return G_SOURCE_REMOVE;
}

static void
dzl_application_window_real_set_fullscreen (DzlApplicationWindow *self,
                                            gboolean              fullscreen)
{
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);

  g_assert (DZL_IS_APPLICATION_WINDOW (self));
  g_assert (priv->fullscreen != fullscreen);

  priv->fullscreen = fullscreen;

  if (priv->fullscreen_source != 0)
    {
      g_source_remove (priv->fullscreen_source);
      priv->fullscreen_source = 0;
    }

  if (priv->fullscreen)
    {
      /*
       * Once we go fullscreen, the headerbar will no longer be visible.
       * So we will delay for a short bit until we've likely entered the
       * fullscreen state, and then remove the titlebar and place it into
       * the hover state.
       */
      priv->fullscreen_source =
        gdk_threads_add_timeout_full (G_PRIORITY_LOW,
                                      300,
                                      (GSourceFunc) dzl_application_window_complete_fullscreen,
                                      self, NULL);
      gtk_window_fullscreen (GTK_WINDOW (self));
    }
  else
    {
      /*
       * We must apply our unfullscreen state transition immediately
       * so that we have a titlebar as soon as the window changes.
       */
      dzl_application_window_complete_fullscreen (self);
      gtk_window_unfullscreen (GTK_WINDOW (self));
    }

  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_FULLSCREEN]);
}

static void
dzl_application_window_set_focus (GtkWindow *window,
                                  GtkWidget *widget)
{
  DzlApplicationWindow *self = (DzlApplicationWindow *)window;
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);
  GtkWidget *old_focus;
  gboolean titlebar_had_focus = FALSE;

  g_assert (DZL_IS_APPLICATION_WINDOW (self));
  g_assert (!widget || GTK_IS_WIDGET (widget));

  /* Check if we have titlebar focus already */
  old_focus = gtk_window_get_focus (window);
  if (old_focus != NULL &&
      dzl_gtk_widget_is_ancestor_or_relative (old_focus, GTK_WIDGET (priv->titlebar_revealer)))
    titlebar_had_focus = TRUE;

  /* Chain-up first */
  GTK_WINDOW_CLASS (dzl_application_window_parent_class)->set_focus (window, widget);

  /* Now see what is selected */
  widget = gtk_window_get_focus (window);

  if (widget != NULL)
    {
      if (dzl_gtk_widget_is_ancestor_or_relative (widget, GTK_WIDGET (priv->titlebar_revealer)))
        {
          /* Disable transition while the revealer is focused */
          if (priv->fullscreen_reveal_source != 0)
            {
              g_source_remove (priv->fullscreen_reveal_source);
              priv->fullscreen_reveal_source = 0;
            }

          /* If this was just focused, we might need to make it visible */
          gtk_revealer_set_reveal_child (priv->titlebar_revealer, TRUE);
        }
      else if (titlebar_had_focus)
        {
          /* We are going from titlebar to non-titlebar focus. Dismiss
           * the titlebar immediately to get out of the users way.
           */
          gtk_revealer_set_reveal_child (priv->titlebar_revealer, FALSE);
          if (priv->fullscreen_reveal_source != 0)
            {
              g_source_remove (priv->fullscreen_reveal_source);
              priv->fullscreen_reveal_source = 0;
            }
        }
    }
}

static void
dzl_application_window_add (GtkContainer *container,
                            GtkWidget    *widget)
{
  DzlApplicationWindow *self = (DzlApplicationWindow *)container;
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);

  g_assert (DZL_IS_APPLICATION_WINDOW (self));
  g_assert (GTK_IS_WIDGET (widget));

  gtk_container_add (GTK_CONTAINER (priv->event_box), widget);
}

static gboolean
dzl_application_window_key_press_event (GtkWidget   *widget,
                                        GdkEventKey *event)
{
  DzlApplicationWindow *self = (DzlApplicationWindow *)widget;
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);
  gboolean ret;

  g_assert (DZL_IS_APPLICATION_WINDOW (self));
  g_assert (event != NULL);

  /* Be re-entrant safe from the shortcut manager */
  if (priv->in_key_press)
    return GTK_WIDGET_CLASS (dzl_application_window_parent_class)->key_press_event (widget, event);

  priv->in_key_press = TRUE;
  ret = dzl_shortcut_manager_handle_event (NULL, event, widget);
  priv->in_key_press = FALSE;

  return ret;
}

static void
dzl_application_window_destroy (GtkWidget *widget)
{
  DzlApplicationWindow *self = (DzlApplicationWindow *)widget;
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);

  g_assert (DZL_IS_APPLICATION_WINDOW (self));

  if (priv->event_box != NULL)
    {
      g_signal_handler_disconnect (priv->event_box, priv->motion_notify_handler);
      priv->motion_notify_handler = 0;
    }

  dzl_clear_pointer ((GtkWidget **)&priv->titlebar_container, gtk_widget_destroy);
  dzl_clear_pointer ((GtkWidget **)&priv->titlebar_revealer, gtk_widget_destroy);
  dzl_clear_pointer ((GtkWidget **)&priv->event_box, gtk_widget_destroy);
  dzl_clear_pointer ((GtkWidget **)&priv->overlay, gtk_widget_destroy);

  dzl_clear_source (&priv->fullscreen_source);
  dzl_clear_source (&priv->fullscreen_reveal_source);

  GTK_WIDGET_CLASS (dzl_application_window_parent_class)->destroy (widget);
}

static void
dzl_application_window_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  DzlApplicationWindow *self = DZL_APPLICATION_WINDOW (object);

  switch (prop_id)
    {
    case PROP_FULLSCREEN:
      g_value_set_boolean (value, dzl_application_window_get_fullscreen (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_application_window_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  DzlApplicationWindow *self = DZL_APPLICATION_WINDOW (object);

  switch (prop_id)
    {
    case PROP_FULLSCREEN:
      dzl_application_window_set_fullscreen (self, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_application_window_class_init (DzlApplicationWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
  GtkWindowClass *window_class = GTK_WINDOW_CLASS (klass);

  object_class->get_property = dzl_application_window_get_property;
  object_class->set_property = dzl_application_window_set_property;

  widget_class->destroy = dzl_application_window_destroy;
  widget_class->key_press_event = dzl_application_window_key_press_event;

  container_class->add = dzl_application_window_add;

  window_class->set_focus = dzl_application_window_set_focus;

  klass->get_fullscreen = dzl_application_window_real_get_fullscreen;
  klass->set_fullscreen = dzl_application_window_real_set_fullscreen;

  /**
   * DzlApplicationWindow:fullscreen:
   *
   * The "fullscreen" property denotes if the window is in the fullscreen
   * state. The titlebar of the #DzlApplicationWindow contains a revealer
   * which will be repurposed into a floating bar while the window is in
   * the fullscreen mode.
   *
   * Set this property to %FALSE to unfullscreen.
   */
  properties [PROP_FULLSCREEN] =
    g_param_spec_boolean ("fullscreen",
                          "Fullscreen",
                          "If the window is fullscreen",
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_application_window_init (DzlApplicationWindow *self)
{
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);
  g_autoptr(GPropertyAction) fullscreen = NULL;

  priv->titlebar_container = g_object_new (GTK_TYPE_STACK,
                                           "name", "titlebar_container",
                                           "visible", TRUE,
                                           NULL);
  g_signal_connect (priv->titlebar_container,
                    "destroy",
                    G_CALLBACK (gtk_widget_destroyed),
                    &priv->titlebar_container);
  gtk_window_set_titlebar (GTK_WINDOW (self), GTK_WIDGET (priv->titlebar_container));

  priv->overlay = g_object_new (GTK_TYPE_OVERLAY,
                                "visible", TRUE,
                                NULL);
  g_signal_connect (priv->overlay,
                    "destroy",
                    G_CALLBACK (gtk_widget_destroyed),
                    &priv->overlay);
  GTK_CONTAINER_CLASS (dzl_application_window_parent_class)->add (GTK_CONTAINER (self),
                                                                  GTK_WIDGET (priv->overlay));

  priv->event_box = g_object_new (GTK_TYPE_EVENT_BOX,
                                  "above-child", FALSE,
                                  "visible", TRUE,
                                  "visible-window", TRUE,
                                  NULL);
  gtk_widget_set_events (GTK_WIDGET (priv->event_box), GDK_POINTER_MOTION_MASK);
  g_signal_connect (priv->event_box,
                    "destroy",
                    G_CALLBACK (gtk_widget_destroyed),
                    &priv->event_box);
  priv->motion_notify_handler =
    g_signal_connect_swapped (priv->event_box,
                              "motion-notify-event",
                              G_CALLBACK (dzl_application_window_event_box_motion),
                              self);
  g_signal_handler_block (priv->event_box, priv->motion_notify_handler);
  gtk_container_add (GTK_CONTAINER (priv->overlay), GTK_WIDGET (priv->event_box));

  priv->titlebar_revealer = g_object_new (GTK_TYPE_REVEALER,
                                          "valign", GTK_ALIGN_START,
                                          "hexpand", TRUE,
                                          "transition-duration", 500,
                                          "transition-type", GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN,
                                          "reveal-child", TRUE,
                                          "visible", TRUE,
                                          NULL);
  g_signal_connect (priv->titlebar_revealer,
                    "destroy",
                    G_CALLBACK (gtk_widget_destroyed),
                    &priv->titlebar_revealer);
  gtk_overlay_add_overlay (priv->overlay, GTK_WIDGET (priv->titlebar_revealer));

  fullscreen = g_property_action_new ("fullscreen", self, "fullscreen");
  g_action_map_add_action (G_ACTION_MAP (self), G_ACTION (fullscreen));
}

/**
 * dzl_application_window_get_fullscreen:
 * @self: a #DzlApplicationWindow
 *
 * Gets if the window is in the fullscreen state.
 *
 * Returns: %TRUE if @self is fullscreen, otherwise %FALSE.
 *
 * Since: 3.26
 */
gboolean
dzl_application_window_get_fullscreen (DzlApplicationWindow *self)
{
  g_return_val_if_fail (DZL_IS_APPLICATION_WINDOW (self), FALSE);

  return DZL_APPLICATION_WINDOW_GET_CLASS (self)->get_fullscreen (self);
}

/**
 * dzl_application_window_set_fullscreen:
 * @self: a #DzlApplicationWindow
 * @fullscreen: if the window should be in the fullscreen state
 *
 * Sets the #DzlApplicationWindow into either the fullscreen or unfullscreen
 * state based on @fullscreen.
 *
 * The titlebar for the window is contained within a #GtkRevealer which is
 * repurposed as a floating bar when the application is in fullscreen mode.
 *
 * See dzl_application_window_get_fullscreen() to get the current fullscreen
 * state.
 *
 * Since: 3.26
 */
void
dzl_application_window_set_fullscreen (DzlApplicationWindow *self,
                                       gboolean              fullscreen)
{
  g_return_if_fail (DZL_IS_APPLICATION_WINDOW (self));

  fullscreen = !!fullscreen;

  if (fullscreen != dzl_application_window_get_fullscreen (self))
    DZL_APPLICATION_WINDOW_GET_CLASS (self)->set_fullscreen (self, fullscreen);
}

static void
dzl_application_window_add_child (GtkBuildable *buildable,
                                  GtkBuilder   *builder,
                                  GObject      *child,
                                  const gchar  *type)
{
  DzlApplicationWindow *self = (DzlApplicationWindow *)buildable;
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);

  g_assert (DZL_IS_APPLICATION_WINDOW (self));
  g_assert (GTK_IS_BUILDER (builder));
  g_assert (G_IS_OBJECT (child));

  if (g_strcmp0 (type, "titlebar") == 0)
    gtk_container_add (GTK_CONTAINER (priv->titlebar_container), GTK_WIDGET (child));
  else
    parent_buildable->add_child (buildable, builder, child, type);
}

static void
buildable_iface_init (GtkBuildableIface *iface)
{
  parent_buildable = g_type_interface_peek_parent (iface);

  iface->add_child = dzl_application_window_add_child;
}

/**
 * dzl_application_window_get_titlebar:
 * @self: a #DzlApplicationWindow
 *
 * Gets the titlebar for the window, if there is one.
 *
 * Returns: (transfer none): A #GtkWidget or %NULL
 *
 * Since: 3.26
 */
GtkWidget *
dzl_application_window_get_titlebar (DzlApplicationWindow *self)
{
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);
  GtkWidget *child;

  g_return_val_if_fail (DZL_IS_APPLICATION_WINDOW (self), NULL);

  child = gtk_stack_get_visible_child (priv->titlebar_container);
  if (child == NULL)
    child = gtk_bin_get_child (GTK_BIN (priv->titlebar_revealer));

  return child;
}

/**
 * dzl_application_window_set_titlebar:
 * @self: a #DzlApplicationWindow
 *
 * Sets the titlebar for the window.
 *
 * Generally, you want to do this from your GTK ui template by setting
 * the &lt;child type="titlebar"&gt;
 *
 * Since: 3.26
 */
void
dzl_application_window_set_titlebar (DzlApplicationWindow *self,
                                     GtkWidget            *titlebar)
{
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);

  g_return_if_fail (DZL_IS_APPLICATION_WINDOW (self));
  g_return_if_fail (GTK_IS_WIDGET (titlebar));

  if (titlebar != NULL)
    gtk_container_add (GTK_CONTAINER (priv->titlebar_container), titlebar);
}
