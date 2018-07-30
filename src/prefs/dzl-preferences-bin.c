/* dzl-preferences-bin.c
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

#define G_LOG_DOMAIN "dzl-preferences-bin"

#include "config.h"

#include <string.h>

#include "prefs/dzl-preferences-bin.h"
#include "util/dzl-macros.h"

typedef struct
{
  GtkBin      parent_instance;

  gint        priority;

  gchar      *keywords;
  gchar      *schema_id;
  gchar      *path;
  GSettings  *settings;
  GHashTable *map;
} DzlPreferencesBinPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (DzlPreferencesBin, dzl_preferences_bin, GTK_TYPE_BIN)

enum {
  PROP_0,
  PROP_KEYWORDS,
  PROP_PRIORITY,
  PROP_SCHEMA_ID,
  PROP_PATH,
  LAST_PROP
};

enum {
  PREFERENCE_ACTIVATED,
  N_SIGNALS
};

static guint signals [N_SIGNALS];

static GParamSpec *properties [LAST_PROP];
static GHashTable *settings_cache;

static gchar *
dzl_preferences_bin_expand (DzlPreferencesBin *self,
                            const gchar       *spec)
{
  DzlPreferencesBinPrivate *priv = dzl_preferences_bin_get_instance_private (self);
  GHashTableIter iter;
  const gchar *key;
  const gchar *value;
  gchar *expanded;

  g_assert (DZL_IS_PREFERENCES_BIN (self));

  if (spec == NULL)
    return NULL;

  expanded = g_strdup (spec);

  if (priv->map == NULL)
    goto validate;

  g_hash_table_iter_init (&iter, priv->map);

  while (g_hash_table_iter_next (&iter, (gpointer *)&key, (gpointer *)&value))
    {
      gchar *tmp = expanded;
      gchar **split;

      split = g_strsplit (tmp, key, 0);
      expanded = g_strjoinv (value, split);

      g_strfreev (split);
      g_free (tmp);
    }

validate:
  if (strchr (expanded, '{') != NULL)
    {
      g_free (expanded);
      return NULL;
    }

  return expanded;
}

static void
dzl_preferences_bin_evict_settings (gpointer  data,
                                    GObject  *where_object_was)
{
  g_assert (data != NULL);
  g_assert (where_object_was != NULL);

  g_hash_table_remove (settings_cache, (gchar *)data);
}

static void
dzl_preferences_bin_cache_settings (const gchar *hash_key,
                                    GSettings   *settings)
{
  gchar *key;

  g_assert (hash_key != NULL);
  g_assert (G_IS_SETTINGS (settings));

  key = g_strdup (hash_key);
  g_hash_table_insert (settings_cache, key, settings);
  g_object_weak_ref (G_OBJECT (settings), dzl_preferences_bin_evict_settings, key);
}

static GSettings *
dzl_preferences_bin_get_settings (DzlPreferencesBin *self)
{
  DzlPreferencesBinPrivate *priv = dzl_preferences_bin_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_PREFERENCES_BIN (self), NULL);

  if (priv->settings == NULL)
    {
      g_autofree gchar *resolved_schema_id = NULL;
      g_autofree gchar *resolved_path = NULL;
      g_autofree gchar *hash_key = NULL;

      resolved_schema_id = dzl_preferences_bin_expand (self, priv->schema_id);
      resolved_path = dzl_preferences_bin_expand (self, priv->path);

      if (resolved_schema_id == NULL)
        return NULL;

      if ((priv->path != NULL) && (resolved_path == NULL))
        return NULL;

      hash_key = g_strdup_printf ("%s|%s",
                                  resolved_schema_id,
                                  resolved_path ?: "");

      if (!g_hash_table_contains (settings_cache, hash_key))
        {
          GSettingsSchemaSource *source;
          GSettingsSchema *schema;

          source = g_settings_schema_source_get_default ();
          schema = g_settings_schema_source_lookup (source, resolved_schema_id, TRUE);

          if (schema != NULL)
            {
              if (resolved_path)
                priv->settings = g_settings_new_with_path (resolved_schema_id, resolved_path);
              else
                priv->settings = g_settings_new (resolved_schema_id);
              dzl_preferences_bin_cache_settings (hash_key, priv->settings);
            }

          g_clear_pointer (&schema, g_settings_schema_unref);
        }
      else
        {
          priv->settings = g_object_ref (g_hash_table_lookup (settings_cache, hash_key));
        }

      g_clear_pointer (&hash_key, g_free);
      g_clear_pointer (&resolved_schema_id, g_free);
      g_clear_pointer (&resolved_path, g_free);
    }

  return (priv->settings != NULL) ? g_object_ref (priv->settings) : NULL;
}


static void
dzl_preferences_bin_connect (DzlPreferencesBin *self,
                             GSettings         *settings)
{
  g_assert (DZL_IS_PREFERENCES_BIN (self));
  g_assert (G_IS_SETTINGS (settings));

  if (DZL_PREFERENCES_BIN_GET_CLASS (self)->connect != NULL)
    DZL_PREFERENCES_BIN_GET_CLASS (self)->connect (self, settings);
}

static void
dzl_preferences_bin_disconnect (DzlPreferencesBin *self,
                                GSettings         *settings)
{
  g_assert (DZL_IS_PREFERENCES_BIN (self));
  g_assert (G_IS_SETTINGS (settings));

  if (DZL_PREFERENCES_BIN_GET_CLASS (self)->disconnect != NULL)
    DZL_PREFERENCES_BIN_GET_CLASS (self)->disconnect (self, settings);
}

static void
dzl_preferences_bin_reload (DzlPreferencesBin *self)
{
  DzlPreferencesBinPrivate *priv = dzl_preferences_bin_get_instance_private (self);
  GSettings *settings;

  g_assert (DZL_IS_PREFERENCES_BIN (self));

  if (priv->settings != NULL)
    {
      dzl_preferences_bin_disconnect (self, priv->settings);
      g_clear_object (&priv->settings);
    }

  settings = dzl_preferences_bin_get_settings (self);

  if (settings != NULL)
    {
      dzl_preferences_bin_connect (self, settings);
      g_object_unref (settings);
    }
}

static void
dzl_preferences_bin_constructed (GObject *object)
{
  DzlPreferencesBin *self = (DzlPreferencesBin *)object;

  G_OBJECT_CLASS (dzl_preferences_bin_parent_class)->constructed (object);

  dzl_preferences_bin_reload (self);
}

static void
dzl_preferences_bin_destroy (GtkWidget *widget)
{
  DzlPreferencesBin *self = (DzlPreferencesBin *)widget;
  DzlPreferencesBinPrivate *priv = dzl_preferences_bin_get_instance_private (self);

  g_assert (DZL_IS_PREFERENCES_BIN (self));

  if (priv->settings != NULL)
    {
      dzl_preferences_bin_disconnect (self, priv->settings);
      g_clear_object (&priv->settings);
    }

  GTK_WIDGET_CLASS (dzl_preferences_bin_parent_class)->destroy (widget);
}

static void
dzl_preferences_bin_activated (GtkWidget *widget)
{
  GtkWidget *child = gtk_bin_get_child (GTK_BIN (widget));

  if (child)
    gtk_widget_activate (child);
}

static void
dzl_preferences_bin_finalize (GObject *object)
{
  DzlPreferencesBin *self = (DzlPreferencesBin *)object;
  DzlPreferencesBinPrivate *priv = dzl_preferences_bin_get_instance_private (self);

  g_clear_pointer (&priv->schema_id, g_free);
  g_clear_pointer (&priv->path, g_free);
  g_clear_pointer (&priv->keywords, g_free);
  g_clear_pointer (&priv->map, g_hash_table_unref);
  g_clear_object (&priv->settings);

  G_OBJECT_CLASS (dzl_preferences_bin_parent_class)->finalize (object);
}

static void
dzl_preferences_bin_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  DzlPreferencesBin *self = DZL_PREFERENCES_BIN (object);
  DzlPreferencesBinPrivate *priv = dzl_preferences_bin_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_SCHEMA_ID:
      g_value_set_string (value, priv->schema_id);
      break;

    case PROP_PATH:
      g_value_set_string (value, priv->path);
      break;

    case PROP_KEYWORDS:
      g_value_set_string (value, priv->keywords);
      break;

    case PROP_PRIORITY:
      g_value_set_int (value, priv->priority);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_bin_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  DzlPreferencesBin *self = DZL_PREFERENCES_BIN (object);
  DzlPreferencesBinPrivate *priv = dzl_preferences_bin_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_SCHEMA_ID:
      priv->schema_id = g_value_dup_string (value);
      break;

    case PROP_PATH:
      priv->path = g_value_dup_string (value);
      break;

    case PROP_KEYWORDS:
      priv->keywords = g_value_dup_string (value);
      break;

    case PROP_PRIORITY:
      priv->priority = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_bin_class_init (DzlPreferencesBinClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = dzl_preferences_bin_constructed;
  object_class->finalize = dzl_preferences_bin_finalize;
  object_class->get_property = dzl_preferences_bin_get_property;
  object_class->set_property = dzl_preferences_bin_set_property;

  widget_class->destroy = dzl_preferences_bin_destroy;

  properties [PROP_KEYWORDS] =
    g_param_spec_string ("keywords",
                         "Keywords",
                         "Search keywords for the widget.",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_PATH] =
    g_param_spec_string ("path",
                         "Path",
                         "Path",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_PRIORITY] =
    g_param_spec_int ("priority",
                      "Priority",
                      "The widget priority within the group.",
                      G_MININT,
                      G_MAXINT,
                      0,
                      (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_SCHEMA_ID] =
    g_param_spec_string ("schema-id",
                         "Schema Id",
                         "Schema Id",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);

  signals [PREFERENCE_ACTIVATED] = widget_class->activate_signal =
    g_signal_new_class_handler ("preference-activated",
                                G_TYPE_FROM_CLASS (klass),
                                G_SIGNAL_RUN_LAST,
                                G_CALLBACK (dzl_preferences_bin_activated),
                                NULL, NULL, NULL, G_TYPE_NONE, 0);

  gtk_widget_class_set_css_name (widget_class, "preferencesbin");

  settings_cache = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}

static void
dzl_preferences_bin_init (DzlPreferencesBin *self)
{
}

void
_dzl_preferences_bin_set_map (DzlPreferencesBin *self,
                              GHashTable        *map)
{
  DzlPreferencesBinPrivate *priv = dzl_preferences_bin_get_instance_private (self);

  g_return_if_fail (DZL_IS_PREFERENCES_BIN (self));

  if (map != priv->map)
    {
      g_clear_pointer (&priv->map, g_hash_table_unref);
      priv->map = map ? g_hash_table_ref (map) : NULL;
      dzl_preferences_bin_reload (self);
    }
}

gboolean
_dzl_preferences_bin_matches (DzlPreferencesBin *self,
                              DzlPatternSpec    *spec)
{
  DzlPreferencesBinPrivate *priv = dzl_preferences_bin_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_PREFERENCES_BIN (self), FALSE);

  if (spec == NULL)
    return TRUE;

  if (priv->keywords && dzl_pattern_spec_match (spec, priv->keywords))
    return TRUE;

  if (priv->schema_id && dzl_pattern_spec_match (spec, priv->schema_id))
    return TRUE;

  if (priv->path && dzl_pattern_spec_match (spec, priv->path))
    return TRUE;

  if (DZL_PREFERENCES_BIN_GET_CLASS (self)->matches)
    return DZL_PREFERENCES_BIN_GET_CLASS (self)->matches (self, spec);

  return FALSE;
}
