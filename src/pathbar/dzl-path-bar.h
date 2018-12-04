/* dzl-path-bar.h
 *
 * Copyright (C) 2016-2017 Christian Hergert <chergert@redhat.com>
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

#ifndef DZL_PATH_BAR_H
#define DZL_PATH_BAR_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

#include "pathbar/dzl-path.h"

G_BEGIN_DECLS

#define DZL_TYPE_PATH_BAR (dzl_path_bar_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlPathBar, dzl_path_bar, DZL, PATH_BAR, GtkBox)

DZL_AVAILABLE_IN_ALL
GtkWidget *dzl_path_bar_new                (void);
DZL_AVAILABLE_IN_ALL
DzlPath   *dzl_path_bar_get_path           (DzlPathBar *self);
DZL_AVAILABLE_IN_ALL
void       dzl_path_bar_set_path           (DzlPathBar *self,
                                            DzlPath    *path);
DZL_AVAILABLE_IN_ALL
void       dzl_path_bar_set_selected_index (DzlPathBar *self,
                                            guint       index);

G_END_DECLS

#endif /* DZL_PATH_BAR_H */

