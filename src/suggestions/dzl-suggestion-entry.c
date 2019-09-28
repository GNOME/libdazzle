/* dzl-suggestion-entry.c
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
 *
 * This file is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define G_LOG_DOMAIN "dzl-suggestion-entry"

#include "config.h"

#include <glib/gi18n.h>

#include "dzl-debug.h"

#include "suggestions/dzl-suggestion.h"
#include "suggestions/dzl-suggestion-entry.h"
#include "suggestions/dzl-suggestion-entry-buffer.h"
#include "suggestions/dzl-suggestion-popover.h"
#include "suggestions/dzl-suggestion-private.h"
#include "util/dzl-gtk.h"
#include "util/dzl-util-private.h"

typedef struct
{
  DzlSuggestionPopover      *popover;
  DzlSuggestionEntryBuffer  *buffer;
  GListModel                *model;
  GtkGesture                *press_gesture;

  gulong                     changed_handler;

  DzlSuggestionPositionFunc  func;
  gpointer                   func_data;
  GDestroyNotify             func_data_destroy;

  guint                      activate_on_single_click : 1;
  guint                      compact : 1;
  guint                      allow_touch_selection : 1;

  gint                       in_key_press;
  gint                       in_move_by;
} DzlSuggestionEntryPrivate;

enum {
  PROP_0,
  PROP_ACTIVATE_ON_SINGLE_CLICK,
  PROP_COMPACT,
  PROP_MODEL,
  PROP_TYPED_TEXT,
  PROP_SUGGESTION,
  N_PROPS
};

enum {
  ACTION,
  ACTIVATE_SUGGESTION,
  HIDE_SUGGESTIONS,
  MOVE_SUGGESTION,
  SHOW_SUGGESTIONS,
  SUGGESTION_ACTIVATED,
  SUGGESTION_SELECTED,
  N_SIGNALS
};

static void buildable_iface_init (GtkBuildableIface    *iface);
static void editable_iface_init  (GtkEditableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (DzlSuggestionEntry, dzl_suggestion_entry, GTK_TYPE_ENTRY,
                         G_ADD_PRIVATE (DzlSuggestionEntry)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, buildable_iface_init)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_EDITABLE, editable_iface_init))

static GParamSpec *properties [N_PROPS];
static guint signals [N_SIGNALS];
static guint changed_signal_id;
static GtkEditableInterface *editable_parent_iface;

void
dzl_suggestion_entry_set_activate_on_single_click (DzlSuggestionEntry *self,
                                                   gboolean            activate_on_single_click)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  g_assert (DZL_IS_SUGGESTION_ENTRY (self));

  activate_on_single_click = !!activate_on_single_click;

  if (activate_on_single_click != priv->activate_on_single_click)
    {
      priv->activate_on_single_click = activate_on_single_click;
      _dzl_suggestion_popover_set_click_mode (priv->popover, activate_on_single_click);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_ACTIVATE_ON_SINGLE_CLICK]);
    }
}

gboolean
dzl_suggestion_entry_get_activate_on_single_click (DzlSuggestionEntry *self)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SUGGESTION_ENTRY (self), FALSE);

  return priv->activate_on_single_click;
}

static void
dzl_suggestion_entry_show_suggestions (DzlSuggestionEntry *self)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  DZL_ENTRY;

  g_assert (DZL_IS_SUGGESTION_ENTRY (self));

  _dzl_suggestion_entry_reposition (self, priv->popover);
  dzl_suggestion_popover_popup (priv->popover);

  DZL_EXIT;
}

void
dzl_suggestion_entry_hide_suggestions (DzlSuggestionEntry *self)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  DZL_ENTRY;

  g_assert (DZL_IS_SUGGESTION_ENTRY (self));

  if (priv->popover != NULL)
    dzl_suggestion_popover_popdown (priv->popover);

  DZL_EXIT;
}

static gboolean
dzl_suggestion_entry_focus_in_event (GtkWidget     *widget,
                                     GdkEventFocus *event)
{
  DzlSuggestionEntry *self = (DzlSuggestionEntry *)widget;
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  g_assert (DZL_IS_SUGGESTION_ENTRY (self));
  g_assert (event != NULL);

  if (priv->popover)
    _dzl_suggestion_popover_set_focused (priv->popover, TRUE);

  return GTK_WIDGET_CLASS (dzl_suggestion_entry_parent_class)->focus_in_event (widget, event);
}

static gboolean
dzl_suggestion_entry_focus_out_event (GtkWidget     *widget,
                                      GdkEventFocus *event)
{
  DzlSuggestionEntry *self = (DzlSuggestionEntry *)widget;
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  g_assert (DZL_IS_SUGGESTION_ENTRY (self));
  g_assert (event != NULL);

  if (priv->popover)
    _dzl_suggestion_popover_set_focused (priv->popover, FALSE);

  g_signal_emit (self, signals [HIDE_SUGGESTIONS], 0);
  priv->allow_touch_selection = TRUE;

  return GTK_WIDGET_CLASS (dzl_suggestion_entry_parent_class)->focus_out_event (widget, event);
}

static void
dzl_suggestion_entry_hierarchy_changed (GtkWidget *widget,
                                        GtkWidget *old_toplevel)
{
  DzlSuggestionEntry *self = (DzlSuggestionEntry *)widget;
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  g_assert (DZL_IS_SUGGESTION_ENTRY (self));
  g_assert (!old_toplevel || GTK_IS_WIDGET (old_toplevel));

  if (priv->popover != NULL)
    {
      GtkWidget *toplevel = gtk_widget_get_ancestor (widget, GTK_TYPE_WINDOW);

      gtk_window_set_transient_for (GTK_WINDOW (priv->popover), GTK_WINDOW (toplevel));
    }
}

static gboolean
dzl_suggestion_entry_key_press_event (GtkWidget   *widget,
                                      GdkEventKey *key)
{
  DzlSuggestionEntry *self = (DzlSuggestionEntry *)widget;
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);
  GdkDevice *device;
  gboolean ret;

  g_assert (DZL_IS_SUGGESTION_ENTRY (self));
  g_assert (priv->in_key_press >= 0);

  if ((device = gdk_event_get_device ((GdkEvent *)key)) &&
      gdk_device_get_source (device) == GDK_SOURCE_KEYBOARD)
    device = gdk_device_get_associated_device (device);

  _dzl_suggestion_popover_set_device (priv->popover, device);

  /*
   * If Tab was pressed, and there is uncommitted suggested text,
   * commit it and stop propagation of the key press.
   */
  if (key->keyval == GDK_KEY_Tab && (key->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK)) == 0)
    {
      const gchar *typed_text;
      DzlSuggestion *suggestion;

      typed_text = dzl_suggestion_entry_buffer_get_typed_text (priv->buffer);
      suggestion = dzl_suggestion_popover_get_selected (priv->popover);

      if (typed_text != NULL && suggestion != NULL)
        {
          g_autofree gchar *replace = dzl_suggestion_replace_typed_text (suggestion, typed_text);

          g_signal_handler_block (self, priv->changed_handler);

          if (replace != NULL)
            gtk_entry_set_text (GTK_ENTRY (self), replace);
          else
            dzl_suggestion_entry_buffer_commit (priv->buffer);
          gtk_editable_set_position (GTK_EDITABLE (self), -1);

          g_signal_handler_unblock (self, priv->changed_handler);

          return GDK_EVENT_STOP;
        }
    }

  priv->in_key_press++;
  ret = GTK_WIDGET_CLASS (dzl_suggestion_entry_parent_class)->key_press_event (widget, key);
  priv->in_key_press--;

  return ret;
}

static void
dzl_suggestion_entry_update_attrs (DzlSuggestionEntry *self)
{
  PangoAttribute *attr;
  PangoAttrList *list;
  const gchar *typed_text;
  const gchar *text;
  GdkRGBA rgba;

  DZL_ENTRY;

  g_assert (DZL_IS_SUGGESTION_ENTRY (self));

  gdk_rgba_parse (&rgba, "#666666");

  text = gtk_entry_get_text (GTK_ENTRY (self));
  typed_text = dzl_suggestion_entry_get_typed_text (self);

  list = pango_attr_list_new ();
  attr = pango_attr_foreground_new (rgba.red * 0xFFFF, rgba.green * 0xFFFF, rgba.blue * 0xFFFF);
  attr->start_index = strlen (typed_text);
  attr->end_index = strlen (text);
  pango_attr_list_insert (list, attr);
  gtk_entry_set_attributes (GTK_ENTRY (self), list);
  pango_attr_list_unref (list);

  DZL_EXIT;
}

static void
dzl_suggestion_entry_changed (GtkEditable *editable)
{
  DzlSuggestionEntry *self = (DzlSuggestionEntry *)editable;
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);
  DzlSuggestion *suggestion;
  const gchar *text;

  DZL_ENTRY;

  g_assert (DZL_IS_SUGGESTION_ENTRY (self));

  g_signal_handler_block (self, priv->changed_handler);

  text = dzl_suggestion_entry_buffer_get_typed_text (priv->buffer);

  if (text == NULL || *text == '\0')
    {
      dzl_suggestion_entry_buffer_set_suggestion (priv->buffer, NULL);
      dzl_suggestion_entry_hide_suggestions (self);
      DZL_GOTO (finish);
    }

  suggestion = dzl_suggestion_popover_get_selected (priv->popover);

  if (suggestion != NULL)
    {
      g_object_ref (suggestion);
      dzl_suggestion_entry_buffer_set_suggestion (priv->buffer, suggestion);
      g_object_unref (suggestion);
    }

finish:
  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_TYPED_TEXT]);

  g_signal_handler_unblock (self, priv->changed_handler);

  dzl_suggestion_entry_update_attrs (self);

  DZL_EXIT;
}

static void
dzl_suggestion_entry_real_suggestion_activated (DzlSuggestionEntry *self,
                                                DzlSuggestion      *suggestion)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  DZL_ENTRY;

  g_assert (DZL_IS_SUGGESTION_ENTRY (self));
  g_assert (DZL_IS_SUGGESTION (suggestion));

  gtk_entry_set_text (GTK_ENTRY (self), "");
  dzl_suggestion_entry_buffer_clear (priv->buffer);
  g_signal_emit (self, signals [HIDE_SUGGESTIONS], 0);

  DZL_EXIT;
}

static void
dzl_suggestion_entry_suggestion_activated (DzlSuggestionEntry   *self,
                                           DzlSuggestion        *suggestion,
                                           DzlSuggestionPopover *popover)
{
  DZL_ENTRY;

  g_assert (DZL_IS_SUGGESTION_ENTRY (self));
  g_assert (DZL_IS_SUGGESTION (suggestion));
  g_assert (DZL_IS_SUGGESTION_POPOVER (popover));

  g_signal_emit (self, signals [SUGGESTION_ACTIVATED], 0, suggestion);

  DZL_EXIT;
}

static void
dzl_suggestion_entry_move_suggestion (DzlSuggestionEntry *self,
                                      gint                amount)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  g_assert (DZL_IS_SUGGESTION_ENTRY (self));

  if (priv->popover == NULL || !gtk_widget_get_mapped (GTK_WIDGET (priv->popover)))
    return;

  priv->in_move_by++;
  dzl_suggestion_popover_move_by (priv->popover, amount);
  priv->in_move_by--;
}

static void
dzl_suggestion_entry_activate_suggestion (DzlSuggestionEntry *self)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  DZL_ENTRY;

  g_assert (DZL_IS_SUGGESTION_ENTRY (self));

  dzl_suggestion_popover_activate_selected (priv->popover);

  DZL_EXIT;
}

static void
dzl_suggestion_entry_notify_selected_cb (DzlSuggestionEntry   *self,
                                         GParamSpec           *pspec,
                                         DzlSuggestionPopover *popover)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  g_assert (DZL_IS_SUGGESTION_ENTRY (self));
  g_assert (DZL_IS_SUGGESTION_POPOVER (popover));

  if (priv->in_move_by > 0)
    {
      DzlSuggestion *suggestion = dzl_suggestion_popover_get_selected (priv->popover);

      if (suggestion != NULL)
        g_signal_emit (self, signals [SUGGESTION_SELECTED], 0, suggestion);
    }

  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_SUGGESTION]);
}

static void
dzl_suggestion_entry_constructed (GObject *object)
{
  DzlSuggestionEntry *self = (DzlSuggestionEntry *)object;
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  G_OBJECT_CLASS (dzl_suggestion_entry_parent_class)->constructed (object);

  gtk_entry_set_buffer (GTK_ENTRY (self), GTK_ENTRY_BUFFER (priv->buffer));
}

static void
dzl_suggestion_entry_destroy (GtkWidget *widget)
{
  DzlSuggestionEntry *self = (DzlSuggestionEntry *)widget;
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  if (priv->func_data_destroy != NULL)
    {
      GDestroyNotify notify = g_steal_pointer (&priv->func_data_destroy);
      gpointer notify_data = g_steal_pointer (&priv->func_data);

      notify (notify_data);
    }

  if (priv->popover != NULL)
    gtk_widget_destroy (GTK_WIDGET (priv->popover));

  g_clear_object (&priv->model);
  g_clear_object (&priv->press_gesture);

  g_assert (priv->popover == NULL);

  GTK_WIDGET_CLASS (dzl_suggestion_entry_parent_class)->destroy (widget);
}

static void
dzl_suggestion_entry_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  DzlSuggestionEntry *self = DZL_SUGGESTION_ENTRY (object);

  switch (prop_id)
    {
    case PROP_ACTIVATE_ON_SINGLE_CLICK:
      g_value_set_boolean (value, dzl_suggestion_entry_get_activate_on_single_click (self));
      break;

    case PROP_COMPACT:
      g_value_set_boolean (value, dzl_suggestion_entry_get_compact (self));
      break;

    case PROP_MODEL:
      g_value_set_object (value, dzl_suggestion_entry_get_model (self));
      break;

    case PROP_SUGGESTION:
      g_value_set_object (value, dzl_suggestion_entry_get_suggestion (self));
      break;

    case PROP_TYPED_TEXT:
      g_value_set_string (value, dzl_suggestion_entry_get_typed_text (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_suggestion_entry_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  DzlSuggestionEntry *self = DZL_SUGGESTION_ENTRY (object);

  switch (prop_id)
    {
    case PROP_ACTIVATE_ON_SINGLE_CLICK:
      dzl_suggestion_entry_set_activate_on_single_click (self, g_value_get_boolean (value));
      break;

    case PROP_COMPACT:
      dzl_suggestion_entry_set_compact (self, g_value_get_boolean (value));
      break;

    case PROP_MODEL:
      dzl_suggestion_entry_set_model (self, g_value_get_object (value));
      break;

    case PROP_SUGGESTION:
      dzl_suggestion_entry_set_suggestion (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_suggestion_entry_class_init (DzlSuggestionEntryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkBindingSet *bindings;

  object_class->constructed = dzl_suggestion_entry_constructed;
  object_class->get_property = dzl_suggestion_entry_get_property;
  object_class->set_property = dzl_suggestion_entry_set_property;

  widget_class->destroy = dzl_suggestion_entry_destroy;
  widget_class->focus_in_event = dzl_suggestion_entry_focus_in_event;
  widget_class->focus_out_event = dzl_suggestion_entry_focus_out_event;
  widget_class->hierarchy_changed = dzl_suggestion_entry_hierarchy_changed;
  widget_class->key_press_event = dzl_suggestion_entry_key_press_event;

  klass->hide_suggestions = dzl_suggestion_entry_hide_suggestions;
  klass->show_suggestions = dzl_suggestion_entry_show_suggestions;
  klass->move_suggestion = dzl_suggestion_entry_move_suggestion;
  klass->suggestion_activated = dzl_suggestion_entry_real_suggestion_activated;

  /**
   * DzlSuggestionEntry:activate-on-single-click:
   *
   * The "activate-on-single-click" property denotes if results should be
   * activated simply by clicking on them. You may want to set this to
   * %FALSE if you want the behavior to only select the item.
   *
   * Since: 3.30
   */
  properties [PROP_ACTIVATE_ON_SINGLE_CLICK] =
    g_param_spec_boolean ("activate-on-single-click",
                          "Activate on Single Click",
                          "If entries should be activated upon a single click",
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  /**
   * DzlSuggestionEntry:compact:
   *
   * The "compact" property denotes if an alternate style should be used to
   * reduce the width of the rows. This may be ideal in size contrained
   * situations with portrait display.
   *
   * Since: 3.34
   */
  properties [PROP_COMPACT] =
    g_param_spec_boolean ("compact",
                          "Compact",
                          "If compact mode should be used",
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_MODEL] =
    g_param_spec_object ("model",
                         "Model",
                         "The model to be visualized",
                         G_TYPE_LIST_MODEL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_TYPED_TEXT] =
    g_param_spec_string ("typed-text",
                         "Typed Text",
                         "Typed text into the entry, does not include suggested text",
                         NULL,
                         (G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  /**
   * DzlSuggestionEntry:suggestion:
   *
   * The "suggestion" property is the currently selected suggestion, if any.
   *
   * Since: 3.30
   */
  properties [PROP_SUGGESTION] =
    g_param_spec_object ("suggestion",
                         "Suggestion",
                         "The currently selected suggestion",
                         DZL_TYPE_SUGGESTION,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  signals [ACTION] =
    g_signal_new_class_handler ("action",
                                G_TYPE_FROM_CLASS (klass),
                                G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                G_CALLBACK (dzl_gtk_widget_action_with_string),
                                NULL, NULL, NULL,
                                G_TYPE_NONE,
                                3,
                                G_TYPE_STRING,
                                G_TYPE_STRING,
                                G_TYPE_STRING);

  signals [HIDE_SUGGESTIONS] =
    g_signal_new ("hide-suggestions",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (DzlSuggestionEntryClass, hide_suggestions),
                  NULL, NULL, NULL, G_TYPE_NONE, 0);

  /**
   * DzlSuggestionEntry::move-suggestion:
   * @self: A #DzlSuggestionEntry
   * @amount: The number of items to move
   *
   * This moves the selected suggestion in the popover by the value
   * provided. -1 moves up one row, 1, moves down a row.
   */
  signals [MOVE_SUGGESTION] =
    g_signal_new ("move-suggestion",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (DzlSuggestionEntryClass, move_suggestion),
                  NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_INT);

  signals [SHOW_SUGGESTIONS] =
    g_signal_new ("show-suggestions",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (DzlSuggestionEntryClass, show_suggestions),
                  NULL, NULL, NULL, G_TYPE_NONE, 0);

  signals [SUGGESTION_ACTIVATED] =
    g_signal_new ("suggestion-activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DzlSuggestionEntryClass, suggestion_activated),
                  NULL, NULL, NULL, G_TYPE_NONE, 1, DZL_TYPE_SUGGESTION);
  g_signal_set_va_marshaller (signals [SUGGESTION_ACTIVATED],
                              G_TYPE_FROM_CLASS (klass),
                              g_cclosure_marshal_VOID__OBJECTv);

  /**
   * DzlSuggestionEntry::suggestion-selected:
   * @self: a #DzlSuggestionEntry
   * @suggestion: a #DzlSuggestion
   *
   * This signal is emitted when a selection has been specifically selected
   * by the user, such as by clicking on the row or moving to the row with
   * keyboard, such as with #DzlSuggestionEntry::move-suggestion
   *
   * Since: 3.30
   */
  signals [SUGGESTION_SELECTED] =
    g_signal_new ("suggestion-selected",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DzlSuggestionEntryClass, suggestion_selected),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, DZL_TYPE_SUGGESTION);
  g_signal_set_va_marshaller (signals [SUGGESTION_SELECTED],
                              G_TYPE_FROM_CLASS (klass),
                              g_cclosure_marshal_VOID__OBJECTv);

  widget_class->activate_signal = signals [ACTIVATE_SUGGESTION] =
    g_signal_new_class_handler ("activate-suggestion",
                                G_TYPE_FROM_CLASS (klass),
                                G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                                G_CALLBACK (dzl_suggestion_entry_activate_suggestion),
                                NULL, NULL, NULL, G_TYPE_NONE, 0);

  bindings = gtk_binding_set_by_class (klass);
  gtk_binding_entry_add_signal (bindings, GDK_KEY_Escape, 0, "hide-suggestions", 0);
  gtk_binding_entry_add_signal (bindings, GDK_KEY_space, GDK_CONTROL_MASK, "show-suggestions", 0);
  gtk_binding_entry_add_signal (bindings, GDK_KEY_Up, 0, "move-suggestion", 1, G_TYPE_INT, -1);
  gtk_binding_entry_add_signal (bindings, GDK_KEY_Down, 0, "move-suggestion", 1, G_TYPE_INT, 1);
  gtk_binding_entry_add_signal (bindings, GDK_KEY_Page_Up, 0, "move-suggestion", 1, G_TYPE_INT, -10);
  gtk_binding_entry_add_signal (bindings, GDK_KEY_KP_Page_Up, 0, "move-suggestion", 1, G_TYPE_INT, -10);
  gtk_binding_entry_add_signal (bindings, GDK_KEY_Prior, 0, "move-suggestion", 1, G_TYPE_INT, -10);
  gtk_binding_entry_add_signal (bindings, GDK_KEY_Next, 0, "move-suggestion", 1, G_TYPE_INT, 10);
  gtk_binding_entry_add_signal (bindings, GDK_KEY_Page_Down, 0, "move-suggestion", 1, G_TYPE_INT, 10);
  gtk_binding_entry_add_signal (bindings, GDK_KEY_KP_Page_Down, 0, "move-suggestion", 1, G_TYPE_INT, 10);
  gtk_binding_entry_add_signal (bindings, GDK_KEY_Return, 0, "activate-suggestion", 0);

  changed_signal_id = g_signal_lookup ("changed", GTK_TYPE_ENTRY);
}

static void
dzl_suggestion_entry_set_selection_bounds (GtkEditable *editable,
                                           gint         start_pos,
                                           gint         end_pos)
{
  DzlSuggestionEntry *self = (DzlSuggestionEntry *)editable;
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  g_assert (DZL_IS_SUGGESTION_ENTRY (self));

  g_signal_handler_block (self, priv->changed_handler);

  if (end_pos < 0)
    end_pos = gtk_entry_buffer_get_length (GTK_ENTRY_BUFFER (priv->buffer));

  if (end_pos > (gint)dzl_suggestion_entry_buffer_get_typed_length (priv->buffer))
    dzl_suggestion_entry_buffer_commit (priv->buffer);

  editable_parent_iface->set_selection_bounds (editable, start_pos, end_pos);

  g_signal_handler_unblock (self, priv->changed_handler);
}

static void
dzl_suggestion_entry_gesture_released (DzlSuggestionEntry   *self,
                                       gint                  n_press,
                                       gdouble               x,
                                       gdouble               y,
                                       GtkGestureMultiPress *gesture)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  g_assert (GTK_IS_GESTURE_MULTI_PRESS (gesture));
  g_assert (DZL_IS_SUGGESTION_ENTRY (self));

  if (n_press == 1 && priv->allow_touch_selection)
    {
      priv->allow_touch_selection = FALSE;
      dzl_suggestion_entry_set_selection_bounds (GTK_EDITABLE (self), 0, -1);
    }
}

static void
dzl_suggestion_entry_init (DzlSuggestionEntry *self)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  priv->activate_on_single_click = TRUE;
  priv->func = dzl_suggestion_entry_default_position_func;

  priv->changed_handler =
    g_signal_connect_after (self,
                            "changed",
                            G_CALLBACK (dzl_suggestion_entry_changed),
                            NULL);

  priv->popover = g_object_new (DZL_TYPE_SUGGESTION_POPOVER,
                                "destroy-with-parent", TRUE,
                                "modal", FALSE,
                                "relative-to", self,
                                "type", GTK_WINDOW_POPUP,
                                "type-hint", GDK_WINDOW_TYPE_HINT_COMBO,
                                NULL);
  g_signal_connect_object (priv->popover,
                           "notify::selected",
                           G_CALLBACK (dzl_suggestion_entry_notify_selected_cb),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->popover,
                           "suggestion-activated",
                           G_CALLBACK (dzl_suggestion_entry_suggestion_activated),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect (priv->popover,
                    "destroy",
                    G_CALLBACK (gtk_widget_destroyed),
                    &priv->popover);
  gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (self)),
                               "suggestion");

  priv->buffer = dzl_suggestion_entry_buffer_new ();

  priv->press_gesture = gtk_gesture_multi_press_new (GTK_WIDGET (self));
  gtk_gesture_single_set_touch_only (GTK_GESTURE_SINGLE (priv->press_gesture), TRUE);
  g_signal_connect_object (priv->press_gesture,
                           "released",
                           G_CALLBACK (dzl_suggestion_entry_gesture_released),
                           self,
                           G_CONNECT_SWAPPED);
}

GtkWidget *
dzl_suggestion_entry_new (void)
{
  return g_object_new (DZL_TYPE_SUGGESTION_ENTRY, NULL);
}

static GObject *
dzl_suggestion_entry_get_internal_child (GtkBuildable *buildable,
                                         GtkBuilder   *builder,
                                         const gchar  *childname)
{
  DzlSuggestionEntry *self = (DzlSuggestionEntry *)buildable;
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  if (g_strcmp0 (childname, "popover") == 0)
    return G_OBJECT (priv->popover);

  return NULL;
}

static void
buildable_iface_init (GtkBuildableIface *iface)
{
  iface->get_internal_child = dzl_suggestion_entry_get_internal_child;
}

static void
editable_iface_init (GtkEditableInterface *iface)
{
  editable_parent_iface = g_type_interface_peek_parent (iface);

  iface->set_selection_bounds = dzl_suggestion_entry_set_selection_bounds;
}


/**
 * dzl_suggestion_entry_get_model:
 * @self: a #DzlSuggestionEntry
 *
 * Gets the model being visualized.
 *
 * Returns: (nullable) (transfer none): A #GListModel or %NULL.
 */
GListModel *
dzl_suggestion_entry_get_model (DzlSuggestionEntry *self)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SUGGESTION_ENTRY (self), NULL);

  return priv->model;
}

void
dzl_suggestion_entry_set_model (DzlSuggestionEntry *self,
                                GListModel         *model)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  DZL_ENTRY;

  g_return_if_fail (DZL_IS_SUGGESTION_ENTRY (self));
  g_return_if_fail (!model || g_type_is_a (g_list_model_get_item_type (model), DZL_TYPE_SUGGESTION));

  if (g_set_object (&priv->model, model))
    {
      DZL_TRACE_MSG ("Model has %u items",
                     model ? g_list_model_get_n_items (model) : 0);
      dzl_suggestion_popover_set_model (priv->popover, model);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_MODEL]);
      dzl_suggestion_entry_update_attrs (self);

      if (gtk_widget_has_focus (GTK_WIDGET (self)))
        g_signal_emit (self, signals [SHOW_SUGGESTIONS], 0);
    }

  DZL_EXIT;
}

/**
 * dzl_suggestion_entry_get_suggestion:
 * @self: a #DzlSuggestionEntry
 *
 * Gets the currently selected suggestion.
 *
 * Returns: (nullable) (transfer none): An #DzlSuggestion or %NULL.
 */
DzlSuggestion *
dzl_suggestion_entry_get_suggestion (DzlSuggestionEntry *self)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SUGGESTION_ENTRY (self), NULL);

  return dzl_suggestion_popover_get_selected (priv->popover);
}

void
dzl_suggestion_entry_set_suggestion (DzlSuggestionEntry *self,
                                     DzlSuggestion      *suggestion)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  DZL_ENTRY;

  g_return_if_fail (DZL_IS_SUGGESTION_ENTRY (self));
  g_return_if_fail (!suggestion || DZL_IS_SUGGESTION_ENTRY (suggestion));

  dzl_suggestion_popover_set_selected (priv->popover, suggestion);
  dzl_suggestion_entry_buffer_set_suggestion (priv->buffer, suggestion);

  DZL_EXIT;
}

const gchar *
dzl_suggestion_entry_get_typed_text (DzlSuggestionEntry *self)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SUGGESTION_ENTRY (self), NULL);

  return dzl_suggestion_entry_buffer_get_typed_text (priv->buffer);
}

void
dzl_suggestion_entry_default_position_func (DzlSuggestionEntry *self,
                                            GdkRectangle       *area,
                                            gboolean           *is_absolute,
                                            gpointer            user_data)
{
  GtkStyleContext *style_context;
  GtkAllocation alloc;
  GtkStateFlags state;
  GtkBorder margin;

  g_return_if_fail (DZL_IS_SUGGESTION_ENTRY (self));
  g_return_if_fail (area != NULL);
  g_return_if_fail (is_absolute != NULL);

  *is_absolute = FALSE;

  gtk_widget_get_allocation (GTK_WIDGET (self), &alloc);

  area->y += alloc.height;
  area->height = 300;

  /* Adjust for bottom margin */
  style_context = gtk_widget_get_style_context (GTK_WIDGET (self));
  state = gtk_style_context_get_state (style_context);
  gtk_style_context_get_margin (style_context, state, &margin);

  area->y -= margin.bottom;
  area->x += margin.left;
  area->width -= margin.left + margin.right;
}

/**
 * dzl_suggestion_entry_window_position_func:
 *
 * This is a #DzlSuggestionPositionFunc that can be used to make the suggestion
 * popover the full width of the window. It is similar to what you might find
 * in a web browser.
 */
void
dzl_suggestion_entry_window_position_func (DzlSuggestionEntry *self,
                                           GdkRectangle       *area,
                                           gboolean           *is_absolute,
                                           gpointer            user_data)
{
  GtkWidget *toplevel;

  g_return_if_fail (DZL_IS_SUGGESTION_ENTRY (self));
  g_return_if_fail (area != NULL);
  g_return_if_fail (is_absolute != NULL);

  toplevel = gtk_widget_get_ancestor (GTK_WIDGET (self), GTK_TYPE_WINDOW);

  if (toplevel != NULL)
    {
      GtkWidget *child = gtk_bin_get_child (GTK_BIN (toplevel));
      GtkAllocation alloc;
      gint x, y;
      gint height = 300;

      gtk_widget_translate_coordinates (child, toplevel, 0, 0, &x, &y);
      gtk_widget_get_allocation (child, &alloc);
      gtk_window_get_size (GTK_WINDOW (toplevel), NULL, &height);

      area->x = x;
      area->y = y;
      area->width = alloc.width;
      area->height = MAX (300, height / 2);

      /* If our widget would get obscurred, adjust it */
      gtk_widget_get_allocation (GTK_WIDGET (self), &alloc);
      gtk_widget_translate_coordinates (GTK_WIDGET (self), toplevel,
                                        0, alloc.height, NULL, &y);
      if (y > area->y)
        area->y = y;

      *is_absolute = TRUE;

      return;
    }

  dzl_suggestion_entry_default_position_func (self, area, is_absolute, NULL);
}

/**
 * dzl_suggestion_entry_set_position_func:
 * @self: a #DzlSuggestionEntry
 * @func: (scope async) (closure func_data) (destroy func_data_destroy) (nullable):
 *   A function to call to position the popover, or %NULL to set the default.
 * @func_data: (nullable): closure data for @func
 * @func_data_destroy: (nullable): a destroy notify for @func_data
 *
 * Sets a position func to position the popover.
 *
 * In @func, you should set the height of the rectangle to the maximum height
 * that the popover should be allowed to grow.
 *
 * Since: 3.26
 */
void
dzl_suggestion_entry_set_position_func (DzlSuggestionEntry        *self,
                                        DzlSuggestionPositionFunc  func,
                                        gpointer                   func_data,
                                        GDestroyNotify             func_data_destroy)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);
  GDestroyNotify notify = NULL;
  gpointer notify_data = NULL;

  g_return_if_fail (DZL_IS_SUGGESTION_ENTRY (self));

  if (func == NULL)
    {
      func = dzl_suggestion_entry_default_position_func;
      func_data = NULL;
      func_data_destroy = NULL;
    }

  if (priv->func_data_destroy != NULL)
    {
      notify = priv->func_data_destroy;
      notify_data = priv->func_data;
    }

  priv->func = func;
  priv->func_data = func_data;
  priv->func_data_destroy = func_data_destroy;

  if (notify)
    notify (notify_data);
}

void
_dzl_suggestion_entry_reposition (DzlSuggestionEntry   *self,
                                  DzlSuggestionPopover *popover)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);
  GtkWidget *toplevel;
  GdkWindow *window;
  GtkAllocation alloc;
  gboolean is_absolute = FALSE;
  gint x;
  gint y;

  g_return_if_fail (DZL_IS_SUGGESTION_ENTRY (self));
  g_return_if_fail (DZL_IS_SUGGESTION_POPOVER (popover));

  if (!gtk_widget_get_realized (GTK_WIDGET (self)) ||
      !gtk_widget_get_realized (GTK_WIDGET (popover)))
    return;

  toplevel = gtk_widget_get_toplevel (GTK_WIDGET (self));
  window = gtk_widget_get_window (toplevel);

  gtk_widget_get_allocation (GTK_WIDGET (self), &alloc);

  alloc.x = 0;
  alloc.y = 0;

  priv->func (self, &alloc, &is_absolute, priv->func_data);

  _dzl_suggestion_popover_set_max_height (priv->popover, alloc.height);

  if (!is_absolute)
    {
      gtk_widget_translate_coordinates (GTK_WIDGET (self), toplevel, 0, 0, &x, &y);
      alloc.x += x;
      alloc.y += y;
    }

  gdk_window_get_position (window, &x, &y);
  alloc.x += x;
  alloc.y += y;

  _dzl_suggestion_popover_adjust_margin (popover, &alloc);

  gtk_widget_set_size_request (GTK_WIDGET (popover), alloc.width, -1);
  gtk_window_move (GTK_WINDOW (popover), alloc.x, alloc.y);
}

/**
 * dzl_suggestion_entry_get_popover:
 * @self: a #DzlSuggestionEntry
 *
 * Gets the result display widget for the entry. This is currently
 * always a #DzlSuggestionPopover.
 *
 * Returns: (transfer none): a #GtkWidget
 *
 * Since: 3.32
 */
GtkWidget *
dzl_suggestion_entry_get_popover (DzlSuggestionEntry *self)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SUGGESTION_ENTRY (self), NULL);

  return GTK_WIDGET (priv->popover);
}

/**
 * dzl_suggestion_entry_set_compact:
 * @self: a #DzlSuggestionEntry
 * @compact: if compact mode should be used
 *
 * Sets the #DzlSuggestionEntry:compact property.
 *
 * Since: 3.34
 */
void
dzl_suggestion_entry_set_compact (DzlSuggestionEntry *self,
                                  gboolean            compact)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  g_return_if_fail (DZL_IS_SUGGESTION_ENTRY (self));

  compact = !!compact;

  if (priv->compact != compact)
    {
      priv->compact = compact;
      _dzl_suggestion_popover_set_compact (priv->popover, compact);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_COMPACT]);
    }
}

/**
 * dzl_suggestion_entry_get_compact:
 * @self: a #DzlSuggestionEntry
 *
 * Gets the #DzlSuggestionEntry:compact property.
 *
 * Returns: %TRUE if compact-mode is in use.
 *
 * Since: 3.34
 */
gboolean
dzl_suggestion_entry_get_compact (DzlSuggestionEntry *self)
{
  DzlSuggestionEntryPrivate *priv = dzl_suggestion_entry_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SUGGESTION_ENTRY (self), FALSE);

  return priv->compact;
}
