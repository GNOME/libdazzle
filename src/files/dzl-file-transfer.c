/* dzl-file-transfer.c
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

#define G_LOG_DOMAIN "dzl-file-transfer"

#include "config.h"

#include "dzl-debug.h"
#include "dzl-enums.h"

#include "files/dzl-directory-reaper.h"
#include "files/dzl-file-transfer.h"
#include "util/dzl-macros.h"

#define QUERY_ATTRS (G_FILE_ATTRIBUTE_STANDARD_NAME"," \
                     G_FILE_ATTRIBUTE_STANDARD_TYPE"," \
                     G_FILE_ATTRIBUTE_STANDARD_IS_SYMLINK"," \
                     G_FILE_ATTRIBUTE_STANDARD_SIZE)
#define QUERY_FLAGS (G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS)

typedef struct
{
  GPtrArray *opers;

  DzlFileTransferStat stat_buf;

  DzlFileTransferFlags flags;

  gint64 last_num_bytes;

  guint executed : 1;
} DzlFileTransferPrivate;

typedef struct
{
  /* Unowned pointers */
  DzlFileTransfer *self;
  GCancellable *cancellable;

  /* Owned pointers */
  GFile *src;
  GFile *dst;
  GError *error;

  DzlFileTransferFlags flags;
} Oper;

typedef void (*FileWalkCallback) (GFile     *file,
                                  GFileInfo *child_info,
                                  gpointer   user_data);

enum {
  PROP_0,
  PROP_FLAGS,
  PROP_PROGRESS,
  N_PROPS
};

G_DEFINE_TYPE_WITH_PRIVATE (DzlFileTransfer, dzl_file_transfer, G_TYPE_OBJECT)

static GParamSpec *properties [N_PROPS];

static void
oper_free (gpointer data)
{
  Oper *oper = data;

  oper->self = NULL;
  oper->cancellable = NULL;

  g_clear_object (&oper->src);
  g_clear_object (&oper->dst);
  g_clear_error (&oper->error);

  g_slice_free (Oper, oper);
}

static void
dzl_file_transfer_finalize (GObject *object)
{
  DzlFileTransfer *self = (DzlFileTransfer *)object;
  DzlFileTransferPrivate *priv = dzl_file_transfer_get_instance_private (self);

  g_clear_pointer (&priv->opers, g_ptr_array_unref);

  G_OBJECT_CLASS (dzl_file_transfer_parent_class)->finalize (object);
}

static void
dzl_file_transfer_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  DzlFileTransfer *self = DZL_FILE_TRANSFER (object);

  switch (prop_id)
    {
    case PROP_FLAGS:
      g_value_set_flags (value, dzl_file_transfer_get_flags (self));
      break;

    case PROP_PROGRESS:
      g_value_set_double (value, dzl_file_transfer_get_progress (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_file_transfer_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  DzlFileTransfer *self = DZL_FILE_TRANSFER (object);

  switch (prop_id)
    {
    case PROP_FLAGS:
      dzl_file_transfer_set_flags (self, g_value_get_flags (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_file_transfer_class_init (DzlFileTransferClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_file_transfer_finalize;
  object_class->get_property = dzl_file_transfer_get_property;
  object_class->set_property = dzl_file_transfer_set_property;

  properties [PROP_FLAGS] =
    g_param_spec_flags ("flags",
                        "Flags",
                        "The transfer flags for the operation",
                        DZL_TYPE_FILE_TRANSFER_FLAGS,
                        DZL_FILE_TRANSFER_FLAGS_NONE,
                        (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_PROGRESS] =
    g_param_spec_double ("progress",
                         "Progress",
                         "The transfer progress, from 0 to 1",
                         0.0, 1.0, 0.0,
                         (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_file_transfer_init (DzlFileTransfer *self)
{
  DzlFileTransferPrivate *priv = dzl_file_transfer_get_instance_private (self);

  priv->opers = g_ptr_array_new_with_free_func (oper_free);
}

DzlFileTransfer *
dzl_file_transfer_new (void)
{
  return g_object_new (DZL_TYPE_FILE_TRANSFER, NULL);
}

void
dzl_file_transfer_add (DzlFileTransfer *self,
                       GFile           *src,
                       GFile           *dst)
{
  DzlFileTransferPrivate *priv = dzl_file_transfer_get_instance_private (self);
  Oper *oper;

  DZL_ENTRY;

  g_return_if_fail (DZL_IS_FILE_TRANSFER (self));
  g_return_if_fail (G_IS_FILE (src));
  g_return_if_fail (G_IS_FILE (dst));

  if (priv->executed)
    {
      g_warning ("Cannot add files to transfer after executing");
      DZL_EXIT;
    }

  if (g_file_equal (src, dst))
    {
      g_warning ("Source and destination cannot be the same");
      DZL_EXIT;
    }

  if (g_file_has_prefix (dst, src))
    {
      g_warning ("Destination cannot be within source");
      DZL_EXIT;
    }

  oper = g_slice_new0 (Oper);
  oper->src = g_object_ref (src);
  oper->dst = g_object_ref (dst);
  oper->self = self;

  g_assert (priv->opers != NULL);

  g_ptr_array_add (priv->opers, oper);

  DZL_EXIT;
}

DzlFileTransferFlags
dzl_file_transfer_get_flags (DzlFileTransfer *self)
{
  DzlFileTransferPrivate *priv = dzl_file_transfer_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_FILE_TRANSFER (self), 0);

  return priv->flags;
}

void
dzl_file_transfer_set_flags (DzlFileTransfer      *self,
                             DzlFileTransferFlags  flags)
{
  DzlFileTransferPrivate *priv = dzl_file_transfer_get_instance_private (self);

  DZL_ENTRY;

  g_return_if_fail (DZL_IS_FILE_TRANSFER (self));

  if (priv->executed)
    {
      g_warning ("Cannot set flags after executing transfer");
      DZL_EXIT;
    }

  if (priv->flags != flags)
    {
      priv->flags = flags;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_FLAGS]);
    }

  DZL_EXIT;
}

gdouble
dzl_file_transfer_get_progress (DzlFileTransfer *self)
{
  DzlFileTransferPrivate *priv = dzl_file_transfer_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_FILE_TRANSFER (self), 0.0);

  if (priv->stat_buf.n_bytes_total != 0)
    return (gdouble)priv->stat_buf.n_bytes / (gdouble)priv->stat_buf.n_bytes_total;

  return 0.0;
}

static void
file_walk_full (GFile            *parent,
                GFileInfo        *info,
                GCancellable     *cancellable,
                FileWalkCallback  callback,
                gpointer          user_data)
{
  DZL_ENTRY;

  g_assert (!parent || G_IS_FILE (parent));
  g_assert (G_IS_FILE_INFO (info));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));
  g_assert (callback != NULL);

  if (g_cancellable_is_cancelled (cancellable))
    DZL_EXIT;

  callback (parent, info, user_data);

  if (g_file_info_get_is_symlink (info))
    DZL_EXIT;

  if (g_file_info_get_file_type (info) == G_FILE_TYPE_DIRECTORY)
    {
      g_autoptr(GFileEnumerator) enumerator = NULL;
      g_autoptr(GFile) child = NULL;
      const gchar *name = g_file_info_get_name (info);

      if (name == NULL)
        DZL_EXIT;

      child = g_file_get_child (parent, name);
      enumerator = g_file_enumerate_children (child, QUERY_ATTRS, QUERY_FLAGS, cancellable, NULL);

      if (enumerator != NULL)
        {
          gpointer infoptr;

          while ((infoptr = g_file_enumerator_next_file (enumerator, cancellable, NULL)))
            {
              g_autoptr(GFileInfo) grandchild_info = infoptr;

              file_walk_full (child, grandchild_info, cancellable, callback, user_data);
            }

          g_file_enumerator_close (enumerator, cancellable, NULL);
        }
    }

  DZL_EXIT;
}

static void
file_walk (GFile            *root,
           GCancellable     *cancellable,
           FileWalkCallback  callback,
           gpointer          user_data)
{
  g_autoptr(GFile) parent = NULL;
  g_autoptr(GFileInfo) info = NULL;

  DZL_ENTRY;

  g_assert (G_IS_FILE (root));
  g_assert (callback != NULL);

  parent = g_file_get_parent (root);
  if (g_file_equal (root, parent))
    g_clear_object (&parent);

  info = g_file_query_info (root, QUERY_ATTRS, QUERY_FLAGS, cancellable, NULL);
  if (info != NULL)
    file_walk_full (parent, info, cancellable, callback, user_data);

  DZL_EXIT;
}

static void
handle_preflight_cb (GFile     *file,
                     GFileInfo *child_info,
                     gpointer   user_data)
{
  DzlFileTransferStat *stat_buf = user_data;
  GFileType file_type;

  DZL_ENTRY;

  g_assert (G_IS_FILE (file));
  g_assert (G_IS_FILE_INFO (child_info));
  g_assert (stat_buf != NULL);

  file_type = g_file_info_get_file_type (child_info);

  if (file_type == G_FILE_TYPE_DIRECTORY)
    {
      stat_buf->n_dirs_total++;
    }
  else if (file_type == G_FILE_TYPE_REGULAR)
    {
      stat_buf->n_files_total++;
      stat_buf->n_bytes_total += g_file_info_get_size (child_info);
    }

  DZL_EXIT;
}

static void
handle_preflight (DzlFileTransfer     *self,
                  GPtrArray           *opers,
                  GCancellable        *cancellable)
{
  DzlFileTransferPrivate *priv = dzl_file_transfer_get_instance_private (self);

  DZL_ENTRY;

  g_assert (DZL_IS_FILE_TRANSFER (self));
  g_assert (opers != NULL);

  if (g_cancellable_is_cancelled (cancellable))
    DZL_EXIT;

  for (guint i = 0; i < opers->len; i++)
    {
      Oper *oper = g_ptr_array_index (opers, i);

      g_assert (oper != NULL);
      g_assert (DZL_IS_FILE_TRANSFER (oper->self));
      g_assert (G_IS_FILE (oper->src));
      g_assert (G_IS_FILE (oper->dst));

      file_walk (oper->src, cancellable, handle_preflight_cb, &priv->stat_buf);

      if (oper->error != NULL)
        break;
    }

  DZL_EXIT;
}

static void
dzl_file_transfer_progress_cb (goffset  current_num_bytes,
                               goffset  total_num_bytes,
                               gpointer user_data)
{
  DzlFileTransfer *self = user_data;
  DzlFileTransferPrivate *priv = dzl_file_transfer_get_instance_private (self);

  priv->stat_buf.n_bytes += (current_num_bytes - priv->last_num_bytes);
}

static void
handle_copy_cb (GFile     *file,
                GFileInfo *child_info,
                gpointer   user_data)
{
  DzlFileTransferPrivate *priv;
  g_autoptr(GFile) src = NULL;
  g_autoptr(GFile) dst = NULL;
  const gchar *name;
  Oper *oper = user_data;
  GFileType file_type;

  DZL_ENTRY;

  g_assert (DZL_IS_FILE_TRANSFER (oper->self));
  g_assert (G_IS_FILE (oper->src));
  g_assert (G_IS_FILE (oper->dst));
  g_assert (G_IS_FILE (file));
  g_assert (G_IS_FILE_INFO (child_info));

  if (oper->error != NULL)
    DZL_EXIT;

  if (g_cancellable_is_cancelled (oper->cancellable))
    DZL_EXIT;

  priv = dzl_file_transfer_get_instance_private (oper->self);

  file_type = g_file_info_get_file_type (child_info);
  name = g_file_info_get_name (child_info);

  if (name == NULL)
    DZL_EXIT;

  src = g_file_get_child (file, name);

  if (!g_file_equal (oper->src, src))
    {
      g_autofree gchar *relative = NULL;

      relative = g_file_get_relative_path (oper->src, src);
      dst = g_file_get_child (oper->dst, relative);
    }
  else
    {
      dst = g_object_ref (oper->dst);
    }

  priv->last_num_bytes = 0;

  switch (file_type)
    {
    case G_FILE_TYPE_DIRECTORY:
      g_file_make_directory_with_parents (dst, oper->cancellable, &oper->error);
      break;

    case G_FILE_TYPE_REGULAR:
    case G_FILE_TYPE_SPECIAL:
    case G_FILE_TYPE_SHORTCUT:
    case G_FILE_TYPE_SYMBOLIC_LINK:
      /* Try to use g_file_move() when we can */
      if ((oper->flags & DZL_FILE_TRANSFER_FLAGS_MOVE) != 0)
        g_file_move (src, dst,
                     G_FILE_COPY_NOFOLLOW_SYMLINKS | G_FILE_COPY_ALL_METADATA,
                     oper->cancellable,
                     dzl_file_transfer_progress_cb,
                     oper->self,
                     &oper->error);
      else
        g_file_copy (src, dst,
                     G_FILE_COPY_NOFOLLOW_SYMLINKS | G_FILE_COPY_ALL_METADATA,
                     oper->cancellable,
                     dzl_file_transfer_progress_cb,
                     oper->self,
                     &oper->error);
      break;

    case G_FILE_TYPE_UNKNOWN:
    case G_FILE_TYPE_MOUNTABLE:
    default:
      break;
    }

  DZL_EXIT;
}

static void
handle_copy (DzlFileTransfer *self,
             GPtrArray       *opers,
             GCancellable    *cancellable)
{
  DZL_ENTRY;

  g_assert (DZL_IS_FILE_TRANSFER (self));
  g_assert (opers != NULL);
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  if (g_cancellable_is_cancelled (cancellable))
    DZL_EXIT;

  for (guint i = 0; i < opers->len; i++)
    {
      Oper *oper = g_ptr_array_index (opers, i);

      g_assert (oper != NULL);
      g_assert (G_IS_FILE (oper->src));
      g_assert (G_IS_FILE (oper->dst));

      oper->self = self;
      oper->cancellable = cancellable;

      if (oper->error == NULL)
        {
          file_walk (oper->src, cancellable, handle_copy_cb, oper);

          if (oper->error != NULL)
            break;
        }
    }

  DZL_EXIT;
}

static void
handle_removal (DzlFileTransfer *self,
                GPtrArray       *opers,
                GCancellable    *cancellable)
{
  g_autoptr(DzlDirectoryReaper) reaper = NULL;

  DZL_ENTRY;

  g_assert (DZL_IS_FILE_TRANSFER (self));
  g_assert (opers != NULL);
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  if (g_cancellable_is_cancelled (cancellable))
    DZL_EXIT;

  reaper = dzl_directory_reaper_new ();

  for (guint i = 0; i < opers->len; i++)
    {
      Oper *oper = g_ptr_array_index (opers, i);

      g_assert (oper != NULL);
      g_assert (G_IS_FILE (oper->src));
      g_assert (G_IS_FILE (oper->dst));

      /* Don't delete anything if there was a failure */
      if (oper->error != NULL)
        DZL_EXIT;

      if (g_file_query_file_type (oper->src, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) == G_FILE_TYPE_DIRECTORY)
        dzl_directory_reaper_add_directory (reaper, oper->src, 0);

      dzl_directory_reaper_add_file (reaper, oper->src, 0);
    }

  dzl_directory_reaper_execute (reaper, cancellable, NULL);

  DZL_EXIT;
}

static gboolean
dzl_file_transfer_do_notify_progress (gpointer data)
{
  DzlFileTransfer *self = data;

  DZL_ENTRY;

  g_assert (DZL_IS_FILE_TRANSFER (self));

  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_PROGRESS]);

  DZL_RETURN (G_SOURCE_CONTINUE);
}

static void
dzl_file_transfer_worker (GTask        *task,
                          gpointer      source_object,
                          gpointer      task_data,
                          GCancellable *cancellable)
{
  DzlFileTransfer *self = source_object;
  DzlFileTransferPrivate *priv = dzl_file_transfer_get_instance_private (self);
  GPtrArray *opers = task_data;
  guint notify_source;

  DZL_ENTRY;

  g_assert (G_IS_TASK (task));
  g_assert (DZL_IS_FILE_TRANSFER (self));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));
  g_assert (opers != NULL);

  notify_source = g_timeout_add_full (G_PRIORITY_LOW,
                                      1000 / 4, /* 4x a second */
                                      dzl_file_transfer_do_notify_progress,
                                      g_object_ref (self),
                                      g_object_unref);

  for (guint i = 0; i < opers->len; i++)
    {
      Oper *oper = g_ptr_array_index (opers, i);

      oper->self = self;
      oper->cancellable = cancellable;
      oper->flags = priv->flags;
    }

  handle_preflight (self, opers, cancellable);
  handle_copy (self, opers, cancellable);
  if ((priv->flags & DZL_FILE_TRANSFER_FLAGS_MOVE) != 0)
    handle_removal (self, opers, cancellable);

  for (guint i = 0; i < opers->len; i++)
    {
      Oper *oper = g_ptr_array_index (opers, i);

      if (oper->error != NULL)
        {
          g_task_return_error (task, g_steal_pointer (&oper->error));
          DZL_GOTO (cleanup);
        }
    }

  g_task_return_boolean (task, TRUE);

cleanup:
  g_source_remove (notify_source);

  DZL_EXIT;
}

gboolean
dzl_file_transfer_execute (DzlFileTransfer  *self,
                           gint              io_priority,
                           GCancellable     *cancellable,
                           GError          **error)
{
  DzlFileTransferPrivate *priv = dzl_file_transfer_get_instance_private (self);
  g_autoptr(GTask) task = NULL;
  gboolean ret;

  DZL_ENTRY;

  g_return_val_if_fail (DZL_IS_FILE_TRANSFER (self), FALSE);
  g_return_val_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable), FALSE);

  task = g_task_new (self, cancellable, NULL, NULL);
  g_task_set_source_tag (task, dzl_file_transfer_execute);

  if (priv->executed)
    {
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_INVAL,
                   "Transfer can only be executed once.");
      DZL_RETURN (FALSE);
    }

  if (priv->opers->len == 0)
    {
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_INVAL,
                   "Transfer can only be executed once.");
      DZL_RETURN (FALSE);
    }

  g_task_set_check_cancellable (task, TRUE);
  g_task_set_return_on_cancel (task, TRUE);
  g_task_set_priority (task, io_priority);
  g_task_set_task_data (task, g_steal_pointer (&priv->opers), (GDestroyNotify)g_ptr_array_unref);
  g_task_run_in_thread_sync (task, dzl_file_transfer_worker);

  ret = g_task_propagate_boolean (task, error);

  DZL_RETURN (ret);
}

void
dzl_file_transfer_execute_async (DzlFileTransfer     *self,
                                 gint                 io_priority,
                                 GCancellable        *cancellable,
                                 GAsyncReadyCallback  callback,
                                 gpointer             user_data)
{
  DzlFileTransferPrivate *priv = dzl_file_transfer_get_instance_private (self);
  g_autoptr(GTask) task = NULL;

  DZL_ENTRY;

  g_return_if_fail (DZL_IS_FILE_TRANSFER (self));
  g_return_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable));

  task = g_task_new (self, cancellable, callback, user_data);
  g_task_set_source_tag (task, dzl_file_transfer_execute);

  if (priv->executed)
    {
      g_task_return_new_error (task,
                               G_IO_ERROR,
                               G_IO_ERROR_INVAL,
                               "Transfer can only be executed once.");
      DZL_EXIT;
    }

  priv->executed = TRUE;

  if (priv->opers->len == 0)
    {
      g_task_return_new_error (task,
                               G_IO_ERROR,
                               G_IO_ERROR_INVAL,
                               "No transfers were provided to execute");
      DZL_EXIT;
    }

  g_task_set_check_cancellable (task, TRUE);
  g_task_set_return_on_cancel (task, TRUE);
  g_task_set_priority (task, io_priority);
  g_task_set_task_data (task, g_steal_pointer (&priv->opers), (GDestroyNotify)g_ptr_array_unref);
  g_task_run_in_thread (task, dzl_file_transfer_worker);

  DZL_EXIT;
}

gboolean
dzl_file_transfer_execute_finish (DzlFileTransfer  *self,
                                  GAsyncResult     *result,
                                  GError          **error)
{
  gboolean ret;

  DZL_ENTRY;

  g_return_val_if_fail (DZL_IS_FILE_TRANSFER (self), FALSE);
  g_return_val_if_fail (G_IS_TASK (result), FALSE);
  g_return_val_if_fail (g_task_is_valid (G_TASK (result), self), FALSE);

  ret = g_task_propagate_boolean (G_TASK (result), error);

  DZL_RETURN (ret);
}

/**
 * dzl_file_transfer_stat:
 * @self: a #DzlFileTransfer
 * @stat_buf: (out): a #DzlFileTransferStat
 *
 * Gets statistics about the transfer progress.
 *
 * Since: 3.28
 */
void
dzl_file_transfer_stat (DzlFileTransfer     *self,
                        DzlFileTransferStat *stat_buf)
{
  DzlFileTransferPrivate *priv = dzl_file_transfer_get_instance_private (self);

  g_return_if_fail (DZL_IS_FILE_TRANSFER (self));
  g_return_if_fail (stat_buf != NULL);

  *stat_buf = priv->stat_buf;
}
