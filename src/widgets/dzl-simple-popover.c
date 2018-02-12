/* dzl-simple-popover.c
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
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

#include "config.h"

#include "dzl-simple-popover.h"

typedef struct
{
  GtkPopover parent_instance;

  GtkLabel  *title;
  GtkLabel  *message;
  GtkEntry  *entry;
  GtkButton *button;
} DzlSimplePopoverPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (DzlSimplePopover, dzl_simple_popover, GTK_TYPE_POPOVER)

enum {
  PROP_0,
  PROP_BUTTON_TEXT,
  PROP_MESSAGE,
  PROP_READY,
  PROP_TEXT,
  PROP_TITLE,
  LAST_PROP
};

enum {
  ACTIVATE,
  CHANGED,
  INSERT_TEXT,
  LAST_SIGNAL
};

static GParamSpec *properties [LAST_PROP];
static guint signals [LAST_SIGNAL];

const gchar *
dzl_simple_popover_get_button_text (DzlSimplePopover *self)
{
  DzlSimplePopoverPrivate *priv = dzl_simple_popover_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SIMPLE_POPOVER (self), NULL);

  return gtk_button_get_label (priv->button);
}

void
dzl_simple_popover_set_button_text (DzlSimplePopover *self,
                                   const gchar     *button_text)
{
  DzlSimplePopoverPrivate *priv = dzl_simple_popover_get_instance_private (self);

  g_return_if_fail (DZL_IS_SIMPLE_POPOVER (self));

  gtk_button_set_label (priv->button, button_text);
  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_BUTTON_TEXT]);
}

const gchar *
dzl_simple_popover_get_message (DzlSimplePopover *self)
{
  DzlSimplePopoverPrivate *priv = dzl_simple_popover_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SIMPLE_POPOVER (self), NULL);

  return gtk_label_get_text (priv->message);
}

void
dzl_simple_popover_set_message (DzlSimplePopover *self,
                               const gchar     *message)
{
  DzlSimplePopoverPrivate *priv = dzl_simple_popover_get_instance_private (self);

  g_return_if_fail (DZL_IS_SIMPLE_POPOVER (self));

  gtk_label_set_label (priv->message, message);
  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_MESSAGE]);
}

gboolean
dzl_simple_popover_get_ready (DzlSimplePopover *self)
{
  DzlSimplePopoverPrivate *priv = dzl_simple_popover_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SIMPLE_POPOVER (self), FALSE);

  return gtk_widget_get_sensitive (GTK_WIDGET (priv->button));
}

void
dzl_simple_popover_set_ready (DzlSimplePopover *self,
                             gboolean         ready)
{
  DzlSimplePopoverPrivate *priv = dzl_simple_popover_get_instance_private (self);

  g_return_if_fail (DZL_IS_SIMPLE_POPOVER (self));

  gtk_widget_set_sensitive (GTK_WIDGET (priv->button), ready);
  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_READY]);
}

const gchar *
dzl_simple_popover_get_text (DzlSimplePopover *self)
{
  DzlSimplePopoverPrivate *priv = dzl_simple_popover_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SIMPLE_POPOVER (self), NULL);

  return gtk_entry_get_text (priv->entry);
}

void
dzl_simple_popover_set_text (DzlSimplePopover *self,
                            const gchar     *text)
{
  DzlSimplePopoverPrivate *priv = dzl_simple_popover_get_instance_private (self);

  g_return_if_fail (DZL_IS_SIMPLE_POPOVER (self));

  gtk_entry_set_text (priv->entry, text);
  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_TEXT]);
}

const gchar *
dzl_simple_popover_get_title (DzlSimplePopover *self)
{
  DzlSimplePopoverPrivate *priv = dzl_simple_popover_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SIMPLE_POPOVER (self), NULL);

  return gtk_label_get_label (priv->title);
}

void
dzl_simple_popover_set_title (DzlSimplePopover *self,
                             const gchar     *title)
{
  DzlSimplePopoverPrivate *priv = dzl_simple_popover_get_instance_private (self);

  g_return_if_fail (DZL_IS_SIMPLE_POPOVER (self));

  gtk_label_set_label (priv->title, title);
  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_TITLE]);
}

static void
dzl_simple_popover_button_clicked (DzlSimplePopover *self,
                                  GtkButton       *button)
{
  DzlSimplePopoverPrivate *priv = dzl_simple_popover_get_instance_private (self);
  const gchar *text;

  g_assert (DZL_IS_SIMPLE_POPOVER (self));
  g_assert (GTK_IS_BUTTON (button));

  text = gtk_entry_get_text (GTK_ENTRY (priv->entry));
  g_signal_emit (self, signals [ACTIVATE], 0, text);
  gtk_popover_popdown (GTK_POPOVER (self));
}

static void
dzl_simple_popover_entry_activate (DzlSimplePopover *self,
                                  GtkEntry        *entry)
{
  DzlSimplePopoverPrivate *priv = dzl_simple_popover_get_instance_private (self);

  g_assert (DZL_IS_SIMPLE_POPOVER (self));
  g_assert (GTK_IS_ENTRY (entry));

  if (dzl_simple_popover_get_ready (self))
    gtk_widget_activate (GTK_WIDGET (priv->button));
}

static void
dzl_simple_popover_entry_changed (DzlSimplePopover *self,
                                 GtkEntry        *entry)
{
  g_assert (DZL_IS_SIMPLE_POPOVER (self));
  g_assert (GTK_IS_ENTRY (entry));

  g_signal_emit (self, signals [CHANGED], 0);
}

static void
dzl_simple_popover_entry_insert_text (DzlSimplePopover *self,
                                     gchar           *new_text,
                                     gint             new_text_length,
                                     gint            *position,
                                     GtkEntry        *entry)
{
  gboolean ret = GDK_EVENT_PROPAGATE;
  guint pos;
  guint n_chars;

  g_assert (DZL_IS_SIMPLE_POPOVER (self));
  g_assert (new_text != NULL);
  g_assert (position != NULL);

  pos = *position;
  n_chars = (new_text_length >= 0) ? new_text_length : g_utf8_strlen (new_text, -1);

  g_signal_emit (self, signals [INSERT_TEXT], 0, pos, new_text, n_chars, &ret);

  if (ret == GDK_EVENT_STOP)
    g_signal_stop_emission_by_name (entry, "insert-text");
}

static void
dzl_simple_popover_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  DzlSimplePopover *self = DZL_SIMPLE_POPOVER (object);

  switch (prop_id)
    {
    case PROP_BUTTON_TEXT:
      g_value_set_string (value, dzl_simple_popover_get_button_text (self));
      break;

    case PROP_MESSAGE:
      g_value_set_string (value, dzl_simple_popover_get_message (self));
      break;

    case PROP_READY:
      g_value_set_boolean (value, dzl_simple_popover_get_ready (self));
      break;

    case PROP_TEXT:
      g_value_set_string (value, dzl_simple_popover_get_text (self));
      break;

    case PROP_TITLE:
      g_value_set_string (value, dzl_simple_popover_get_title (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_simple_popover_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  DzlSimplePopover *self = DZL_SIMPLE_POPOVER (object);

  switch (prop_id)
    {
    case PROP_BUTTON_TEXT:
      dzl_simple_popover_set_button_text (self, g_value_get_string (value));
      break;

    case PROP_MESSAGE:
      dzl_simple_popover_set_message (self, g_value_get_string (value));
      break;

    case PROP_READY:
      dzl_simple_popover_set_ready (self, g_value_get_boolean (value));
      break;

    case PROP_TEXT:
      dzl_simple_popover_set_text (self, g_value_get_string (value));
      break;

    case PROP_TITLE:
      dzl_simple_popover_set_title (self, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_simple_popover_class_init (DzlSimplePopoverClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = dzl_simple_popover_get_property;
  object_class->set_property = dzl_simple_popover_set_property;

  properties [PROP_BUTTON_TEXT] =
    g_param_spec_string ("button-text",
                         "Button Text",
                         "Button Text",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_MESSAGE] =
    g_param_spec_string ("message",
                         "Message",
                         "Message",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_READY] =
    g_param_spec_boolean ("ready",
                          "Ready",
                          "Ready",
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_TEXT] =
    g_param_spec_string ("text",
                         "Text",
                         "Text",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "Title",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);

  signals [ACTIVATE] =
    g_signal_new ("activate",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DzlSimplePopoverClass, activate),
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_STRING);

  signals [CHANGED] =
    g_signal_new ("changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DzlSimplePopoverClass, insert_text),
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  0);

  signals [INSERT_TEXT] =
    g_signal_new ("insert-text",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DzlSimplePopoverClass, insert_text),
                  NULL, NULL, NULL,
                  G_TYPE_BOOLEAN,
                  3,
                  G_TYPE_UINT,
                  G_TYPE_STRING,
                  G_TYPE_UINT);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/dazzle/ui/dzl-simple-popover.ui");
  gtk_widget_class_bind_template_child_private (widget_class, DzlSimplePopover, title);
  gtk_widget_class_bind_template_child_private (widget_class, DzlSimplePopover, message);
  gtk_widget_class_bind_template_child_private (widget_class, DzlSimplePopover, entry);
  gtk_widget_class_bind_template_child_private (widget_class, DzlSimplePopover, button);
}

static void
dzl_simple_popover_init (DzlSimplePopover *self)
{
  DzlSimplePopoverPrivate *priv = dzl_simple_popover_get_instance_private (self);

  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect_object (priv->button,
                           "clicked",
                           G_CALLBACK (dzl_simple_popover_button_clicked),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (priv->entry,
                           "changed",
                           G_CALLBACK (dzl_simple_popover_entry_changed),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (priv->entry,
                           "activate",
                           G_CALLBACK (dzl_simple_popover_entry_activate),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (priv->entry,
                           "insert-text",
                           G_CALLBACK (dzl_simple_popover_entry_insert_text),
                           self,
                           G_CONNECT_SWAPPED);
}

GtkWidget *
dzl_simple_popover_new (void)
{
  return g_object_new (DZL_TYPE_SIMPLE_POPOVER, NULL);
}
