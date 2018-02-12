/* dzl-directory-model.h
 *
 * Copyright (C) 2015 Christian Hergert <christian hergert me>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DZL_DIRECTORY_MODEL_H
#define DZL_DIRECTORY_MODEL_H

#include <gio/gio.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_DIRECTORY_MODEL (dzl_directory_model_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlDirectoryModel, dzl_directory_model, DZL, DIRECTORY_MODEL, GObject)

typedef gboolean (*DzlDirectoryModelVisibleFunc) (DzlDirectoryModel *self,
                                                  GFile             *directory,
                                                  GFileInfo         *file_info,
                                                  gpointer           user_data);

DZL_AVAILABLE_IN_ALL
GListModel *dzl_directory_model_new              (GFile                        *directory);
DZL_AVAILABLE_IN_ALL
GFile      *dzl_directory_model_get_directory    (DzlDirectoryModel            *self);
DZL_AVAILABLE_IN_ALL
void        dzl_directory_model_set_directory    (DzlDirectoryModel            *self,
                                                  GFile                        *directory);
DZL_AVAILABLE_IN_ALL
void        dzl_directory_model_set_visible_func (DzlDirectoryModel            *self,
                                                  DzlDirectoryModelVisibleFunc  visible_func,
                                                  gpointer                      user_data,
                                                  GDestroyNotify                user_data_free_func);

G_END_DECLS

#endif /* DZL_DIRECTORY_MODEL_H */
