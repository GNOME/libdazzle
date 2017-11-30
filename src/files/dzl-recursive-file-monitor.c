/* dzl-recursive-file-monitor.c
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

#define G_LOG_DOMAIN "dzl-recursive-file-monitor"

#include "files/dzl-recursive-file-monitor.h"
#include "util/dzl-macros.h"

/**
 * SECTION:dzl-recursive-file-monitor
 * @title: DzlRecursiveFileMonitor
 * @short_description: a recursive directory monitor
 *
 * This works by creating a #GFileMonitor for each directory underneath a root
 * directory (and recursively beyond that).
 *
 * This is only designed for use on Linux, where we are using a single inotify
 * FD. You can still hit the max watch limit, but it is much higher than the FD
 * limit.
 *
 * Since: 3.28
 */

struct _DzlRecursiveFileMonitor
{
  GObject                 parent_instance;

  GFile                  *root;
  GCancellable           *cancellable;

  GMutex                  monitor_lock;
  GHashTable             *monitors_by_file;
  GHashTable             *files_by_monitor;

  DzlRecursiveIgnoreFunc  ignore_func;
  gpointer                ignore_func_data;
  GDestroyNotify          ignore_func_data_destroy;

  guint                   start_handler;
};

enum {
  PROP_0,
  PROP_ROOT,
  N_PROPS
};

enum {
  CHANGED,
  N_SIGNALS
};

static gboolean
dzl_recursive_file_monitor_watch (DzlRecursiveFileMonitor  *self,
                                  GFile                    *directory,
                                  GCancellable             *cancellable,
                                  GError                  **error);

G_DEFINE_TYPE (DzlRecursiveFileMonitor, dzl_recursive_file_monitor, G_TYPE_OBJECT)

static GParamSpec *properties [N_PROPS];
static guint signals [N_SIGNALS];

/*
 * You must hold the lock when calling this function.
 */
static gboolean
dzl_recursive_file_monitor_ignored (DzlRecursiveFileMonitor *self,
                                    GFile                   *file)
{
  g_assert (DZL_IS_RECURSIVE_FILE_MONITOR (self));
  g_assert (G_IS_FILE (file));

  return self->ignore_func (file, self->ignore_func_data);
}

/*
 * You must hold the lock when calling this function.
 */
static void
dzl_recursive_file_monitor_unwatch (DzlRecursiveFileMonitor *self,
                                    GFile                   *file)
{
  GFileMonitor *monitor;

  g_assert (DZL_IS_RECURSIVE_FILE_MONITOR (self));
  g_assert (G_IS_FILE (file));

  monitor = g_hash_table_lookup (self->monitors_by_file, file);

  if (monitor != NULL)
    {
      g_object_ref (monitor);
      g_file_monitor_cancel (monitor);
      g_hash_table_remove (self->monitors_by_file, file);
      g_hash_table_remove (self->files_by_monitor, monitor);
      g_object_unref (monitor);
    }
}

static void
dzl_recursive_file_monitor_changed (DzlRecursiveFileMonitor *self,
                                    GFile                   *file,
                                    GFile                   *other_file,
                                    GFileMonitorEvent        event,
                                    GFileMonitor            *monitor)
{
  g_assert (DZL_IS_RECURSIVE_FILE_MONITOR (self));
  g_assert (G_IS_FILE (file));
  g_assert (!other_file || G_IS_FILE (file));
  g_assert (G_IS_FILE_MONITOR (monitor));

  if (g_cancellable_is_cancelled (self->cancellable))
    return;

  g_mutex_lock (&self->monitor_lock);

  if (dzl_recursive_file_monitor_ignored (self, file))
    goto unlock;

  if (event == G_FILE_MONITOR_EVENT_DELETED)
    {
      if (g_hash_table_contains (self->monitors_by_file, file))
        dzl_recursive_file_monitor_unwatch (self, file);
    }
  else if (event == G_FILE_MONITOR_EVENT_CREATED)
    {
      if (g_file_query_file_type (file, 0, NULL) == G_FILE_TYPE_DIRECTORY)
        dzl_recursive_file_monitor_watch (self, file, self->cancellable, NULL);
    }

  g_signal_emit (self, signals [CHANGED], 0, file, other_file, event);

unlock:
  g_mutex_unlock (&self->monitor_lock);
}

/*
 * You must hold the lock when calling this function.
 */
static gboolean
dzl_recursive_file_monitor_watch (DzlRecursiveFileMonitor  *self,
                                  GFile                    *directory,
                                  GCancellable             *cancellable,
                                  GError                  **error)
{
  g_autoptr(GFileMonitor) monitor = NULL;

  g_assert (DZL_IS_RECURSIVE_FILE_MONITOR (self));
  g_assert (G_IS_FILE (directory));
  g_assert (G_IS_CANCELLABLE (cancellable));

  monitor = g_file_monitor_directory (directory, 0, cancellable, error);

  if (monitor == NULL)
    return FALSE;

  g_signal_connect_object (monitor,
                           "changed",
                           G_CALLBACK (dzl_recursive_file_monitor_changed),
                           self,
                           G_CONNECT_SWAPPED);

  g_hash_table_insert (self->monitors_by_file,
                       g_object_ref (directory),
                       g_object_ref (monitor));

  g_hash_table_insert (self->files_by_monitor,
                       g_object_ref (monitor),
                       g_object_ref (directory));

  return TRUE;
}

static void
dzl_recursive_file_monitor_worker (GTask        *task,
                                   gpointer      source_object,
                                   gpointer      task_data,
                                   GCancellable *cancellable)
{
  DzlRecursiveFileMonitor *self = source_object;
  g_autoptr(GFileEnumerator) enumerator = NULL;
  g_autoptr(GError) error = NULL;
  gpointer infoptr;
  GFile *root = task_data;

  g_assert (DZL_IS_RECURSIVE_FILE_MONITOR (self));
  g_assert (G_IS_FILE (root));

  /* Short circuit if we were cancelled */
  if (g_cancellable_is_cancelled (cancellable))
    return;

  /* Make sure our root is a directory that exists */
  if (g_file_query_file_type (root, 0, cancellable) != G_FILE_TYPE_DIRECTORY)
    return;

  g_mutex_lock (&self->monitor_lock);

  if (!dzl_recursive_file_monitor_watch (self, root, cancellable, &error))
    goto cleanup;

  enumerator = g_file_enumerate_children (root,
                                          G_FILE_ATTRIBUTE_STANDARD_NAME","
                                          G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                          G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                          cancellable, &error);

  while (NULL != (infoptr = g_file_enumerator_next_file (enumerator, cancellable, &error)))
    {
      g_autoptr(GFileInfo) info = infoptr;
      g_autoptr(GFile) child = NULL;

      if (g_file_info_get_file_type (info) != G_FILE_TYPE_DIRECTORY)
        continue;

      child = g_file_get_child (root, g_file_info_get_name (info));

      if (dzl_recursive_file_monitor_ignored (self, child))
        continue;

      if (!dzl_recursive_file_monitor_watch (self, child, cancellable, &error))
        break;
    }

cleanup:
  g_file_enumerator_close (enumerator, cancellable, NULL);

  g_mutex_unlock (&self->monitor_lock);

  if (error != NULL)
    g_task_return_error (task, g_steal_pointer (&error));
  else
    g_task_return_boolean (task, TRUE);
}

static gboolean
dzl_recursive_file_monitor_start (gpointer data)
{
  DzlRecursiveFileMonitor *self = data;
  g_autoptr(GTask) task = NULL;

  g_assert (DZL_IS_RECURSIVE_FILE_MONITOR (self));
  g_assert (G_IS_FILE (self->root));

  self->start_handler = 0;

  task = g_task_new (self, self->cancellable, NULL, NULL);
  g_task_set_source_tag (task, dzl_recursive_file_monitor_start);
  g_task_set_priority (task, G_PRIORITY_LOW + 100);
  g_task_set_task_data (task, g_object_ref (self->root), g_object_unref);
  g_task_set_return_on_cancel (task, TRUE);
  g_task_run_in_thread (task, dzl_recursive_file_monitor_worker);

  return G_SOURCE_REMOVE;
}

static void
dzl_recursive_file_monitor_constructed (GObject *object)
{
  DzlRecursiveFileMonitor *self = (DzlRecursiveFileMonitor *)object;

  G_OBJECT_CLASS (dzl_recursive_file_monitor_parent_class)->constructed (object);

  if (self->root == NULL)
    {
      g_warning ("%s created without a root directory", G_OBJECT_TYPE_NAME (self));
      return;
    }

  /*
   * Defer start to the main loop so that the caller can set
   * things like the ignore func, but not require us to have
   * soem annoying start/stop API.
   */

  self->start_handler = g_idle_add (dzl_recursive_file_monitor_start, self);
}

static gboolean
default_ignore_func (GFile    *file,
                     gpointer  user_data)
{
  return FALSE;
}

static void
dzl_recursive_file_monitor_dispose (GObject *object)
{
  DzlRecursiveFileMonitor *self = (DzlRecursiveFileMonitor *)object;

  dzl_clear_source (&self->start_handler);
  g_cancellable_cancel (self->cancellable);
  dzl_recursive_file_monitor_set_ignore_func (self, NULL, NULL, NULL);

  G_OBJECT_CLASS (dzl_recursive_file_monitor_parent_class)->dispose (object);
}

static void
dzl_recursive_file_monitor_finalize (GObject *object)
{
  DzlRecursiveFileMonitor *self = (DzlRecursiveFileMonitor *)object;

  g_clear_object (&self->root);
  g_clear_object (&self->cancellable);

  g_clear_pointer (&self->files_by_monitor, g_hash_table_unref);
  g_clear_pointer (&self->monitors_by_file, g_hash_table_unref);

  G_OBJECT_CLASS (dzl_recursive_file_monitor_parent_class)->finalize (object);
}

static void
dzl_recursive_file_monitor_get_property (GObject    *object,
                                         guint       prop_id,
                                         GValue     *value,
                                         GParamSpec *pspec)
{
  DzlRecursiveFileMonitor *self = DZL_RECURSIVE_FILE_MONITOR (object);

  switch (prop_id)
    {
    case PROP_ROOT:
      g_value_set_object (value, dzl_recursive_file_monitor_get_root (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_recursive_file_monitor_set_property (GObject      *object,
                                         guint         prop_id,
                                         const GValue *value,
                                         GParamSpec   *pspec)
{
  DzlRecursiveFileMonitor *self = DZL_RECURSIVE_FILE_MONITOR (object);

  switch (prop_id)
    {
    case PROP_ROOT:
      self->root = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_recursive_file_monitor_class_init (DzlRecursiveFileMonitorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = dzl_recursive_file_monitor_constructed;
  object_class->dispose = dzl_recursive_file_monitor_dispose;
  object_class->finalize = dzl_recursive_file_monitor_finalize;
  object_class->get_property = dzl_recursive_file_monitor_get_property;
  object_class->set_property = dzl_recursive_file_monitor_set_property;

  properties [PROP_ROOT] =
    g_param_spec_object ("root",
                         "Root",
                         "The root directory to monitor",
                         G_TYPE_FILE,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  
  g_object_class_install_properties (object_class, N_PROPS, properties);

  /**
   * DzlRecursiveFileMonitor::changed:
   * @self: a #DzlRecursiveFileMonitor
   * @file: a #GFile
   * @other_file: (nullable): a #GFile for the other file when applicable
   * @event: the #GFileMonitorEvent event
   *
   * This event is similar to #GFileMonitor::changed but can be fired from
   * any of the monitored directories in the recursive mount.
   *
   * Since: 3.28
   */
  signals [CHANGED] =
    g_signal_new ("changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL,
                  G_TYPE_NONE, 3, G_TYPE_FILE, G_TYPE_FILE, G_TYPE_FILE_MONITOR_EVENT);
}

static void
dzl_recursive_file_monitor_init (DzlRecursiveFileMonitor *self)
{
  g_mutex_init (&self->monitor_lock);
  self->cancellable = g_cancellable_new ();
  self->files_by_monitor = g_hash_table_new_full (NULL, NULL, g_object_unref, g_object_unref);
  self->monitors_by_file = g_hash_table_new_full (g_file_hash,
                                                  (GEqualFunc) g_file_equal,
                                                  g_object_unref,
                                                  g_object_unref);
  self->ignore_func = default_ignore_func;
}

DzlRecursiveFileMonitor *
dzl_recursive_file_monitor_new (GFile *file)
{
  g_return_val_if_fail (G_IS_FILE (file), NULL);

  return g_object_new (DZL_TYPE_RECURSIVE_FILE_MONITOR,
                       "root", file,
                       NULL);
}

/**
 * dzl_recursive_file_monitor_cancel:
 * @self: a #DzlRecursiveFileMonitor
 *
 * Cancels the recursive file monitor.
 *
 * Since: 3.28
 */
void
dzl_recursive_file_monitor_cancel (DzlRecursiveFileMonitor *self)
{
  g_return_if_fail (DZL_IS_RECURSIVE_FILE_MONITOR (self));

  g_object_run_dispose (G_OBJECT (self));
}

/**
 * dzl_recursive_file_monitor_get_root:
 * @self: a #DzlRecursiveFileMonitor
 *
 * Gets the root directory used forthe file monitor.
 *
 * Returns: (transfer none): a #GFile
 *
 * Since: 3.28
 */
GFile *
dzl_recursive_file_monitor_get_root (DzlRecursiveFileMonitor *self)
{
  g_return_val_if_fail (DZL_IS_RECURSIVE_FILE_MONITOR (self), NULL);

  return self->root;
}

/**
 * dzl_recursive_file_monitor_set_ignore_func:
 * @self: a #DzlRecursiveFileMonitor
 * @ignore_func: (scope async): a thread-safe #DzlRecursiveIgnoreFunc
 * @ignore_func_data: closure data for @ignore_func
 * @ignore_func_data_destroy: destroy notify for @ignore_func_data
 *
 * Sets a callback function to determine if a #GFile should be ignored
 * from signal emission.
 *
 * @ignore_func may be called from a thread other than the default
 * main thread, so any function used here MUST be thread-safe.
 *
 * Any use of a non-thread-safe callback for @ignore_func is a programmer
 * error.
 *
 * If @ignore_func is %NULL, it is set to the default which does not
 * ignore any files or directories.
 *
 * Since: 3.28
 */
void
dzl_recursive_file_monitor_set_ignore_func (DzlRecursiveFileMonitor *self,
                                            DzlRecursiveIgnoreFunc   ignore_func,
                                            gpointer                 ignore_func_data,
                                            GDestroyNotify           ignore_func_data_destroy)
{
  g_return_if_fail (DZL_IS_RECURSIVE_FILE_MONITOR (self));

  if (ignore_func == NULL)
    {
      ignore_func = default_ignore_func;
      ignore_func_data = NULL;
      ignore_func_data_destroy = NULL;
    }

  g_mutex_lock (&self->monitor_lock);

  if (self->ignore_func_data && self->ignore_func_data_destroy)
    {
      gpointer data = self->ignore_func_data;
      GDestroyNotify notify = self->ignore_func_data_destroy;

      self->ignore_func = NULL;
      self->ignore_func_data = NULL;
      self->ignore_func_data_destroy = NULL;

      g_mutex_unlock (&self->monitor_lock);

      notify (data);

      g_mutex_lock (&self->monitor_lock);
    }

  self->ignore_func = ignore_func;
  self->ignore_func_data = ignore_func_data;
  self->ignore_func_data_destroy = ignore_func_data_destroy;

  g_mutex_unlock (&self->monitor_lock);
}
