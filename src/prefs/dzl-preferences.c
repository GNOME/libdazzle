/* dzl-preferences.c
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

#define G_LOG_DOMAIN "dzl-preferences"

#include "config.h"

#include <string.h>

#include "prefs/dzl-preferences.h"

G_DEFINE_INTERFACE (DzlPreferences, dzl_preferences, G_TYPE_OBJECT)

static void
dzl_preferences_default_init (DzlPreferencesInterface *iface)
{
}

void
dzl_preferences_add_page (DzlPreferences *self,
                          const gchar    *page_name,
                          const gchar    *title,
                          gint            priority)
{
  g_return_if_fail (DZL_IS_PREFERENCES (self));
  g_return_if_fail (page_name != NULL);
  g_return_if_fail ((title != NULL) || (strchr (page_name, '.') != NULL));

  DZL_PREFERENCES_GET_IFACE (self)->add_page (self, page_name, title, priority);
}

void
dzl_preferences_add_group (DzlPreferences *self,
                           const gchar    *page_name,
                           const gchar    *group_name,
                           const gchar    *title,
                           gint            priority)
{
  g_return_if_fail (DZL_IS_PREFERENCES (self));
  g_return_if_fail (page_name != NULL);
  g_return_if_fail (group_name != NULL);

  DZL_PREFERENCES_GET_IFACE (self)->add_group (self, page_name, group_name, title, priority);
}

/**
 * dzl_preferences_add_switch:
 * @path: (nullable): An optional path
 * @variant_string: (nullable): An optional gvariant string
 * @title: (nullable): An optional title
 * @subtitle: (nullable): An optional subtitle
 * @keywords: (nullable): Optional keywords for search
 *
 */
guint
dzl_preferences_add_switch (DzlPreferences *self,
                            const gchar    *page_name,
                            const gchar    *group_name,
                            const gchar    *schema_id,
                            const gchar    *key,
                            const gchar    *path,
                            const gchar    *variant_string,
                            const gchar    *title,
                            const gchar    *subtitle,
                            const gchar    *keywords,
                            gint            priority)
{
  g_return_val_if_fail (DZL_IS_PREFERENCES (self), 0);
  g_return_val_if_fail (page_name != NULL, 0);
  g_return_val_if_fail (group_name != NULL, 0);
  g_return_val_if_fail (schema_id != NULL, 0);
  g_return_val_if_fail (key != NULL, 0);
  g_return_val_if_fail (title != NULL, 0);

  return DZL_PREFERENCES_GET_IFACE (self)->add_switch (self, page_name, group_name, schema_id, key, path, variant_string, title, subtitle, keywords, priority);
}

guint
dzl_preferences_add_spin_button (DzlPreferences *self,
                                 const gchar    *page_name,
                                 const gchar    *group_name,
                                 const gchar    *schema_id,
                                 const gchar    *key,
                                 const gchar    *path,
                                 const gchar    *title,
                                 const gchar    *subtitle,
                                 const gchar    *keywords,
                                 gint            priority)
{
  g_return_val_if_fail (DZL_IS_PREFERENCES (self), 0);
  g_return_val_if_fail (page_name != NULL, 0);
  g_return_val_if_fail (group_name != NULL, 0);
  g_return_val_if_fail (schema_id != NULL, 0);
  g_return_val_if_fail (key != NULL, 0);
  g_return_val_if_fail (title != NULL, 0);

  return DZL_PREFERENCES_GET_IFACE (self)->add_spin_button (self, page_name, group_name, schema_id, key, path, title, subtitle, keywords, priority);
}

/**
 * dzl_preferences_add_custom:
 * @keywords: (nullable): Optional keywords for search
 *
 */
guint
dzl_preferences_add_custom (DzlPreferences *self,
                            const gchar    *page_name,
                            const gchar    *group_name,
                            GtkWidget      *widget,
                            const gchar    *keywords,
                            gint            priority)
{
  g_return_val_if_fail (DZL_IS_PREFERENCES (self), 0);
  g_return_val_if_fail (page_name != NULL, 0);
  g_return_val_if_fail (group_name != NULL, 0);
  g_return_val_if_fail (GTK_IS_WIDGET (widget), 0);

  return DZL_PREFERENCES_GET_IFACE (self)->add_custom (self, page_name, group_name, widget, keywords, priority);
}

void
dzl_preferences_add_list_group (DzlPreferences   *self,
                                const gchar      *page_name,
                                const gchar      *group_name,
                                const gchar      *title,
                                GtkSelectionMode  mode,
                                gint              priority)
{
  g_return_if_fail (DZL_IS_PREFERENCES (self));
  g_return_if_fail (page_name != NULL);
  g_return_if_fail (group_name != NULL);

  return DZL_PREFERENCES_GET_IFACE (self)->add_list_group  (self, page_name, group_name, title, mode, priority);
}

/**
 * dzl_preferences_add_radio:
 * @path: (nullable): An optional path
 * @variant_string: (nullable): An optional gvariant string
 * @title: (nullable): An optional title
 * @subtitle: (nullable): An optional subtitle
 * @keywords: (nullable): Optional keywords for search
 *
 */
guint
dzl_preferences_add_radio (DzlPreferences *self,
                           const gchar    *page_name,
                           const gchar    *group_name,
                           const gchar    *schema_id,
                           const gchar    *key,
                           const gchar    *path,
                           const gchar    *variant_string,
                           const gchar    *title,
                           const gchar    *subtitle,
                           const gchar    *keywords,
                           gint            priority)
{
  g_return_val_if_fail (DZL_IS_PREFERENCES (self), 0);
  g_return_val_if_fail (page_name != NULL, 0);
  g_return_val_if_fail (group_name != NULL, 0);
  g_return_val_if_fail (schema_id != NULL, 0);
  g_return_val_if_fail (key != NULL, 0);
  g_return_val_if_fail (title != NULL, 0);

  return DZL_PREFERENCES_GET_IFACE (self)->add_radio (self, page_name, group_name, schema_id, key, path, variant_string, title, subtitle, keywords, priority);
}

guint
dzl_preferences_add_font_button (DzlPreferences *self,
                                 const gchar    *page_name,
                                 const gchar    *group_name,
                                 const gchar    *schema_id,
                                 const gchar    *key,
                                 const gchar    *title,
                                 const gchar    *keywords,
                                 gint            priority)
{
  g_return_val_if_fail (DZL_IS_PREFERENCES (self), 0);
  g_return_val_if_fail (page_name != NULL, 0);
  g_return_val_if_fail (group_name != NULL, 0);
  g_return_val_if_fail (schema_id != NULL, 0);
  g_return_val_if_fail (key != NULL, 0);
  g_return_val_if_fail (title != NULL, 0);

  return DZL_PREFERENCES_GET_IFACE (self)->add_font_button (self, page_name, group_name, schema_id, key, title, keywords, priority);
}

guint
dzl_preferences_add_file_chooser (DzlPreferences      *self,
                                  const gchar         *page_name,
                                  const gchar         *group_name,
                                  const gchar         *schema_id,
                                  const gchar         *key,
                                  const gchar         *path,
                                  const gchar         *title,
                                  const gchar         *subtitle,
                                  GtkFileChooserAction action,
                                  const gchar         *keywords,
                                  gint                 priority)
{
  g_return_val_if_fail (DZL_IS_PREFERENCES (self), 0);
  g_return_val_if_fail (page_name != NULL, 0);
  g_return_val_if_fail (group_name != NULL, 0);
  g_return_val_if_fail (schema_id != NULL, 0);
  g_return_val_if_fail (key != NULL, 0);
  g_return_val_if_fail (title != NULL, 0);

  return DZL_PREFERENCES_GET_IFACE (self)->add_file_chooser (self, page_name, group_name, schema_id, key, path, title, subtitle, action, keywords, priority);
}

/**
 * ide_preference_remove_id:
 * @widget_id: An preferences widget id
 *
 */
gboolean
dzl_preferences_remove_id (DzlPreferences *self,
                           guint           widget_id)
{
  g_return_val_if_fail (DZL_IS_PREFERENCES (self), FALSE);
  g_return_val_if_fail (widget_id, FALSE);

  return DZL_PREFERENCES_GET_IFACE (self)->remove_id (self, widget_id);
}

void
dzl_preferences_set_page (DzlPreferences *self,
                          const gchar    *page_name,
                          GHashTable     *map)
{
  g_return_if_fail (DZL_IS_PREFERENCES (self));
  g_return_if_fail (page_name != NULL);

  DZL_PREFERENCES_GET_IFACE (self)->set_page (self, page_name, map);
}

/**
 * dzl_preferences_get_widget:
 *
 * Returns: (transfer none) (nullable): A #GtkWidget or %NULL.
 */
GtkWidget *
dzl_preferences_get_widget (DzlPreferences *self,
                            guint           widget_id)
{
  g_return_val_if_fail (DZL_IS_PREFERENCES (self), NULL);

  return DZL_PREFERENCES_GET_IFACE (self)->get_widget (self, widget_id);
}

guint
dzl_preferences_add_table_row (DzlPreferences *self,
                               const gchar    *page_name,
                               const gchar    *group_name,
                               GtkWidget      *first_widget,
                               ...)
{
  va_list args;
  gint ret;

  g_return_val_if_fail (DZL_IS_PREFERENCES (self), 0);
  g_return_val_if_fail (page_name != NULL, 0);
  g_return_val_if_fail (group_name != NULL, 0);
  g_return_val_if_fail (GTK_IS_WIDGET (first_widget), 0);

  va_start (args, first_widget);
  ret = DZL_PREFERENCES_GET_IFACE (self)->add_table_row_va (self, page_name, group_name, first_widget, args);
  va_end (args);

  return ret;
}
