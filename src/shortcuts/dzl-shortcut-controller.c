/* dzl-shortcut-controller.c
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#define G_LOG_DOMAIN "dzl-shortcut-controller"

#include <stdlib.h>
#include <string.h>

#include "dzl-debug.h"

#include "shortcuts/dzl-shortcut-closure-chain.h"
#include "shortcuts/dzl-shortcut-context.h"
#include "shortcuts/dzl-shortcut-controller.h"
#include "shortcuts/dzl-shortcut-manager.h"
#include "shortcuts/dzl-shortcut-private.h"

typedef struct
{
  /*
   * This is the widget for which we are the shortcut controller. There are
   * zero or one shortcut controller for a given widget. These are persistent
   * and dispatch events to the current DzlShortcutContext (which can be
   * changed upon theme changes or shortcuts emitting the ::set-context signal.
   */
  GtkWidget *widget;

  /*
   * This is the current context for the controller. These are collections of
   * shortcuts to signals, actions, etc. The context can be changed in reaction
   * to different events.
   */
  DzlShortcutContext *context;

  /*
   * If we are building a chord, it will be tracked here. Each incoming
   * GdkEventKey will contribute to the creation of this chord.
   */
  DzlShortcutChord *current_chord;

  /*
   * This is a pointer to the root controller for the window. We register with
   * the root controller so that keybindings can be activated even when the
   * focus widget is somewhere else.
   */
  DzlShortcutController *root;

  /*
   * The commands that are attached to this controller including callbacks,
   * signals, or actions. We use the commands_table to get a chord to the
   * intern'd string containing the command id (for direct comparisons).
   */
  GHashTable *commands;
  DzlShortcutChordTable *commands_table;

  /*
   * The root controller may have a manager associated with it to determine
   * what themes and shortcuts are available.
   */
  DzlShortcutManager *manager;

  /*
   * The root controller keeps track of the children controllers in the window.
   * Instead of allocating GList entries, we use an inline GList for the Queue
   * link nodes.
   */
  GQueue descendants;
  GList  descendants_link;

  /*
   * Signal handlers to react to various changes in the system.
   */
  gulong hierarchy_changed_handler;
  gulong widget_destroy_handler;
  gulong manager_changed_handler;
} DzlShortcutControllerPrivate;

enum {
  PROP_0,
  PROP_CONTEXT,
  PROP_CURRENT_CHORD,
  PROP_MANAGER,
  PROP_WIDGET,
  N_PROPS
};

enum {
  RESET,
  SET_CONTEXT_NAMED,
  N_SIGNALS
};

struct _DzlShortcutController { GObject object; };
G_DEFINE_TYPE_WITH_PRIVATE (DzlShortcutController, dzl_shortcut_controller, G_TYPE_OBJECT)

static GParamSpec *properties [N_PROPS];
static guint       signals [N_SIGNALS];
static GQuark      root_quark;
static GQuark      controller_quark;

static void dzl_shortcut_controller_connect    (DzlShortcutController *self);
static void dzl_shortcut_controller_disconnect (DzlShortcutController *self);

/**
 * dzl_shortcut_controller_get_manager:
 * @self: a #DzlShortcutController
 *
 * Gets the #DzlShortcutManager associated with this controller.
 *
 * Generally, this will look for the root controller's manager as mixing and
 * matching managers in a single window hierarchy is not supported.
 *
 * Returns: (not nullable) (transfer none): A #DzlShortcutManager.
 */
DzlShortcutManager *
dzl_shortcut_controller_get_manager (DzlShortcutController *self)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));

  if (priv->root != NULL)
    return dzl_shortcut_controller_get_manager (priv->root);

  if (priv->manager != NULL)
    return priv->manager;

  return dzl_shortcut_manager_get_default ();
}

/**
 * dzl_shortcut_controller_set_manager:
 * @self: a #DzlShortcutController
 * @manager: (nullable): A #DzlShortcutManager or %NULL
 *
 * Sets the #DzlShortcutController:manager property.
 *
 * If you set this to %NULL, it will revert to the default #DzlShortcutManager
 * for the process.
 */
void
dzl_shortcut_controller_set_manager (DzlShortcutController *self,
                                     DzlShortcutManager     *manager)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  g_return_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_return_if_fail (!manager || DZL_IS_SHORTCUT_MANAGER (manager));

  if (g_set_object (&priv->manager, manager))
    g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_MANAGER]);
}

static gboolean
dzl_shortcut_controller_is_mapped (DzlShortcutController *self)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  return priv->widget != NULL && gtk_widget_get_mapped (priv->widget);
}

static void
dzl_shortcut_controller_add (DzlShortcutController *self,
                             DzlShortcutController *descendant)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  DzlShortcutControllerPrivate *dpriv = dzl_shortcut_controller_get_instance_private (descendant);

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_assert (DZL_IS_SHORTCUT_CONTROLLER (descendant));

  g_object_ref (descendant);

  if (dzl_shortcut_controller_is_mapped (descendant))
    g_queue_push_head_link (&priv->descendants, &dpriv->descendants_link);
  else
    g_queue_push_tail_link (&priv->descendants, &dpriv->descendants_link);
}

static void
dzl_shortcut_controller_remove (DzlShortcutController *self,
                                DzlShortcutController *descendant)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  DzlShortcutControllerPrivate *dpriv = dzl_shortcut_controller_get_instance_private (descendant);

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_assert (DZL_IS_SHORTCUT_CONTROLLER (descendant));

  g_queue_unlink (&priv->descendants, &dpriv->descendants_link);
}

static void
dzl_shortcut_controller_on_manager_changed (DzlShortcutController *self,
                                            DzlShortcutManager    *manager)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_assert (DZL_IS_SHORTCUT_MANAGER (manager));

  g_clear_pointer (&priv->current_chord, dzl_shortcut_chord_free);
  g_clear_object (&priv->context);
}

static void
dzl_shortcut_controller_widget_destroy (DzlShortcutController *self,
                                        GtkWidget             *widget)
{
  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_assert (GTK_IS_WIDGET (widget));

  dzl_shortcut_controller_disconnect (self);
}

static void
dzl_shortcut_controller_widget_hierarchy_changed (DzlShortcutController *self,
                                                  GtkWidget             *previous_toplevel,
                                                  GtkWidget             *widget)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  GtkWidget *toplevel;

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_assert (!previous_toplevel || GTK_IS_WIDGET (previous_toplevel));
  g_assert (GTK_IS_WIDGET (widget));

  g_object_ref (self);

  /*
   * Here we register our controller with the toplevel controller. If that
   * widget doesn't yet have a placeholder toplevel controller, then we
   * create that and attach to it.
   *
   * The toplevel controller is used to dispatch events from the window
   * to any controller that could be activating for the window.
   */

  if (priv->root != NULL)
    {
      dzl_shortcut_controller_remove (priv->root, self);
      g_clear_object (&priv->root);
    }

  toplevel = gtk_widget_get_toplevel (widget);

  if (toplevel != widget)
    {
      priv->root = g_object_get_qdata (G_OBJECT (toplevel), root_quark);
      if (priv->root == NULL)
        priv->root = dzl_shortcut_controller_new (toplevel);
      dzl_shortcut_controller_add (priv->root, self);
    }

  g_object_unref (self);
}

static void
dzl_shortcut_controller_disconnect (DzlShortcutController *self)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  DzlShortcutManager *manager;

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_assert (GTK_IS_WIDGET (priv->widget));

  manager = dzl_shortcut_controller_get_manager (self);

  g_signal_handler_disconnect (priv->widget, priv->widget_destroy_handler);
  priv->widget_destroy_handler = 0;

  g_signal_handler_disconnect (priv->widget, priv->hierarchy_changed_handler);
  priv->hierarchy_changed_handler = 0;

  g_signal_handler_disconnect (manager, priv->manager_changed_handler);
  priv->manager_changed_handler = 0;
}

static void
dzl_shortcut_controller_connect (DzlShortcutController *self)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  DzlShortcutManager *manager;

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_assert (GTK_IS_WIDGET (priv->widget));

  manager = dzl_shortcut_controller_get_manager (self);

  g_clear_pointer (&priv->current_chord, dzl_shortcut_chord_free);
  g_clear_object (&priv->context);

  priv->widget_destroy_handler =
    g_signal_connect_swapped (priv->widget,
                              "destroy",
                              G_CALLBACK (dzl_shortcut_controller_widget_destroy),
                              self);

  priv->hierarchy_changed_handler =
    g_signal_connect_swapped (priv->widget,
                              "hierarchy-changed",
                              G_CALLBACK (dzl_shortcut_controller_widget_hierarchy_changed),
                              self);

  priv->manager_changed_handler =
    g_signal_connect_swapped (manager,
                              "changed",
                              G_CALLBACK (dzl_shortcut_controller_on_manager_changed),
                              self);

  dzl_shortcut_controller_widget_hierarchy_changed (self, NULL, priv->widget);
}

static void
dzl_shortcut_controller_set_widget (DzlShortcutController *self,
                                    GtkWidget             *widget)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_assert (GTK_IS_WIDGET (widget));

  if (widget != priv->widget)
    {
      if (priv->widget != NULL)
        {
          dzl_shortcut_controller_disconnect (self);
          g_object_remove_weak_pointer (G_OBJECT (priv->widget), (gpointer *)&priv->widget);
          priv->widget = NULL;
        }

      if (widget != NULL && widget != priv->widget)
        {
          priv->widget = widget;
          g_object_add_weak_pointer (G_OBJECT (priv->widget), (gpointer *)&priv->widget);
          dzl_shortcut_controller_connect (self);
        }

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_WIDGET]);
    }
}

static void
dzl_shortcut_controller_emit_reset (DzlShortcutController *self)
{
  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));

  g_signal_emit (self, signals[RESET], 0);
}

void
dzl_shortcut_controller_set_context (DzlShortcutController *self,
                                     DzlShortcutContext    *context)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  g_return_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_return_if_fail (!context || DZL_IS_SHORTCUT_CONTEXT (context));

  if (g_set_object (&priv->context, context))
    {
      g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CONTEXT]);
      dzl_shortcut_controller_emit_reset (self);
    }
}

static void
dzl_shortcut_controller_real_set_context_named (DzlShortcutController *self,
                                                const gchar           *name)
{
  g_autoptr(DzlShortcutContext) context = NULL;
  DzlShortcutManager *manager;
  DzlShortcutTheme *theme;

  g_return_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_return_if_fail (name != NULL);

  manager = dzl_shortcut_controller_get_manager (self);
  theme = dzl_shortcut_manager_get_theme (manager);
  context = dzl_shortcut_theme_find_context_by_name (theme, name);

  dzl_shortcut_controller_set_context (self, context);
}

static void
dzl_shortcut_controller_finalize (GObject *object)
{
  DzlShortcutController *self = (DzlShortcutController *)object;
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  if (priv->widget != NULL)
    {
      g_object_remove_weak_pointer (G_OBJECT (priv->widget), (gpointer *)&priv->widget);
      priv->widget = NULL;
    }

  g_clear_pointer (&priv->commands, g_hash_table_unref);

  g_clear_object (&priv->context);
  g_clear_object (&priv->root);

  while (priv->descendants.length > 0)
    g_queue_unlink (&priv->descendants, priv->descendants.head);

  G_OBJECT_CLASS (dzl_shortcut_controller_parent_class)->finalize (object);
}

static void
dzl_shortcut_controller_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  DzlShortcutController *self = (DzlShortcutController *)object;
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_CONTEXT:
      g_value_set_object (value, priv->context);
      break;

    case PROP_CURRENT_CHORD:
      g_value_set_boxed (value, dzl_shortcut_controller_get_current_chord (self));
      break;

    case PROP_MANAGER:
      g_value_set_object (value, dzl_shortcut_controller_get_manager (self));
      break;

    case PROP_WIDGET:
      g_value_set_object (value, priv->widget);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_controller_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  DzlShortcutController *self = (DzlShortcutController *)object;

  switch (prop_id)
    {
    case PROP_CONTEXT:
      dzl_shortcut_controller_set_context (self, g_value_get_object (value));
      break;

    case PROP_MANAGER:
      dzl_shortcut_controller_set_manager (self, g_value_get_object (value));
      break;

    case PROP_WIDGET:
      dzl_shortcut_controller_set_widget (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_controller_class_init (DzlShortcutControllerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_shortcut_controller_finalize;
  object_class->get_property = dzl_shortcut_controller_get_property;
  object_class->set_property = dzl_shortcut_controller_set_property;

  properties [PROP_CURRENT_CHORD] =
    g_param_spec_boxed ("current-chord",
                        "Current Chord",
                        "The current chord for the controller",
                        DZL_TYPE_SHORTCUT_CHORD,
                        (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  properties [PROP_CONTEXT] =
    g_param_spec_object ("context",
                         "Context",
                         "The current context of the controller",
                         DZL_TYPE_SHORTCUT_CONTEXT,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_MANAGER] =
    g_param_spec_object ("manager",
                         "Manager",
                         "The shortcut manager",
                         DZL_TYPE_SHORTCUT_MANAGER,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_WIDGET] =
    g_param_spec_object ("widget",
                         "Widget",
                         "The widget for which the controller attached",
                         GTK_TYPE_WIDGET,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  /**
   * DzlShortcutController::reset:
   *
   * This signal is emitted when the shortcut controller is requesting
   * the widget to reset any state it may have regarding the shortcut
   * controller. Such an example might be a modal system that lives
   * outside the controller whose state should be cleared in response
   * to the controller changing modes.
   */
  signals [RESET] =
    g_signal_new_class_handler ("reset",
                                G_TYPE_FROM_CLASS (klass),
                                G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                NULL, NULL, NULL, NULL, G_TYPE_NONE, 0);

  /**
   * DzlShortcutController::set-context-named:
   * @self: An #DzlShortcutController
   * @name: The name of the context
   *
   * This changes the current context on the #DzlShortcutController to be the
   * context matching @name. This is found by looking up the context by name
   * in the active #DzlShortcutTheme.
   */
  signals [SET_CONTEXT_NAMED] =
    g_signal_new_class_handler ("set-context-named",
                                G_TYPE_FROM_CLASS (klass),
                                G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                G_CALLBACK (dzl_shortcut_controller_real_set_context_named),
                                NULL, NULL, NULL,
                                G_TYPE_NONE, 1, G_TYPE_STRING);

  controller_quark = g_quark_from_static_string ("DZL_SHORTCUT_CONTROLLER");
  root_quark = g_quark_from_static_string ("DZL_SHORTCUT_CONTROLLER_ROOT");
}

static void
dzl_shortcut_controller_init (DzlShortcutController *self)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  g_queue_init (&priv->descendants);

  priv->descendants_link.data = self;
}

DzlShortcutController *
dzl_shortcut_controller_new (GtkWidget *widget)
{
  DzlShortcutController *ret;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

  if (NULL != (ret = g_object_get_qdata (G_OBJECT (widget), controller_quark)))
    return g_object_ref (ret);

  ret = g_object_new (DZL_TYPE_SHORTCUT_CONTROLLER,
                      "widget", widget,
                      NULL);

  g_object_set_qdata_full (G_OBJECT (widget),
                           controller_quark,
                           g_object_ref (ret),
                           g_object_unref);

  return ret;
}

/**
 * dzl_shortcut_controller_try_find:
 *
 * Finds the registered #DzlShortcutController for a widget.
 *
 * If no controller is found, %NULL is returned.
 *
 * Returns: (nullable) (transfer none): An #DzlShortcutController or %NULL.
 */
DzlShortcutController *
dzl_shortcut_controller_try_find (GtkWidget *widget)
{
  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

  return g_object_get_qdata (G_OBJECT (widget), controller_quark);
}

/**
 * dzl_shortcut_controller_find:
 *
 * Finds the registered #DzlShortcutController for a widget.
 *
 * The controller is created if it does not already exist.
 *
 * Returns: (not nullable) (transfer none): An #DzlShortcutController or %NULL.
 */
DzlShortcutController *
dzl_shortcut_controller_find (GtkWidget *widget)
{
  DzlShortcutController *controller;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

  controller = g_object_get_qdata (G_OBJECT (widget), controller_quark);

  if (controller == NULL)
    {
      /* We want to pass a borrowed reference */
      g_object_unref (dzl_shortcut_controller_new (widget));
      controller = g_object_get_qdata (G_OBJECT (widget), controller_quark);
    }

  return controller;
}

/**
 * dzl_shortcut_controller_get_context:
 * @self: An #DzlShortcutController
 *
 * This function gets the #DzlShortcutController:context property, which
 * is the current context to dispatch events to. An #DzlShortcutContext
 * is a group of keybindings that may be activated in response to a
 * single or series of #GdkEventKey.
 *
 * Returns: (transfer none) (nullable): An #DzlShortcutContext or %NULL.
 */
DzlShortcutContext *
dzl_shortcut_controller_get_context (DzlShortcutController *self)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self), NULL);

  if (priv->widget == NULL)
    return NULL;

  if (priv->context == NULL)
    {
      DzlShortcutManager *manager;
      DzlShortcutTheme *theme;

      manager = dzl_shortcut_controller_get_manager (self);
      theme = dzl_shortcut_manager_get_theme (manager);

      /*
       * If we have not set an explicit context, then we want to just return
       * our borrowed context so if the theme changes we adapt.
       */

      return dzl_shortcut_theme_find_default_context (theme, priv->widget);
    }

  return priv->context;
}

static DzlShortcutContext *
dzl_shortcut_controller_get_parent_context (DzlShortcutController *self)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  DzlShortcutManager *manager;
  DzlShortcutTheme *theme;
  DzlShortcutTheme *parent;
  const gchar *name = NULL;
  const gchar *parent_name = NULL;

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));

  manager = dzl_shortcut_controller_get_manager (self);

  theme = dzl_shortcut_manager_get_theme (manager);
  if (theme == NULL)
    return NULL;

  parent_name = dzl_shortcut_theme_get_parent_name (theme);
  if (parent_name == NULL)
    return NULL;

  parent = dzl_shortcut_manager_get_theme_by_name (manager, parent_name);
  if (parent == NULL)
    return NULL;

  if (priv->context != NULL)
    {
      name = dzl_shortcut_context_get_name (priv->context);

      if (name != NULL)
        return dzl_shortcut_theme_find_context_by_name (theme, name);
    }

  return dzl_shortcut_theme_find_default_context (theme, priv->widget);
}

static DzlShortcutMatch
dzl_shortcut_controller_process (DzlShortcutController  *self,
                                 const DzlShortcutChord *chord)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  DzlShortcutContext *context;
  DzlShortcutMatch match = DZL_SHORTCUT_MATCH_NONE;

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_assert (chord != NULL);

  /* Short-circuit if we can't make forward progress */
  if (priv->widget == NULL ||
      !gtk_widget_get_visible (priv->widget) ||
      !gtk_widget_is_sensitive (priv->widget))
    return DZL_SHORTCUT_MATCH_NONE;

  /* Try to activate our current context */
  if (match == DZL_SHORTCUT_MATCH_NONE &&
      NULL != (context = dzl_shortcut_controller_get_context (self)))
    match = dzl_shortcut_context_activate (context, priv->widget, chord);

  /* If we didn't get a match, locate the context within the parent theme */
  if (match == DZL_SHORTCUT_MATCH_NONE &&
      NULL != (context = dzl_shortcut_controller_get_parent_context (self)))
    match = dzl_shortcut_context_activate (context, priv->widget, chord);

  /* Try to activate one of our descendant controllers */
  for (GList *iter = priv->descendants.head;
       match != DZL_SHORTCUT_MATCH_EQUAL && iter != NULL;
       iter = iter->next)
    {
      DzlShortcutController *descendant = iter->data;
      DzlShortcutMatch child_match;

      child_match = dzl_shortcut_controller_process (descendant, chord);

      if (child_match != DZL_SHORTCUT_MATCH_NONE)
        match = child_match;
    }

  return match;
}

/**
 * dzl_shortcut_controller_handle_event:
 * @self: An #DzlShortcutController
 * @event: A #GdkEventKey
 *
 * This function uses @event to determine if the current context has a shortcut
 * registered matching the event. If so, the shortcut will be dispatched and
 * %TRUE is returned.
 *
 * Otherwise, %FALSE is returned.
 *
 * Returns: %TRUE if @event has been handled, otherwise %FALSE.
 */
gboolean
dzl_shortcut_controller_handle_event (DzlShortcutController *self,
                                      const GdkEventKey     *event)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  DzlShortcutMatch match;

  DZL_ENTRY;

  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  /*
   * This handles the activation of the event starting from this context,
   * and working our way down into the children controllers.
   *
   * We process things in the order of:
   *
   *   1) Our current shortcut context
   *   2) Our current shortcut context for "commands"
   *   3) Each of our registered controllers.
   *
   * This gets a bit complicated once we start talking about chords. A chord is
   * a sequence of GdkEventKey which can activate a shortcut. That might be
   * something like Ctrl+X|Ctrl+O. Ctrl+X does not activate something on its
   * own, but if the following event is a CTRL+O, it will activate that
   * shortcut.
   *
   * This means we need to stash the chord sequence while we have partial
   * matches up until we get a match. If no match is found (nor a partial),
   * then we can ignore the event and return GDK_EVENT_PROPAGATE.
   *
   * If we swallow the event, because we are building a chord, then we will
   * return GDK_EVENT_STOP and stash the chord for future use.
   *
   * While unfortunate, we do not try to handle a situation where we have a
   * collision between an exact match and a partial match. The first item we
   * come across wins. This is considered undefined behavior.
   */

  if (priv->current_chord == NULL)
    {
      priv->current_chord = dzl_shortcut_chord_new_from_event (event);
      if (priv->current_chord == NULL)
        DZL_RETURN (GDK_EVENT_PROPAGATE);
    }
  else
    {
      if (!dzl_shortcut_chord_append_event (priv->current_chord, event))
        {
          g_clear_pointer (&priv->current_chord, dzl_shortcut_chord_free);
          g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_CURRENT_CHORD]);
          DZL_RETURN (GDK_EVENT_PROPAGATE);
        }
    }

  g_assert (priv->current_chord != NULL);

  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_CURRENT_CHORD]);

#if 0
  {
    g_autofree gchar *str = dzl_shortcut_chord_to_string (priv->current_chord);
    g_debug ("Chord = %s", str);
  }
#endif

  match = dzl_shortcut_controller_process (self, priv->current_chord);

  /*
   * If we still haven't located a match, we need to ask the theme if there
   * is a keybinding registered for this chord to a command or action. If
   * so, we will try to dispatch it now.
   */
  if (match != DZL_SHORTCUT_MATCH_EQUAL && priv->current_chord != NULL)
    {
      DzlShortcutClosureChain *chain = NULL;
      DzlShortcutManager *manager;
      DzlShortcutTheme *theme;
      DzlShortcutMatch theme_match;

      manager = dzl_shortcut_controller_get_manager (self);
      theme = dzl_shortcut_manager_get_theme (manager);
      theme_match = _dzl_shortcut_theme_match (theme, priv->current_chord, &chain);

      if (theme_match == DZL_SHORTCUT_MATCH_EQUAL)
        dzl_shortcut_closure_chain_execute (chain, priv->widget);

      if (theme_match != DZL_SHORTCUT_MATCH_NONE)
        match = theme_match;
    }

#if 0
  g_message ("match = %d", match);
#endif

  if (match != DZL_SHORTCUT_MATCH_PARTIAL)
    {
      g_clear_pointer (&priv->current_chord, dzl_shortcut_chord_free);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_CURRENT_CHORD]);
    }

  DZL_TRACE_MSG ("match = %s",
		 match == DZL_SHORTCUT_MATCH_NONE ? "none" :
		 match == DZL_SHORTCUT_MATCH_PARTIAL ? "partial" : "equal");

  if (match != DZL_SHORTCUT_MATCH_NONE)
    DZL_RETURN (GDK_EVENT_STOP);

  DZL_RETURN (GDK_EVENT_PROPAGATE);
}

/**
 * dzl_shortcut_controller_get_current_chord:
 * @self: a #DzlShortcutController
 *
 * This method gets the #DzlShortcutController:current-chord property.
 *
 * This is useful if you want to monitor in-progress chord building.
 *
 * Returns: (transfer none) (nullable): A #DzlShortcutChord or %NULL.
 */
const DzlShortcutChord *
dzl_shortcut_controller_get_current_chord (DzlShortcutController *self)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self), NULL);

  return priv->current_chord;
}

/**
 * dzl_shortcut_controller_execute_command:
 * @self: a #DzlShortcutController
 * @command: the id of the command
 *
 * This method will locate and execute the command matching the id @command.
 *
 * If the command is not found, %FALSE is returned.
 *
 * Returns: %TRUE if the command was found and executed.
 */
gboolean
dzl_shortcut_controller_execute_command (DzlShortcutController *self,
                                         const gchar           *command)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self), FALSE);
  g_return_val_if_fail (command != NULL, FALSE);

  if (priv->commands != NULL)
    {
      DzlShortcutClosureChain *chain;

      chain = g_hash_table_lookup (priv->commands, g_intern_string (command));

      if (chain != NULL)
        return dzl_shortcut_closure_chain_execute (chain, priv->widget);
    }

  for (const GList *iter = priv->descendants.head; iter != NULL; iter = iter->next)
    {
      DzlShortcutController *descendant = iter->data;

      if (dzl_shortcut_controller_execute_command (descendant, command))
        return TRUE;
    }

  return FALSE;
}

static void
dzl_shortcut_controller_add_command (DzlShortcutController   *self,
                                     const gchar             *command_id,
                                     const gchar             *default_accel,
                                     DzlShortcutClosureChain *chain)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  g_autoptr(DzlShortcutChord) chord = NULL;
  DzlShortcutManager *manager;
  DzlShortcutTheme *theme;

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_assert (command_id != NULL);
  g_assert (chain != NULL);

  command_id = g_intern_string (command_id);

  if (priv->commands == NULL)
    priv->commands = g_hash_table_new_full (NULL, NULL, NULL,
                                            (GDestroyNotify)dzl_shortcut_closure_chain_free);

  g_hash_table_insert (priv->commands, (gpointer)command_id, chain);

  if (priv->commands_table == NULL)
    priv->commands_table = dzl_shortcut_chord_table_new ();

  if (default_accel != NULL)
    {
      chord = dzl_shortcut_chord_new_from_string (default_accel);

      if (chord != NULL)
        {
          dzl_shortcut_chord_table_add (priv->commands_table, chord, (gpointer)command_id);
          manager = dzl_shortcut_controller_get_manager (self);
          theme = _dzl_shortcut_manager_get_internal_theme (manager);
          dzl_shortcut_theme_set_chord_for_command (theme, command_id, chord);

#if 0
          g_print ("Added %s for command %s\n", default_accel, command_id);
#endif
        }
    }
}

void
dzl_shortcut_controller_add_command_action (DzlShortcutController *self,
                                            const gchar           *command_id,
                                            const gchar           *default_accel,
                                            const gchar           *action)
{
  DzlShortcutClosureChain *chain;

  g_return_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_return_if_fail (command_id != NULL);

  chain = dzl_shortcut_closure_chain_append_action_string (NULL, action);

  dzl_shortcut_controller_add_command (self, command_id, default_accel, chain);
}

void
dzl_shortcut_controller_add_command_callback (DzlShortcutController *self,
                                              const gchar           *command_id,
                                              const gchar           *default_accel,
                                              GtkCallback            callback,
                                              gpointer               callback_data,
                                              GDestroyNotify         callback_data_destroy)
{
  DzlShortcutClosureChain *chain;

  g_return_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_return_if_fail (command_id != NULL);

  chain = dzl_shortcut_closure_chain_append_callback (NULL,
                                                      callback,
                                                      callback_data,
                                                      callback_data_destroy);

  dzl_shortcut_controller_add_command (self, command_id, default_accel, chain);
}

void
dzl_shortcut_controller_add_command_signal (DzlShortcutController *self,
                                            const gchar           *command_id,
                                            const gchar           *default_accel,
                                            const gchar           *signal_name,
                                            guint                  n_args,
                                            ...)
{
  DzlShortcutClosureChain *chain;
  va_list args;

  g_return_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_return_if_fail (command_id != NULL);

  va_start (args, n_args);
  chain = dzl_shortcut_closure_chain_append_signal (NULL, signal_name, n_args, args);
  va_end (args);

  dzl_shortcut_controller_add_command (self, command_id, default_accel, chain);
}
