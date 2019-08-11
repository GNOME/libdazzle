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

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "dzl-debug.h"

#include "shortcuts/dzl-shortcut-closure-chain.h"
#include "shortcuts/dzl-shortcut-context.h"
#include "shortcuts/dzl-shortcut-controller.h"
#include "shortcuts/dzl-shortcut-manager.h"
#include "shortcuts/dzl-shortcut-private.h"
#include "util/dzl-macros.h"

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
   * This is the name of the current context. Contexts are resolved at runtime
   * by locating them within the theme (or inherited theme). They are interned
   * strings to avoid lots of allocations between widgets.
   */
  const gchar *context_name;

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

  /*
   * The command table is used to provide a mapping from accelerator/chord
   * to the key for @commands. The data for each chord is an interned string
   * which can be used as a direct pointer for lookups in @commands.
   */
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

  /*
   * To avoid allocating GList nodes for controllers, we just inline a link
   * here and attach it to @descendants when necessary.
   */
  GList descendants_link;

  /* Signal handlers to react to various changes in the system. */
  gulong hierarchy_changed_handler;
  gulong widget_destroy_handler;
  gulong manager_changed_handler;

  /* If we have any global shortcuts registered */
  guint have_global : 1;
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

static void
dzl_shortcut_controller_emit_reset (DzlShortcutController *self)
{
  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));

  g_signal_emit (self, signals[RESET], 0);
}

static inline gboolean
dzl_shortcut_controller_is_root (DzlShortcutController *self)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  return priv->root == NULL;
}

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
  g_object_unref (descendant);
}

static void
dzl_shortcut_controller_on_manager_changed (DzlShortcutController *self,
                                            DzlShortcutManager    *manager)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_assert (DZL_IS_SHORTCUT_MANAGER (manager));

  priv->context_name = NULL;
  _dzl_shortcut_controller_clear (self);
  dzl_shortcut_controller_emit_reset (self);
}

static void
dzl_shortcut_controller_widget_destroy (DzlShortcutController *self,
                                        GtkWidget             *widget)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_assert (GTK_IS_WIDGET (widget));

  dzl_shortcut_controller_disconnect (self);
  dzl_clear_weak_pointer (&priv->widget);

  if (priv->root != NULL)
    {
      dzl_shortcut_controller_remove (priv->root, self);
      g_clear_object (&priv->root);
    }
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

  /*
   * We attach our controller to the root controller if we have shortcuts in
   * the global activation phase. That allows the bubble/capture phase to
   * potentially dispatch to our controller.
   */

  g_object_ref (self);

  if (priv->root != NULL)
    {
      dzl_shortcut_controller_remove (priv->root, self);
      g_clear_object (&priv->root);
    }

  if (priv->have_global)
    {
      toplevel = gtk_widget_get_toplevel (widget);

      if (toplevel != widget)
        {
          priv->root = g_object_get_qdata (G_OBJECT (toplevel), root_quark);
          if (priv->root == NULL)
            priv->root = dzl_shortcut_controller_new (toplevel);
          dzl_shortcut_controller_add (priv->root, self);
        }
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
  priv->context_name = NULL;

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
          dzl_clear_weak_pointer (&priv->widget);
        }

      if (widget != NULL && widget != priv->widget)
        {
          dzl_set_weak_pointer (&priv->widget, widget);
          dzl_shortcut_controller_connect (self);
        }

      g_assert (widget == priv->widget);

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_WIDGET]);
    }
}

/**
 * dzl_shortcut_controller_set_context_by_name:
 * @self: a #DzlShortcutController
 * @name: (nullable): The name of the context
 *
 * Changes the context for the controller to the context matching @name.
 *
 * Contexts are resolved at runtime through the current theme (and possibly
 * a parent theme if it inherits from one).
 *
 * Since: 3.26
 */
void
dzl_shortcut_controller_set_context_by_name (DzlShortcutController *self,
                                             const gchar           *name)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  g_return_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self));

  name = g_intern_string (name);

  if (name != priv->context_name)
    {
      priv->context_name = name;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_CONTEXT]);
      dzl_shortcut_controller_emit_reset (self);
    }
}

static void
dzl_shortcut_controller_real_set_context_named (DzlShortcutController *self,
                                                const gchar           *name)
{
  g_return_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self));

  dzl_shortcut_controller_set_context_by_name (self, name);
}

static void
dzl_shortcut_controller_finalize (GObject *object)
{
  DzlShortcutController *self = (DzlShortcutController *)object;
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  dzl_clear_weak_pointer (&priv->widget);
  g_clear_pointer (&priv->commands, g_hash_table_unref);
  g_clear_pointer (&priv->commands_table, dzl_shortcut_chord_table_free);
  g_clear_object (&priv->root);

  while (priv->descendants.length > 0)
    g_queue_unlink (&priv->descendants, priv->descendants.head);

  priv->context_name = NULL;

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
      g_value_set_object (value, dzl_shortcut_controller_get_context (self));
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
                         "The current context of the controller, for dispatch phase",
                         DZL_TYPE_SHORTCUT_CONTEXT,
                         (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

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

  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTROLLER (controller), NULL);

  return controller;
}

static DzlShortcutContext *
_dzl_shortcut_controller_get_context_for_phase (DzlShortcutController *self,
                                                DzlShortcutTheme      *theme,
                                                DzlShortcutPhase       phase)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  g_autofree gchar *phased_name = NULL;
  DzlShortcutContext *ret;
  const gchar *name;

  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self), NULL);
  g_return_val_if_fail (DZL_IS_SHORTCUT_THEME (theme), NULL);

  if (priv->widget == NULL)
    return NULL;

  name = priv->context_name ? priv->context_name : G_OBJECT_TYPE_NAME (priv->widget);

  g_return_val_if_fail (name != NULL, NULL);

  /* If we are in dispatch phase, we use our direct context */
  if (phase == DZL_SHORTCUT_PHASE_BUBBLE)
    name = phased_name = g_strdup_printf ("%s:bubble", name);
  else if (phase == DZL_SHORTCUT_PHASE_CAPTURE)
    name = phased_name = g_strdup_printf ("%s:capture", name);

  ret = _dzl_shortcut_theme_try_find_context_by_name (theme, name);

  g_return_val_if_fail (!ret || DZL_IS_SHORTCUT_CONTEXT (ret), NULL);

  return ret;
}

/**
 * dzl_shortcut_controller_get_context_for_phase:
 * @self: a #DzlShortcutController
 * @phase: the phase for the shorcut delivery
 *
 * Controllers can have a different context for a particular phase, which allows
 * them to activate different keybindings depending if the event in capture,
 * bubble, or dispatch.
 *
 * Returns: (transfer none) (nullable): A #DzlShortcutContext or %NULL.
 *
 * Since: 3.26
 */
DzlShortcutContext *
dzl_shortcut_controller_get_context_for_phase (DzlShortcutController *self,
                                               DzlShortcutPhase       phase)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  DzlShortcutManager *manager;
  DzlShortcutTheme *theme;

  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self), NULL);

  if (NULL == priv->widget ||
      NULL == (manager = dzl_shortcut_controller_get_manager (self)) ||
      NULL == (theme = dzl_shortcut_manager_get_theme (manager)))
    return NULL;

  return _dzl_shortcut_controller_get_context_for_phase (self, theme, phase);
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
 * Returns: (transfer none) (nullable): A #DzlShortcutContext or %NULL.
 *
 * Since: 3.26
 */
DzlShortcutContext *
dzl_shortcut_controller_get_context (DzlShortcutController *self)
{
  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self), NULL);

  return dzl_shortcut_controller_get_context_for_phase (self, DZL_SHORTCUT_PHASE_DISPATCH);
}

static DzlShortcutContext *
dzl_shortcut_controller_get_inherited_context (DzlShortcutController *self,
                                               DzlShortcutPhase       phase)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  DzlShortcutManager *manager;
  DzlShortcutContext *ret;
  DzlShortcutTheme *theme;
  DzlShortcutTheme *parent;
  const gchar *parent_name = NULL;

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));

  if (NULL == priv->widget ||
      NULL == (manager = dzl_shortcut_controller_get_manager (self)) ||
      NULL == (theme = dzl_shortcut_manager_get_theme (manager)) ||
      NULL == (parent_name = dzl_shortcut_theme_get_parent_name (theme)) ||
      NULL == (parent = dzl_shortcut_manager_get_theme_by_name (manager, parent_name)))
    return NULL;

  ret = _dzl_shortcut_controller_get_context_for_phase (self, parent, phase);

  g_return_val_if_fail (!ret || DZL_IS_SHORTCUT_CONTEXT (ret), NULL);

  return ret;
}

static DzlShortcutMatch
dzl_shortcut_controller_process (DzlShortcutController  *self,
                                 const DzlShortcutChord *chord,
                                 DzlShortcutPhase        phase)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  DzlShortcutContext *context;
  DzlShortcutMatch match = DZL_SHORTCUT_MATCH_NONE;

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_assert (chord != NULL);

  /* Try to activate our current context */
  if (match == DZL_SHORTCUT_MATCH_NONE &&
      NULL != (context = dzl_shortcut_controller_get_context_for_phase (self, phase)))
    match = dzl_shortcut_context_activate (context, priv->widget, chord);

  /* If we didn't get a match, locate the context within the parent theme */
  if (match == DZL_SHORTCUT_MATCH_NONE &&
      NULL != (context = dzl_shortcut_controller_get_inherited_context (self, phase)))
    match = dzl_shortcut_context_activate (context, priv->widget, chord);

  return match;
}

static void
dzl_shortcut_controller_do_global_chain (DzlShortcutController   *self,
                                         DzlShortcutClosureChain *chain,
                                         GtkWidget               *widget,
                                         GList                   *next)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_assert (chain != NULL);
  g_assert (GTK_IS_WIDGET (widget));

  /*
   * If this is an action chain, the best we're going to be able to do is
   * activate the action from the current widget. For commands, we can try to
   * resolve them by locating commands within our registered controllers.
   */

  if (chain->type != DZL_SHORTCUT_CLOSURE_COMMAND)
    {
      dzl_shortcut_closure_chain_execute (chain, widget);
      return;
    }

  if (priv->commands != NULL &&
      g_hash_table_contains (priv->commands, chain->command.name))
    {
      dzl_shortcut_closure_chain_execute (chain, priv->widget);
      return;
    }

  if (next == NULL)
    {
      dzl_shortcut_closure_chain_execute (chain, widget);
      return;
    }

  dzl_shortcut_controller_do_global_chain (next->data, chain, widget, next->next);
}

static DzlShortcutMatch
dzl_shortcut_controller_do_global (DzlShortcutController  *self,
                                   const DzlShortcutChord *chord,
                                   DzlShortcutPhase        phase,
                                   GtkWidget              *widget)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  DzlShortcutClosureChain *chain = NULL;
  DzlShortcutManager *manager;
  DzlShortcutTheme *theme;
  DzlShortcutMatch match;

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_assert (chord != NULL);
  g_assert ((phase & DZL_SHORTCUT_PHASE_GLOBAL) != 0);
  g_assert (GTK_IS_WIDGET (widget));

  manager = dzl_shortcut_controller_get_manager (self);
  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));

  theme = dzl_shortcut_manager_get_theme (manager);
  g_assert (DZL_IS_SHORTCUT_THEME (theme));

  /* See if we have a chain for this chord */
  match = _dzl_shortcut_theme_match (theme, phase, chord, &chain);

  /* If we matched, execute the chain, trying to locate the proper widget for
   * the event delivery.
   */
  if (match == DZL_SHORTCUT_MATCH_EQUAL && chain->phase == phase)
    dzl_shortcut_controller_do_global_chain (self, chain, widget, priv->descendants.head);

  return match;
}

/**
 * _dzl_shortcut_controller_handle:
 * @self: An #DzlShortcutController
 * @event: A #GdkEventKey
 * @chord: the current chord for the toplevel
 * @phase: the dispatch phase
 * @widget: the widget receiving @event
 *
 * This function uses @event to determine if the current context has a shortcut
 * registered matching the event. If so, the shortcut will be dispatched and
 * %TRUE is returned. Otherwise, %FALSE is returned.
 *
 * @chord is used to track the current chord from the toplevel. Chord tracking
 * is done in a single place to avoid inconsistencies between controllers.
 *
 * @phase should indicate the phase of the event dispatch. Capture is used
 * to capture events before the destination #GdkWindow can process them, and
 * bubble is to allow the destination window to handle it before processing
 * the result afterwards if not yet handled.
 *
 * Returns: A #DzlShortcutMatch based on if the event was dispatched.
 *
 * Since: 3.26
 */
DzlShortcutMatch
_dzl_shortcut_controller_handle (DzlShortcutController  *self,
                                 const GdkEventKey      *event,
                                 const DzlShortcutChord *chord,
                                 DzlShortcutPhase        phase,
                                 GtkWidget              *widget)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  DzlShortcutMatch match = DZL_SHORTCUT_MATCH_NONE;

  DZL_ENTRY;

  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);
  g_return_val_if_fail (chord != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

  /* Nothing to do if the widget isn't visible/mapped/etc */
  if (priv->widget == NULL ||
      !gtk_widget_get_visible (priv->widget) ||
      !gtk_widget_get_child_visible (priv->widget) ||
      !gtk_widget_is_sensitive (priv->widget))
    DZL_RETURN (DZL_SHORTCUT_MATCH_NONE);

  DZL_TRACE_MSG ("widget = %s, phase = %d", G_OBJECT_TYPE_NAME (priv->widget), phase);

  /* Try to dispatch our capture global shortcuts first */
  if (phase == (DZL_SHORTCUT_PHASE_CAPTURE | DZL_SHORTCUT_PHASE_GLOBAL) &&
      dzl_shortcut_controller_is_root (self))
    match = dzl_shortcut_controller_do_global (self, chord, phase, widget);

  /*
   * This function processes a particular phase for the event. If our phase
   * is DZL_SHORTCUT_PHASE_CAPTURE, that means we are in the process of working
   * our way from the toplevel down to the widget containing the event window.
   *
   * If our phase is DZL_SHORTCUT_PHASE_BUBBLE, then we are working our way
   * up from the widget containing the event window to the toplevel. This is
   * the phase where most activations should occur.
   *
   * During the capture phase, we look for a context matching the current
   * context, but with a suffix on the context name like ":capture". So for
   * the default GtkEntry, the capture context name would be something like
   * "GtkEntry:capture". The bubble phase does not have a suffix.
   *
   *   Toplevel Global Capture Accels
   *   Toplevel Capture
   *     - Child 1 Capture
   *       - Grandchild 1 Capture
   *       - Grandchild 1 Bubble
   *     - Child 1 Bubble
   *   Toplevel Bubble
   *   Toplevel Global Bubble Accels
   *
   * If we come across a keybinding that is a partial match, we assume that
   * is the closest match in the dispatch chain and stop processing further.
   * Overlapping and conflicting keybindings are considered undefined behavior
   * and this falls under such a situation.
   *
   * Note that we do not perform the bubble/capture phase here, that is handled
   * by our caller in DzlShortcutManager.
   */

  if (match == DZL_SHORTCUT_MATCH_NONE)
    match = dzl_shortcut_controller_process (self, chord, phase);

  /* Try to dispatch our capture global shortcuts first */
  if (match == DZL_SHORTCUT_MATCH_NONE &&
      dzl_shortcut_controller_is_root (self) &&
      phase == (DZL_SHORTCUT_PHASE_BUBBLE | DZL_SHORTCUT_PHASE_GLOBAL))
    match = dzl_shortcut_controller_do_global (self, chord, phase, widget);

  DZL_TRACE_MSG ("match = %s",
                 match == DZL_SHORTCUT_MATCH_NONE ? "none" :
                 match == DZL_SHORTCUT_MATCH_PARTIAL ? "partial" : "equal");

  DZL_RETURN (match);
}

/**
 * dzl_shortcut_controller_get_current_chord:
 * @self: a #DzlShortcutController
 *
 * This method gets the #DzlShortcutController:current-chord property.
 * This is useful if you want to monitor in-progress chord building.
 *
 * Note that this value will only be valid on the controller for the
 * toplevel widget (a #GtkWindow). Chords are not tracked at the
 * individual widget controller level.
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
                                     DzlShortcutPhase         phase,
                                     DzlShortcutClosureChain *chain)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  g_autoptr(DzlShortcutChord) chord = NULL;
  DzlShortcutManager *manager;
  DzlShortcutTheme *theme;

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_assert (command_id != NULL);
  g_assert (chain != NULL);

  /* Always use interned strings for command ids */
  command_id = g_intern_string (command_id);

  /*
   * Set the phase on the closure chain so we know what phase we are allowed
   * to execute the chain within during capture/dispatch/bubble. There is no
   * "global + dispatch" phase, so if global is set, default to bubble.
   */
  if (phase == DZL_SHORTCUT_PHASE_GLOBAL)
    phase |= DZL_SHORTCUT_PHASE_BUBBLE;
  chain->phase = phase;

  /* Add the closure chain to our set of commands. */
  if (priv->commands == NULL)
    priv->commands = g_hash_table_new_full (NULL, NULL, NULL,
                                            (GDestroyNotify)dzl_shortcut_closure_chain_free);
  g_hash_table_insert (priv->commands, (gpointer)command_id, chain);

  /*
   * If this command can be executed in the global phase, we need to be
   * sure that the root controller knows that we must be checked during
   * global activation checks.
   */
  if ((phase & DZL_SHORTCUT_PHASE_GLOBAL) != 0)
    {
      if (priv->have_global != TRUE)
        {
          priv->have_global = TRUE;
          if (priv->widget != NULL)
            dzl_shortcut_controller_widget_hierarchy_changed (self, NULL, priv->widget);
        }
    }

  /* If an accel was provided, we need to register it in various places */
  if (default_accel != NULL)
    {
      /* Make sure this is a valid accelerator */
      chord = dzl_shortcut_chord_new_from_string (default_accel);

      if (chord != NULL)
        {
          DzlShortcutContext *context;

          /* Add the chord to our chord table for lookups */
          if (priv->commands_table == NULL)
            priv->commands_table = dzl_shortcut_chord_table_new ();
          dzl_shortcut_chord_table_add (priv->commands_table, chord, (gpointer)command_id);

          /* Set the value in the theme so it can have overrides by users */
          manager = dzl_shortcut_controller_get_manager (self);
          theme = _dzl_shortcut_manager_get_internal_theme (manager);
          dzl_shortcut_theme_set_chord_for_command (theme, command_id, chord, phase);

          /* Hook things up into the default context */
          context = _dzl_shortcut_theme_find_default_context_with_phase (theme, priv->widget, phase);
          if (!_dzl_shortcut_context_contains (context, chord))
            dzl_shortcut_context_add_command (context, default_accel, command_id);
        }
      else
        g_warning ("\"%s\" is not a valid accelerator chord", default_accel);
    }
}

void
dzl_shortcut_controller_add_command_action (DzlShortcutController *self,
                                            const gchar           *command_id,
                                            const gchar           *default_accel,
                                            DzlShortcutPhase       phase,
                                            const gchar           *action)
{
  DzlShortcutClosureChain *chain;

  g_return_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_return_if_fail (command_id != NULL);

  chain = dzl_shortcut_closure_chain_append_action_string (NULL, action);
  dzl_shortcut_controller_add_command (self, command_id, default_accel, phase, chain);
}

void
dzl_shortcut_controller_add_command_callback (DzlShortcutController *self,
                                              const gchar           *command_id,
                                              const gchar           *default_accel,
                                              DzlShortcutPhase       phase,
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

  dzl_shortcut_controller_add_command (self, command_id, default_accel, phase, chain);
}

void
dzl_shortcut_controller_add_command_signal (DzlShortcutController *self,
                                            const gchar           *command_id,
                                            const gchar           *default_accel,
                                            DzlShortcutPhase       phase,
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

  dzl_shortcut_controller_add_command (self, command_id, default_accel, phase, chain);
}

DzlShortcutChord *
_dzl_shortcut_controller_push (DzlShortcutController *self,
                               const GdkEventKey     *event)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self), NULL);
  g_return_val_if_fail (event != NULL, NULL);

  /*
   * Only the toplevel controller handling the event needs to determine
   * the current "chord" as that state lives in the root controller only.
   *
   * So our first step is to determine the current chord, or if this input
   * breaks further chord processing.
   *
   * We will use these chords during capture/dispatch/bubble later on.
   */
  if (priv->current_chord == NULL)
    {
      /* Try to create a new chord starting with this key.
       * current_chord may still be NULL after this.
       */
      priv->current_chord = dzl_shortcut_chord_new_from_event (event);
    }
  else
    {
      if (!dzl_shortcut_chord_append_event (priv->current_chord, event))
        {
          /* Failed to add the key to the chord, cancel */
          _dzl_shortcut_controller_clear (self);
          return NULL;
        }
    }

  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_CURRENT_CHORD]);

  return dzl_shortcut_chord_copy (priv->current_chord);
}

void
_dzl_shortcut_controller_clear (DzlShortcutController *self)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  g_return_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self));

  g_clear_pointer (&priv->current_chord, dzl_shortcut_chord_free);
  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_CURRENT_CHORD]);
}

/**
 * dzl_shortcut_controller_get_widget:
 * @self: a #DzlShortcutController
 *
 * Returns: (transfer none): the widget for the controller
 *
 * Since: 3.34
 */
GtkWidget *
dzl_shortcut_controller_get_widget (DzlShortcutController *self)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self), NULL);

  return priv->widget;
}

void
dzl_shortcut_controller_remove_accel (DzlShortcutController *self,
                                      const gchar           *accel,
                                      DzlShortcutPhase       phase)
{
  DzlShortcutControllerPrivate *priv = dzl_shortcut_controller_get_instance_private (self);
  g_autoptr(DzlShortcutChord) chord = NULL;

  g_return_if_fail (DZL_IS_SHORTCUT_CONTROLLER (self));
  g_return_if_fail (accel != NULL);

  chord = dzl_shortcut_chord_new_from_string (accel);

  if (chord != NULL)
    {
      DzlShortcutContext *context;
      DzlShortcutManager *manager;
      DzlShortcutTheme *theme;

      /* Add the chord to our chord table for lookups */
      if (priv->commands_table != NULL)
        dzl_shortcut_chord_table_remove (priv->commands_table, chord);

      /* Set the value in the theme so it can have overrides by users */
      manager = dzl_shortcut_controller_get_manager (self);
      theme = _dzl_shortcut_manager_get_internal_theme (manager);
      dzl_shortcut_theme_set_chord_for_command (theme, NULL, chord, 0);

      if (priv->widget != NULL)
        {
          context = _dzl_shortcut_theme_find_default_context_with_phase (theme, priv->widget, phase);
          if (context && _dzl_shortcut_context_contains (context, chord))
            dzl_shortcut_context_remove (context, accel);
        }
    }
}
