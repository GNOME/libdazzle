/* dzl-recursive-file-monitor.h
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

#ifndef DZL_RECURSIVE_FILE_MONITOR_H
#define DZL_RECURSIVE_FILE_MONITOR_H

#include <gio/gio.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_RECURSIVE_FILE_MONITOR (dzl_recursive_file_monitor_get_type())

DZL_AVAILABLE_IN_3_28
G_DECLARE_FINAL_TYPE (DzlRecursiveFileMonitor, dzl_recursive_file_monitor, DZL, RECURSIVE_FILE_MONITOR, GObject)

typedef gboolean (*DzlRecursiveIgnoreFunc) (GFile    *file,
                                            gpointer  user_data);

DZL_AVAILABLE_IN_3_28
DzlRecursiveFileMonitor *dzl_recursive_file_monitor_new             (GFile                    *root);
DZL_AVAILABLE_IN_3_28
GFile                   *dzl_recursive_file_monitor_get_root        (DzlRecursiveFileMonitor  *self);
DZL_AVAILABLE_IN_3_28
void                     dzl_recursive_file_monitor_start_async     (DzlRecursiveFileMonitor  *self,
                                                                     GCancellable             *cancellable,
                                                                     GAsyncReadyCallback       callback,
                                                                     gpointer                  user_data);
DZL_AVAILABLE_IN_3_28
gboolean                 dzl_recursive_file_monitor_start_finish    (DzlRecursiveFileMonitor  *self,
                                                                     GAsyncResult             *result,
                                                                     GError                  **error);
DZL_AVAILABLE_IN_3_28
void                     dzl_recursive_file_monitor_cancel          (DzlRecursiveFileMonitor  *self);
DZL_AVAILABLE_IN_3_28
void                     dzl_recursive_file_monitor_set_ignore_func (DzlRecursiveFileMonitor  *self,
                                                                     DzlRecursiveIgnoreFunc    ignore_func,
                                                                     gpointer                  ignore_func_data,
                                                                     GDestroyNotify            ignore_func_data_destroy);

G_END_DECLS

#endif /* DZL_RECURSIVE_FILE_MONITOR_H */
