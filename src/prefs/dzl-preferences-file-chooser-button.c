/* dzl-preferences-file-chooser-button.c
 *
 * Copyright (C) 2016 Akshaya Kakkilaya <akshaya.kakkilaya@gmail.com>
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

#define G_LOG_DOMAIN "dzl-preferences-file-chooser-button"

#include "config.h"

#include "util/dzl-util-private.h"
#include "prefs/dzl-preferences-file-chooser-button.h"

struct _DzlPreferencesFileChooserButton
{
  DzlPreferencesBin    parent_instance;

  gchar                *key;
  GSettings            *settings;

  GtkFileChooserButton *widget;
  GtkLabel             *title;
  GtkLabel             *subtitle;
};

G_DEFINE_TYPE (DzlPreferencesFileChooserButton, dzl_preferences_file_chooser_button, DZL_TYPE_PREFERENCES_BIN)

enum {
  PROP_0,
  PROP_ACTION,
  PROP_KEY,
  PROP_SUBTITLE,
  PROP_TITLE,
  LAST_PROP
};

static GParamSpec *properties [LAST_PROP];

static void
dzl_preferences_file_chooser_button_save_file (DzlPreferencesFileChooserButton *self,
                                               GtkFileChooserButton            *widget)
{
  g_autofree gchar *path = NULL;

  g_assert (DZL_IS_PREFERENCES_FILE_CHOOSER_BUTTON (self));

  path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (self->widget));

  g_settings_set_string (self->settings, self->key, path);

}

static void
dzl_preferences_file_chooser_button_connect (DzlPreferencesBin *bin,
                                             GSettings         *settings)
{
  DzlPreferencesFileChooserButton *self = (DzlPreferencesFileChooserButton *)bin;
  g_autofree gchar *file = NULL;
  g_autofree gchar *path = NULL;

  g_assert (DZL_IS_PREFERENCES_FILE_CHOOSER_BUTTON (self));
  g_assert (G_IS_SETTINGS (settings));

  self->settings = g_object_ref (settings);

  file = g_settings_get_string (settings, self->key);

  if (!dzl_str_empty0 (file))
    {
      if (!g_path_is_absolute (file))
        path = g_build_filename (g_get_home_dir (), file, NULL);
      else
        path = g_steal_pointer (&file);

      gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (self->widget), path);
    }

  g_signal_connect_object (self->widget,
                           "file-set",
                           G_CALLBACK (dzl_preferences_file_chooser_button_save_file),
                           self,
                           G_CONNECT_SWAPPED);
}

static gboolean
dzl_preferences_file_chooser_button_matches (DzlPreferencesBin *bin,
                                             DzlPatternSpec    *spec)
{
  DzlPreferencesFileChooserButton *self = (DzlPreferencesFileChooserButton *)bin;
  const gchar *tmp;

  g_assert (DZL_IS_PREFERENCES_FILE_CHOOSER_BUTTON (self));
  g_assert (spec != NULL);

  tmp = gtk_label_get_label (self->title);
  if (tmp && dzl_pattern_spec_match (spec, tmp))
    return TRUE;

  tmp = gtk_label_get_label (self->subtitle);
  if (tmp && dzl_pattern_spec_match (spec, tmp))
    return TRUE;

  if (self->key && dzl_pattern_spec_match (spec, self->key))
    return TRUE;

  return FALSE;
}

static void
dzl_preferences_file_chooser_button_finalize (GObject *object)
{
  DzlPreferencesFileChooserButton *self = (DzlPreferencesFileChooserButton *)object;

  g_clear_pointer (&self->key, g_free);
  g_clear_object (&self->settings);

  G_OBJECT_CLASS (dzl_preferences_file_chooser_button_parent_class)->finalize (object);
}

static void
dzl_preferences_file_chooser_button_get_property (GObject    *object,
                                                  guint       prop_id,
                                                  GValue     *value,
                                                  GParamSpec *pspec)
{
  DzlPreferencesFileChooserButton *self = DZL_PREFERENCES_FILE_CHOOSER_BUTTON (object);

  switch (prop_id)
    {
    case PROP_ACTION:
      g_value_set_enum (value, gtk_file_chooser_get_action (GTK_FILE_CHOOSER (self->widget)));
      break;

    case PROP_KEY:
      g_value_set_string (value, self->key);
      break;

    case PROP_TITLE:
      g_value_set_string (value, gtk_label_get_label (self->title));
      break;

    case PROP_SUBTITLE:
      g_value_set_string (value, gtk_label_get_label (self->title));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_file_chooser_button_set_property (GObject      *object,
                                                  guint         prop_id,
                                                  const GValue *value,
                                                  GParamSpec   *pspec)
{
  DzlPreferencesFileChooserButton *self = DZL_PREFERENCES_FILE_CHOOSER_BUTTON (object);

  switch (prop_id)
    {
    case PROP_ACTION:
      gtk_file_chooser_set_action (GTK_FILE_CHOOSER (self->widget), g_value_get_enum (value));
      break;

    case PROP_KEY:
      self->key = g_value_dup_string (value);
      break;

    case PROP_TITLE:
      gtk_label_set_label (self->title, g_value_get_string (value));
      break;

    case PROP_SUBTITLE:
      gtk_label_set_label (self->subtitle, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_file_chooser_button_class_init (DzlPreferencesFileChooserButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  DzlPreferencesBinClass *bin_class = DZL_PREFERENCES_BIN_CLASS (klass);

  object_class->finalize = dzl_preferences_file_chooser_button_finalize;
  object_class->get_property = dzl_preferences_file_chooser_button_get_property;
  object_class->set_property = dzl_preferences_file_chooser_button_set_property;

  bin_class->connect = dzl_preferences_file_chooser_button_connect;
  bin_class->matches = dzl_preferences_file_chooser_button_matches;

  properties [PROP_ACTION] =
    g_param_spec_enum ("action",
                       "Action",
                       "Action",
                       GTK_TYPE_FILE_CHOOSER_ACTION,
                       GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                       (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_KEY] =
    g_param_spec_string ("key",
                         "Key",
                         "Key",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "Title",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_SUBTITLE] =
    g_param_spec_string ("subtitle",
                         "Subtitle",
                         "Subtitle",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/dazzle/ui/dzl-preferences-file-chooser-button.ui");
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesFileChooserButton, widget);
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesFileChooserButton, title);
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesFileChooserButton, subtitle);
}

static void
dzl_preferences_file_chooser_button_init (DzlPreferencesFileChooserButton *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}
