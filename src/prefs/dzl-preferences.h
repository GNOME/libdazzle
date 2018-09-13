/* dzl-preferences.h
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

#ifndef DZL_PREFERENCES_H
#define DZL_PREFERENCES_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_PREFERENCES (dzl_preferences_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_INTERFACE (DzlPreferences, dzl_preferences, DZL, PREFERENCES, GObject)

struct _DzlPreferencesInterface
{
  GTypeInterface parent_interface;

  void        (*set_page)                   (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             GHashTable           *map);
  void        (*add_page)                   (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *title,
                                             gint                  priority);
  void        (*add_group)                  (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             const gchar          *title,
                                             gint                  priority);
  void        (*add_list_group)             (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             const gchar          *title,
                                             GtkSelectionMode      mode,
                                             gint                  priority);
  guint       (*add_radio)                  (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             const gchar          *schema_id,
                                             const gchar          *key,
                                             const gchar          *path,
                                             const gchar          *variant_string,
                                             const gchar          *title,
                                             const gchar          *subtitle,
                                             const gchar          *keywords,
                                             gint                  priority);
  guint       (*add_font_button)            (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             const gchar          *schema_id,
                                             const gchar          *key,
                                             const gchar          *title,
                                             const gchar          *keywords,
                                             gint                  priority);
  guint       (*add_switch)                 (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             const gchar          *schema_id,
                                             const gchar          *key,
                                             const gchar          *path,
                                             const gchar          *variant_string,
                                             const gchar          *title,
                                             const gchar          *subtitle,
                                             const gchar          *keywords,
                                             gint                  priority);
  guint       (*add_spin_button)            (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             const gchar          *schema_id,
                                             const gchar          *key,
                                             const gchar          *path,
                                             const gchar          *title,
                                             const gchar          *subtitle,
                                             const gchar          *keywords,
                                             gint                  priority);
  guint      (*add_file_chooser)            (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             const gchar          *schema_id,
                                             const gchar          *key,
                                             const gchar          *path,
                                             const gchar          *title,
                                             const gchar          *subtitle,
                                             GtkFileChooserAction  action,
                                             const gchar          *keywords,
                                             gint                  priority);
  guint      (*add_custom)                  (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             GtkWidget            *widget,
                                             const gchar          *keywords,
                                             gint                  priority);
  gboolean   (*remove_id)                   (DzlPreferences       *self,
                                             guint                 widget_id);
  GtkWidget *(*get_widget)                  (DzlPreferences       *self,
                                             guint                 widget_id);
  guint      (*add_table_row_va)            (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             GtkWidget            *first_widget,
                                             va_list               args);
};

DZL_AVAILABLE_IN_ALL
void       dzl_preferences_add_page         (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *title,
                                             gint                  priority);
DZL_AVAILABLE_IN_ALL
void       dzl_preferences_add_group        (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             const gchar          *title,
                                             gint                  priority);
DZL_AVAILABLE_IN_ALL
void       dzl_preferences_add_list_group   (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             const gchar          *title,
                                             GtkSelectionMode      mode,
                                             gint                  priority);
DZL_AVAILABLE_IN_ALL
guint      dzl_preferences_add_radio        (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             const gchar          *schema_id,
                                             const gchar          *key,
                                             const gchar          *path,
                                             const gchar          *variant_string,
                                             const gchar          *title,
                                             const gchar          *subtitle,
                                             const gchar          *keywords,
                                             gint                  priority);
DZL_AVAILABLE_IN_ALL
guint      dzl_preferences_add_switch       (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             const gchar          *schema_id,
                                             const gchar          *key,
                                             const gchar          *path,
                                             const gchar          *variant_string,
                                             const gchar          *title,
                                             const gchar          *subtitle,
                                             const gchar          *keywords,
                                             gint                  priority);
DZL_AVAILABLE_IN_ALL
guint      dzl_preferences_add_spin_button  (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             const gchar          *schema_id,
                                             const gchar          *key,
                                             const gchar          *path,
                                             const gchar          *title,
                                             const gchar          *subtitle,
                                             const gchar          *keywords,
                                             gint                  priority);
DZL_AVAILABLE_IN_ALL
guint      dzl_preferences_add_custom       (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             GtkWidget            *widget,
                                             const gchar          *keywords,
                                             gint                  priority);
DZL_AVAILABLE_IN_ALL
guint      dzl_preferences_add_font_button  (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             const gchar          *schema_id,
                                             const gchar          *key,
                                             const gchar          *title,
                                             const gchar          *keywords,
                                             gint                  priority);
DZL_AVAILABLE_IN_ALL
guint      dzl_preferences_add_file_chooser (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             const gchar          *schema_id,
                                             const gchar          *key,
                                             const gchar          *path,
                                             const gchar          *title,
                                             const gchar          *subtitle,
                                             GtkFileChooserAction  action,
                                             const gchar          *keywords,
                                             gint                  priority);
DZL_AVAILABLE_IN_ALL
gboolean   dzl_preferences_remove_id        (DzlPreferences       *self,
                                             guint                 widget_id);
DZL_AVAILABLE_IN_ALL
void       dzl_preferences_set_page         (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             GHashTable           *map);
DZL_AVAILABLE_IN_ALL
GtkWidget *dzl_preferences_get_widget       (DzlPreferences       *self,
                                             guint                 widget_id);
DZL_AVAILABLE_IN_3_32
guint      dzl_preferences_add_table_row    (DzlPreferences       *self,
                                             const gchar          *page_name,
                                             const gchar          *group_name,
                                             GtkWidget            *first_widget,
                                             ...) G_GNUC_NULL_TERMINATED;

G_END_DECLS

#endif /* DZL_PREFERENCES_H */
