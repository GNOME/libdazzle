/* dzl-suggestion-button.c
 *
 * Copyright 2019 Christian Hergert <chergert@redhat.com>
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
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define G_LOG_DOMAIN "dzl-suggestion-button"

#include "config.h"

#include "suggestions/dzl-suggestion-button.h"
#include "suggestions/dzl-suggestion-entry.h"
#include "util/dzl-gtk.h"

typedef struct
{
  DzlSuggestionEntry *entry;
  GtkButton          *button;
  gint                max_width_chars;
} DzlSuggestionButtonPrivate;

enum {
  PROP_0,
  PROP_BUTTON,
  PROP_ENTRY,
  N_PROPS
};

static void buildable_iface_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (DzlSuggestionButton, dzl_suggestion_button, GTK_TYPE_STACK,
                         G_ADD_PRIVATE (DzlSuggestionButton)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, buildable_iface_init))

static GParamSpec *properties [N_PROPS];

static void
entry_icon_press_cb (DzlSuggestionButton  *self,
                     GtkEntryIconPosition  position,
                     GdkEvent             *event,
                     DzlSuggestionEntry   *entry)
{
  DzlSuggestionButtonPrivate *priv = dzl_suggestion_button_get_instance_private (self);

  g_assert (DZL_IS_SUGGESTION_BUTTON (self));
  g_assert (DZL_IS_SUGGESTION_ENTRY (entry));

  if (position == GTK_ENTRY_ICON_PRIMARY)
    gtk_stack_set_visible_child (GTK_STACK (self), GTK_WIDGET (priv->button));
}

static gboolean
entry_focus_in_event_cb (DzlSuggestionButton *self,
                         GdkEventFocus       *focus,
                         DzlSuggestionEntry  *entry)
{
  DzlSuggestionButtonPrivate *priv = dzl_suggestion_button_get_instance_private (self);

  g_assert (DZL_IS_SUGGESTION_BUTTON (self));
  g_assert (DZL_IS_SUGGESTION_ENTRY (entry));

  gtk_entry_set_width_chars (GTK_ENTRY (priv->entry), 1);
  gtk_entry_set_max_width_chars (GTK_ENTRY (priv->entry), priv->max_width_chars ?: 20);

  return GDK_EVENT_PROPAGATE;
}

static gboolean
entry_focus_out_event_cb (DzlSuggestionButton *self,
                          GdkEventFocus       *focus,
                          DzlSuggestionEntry  *entry)
{
  DzlSuggestionButtonPrivate *priv = dzl_suggestion_button_get_instance_private (self);

  g_assert (DZL_IS_SUGGESTION_BUTTON (self));
  g_assert (DZL_IS_SUGGESTION_ENTRY (entry));

  gtk_entry_set_width_chars (GTK_ENTRY (priv->entry), 0);
  gtk_entry_set_max_width_chars (GTK_ENTRY (priv->entry), 0);
  gtk_stack_set_visible_child (GTK_STACK (self), GTK_WIDGET (priv->button));

  return GDK_EVENT_PROPAGATE;
}

static void
dzl_suggestion_button_begin (DzlSuggestionButton *self)
{
  DzlSuggestionButtonPrivate *priv = dzl_suggestion_button_get_instance_private (self);
  gint max_width_chars;

  g_assert (DZL_IS_SUGGESTION_BUTTON (self));

  max_width_chars = gtk_entry_get_max_width_chars (GTK_ENTRY (priv->entry));

  if (max_width_chars)
    priv->max_width_chars = max_width_chars;

  gtk_entry_set_width_chars (GTK_ENTRY (priv->entry), 1);
  gtk_entry_set_max_width_chars (GTK_ENTRY (priv->entry), priv->max_width_chars ?: 20);
  gtk_stack_set_visible_child (GTK_STACK (self), GTK_WIDGET (priv->entry));
  gtk_widget_grab_focus (GTK_WIDGET (priv->entry));
}

static void
button_clicked_cb (DzlSuggestionButton *self,
                   GtkButton           *button)
{
  g_assert (DZL_IS_SUGGESTION_BUTTON (self));
  g_assert (GTK_IS_BUTTON (button));

  dzl_suggestion_button_begin (self);
}

static void
dzl_suggestion_button_get_preferred_width (GtkWidget *widget,
                                           gint      *min_width,
                                           gint      *nat_width)
{
  DzlSuggestionButton *self = (DzlSuggestionButton *)widget;
  DzlSuggestionButtonPrivate *priv = dzl_suggestion_button_get_instance_private (self);
  gint entry_min_width = -1;
  gint entry_nat_width = -1;

  g_assert (DZL_IS_SUGGESTION_BUTTON (self));
  g_assert (min_width != NULL);
  g_assert (nat_width != NULL);

  GTK_WIDGET_CLASS (dzl_suggestion_button_parent_class)->get_preferred_width (widget, min_width, nat_width);

  if (gtk_stack_get_transition_running (GTK_STACK (self)) ||
      gtk_stack_get_visible_child (GTK_STACK (self)) == GTK_WIDGET (priv->entry))
    {
      gtk_widget_get_preferred_width (GTK_WIDGET (priv->entry), &entry_min_width, &entry_nat_width);
      *min_width = MAX (*min_width, entry_min_width);
      *nat_width = MAX (*nat_width, entry_min_width);
    }
}

static void
dzl_suggestion_button_grab_focus (GtkWidget *widget)
{
  DzlSuggestionButton *self = (DzlSuggestionButton *)widget;

  g_assert (DZL_IS_SUGGESTION_BUTTON (self));

  dzl_suggestion_button_begin (self);
}

static void
dzl_suggestion_button_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  DzlSuggestionButton *self = DZL_SUGGESTION_BUTTON (object);

  switch (prop_id)
    {
    case PROP_BUTTON:
      g_value_set_object (value, dzl_suggestion_button_get_button (self));
      break;

    case PROP_ENTRY:
      g_value_set_object (value, dzl_suggestion_button_get_entry (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_suggestion_button_class_init (DzlSuggestionButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = dzl_suggestion_button_get_property;

  widget_class->grab_focus = dzl_suggestion_button_grab_focus;
  widget_class->get_preferred_width = dzl_suggestion_button_get_preferred_width;

  properties [PROP_BUTTON] =
    g_param_spec_object ("button",
                         "Button",
                         "The button to be displayed",
                         GTK_TYPE_BUTTON,
                         (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  properties [PROP_ENTRY] =
    g_param_spec_object ("entry",
                         "Entry",
                         "The entry for user input",
                         DZL_TYPE_SUGGESTION_ENTRY,
                         (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_suggestion_button_init (DzlSuggestionButton *self)
{
  DzlSuggestionButtonPrivate *priv = dzl_suggestion_button_get_instance_private (self);

  gtk_stack_set_hhomogeneous (GTK_STACK (self), FALSE);
  gtk_stack_set_interpolate_size (GTK_STACK (self), TRUE);
  gtk_stack_set_transition_type (GTK_STACK (self), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
  gtk_stack_set_transition_duration (GTK_STACK (self), 200);

  dzl_gtk_widget_add_style_class (GTK_WIDGET (self), "suggestionbutton");

  priv->button = g_object_new (GTK_TYPE_BUTTON,
                               "child", g_object_new (GTK_TYPE_IMAGE,
                                                      "icon-name", "edit-find-symbolic",
                                                      "halign", GTK_ALIGN_START,
                                                      "visible", TRUE,
                                                      NULL),
                               "visible", TRUE,
                               NULL);
  g_signal_connect_object (priv->button,
                           "clicked",
                           G_CALLBACK (button_clicked_cb),
                           self,
                           G_CONNECT_SWAPPED);
  gtk_container_add_with_properties (GTK_CONTAINER (self), GTK_WIDGET (priv->button),
                                     "name", "button",
                                     NULL);

  priv->entry = g_object_new (DZL_TYPE_SUGGESTION_ENTRY,
                              "max-width-chars", 0,
                              "placeholder-text", NULL,
                              "primary-icon-name", "edit-find-symbolic",
                              "visible", TRUE,
                              "width-chars", 0,
                              NULL);
  g_signal_connect_object (priv->entry,
                           "icon-press",
                           G_CALLBACK (entry_icon_press_cb),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->entry,
                           "focus-in-event",
                           G_CALLBACK (entry_focus_in_event_cb),
                           self,
                           G_CONNECT_SWAPPED);
  g_signal_connect_object (priv->entry,
                           "focus-out-event",
                           G_CALLBACK (entry_focus_out_event_cb),
                           self,
                           G_CONNECT_SWAPPED);
  gtk_container_add_with_properties (GTK_CONTAINER (self), GTK_WIDGET (priv->entry),
                                     "name", "entry",
                                     NULL);
}

GtkWidget *
dzl_suggestion_button_new (void)
{
  return g_object_new (DZL_TYPE_SUGGESTION_BUTTON, NULL);
}

/**
 * dzl_suggestion_button_get_entry:
 * @self: a #DzlSuggestionButton
 *
 * Returns: (transfer none): a #DzlSuggestionEntry
 *
 * Since: 3.34
 */
DzlSuggestionEntry *
dzl_suggestion_button_get_entry (DzlSuggestionButton *self)
{
  DzlSuggestionButtonPrivate *priv = dzl_suggestion_button_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SUGGESTION_BUTTON (self), NULL);

  return priv->entry;
}

/**
 * dzl_suggestion_button_get_button:
 * @self: a #DzlSuggestionButton
 *
 * Returns: (transfer none): a #GtkWidget
 *
 * Since: 3.34
 */
GtkButton *
dzl_suggestion_button_get_button (DzlSuggestionButton *self)
{
  DzlSuggestionButtonPrivate *priv = dzl_suggestion_button_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SUGGESTION_BUTTON (self), NULL);

  return priv->button;
}

static GObject *
get_internal_child (GtkBuildable *buildable,
                    GtkBuilder   *builder,
                    const gchar  *childname)
{
  DzlSuggestionButton *self = (DzlSuggestionButton *)buildable;
  DzlSuggestionButtonPrivate *priv = dzl_suggestion_button_get_instance_private (self);

  g_assert (DZL_IS_SUGGESTION_BUTTON (self));

  if (g_strcmp0 (childname, "entry") == 0)
    return G_OBJECT (priv->entry);
  else if (g_strcmp0 (childname, "button") == 0)
    return G_OBJECT (priv->button);
  else
    return NULL;
}

static void
buildable_iface_init (GtkBuildableIface *iface)
{
  iface->get_internal_child = get_internal_child;
}
