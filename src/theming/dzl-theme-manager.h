/* dzl-theme-manager.h
 *
 * Copyright (C) 2015 Christian Hergert <chergert@redhat.com>
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

#ifndef DZL_THEME_MANAGER_H
#define DZL_THEME_MANAGER_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_THEME_MANAGER (dzl_theme_manager_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlThemeManager, dzl_theme_manager, DZL, THEME_MANAGER, GObject)

DZL_AVAILABLE_IN_ALL
DzlThemeManager *dzl_theme_manager_new              (void);
DZL_AVAILABLE_IN_ALL
void             dzl_theme_manager_add_resources    (DzlThemeManager *self,
                                                     const gchar     *resource_path);
DZL_AVAILABLE_IN_ALL
void             dzl_theme_manager_remove_resources (DzlThemeManager *self,
                                                     const gchar     *resource_path);

G_END_DECLS

#endif /* DZL_THEME_MANAGER_H */
