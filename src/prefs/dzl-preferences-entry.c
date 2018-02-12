/* dzl-preferences-entry.c
 *
 * Copyright (C) 2015-2017 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "dzl-preferences-entry"

#include "config.h"

#include <glib/gi18n.h>

#include "prefs/dzl-preferences-entry.h"

typedef struct
{
  GtkEntry *entry;
  GtkLabel *title;
} DzlPreferencesEntryPrivate;

enum {
  PROP_0,
  PROP_TITLE,
  PROP_TEXT,
  LAST_PROP
};

enum {
  ACTIVATE,
  CHANGED,
  LAST_SIGNAL
};

G_DEFINE_TYPE_WITH_PRIVATE (DzlPreferencesEntry, dzl_preferences_entry, DZL_TYPE_PREFERENCES_BIN)

static GParamSpec *properties [LAST_PROP];
static guint signals [LAST_SIGNAL];

static void
dzl_preferences_entry_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  DzlPreferencesEntry *self = DZL_PREFERENCES_ENTRY (object);
  DzlPreferencesEntryPrivate *priv = dzl_preferences_entry_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_TEXT:
      g_value_set_string (value, gtk_entry_get_text (priv->entry));
      break;

    case PROP_TITLE:
      g_value_set_string (value, gtk_label_get_text (priv->title));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_entry_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  DzlPreferencesEntry *self = DZL_PREFERENCES_ENTRY (object);
  DzlPreferencesEntryPrivate *priv = dzl_preferences_entry_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_TEXT:
      gtk_entry_set_text (priv->entry, g_value_get_string (value));
      break;

    case PROP_TITLE:
      gtk_label_set_label (priv->title, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_entry_activate (DzlPreferencesEntry *self)
{
  DzlPreferencesEntryPrivate *priv = dzl_preferences_entry_get_instance_private (self);

  g_assert (DZL_IS_PREFERENCES_ENTRY (self));

  gtk_widget_grab_focus (GTK_WIDGET (priv->entry));
}

static void
dzl_preferences_entry_changed (DzlPreferencesEntry *self,
                               GtkEntry            *entry)
{
  const gchar *text;

  g_assert (DZL_IS_PREFERENCES_ENTRY (self));
  g_assert (GTK_IS_ENTRY (entry));

  text = gtk_entry_get_text (entry);
  g_signal_emit (self, signals [CHANGED], 0, text);
}

static gboolean
dzl_preferences_entry_matches (DzlPreferencesBin *bin,
                               DzlPatternSpec    *spec)
{
  DzlPreferencesEntry *self = (DzlPreferencesEntry *)bin;
  DzlPreferencesEntryPrivate *priv = dzl_preferences_entry_get_instance_private (self);
  const gchar *tmp;

  g_assert (DZL_IS_PREFERENCES_ENTRY (self));
  g_assert (spec != NULL);

  tmp = gtk_label_get_label (priv->title);
  if (tmp && dzl_pattern_spec_match (spec, tmp))
    return TRUE;

  tmp = gtk_entry_get_text (GTK_ENTRY (priv->entry));
  if (tmp && dzl_pattern_spec_match (spec, tmp))
    return TRUE;

  return FALSE;
}

static void
dzl_preferences_entry_class_init (DzlPreferencesEntryClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  DzlPreferencesBinClass *bin_class = DZL_PREFERENCES_BIN_CLASS (klass);

  object_class->get_property = dzl_preferences_entry_get_property;
  object_class->set_property = dzl_preferences_entry_set_property;

  bin_class->matches = dzl_preferences_entry_matches;

  signals [ACTIVATE] =
    g_signal_new_class_handler ("activate",
                                G_TYPE_FROM_CLASS (klass),
                                G_SIGNAL_RUN_LAST,
                                G_CALLBACK (dzl_preferences_entry_activate),
                                NULL, NULL, NULL, G_TYPE_NONE, 0);

  signals [CHANGED] =
    g_signal_new_class_handler ("changed",
                                G_TYPE_FROM_CLASS (klass),
                                G_SIGNAL_RUN_LAST,
                                NULL, NULL, NULL, NULL,
                                G_TYPE_NONE, 1, G_TYPE_STRING);

  widget_class->activate_signal = signals [ACTIVATE];

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/dazzle/ui/dzl-preferences-entry.ui");
  gtk_widget_class_bind_template_child_private (widget_class, DzlPreferencesEntry, entry);
  gtk_widget_class_bind_template_child_private (widget_class, DzlPreferencesEntry, title);

  properties [PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "Title",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_TEXT] =
    g_param_spec_string ("text",
                         "Text",
                         "Text",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
dzl_preferences_entry_init (DzlPreferencesEntry *self)
{
  DzlPreferencesEntryPrivate *priv = dzl_preferences_entry_get_instance_private (self);

  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect_object (priv->entry,
                           "changed",
                           G_CALLBACK (dzl_preferences_entry_changed),
                           self,
                           G_CONNECT_SWAPPED);
}

/**
 * dzl_preferences_entry_get_title_widget:
 *
 * Returns: (transfer none): A #GtkWidget
 */
GtkWidget *
dzl_preferences_entry_get_title_widget (DzlPreferencesEntry *self)
{
  DzlPreferencesEntryPrivate *priv = dzl_preferences_entry_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_PREFERENCES_ENTRY (self), NULL);

  return GTK_WIDGET (priv->title);
}

/**
 * dzl_preferences_entry_get_entry_widget:
 *
 * Returns: (transfer none): A #GtkWidget
 */
GtkWidget *
dzl_preferences_entry_get_entry_widget (DzlPreferencesEntry *self)
{
  DzlPreferencesEntryPrivate *priv = dzl_preferences_entry_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_PREFERENCES_ENTRY (self), NULL);

  return GTK_WIDGET (priv->entry);
}
