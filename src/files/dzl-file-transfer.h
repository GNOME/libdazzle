/* dzl-file-transfer.h
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

#pragma once

#include <gio/gio.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_FILE_TRANSFER (dzl_file_transfer_get_type())

DZL_AVAILABLE_IN_3_28
G_DECLARE_DERIVABLE_TYPE (DzlFileTransfer, dzl_file_transfer, DZL, FILE_TRANSFER, GObject)

struct _DzlFileTransferClass
{
  GObjectClass parent_class;

  /*< private >*/
  gpointer _padding[12];
};

typedef enum
{
  DZL_FILE_TRANSFER_FLAGS_NONE = 0,
  DZL_FILE_TRANSFER_FLAGS_MOVE = 1 << 0,
} DzlFileTransferFlags;

typedef struct
{
  gint64 n_files_total;
  gint64 n_files;
  gint64 n_dirs_total;
  gint64 n_dirs;
  gint64 n_bytes_total;
  gint64 n_bytes;

  /*< private >*/
  gint64 _padding[10];
} DzlFileTransferStat;

DZL_AVAILABLE_IN_3_28
DzlFileTransfer      *dzl_file_transfer_new            (void);
DZL_AVAILABLE_IN_3_28
DzlFileTransferFlags  dzl_file_transfer_get_flags      (DzlFileTransfer       *self);
DZL_AVAILABLE_IN_3_28
void                  dzl_file_transfer_set_flags      (DzlFileTransfer       *self,
                                                        DzlFileTransferFlags   flags);
DZL_AVAILABLE_IN_3_28
gdouble               dzl_file_transfer_get_progress   (DzlFileTransfer       *self);
DZL_AVAILABLE_IN_3_28
void                  dzl_file_transfer_stat           (DzlFileTransfer       *self,
                                                        DzlFileTransferStat   *stat_buf);
DZL_AVAILABLE_IN_3_28
void                  dzl_file_transfer_add            (DzlFileTransfer       *self,
                                                        GFile                 *src,
                                                        GFile                 *dest);
DZL_AVAILABLE_IN_3_28
void                  dzl_file_transfer_execute_async  (DzlFileTransfer       *self,
                                                        gint                   io_priority,
                                                        GCancellable          *cancellable,
                                                        GAsyncReadyCallback    callback,
                                                        gpointer               user_data);
DZL_AVAILABLE_IN_3_28
gboolean              dzl_file_transfer_execute_finish (DzlFileTransfer       *self,
                                                        GAsyncResult          *result,
                                                        GError               **error);
DZL_AVAILABLE_IN_3_28
gboolean              dzl_file_transfer_execute        (DzlFileTransfer       *self,
                                                        gint                   io_priority,
                                                        GCancellable          *cancellable,
                                                        GError               **error);

G_END_DECLS
