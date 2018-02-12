/* dzl-directory-reaper.h
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
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

#ifndef DZL_DIRECTORY_REAPER_H
#define DZL_DIRECTORY_REAPER_H

#include <gio/gio.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_DIRECTORY_REAPER (dzl_directory_reaper_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlDirectoryReaper, dzl_directory_reaper, DZL, DIRECTORY_REAPER, GObject)

DZL_AVAILABLE_IN_ALL
DzlDirectoryReaper *dzl_directory_reaper_new               (void);
DZL_AVAILABLE_IN_ALL
void                dzl_directory_reaper_add_directory     (DzlDirectoryReaper   *self,
                                                            GFile                *directory,
                                                            GTimeSpan             min_age);
DZL_AVAILABLE_IN_ALL
void                dzl_directory_reaper_add_file          (DzlDirectoryReaper   *self,
                                                            GFile                *file,
                                                            GTimeSpan             min_age);
DZL_AVAILABLE_IN_ALL
void                dzl_directory_reaper_add_glob          (DzlDirectoryReaper   *self,
                                                            GFile                *directory,
                                                            const gchar          *glob,
                                                            GTimeSpan             min_age);
DZL_AVAILABLE_IN_ALL
gboolean            dzl_directory_reaper_execute           (DzlDirectoryReaper   *self,
                                                            GCancellable         *cancellable,
                                                            GError              **error);
DZL_AVAILABLE_IN_ALL
void                dzl_directory_reaper_execute_async     (DzlDirectoryReaper   *self,
                                                            GCancellable         *cancellable,
                                                            GAsyncReadyCallback   callback,
                                                            gpointer              user_data);
DZL_AVAILABLE_IN_ALL
gboolean            dzl_directory_reaper_execute_finish    (DzlDirectoryReaper   *self,
                                                            GAsyncResult         *result,
                                                            GError              **error);

G_END_DECLS

#endif /* DZL_DIRECTORY_REAPER_H */
