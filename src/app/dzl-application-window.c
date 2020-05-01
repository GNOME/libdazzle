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

#include <gtk/gtk.h>

#if !GTK_CHECK_VERSION(3, 24, 0)
# include "backports/gtkeventcontrollermotion.c"
#endif

#include "dzl-enums.h"

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
  GtkOverlay  *overlay;

  GtkEventController *motion_controller;
  gulong              motion_controller_handler;

  DzlTitlebarAnimation  last_titlebar_animation;

  guint        fullscreen_source;
  guint        fullscreen_reveal_source;
  guint        titlebar_hiding;

  guint        fullscreen : 1;
  guint        in_key_press : 1;
} DzlApplicationWindowPrivate;

enum {
  PROP_0,
  PROP_FULLSCREEN,
  PROP_TITLEBAR_ANIMATION,
  N_PROPS
};

static void buildable_iface_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (DzlApplicationWindow, dzl_application_window, GTK_TYPE_APPLICATION_WINDOW,
                         G_ADD_PRIVATE (DzlApplicationWindow)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, buildable_iface_init))

static GParamSpec *properties [N_PROPS];
static GtkBuildableIface *parent_buildable;

static void
update_titlebar_animation_property (DzlApplicationWindow *self)
{
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);
  DzlTitlebarAnimation current;

  g_assert (DZL_IS_APPLICATION_WINDOW (self));

  current = dzl_application_window_get_titlebar_animation (self);

  if (current != priv->last_titlebar_animation)
    {
      priv->last_titlebar_animation = current;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_TITLEBAR_ANIMATION]);
    }
}

static gboolean
dzl_application_window_titlebar_hidden_cb (gpointer data)
{
  DzlApplicationWindow *self = data;
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);

  g_assert (DZL_IS_APPLICATION_WINDOW (self));

  priv->titlebar_hiding--;
  update_titlebar_animation_property (self);

  return G_SOURCE_REMOVE;
}

static gboolean
dzl_application_window_dismissal (DzlApplicationWindow *self)
{
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);

  g_assert (DZL_IS_APPLICATION_WINDOW (self));

  if (dzl_application_window_get_fullscreen (self))
    {
      priv->titlebar_hiding++;
      gtk_revealer_set_reveal_child (priv->titlebar_revealer, FALSE);
      g_timeout_add_full (G_PRIORITY_DEFAULT,
                          gtk_revealer_get_transition_duration (priv->titlebar_revealer),
                          dzl_application_window_titlebar_hidden_cb,
                          g_object_ref (self),
                          g_object_unref);
    }

  update_titlebar_animation_property (self);

  priv->fullscreen_reveal_source = 0;

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

static void
dzl_application_window_motion_cb (DzlApplicationWindow     *self,
                                  gdouble                   x,
                                  gdouble                   y,
                                  GtkEventControllerMotion *controller)
{
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);
  GtkWidget *focus;

  g_assert (DZL_IS_APPLICATION_WINDOW (self));
  g_assert (GTK_IS_EVENT_CONTROLLER_MOTION (controller));

  /*
   * If we are focused in the revealer, ignore this.  We will have already
   * killed the GSource in set_focus().
   */
  focus = gtk_window_get_focus (GTK_WINDOW (self));
  if (focus != NULL &&
      dzl_gtk_widget_is_ancestor_or_relative (focus, GTK_WIDGET (priv->titlebar_revealer)))
    return;

  /* If the headerbar is underneath the pointer or we are within a
   * small distance of the edge of the window (and monitor), ensure
   * that the titlebar is displayed and queue our next dismissal.
   */
  if (y <= SHOW_HEADER_WITHIN_DISTANCE)
    {
      gtk_revealer_set_reveal_child (priv->titlebar_revealer, TRUE);
      dzl_application_window_queue_dismissal (self);
    }
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
      gtk_event_controller_set_propagation_phase (priv->motion_controller, GTK_PHASE_CAPTURE);

      if (titlebar && gtk_widget_is_ancestor (titlebar, GTK_WIDGET (priv->titlebar_container)))
        {
          revealer_set_reveal_child_now (priv->titlebar_revealer, FALSE);
          gtk_container_remove (GTK_CONTAINER (priv->titlebar_container), titlebar);
          gtk_container_add (GTK_CONTAINER (priv->titlebar_revealer), titlebar);
          revealer_set_reveal_child_now (priv->titlebar_revealer, TRUE);
          dzl_application_window_queue_dismissal (self);
        }
    }
  else
    {
      /* Motion events are no longer needed */
      gtk_event_controller_set_propagation_phase (priv->motion_controller, GTK_PHASE_NONE);

      if (gtk_widget_is_ancestor (titlebar, GTK_WIDGET (priv->titlebar_revealer)))
        {
          gtk_container_remove (GTK_CONTAINER (priv->titlebar_revealer), titlebar);
          gtk_container_add (GTK_CONTAINER (priv->titlebar_container), titlebar);
          revealer_set_reveal_child_now (priv->titlebar_revealer, FALSE);
        }
    }

  g_object_unref (titlebar);

  update_titlebar_animation_property (self);

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

  dzl_clear_source (&priv->fullscreen_source);

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
          dzl_clear_source (&priv->fullscreen_reveal_source);

          /* If this was just focused, we might need to make it visible */
          gtk_revealer_set_reveal_child (priv->titlebar_revealer, TRUE);
        }
      else if (titlebar_had_focus)
        {
          /* We are going from titlebar to non-titlebar focus. Dismiss
           * the titlebar immediately to get out of the users way.
           */
          gtk_revealer_set_reveal_child (priv->titlebar_revealer, FALSE);
          dzl_clear_source (&priv->fullscreen_reveal_source);
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

  gtk_container_add (GTK_CONTAINER (priv->overlay), widget);
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

static gboolean
dzl_application_window_window_state_event (GtkWidget           *widget,
                                           GdkEventWindowState *event)
{
  DzlApplicationWindow *self = (DzlApplicationWindow *)widget;
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);
  gboolean ret;

  g_assert (DZL_IS_APPLICATION_WINDOW (self));
  g_assert (event != NULL);

  ret = GTK_WIDGET_CLASS (dzl_application_window_parent_class)->window_state_event (widget, event);

  /* Clear our fullscreen state if the window-manager un-fullscreened us.
   * This fixes an issue on gnome 3.30 (and other window managers) that can
   * clear the fullscreen state out from under the app.
   */
  if (priv->fullscreen && (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN) == 0)
    dzl_application_window_set_fullscreen (self, FALSE);

  return ret;
}

static void
dzl_application_window_revealer_notify_child_state (DzlApplicationWindow *self,
                                                    GParamSpec           *pspec,
                                                    GtkRevealer          *revealer)
{
  g_assert (DZL_IS_APPLICATION_WINDOW (self));
  g_assert (GTK_IS_REVEALER (revealer));

  update_titlebar_animation_property (self);
}

static void
dzl_application_window_destroy (GtkWidget *widget)
{
  DzlApplicationWindow *self = (DzlApplicationWindow *)widget;
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);

  g_assert (DZL_IS_APPLICATION_WINDOW (self));

  g_clear_object (&priv->motion_controller);

  g_clear_pointer ((GtkWidget **)&priv->titlebar_container, gtk_widget_destroy);
  g_clear_pointer ((GtkWidget **)&priv->titlebar_revealer, gtk_widget_destroy);
  g_clear_pointer ((GtkWidget **)&priv->overlay, gtk_widget_destroy);

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

    case PROP_TITLEBAR_ANIMATION:
      g_value_set_enum (value, dzl_application_window_get_titlebar_animation (self));
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
  widget_class->window_state_event = dzl_application_window_window_state_event;

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

  properties [PROP_TITLEBAR_ANIMATION] =
    g_param_spec_enum ("titlebar-animation",
                       "Titlebar Animation",
                       "The state of the titlebar animation",
                       DZL_TYPE_TITLEBAR_ANIMATION,
                       DZL_TITLEBAR_ANIMATION_SHOWN,
                       (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_application_window_init (DzlApplicationWindow *self)
{
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);
  g_autoptr(GPropertyAction) fullscreen = NULL;

  priv->last_titlebar_animation = DZL_TITLEBAR_ANIMATION_SHOWN;

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
  gtk_widget_set_events (GTK_WIDGET (priv->overlay), GDK_POINTER_MOTION_MASK);
  g_signal_connect (priv->overlay,
                    "destroy",
                    G_CALLBACK (gtk_widget_destroyed),
                    &priv->overlay);
  GTK_CONTAINER_CLASS (dzl_application_window_parent_class)->add (GTK_CONTAINER (self),
                                                                  GTK_WIDGET (priv->overlay));

  priv->motion_controller = gtk_event_controller_motion_new (GTK_WIDGET (priv->overlay));
  priv->motion_controller_handler =
    g_signal_connect_swapped (priv->motion_controller,
                              "motion",
                              G_CALLBACK (dzl_application_window_motion_cb),
                              self);
  gtk_event_controller_set_propagation_phase (priv->motion_controller, GTK_PHASE_NONE);

  priv->titlebar_revealer = g_object_new (GTK_TYPE_REVEALER,
                                          "valign", GTK_ALIGN_START,
                                          "hexpand", TRUE,
                                          "transition-duration", 500,
                                          "transition-type", GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN,
                                          "reveal-child", TRUE,
                                          "visible", TRUE,
                                          NULL);
  g_signal_connect_object (priv->titlebar_revealer,
                           "notify::child-revealed",
                           G_CALLBACK (dzl_application_window_revealer_notify_child_state),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->titlebar_revealer,
                           "notify::reveal-child",
                           G_CALLBACK (dzl_application_window_revealer_notify_child_state),
                           self,
                           G_CONNECT_SWAPPED);
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
    {
      DZL_APPLICATION_WINDOW_GET_CLASS (self)->set_fullscreen (self, fullscreen);
      update_titlebar_animation_property (self);
    }
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

DzlTitlebarAnimation
dzl_application_window_get_titlebar_animation (DzlApplicationWindow *self)
{
  DzlApplicationWindowPrivate *priv = dzl_application_window_get_instance_private (self);
  GtkWidget *titlebar;

  g_return_val_if_fail (DZL_IS_APPLICATION_WINDOW (self), 0);

  titlebar = dzl_application_window_get_titlebar (self);
  if (titlebar == NULL)
    return DZL_TITLEBAR_ANIMATION_HIDDEN;

  if (!dzl_application_window_get_fullscreen (self))
    {
      if (gtk_widget_get_visible (titlebar))
        return DZL_TITLEBAR_ANIMATION_SHOWN;
      else
        return DZL_TITLEBAR_ANIMATION_HIDDEN;
    }

  /* If the source from queue_dismissal is 0, then we already
   * fired and we are hiding the titlebar.
   */
  if (priv->titlebar_hiding)
    return DZL_TITLEBAR_ANIMATION_HIDING;

  /* Titlebar currently visible */
  if (gtk_revealer_get_reveal_child (priv->titlebar_revealer) &&
      gtk_revealer_get_child_revealed (priv->titlebar_revealer))
    return DZL_TITLEBAR_ANIMATION_SHOWN;

  /* Working towards becoming visible */
  if (gtk_revealer_get_reveal_child (priv->titlebar_revealer))
    return DZL_TITLEBAR_ANIMATION_SHOWING;

  return DZL_TITLEBAR_ANIMATION_HIDDEN;
}
