/* dzl-preferences-language-row.c
 *
 * Copyright (C) 2015-2017 Christian Hergert <christian@hergert.me>
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

#include "prefs/dzl-preferences.h"
#include "prefs/dzl-preferences-language-row.h"

struct _DzlPreferencesLanguageRow
{
  DzlPreferencesBin parent_instance;
  gchar *id;
  GtkLabel *title;
};

G_DEFINE_TYPE (DzlPreferencesLanguageRow, dzl_preferences_language_row, DZL_TYPE_PREFERENCES_BIN)

enum {
  PROP_0,
  PROP_ID,
  PROP_TITLE,
  LAST_PROP
};

enum {
  ACTIVATE,
  LAST_SIGNAL
};

static GParamSpec *properties [LAST_PROP];
static guint signals [LAST_SIGNAL];

static void
dzl_preferences_language_row_activate (DzlPreferencesLanguageRow *self)
{
  GtkWidget *preferences;
  GHashTable *map;

  g_assert (DZL_IS_PREFERENCES_LANGUAGE_ROW (self));

  if (self->id == NULL)
    return;

  preferences = gtk_widget_get_ancestor (GTK_WIDGET (self), DZL_TYPE_PREFERENCES);
  if (preferences == NULL)
    return;

  map = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_free);
  g_hash_table_insert (map, "{id}", g_strdup (self->id));
  dzl_preferences_set_page (DZL_PREFERENCES (preferences), "languages.id", map);
  g_hash_table_unref (map);
}

static void
dzl_preferences_language_row_get_property (GObject    *object,
                                           guint       prop_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
  DzlPreferencesLanguageRow *self = DZL_PREFERENCES_LANGUAGE_ROW (object);

  switch (prop_id)
    {
    case PROP_ID:
      g_value_set_string (value, self->id);
      break;

    case PROP_TITLE:
      g_value_set_string (value, gtk_label_get_label (self->title));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_language_row_set_property (GObject      *object,
                                           guint         prop_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
  DzlPreferencesLanguageRow *self = DZL_PREFERENCES_LANGUAGE_ROW (object);

  switch (prop_id)
    {
    case PROP_ID:
      self->id = g_value_dup_string (value);
      break;

    case PROP_TITLE:
      gtk_label_set_label (self->title, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_language_row_finalize (GObject *object)
{
  DzlPreferencesLanguageRow *self = (DzlPreferencesLanguageRow *)object;

  g_clear_pointer (&self->id, g_free);

  G_OBJECT_CLASS (dzl_preferences_language_row_parent_class)->finalize (object);
}

static void
dzl_preferences_language_row_class_init (DzlPreferencesLanguageRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = dzl_preferences_language_row_finalize;
  object_class->get_property = dzl_preferences_language_row_get_property;
  object_class->set_property = dzl_preferences_language_row_set_property;

  properties [PROP_ID] =
    g_param_spec_string ("id",
                         "Id",
                         "Id",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "Title",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);

  signals [ACTIVATE] =
    g_signal_new_class_handler ("activate",
                                G_TYPE_FROM_CLASS (klass),
                                G_SIGNAL_RUN_FIRST,
                                G_CALLBACK (dzl_preferences_language_row_activate),
                                NULL, NULL, NULL,
                                G_TYPE_NONE, 0);

  widget_class->activate_signal = signals [ACTIVATE];

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/dazzle/ui/dzl-preferences-language-row.ui");
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesLanguageRow, title);
}

static void
dzl_preferences_language_row_init (DzlPreferencesLanguageRow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
