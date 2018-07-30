/* dzl-directory-model.c
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

#define G_LOG_DOMAIN "dzl-directory-model"

#include "config.h"

#include <glib/gi18n.h>

#include "files/dzl-directory-model.h"
#include "util/dzl-macros.h"

#define NEXT_FILES_CHUNK_SIZE 25

struct _DzlDirectoryModel
{
  GObject                       parent_instance;

  GCancellable                 *cancellable;
  GFile                        *directory;
  GSequence                    *items;
  GFileMonitor                 *monitor;

  DzlDirectoryModelVisibleFunc  visible_func;
  gpointer                      visible_func_data;
  GDestroyNotify                visible_func_destroy;
};

static void list_model_iface_init      (GListModelInterface *iface);
static void dzl_directory_model_reload (DzlDirectoryModel   *self);

G_DEFINE_TYPE_EXTENDED (DzlDirectoryModel, dzl_directory_model, G_TYPE_OBJECT, 0,
                        G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, list_model_iface_init))

enum {
  PROP_0,
  PROP_DIRECTORY,
  LAST_PROP
};

static GParamSpec *gParamSpecs [LAST_PROP];

static gint
compare_display_name (gconstpointer a,
                      gconstpointer b,
                      gpointer      data)
{
  GFileInfo *file_info_a = (GFileInfo *)a;
  GFileInfo *file_info_b = (GFileInfo *)b;
  const gchar *display_name_a = g_file_info_get_display_name (file_info_a);
  const gchar *display_name_b = g_file_info_get_display_name (file_info_b);
  g_autofree gchar *name_a = g_utf8_collate_key_for_filename (display_name_a, -1);
  g_autofree gchar *name_b = g_utf8_collate_key_for_filename (display_name_b, -1);

  return g_utf8_collate (name_a, name_b);
}

static gint
compare_directories_first (gconstpointer a,
                           gconstpointer b,
                           gpointer      data)
{
  GFileInfo *file_info_a = (GFileInfo *)a;
  GFileInfo *file_info_b = (GFileInfo *)b;
  GFileType file_type_a = g_file_info_get_file_type (file_info_a);
  GFileType file_type_b = g_file_info_get_file_type (file_info_b);

  if (file_type_a == file_type_b)
    return compare_display_name (a, b, data);

  return (file_type_a == G_FILE_TYPE_DIRECTORY) ? -1 : 1;
}

static void
dzl_directory_model_remove_all (DzlDirectoryModel *self)
{
  GSequence *seq;
  guint length;

  g_assert (DZL_IS_DIRECTORY_MODEL (self));

  length = g_sequence_get_length (self->items);

  if (length > 0)
    {
      seq = self->items;
      self->items = g_sequence_new (g_object_unref);
      g_list_model_items_changed (G_LIST_MODEL (self), 0, length, 0);
      g_sequence_free (seq);
    }
}

static void
dzl_directory_model_take_item (DzlDirectoryModel *self,
                               GFileInfo         *file_info)
{
  GSequenceIter *iter;
  guint position;

  g_assert (DZL_IS_DIRECTORY_MODEL (self));
  g_assert (G_IS_FILE_INFO (file_info));

  if ((self->visible_func != NULL) &&
      !self->visible_func (self, self->directory, file_info, self->visible_func_data))
    {
      g_object_unref (file_info);
      return;
    }

  iter = g_sequence_insert_sorted (self->items,
                                   file_info,
                                   compare_directories_first,
                                   NULL);
  position = g_sequence_iter_get_position (iter);
  g_list_model_items_changed (G_LIST_MODEL (self), position, 0, 1);
}

static void
dzl_directory_model_next_files_cb (GObject      *object,
                                   GAsyncResult *result,
                                   gpointer      user_data)
{
  GFileEnumerator *enumerator = (GFileEnumerator *)object;
  g_autoptr(GTask) task = user_data;
  DzlDirectoryModel *self;
  GList *files;
  GList *iter;

  g_assert (G_IS_FILE_ENUMERATOR (enumerator));
  g_assert (G_IS_TASK (task));

  if (!(files = g_file_enumerator_next_files_finish (enumerator, result, NULL)))
    return;

  self = g_task_get_source_object (task);

  g_assert (DZL_IS_DIRECTORY_MODEL (self));

  for (iter = files; iter; iter = iter->next)
    {
      GFileInfo *file_info = iter->data;

      dzl_directory_model_take_item (self, file_info);
    }

  g_list_free (files);

  g_file_enumerator_next_files_async (enumerator,
                                      NEXT_FILES_CHUNK_SIZE,
                                      G_PRIORITY_LOW,
                                      g_task_get_cancellable (task),
                                      dzl_directory_model_next_files_cb,
                                      g_object_ref (task));
}

static void
dzl_directory_model_enumerate_children_cb (GObject      *object,
                                           GAsyncResult *result,
                                           gpointer      user_data)
{
  GFile *directory = (GFile *)object;
  g_autoptr(GTask) task = user_data;
  g_autoptr(GFileEnumerator) enumerator = NULL;

  g_assert (G_IS_FILE (directory));
  g_assert (G_IS_TASK (task));

  if (!(enumerator = g_file_enumerate_children_finish (directory, result, NULL)))
    return;

  g_file_enumerator_next_files_async (enumerator,
                                      NEXT_FILES_CHUNK_SIZE,
                                      G_PRIORITY_LOW,
                                      g_task_get_cancellable (task),
                                      dzl_directory_model_next_files_cb,
                                      g_object_ref (task));
}

static void
dzl_directory_model_remove_file (DzlDirectoryModel *self,
                                 GFile             *file)
{
  g_autofree gchar *name = NULL;
  GSequenceIter *iter;

  g_assert (G_IS_FILE (file));

  name = g_file_get_basename (file);

  /*
   * We have to lookup linearly since the items will likely be
   * sorted by name, directory, file-system ordering, or some
   * combination thereof.
   */

  for (iter = g_sequence_get_begin_iter (self->items);
       !g_sequence_iter_is_end (iter);
       iter = g_sequence_iter_next (iter))
    {
      GFileInfo *file_info = g_sequence_get (iter);
      const gchar *file_info_name = g_file_info_get_name (file_info);

      if (0 == g_strcmp0 (file_info_name, name))
        {
          guint position;

          position = g_sequence_iter_get_position (iter);
          g_sequence_remove (iter);
          g_list_model_items_changed (G_LIST_MODEL (self), position, 1, 0);
          break;
        }
    }
}

static void
dzl_directory_model_directory_changed (DzlDirectoryModel *self,
                                       GFile             *file,
                                       GFile             *other_file,
                                       GFileMonitorEvent  event_type,
                                       GFileMonitor      *monitor)
{
  g_assert (DZL_IS_DIRECTORY_MODEL (self));

  switch ((int)event_type)
    {
    case G_FILE_MONITOR_EVENT_CREATED:
      /*
       * TODO: incremental changes
       *
       * When adding, we need to first add the GFileInfo for the file with all
       * of the attributes we load in the primary case.
       */
      dzl_directory_model_reload (self);
      break;

    case G_FILE_MONITOR_EVENT_DELETED:
      dzl_directory_model_remove_file (self, file);
      break;

    default:
      break;
    }
}

static void
dzl_directory_model_reload (DzlDirectoryModel *self)
{
  g_assert (DZL_IS_DIRECTORY_MODEL (self));

  if (self->monitor != NULL)
    {
      g_file_monitor_cancel (self->monitor);
      g_signal_handlers_disconnect_by_func (self->monitor,
                                            G_CALLBACK (dzl_directory_model_directory_changed),
                                            self);
      g_clear_object (&self->monitor);
    }

  if (self->cancellable != NULL)
    {
      g_cancellable_cancel (self->cancellable);
      g_clear_object (&self->cancellable);
    }

  dzl_directory_model_remove_all (self);

  if (self->directory != NULL)
    {
      g_autoptr(GTask) task = NULL;

      self->cancellable = g_cancellable_new ();
      task = g_task_new (self, self->cancellable, NULL, NULL);

      g_file_enumerate_children_async (self->directory,
                                       G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME","
                                       G_FILE_ATTRIBUTE_STANDARD_NAME","
                                       G_FILE_ATTRIBUTE_STANDARD_TYPE","
                                       G_FILE_ATTRIBUTE_STANDARD_SYMBOLIC_ICON,
                                       G_FILE_QUERY_INFO_NONE,
                                       G_PRIORITY_LOW,
                                       self->cancellable,
                                       dzl_directory_model_enumerate_children_cb,
                                       g_object_ref (task));

      self->monitor = g_file_monitor_directory (self->directory,
                                                G_FILE_MONITOR_NONE,
                                                self->cancellable,
                                                NULL);

      g_signal_connect_object (self->monitor,
                               "changed",
                               G_CALLBACK (dzl_directory_model_directory_changed),
                               self,
                               G_CONNECT_SWAPPED);
    }
}

static void
dzl_directory_model_finalize (GObject *object)
{
  DzlDirectoryModel *self = (DzlDirectoryModel *)object;

  g_clear_object (&self->cancellable);
  g_clear_object (&self->directory);
  g_clear_pointer (&self->items, g_sequence_free);

  if (self->visible_func_destroy)
    self->visible_func_destroy (self->visible_func_data);

  G_OBJECT_CLASS (dzl_directory_model_parent_class)->finalize (object);
}

static void
dzl_directory_model_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  DzlDirectoryModel *self = DZL_DIRECTORY_MODEL (object);

  switch (prop_id)
    {
    case PROP_DIRECTORY:
      g_value_set_object (value, dzl_directory_model_get_directory (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_directory_model_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  DzlDirectoryModel *self = DZL_DIRECTORY_MODEL (object);

  switch (prop_id)
    {
    case PROP_DIRECTORY:
      dzl_directory_model_set_directory (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_directory_model_class_init (DzlDirectoryModelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_directory_model_finalize;
  object_class->get_property = dzl_directory_model_get_property;
  object_class->set_property = dzl_directory_model_set_property;

  gParamSpecs [PROP_DIRECTORY] =
    g_param_spec_object ("directory",
                         _("Directory"),
                         _("The directory to list files from."),
                         G_TYPE_FILE,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, gParamSpecs);
}

static void
dzl_directory_model_init (DzlDirectoryModel *self)
{
  self->items = g_sequence_new (g_object_unref);
}

/**
 * dzl_directory_model_new:
 * @directory: A #GFile
 *
 * Creates a new #DzlDirectoryModel using @directory as the directory to monitor.
 *
 * Returns: (transfer full): A newly created #DzlDirectoryModel
 */
GListModel *
dzl_directory_model_new (GFile *directory)
{
  g_return_val_if_fail (G_IS_FILE (directory), NULL);

  return g_object_new (DZL_TYPE_DIRECTORY_MODEL,
                       "directory", directory,
                       NULL);
}

/**
 * dzl_directory_model_get_directory:
 * @self: a #DzlDirectoryModel
 *
 * Gets the directory the model is observing.
 *
 * Returns: (transfer none): A #GFile
 */
GFile *
dzl_directory_model_get_directory (DzlDirectoryModel *self)
{
  g_return_val_if_fail (DZL_IS_DIRECTORY_MODEL (self), NULL);

  return self->directory;
}

void
dzl_directory_model_set_directory (DzlDirectoryModel *self,
                                   GFile             *directory)
{
  g_return_if_fail (DZL_IS_DIRECTORY_MODEL (self));
  g_return_if_fail (!directory || G_IS_FILE (directory));

  if (g_set_object (&self->directory, directory))
    {
      dzl_directory_model_reload (self);
      g_object_notify_by_pspec (G_OBJECT (self), gParamSpecs [PROP_DIRECTORY]);
    }
}

static guint
dzl_directory_model_get_n_items (GListModel *model)
{
  DzlDirectoryModel *self = (DzlDirectoryModel *)model;

  g_return_val_if_fail (DZL_IS_DIRECTORY_MODEL (self), 0);

  return g_sequence_get_length (self->items);
}

static GType
dzl_directory_model_get_item_type (GListModel *model)
{
  return G_TYPE_FILE_INFO;
}

static gpointer
dzl_directory_model_get_item (GListModel *model,
                              guint       position)
{
  DzlDirectoryModel *self = (DzlDirectoryModel *)model;
  GSequenceIter *iter;
  gpointer ret;

  g_return_val_if_fail (DZL_IS_DIRECTORY_MODEL (self), NULL);

  if ((iter = g_sequence_get_iter_at_pos (self->items, position)) &&
      (ret = g_sequence_get (iter)))
    return g_object_ref (ret);

  return NULL;
}

static void
list_model_iface_init (GListModelInterface *iface)
{
  iface->get_n_items = dzl_directory_model_get_n_items;
  iface->get_item = dzl_directory_model_get_item;
  iface->get_item_type = dzl_directory_model_get_item_type;
}

void
dzl_directory_model_set_visible_func (DzlDirectoryModel            *self,
                                      DzlDirectoryModelVisibleFunc  visible_func,
                                      gpointer                      user_data,
                                      GDestroyNotify                user_data_free_func)
{
  g_return_if_fail (DZL_IS_DIRECTORY_MODEL (self));

  if (self->visible_func_destroy != NULL)
    self->visible_func_destroy (self->visible_func_data);

  self->visible_func = visible_func;
  self->visible_func_data = user_data;
  self->visible_func_destroy = user_data_free_func;

  dzl_directory_model_reload (self);
}
