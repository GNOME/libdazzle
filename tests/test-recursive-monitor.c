#include <dazzle.h>
#include <glib/gstdio.h>

static const gchar *layer1[] = { "a", "b", "c", "d", "e", "f", "g", "h", "i", NULL };
static const gchar *layer2[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", NULL };

enum {
  MODE_CREATE,
  MODE_DELETE,
};

typedef struct
{
  GMainLoop               *main_loop;
  DzlRecursiveFileMonitor *monitor;
  GHashTable              *created;
  GHashTable              *deleted;
  GQueue                   dirs;
  GList                   *iter;
  guint                    mode : 1;
  guint                    did_action : 1;
} BasicState;

static gboolean
failed_timeout (gpointer state)
{
  /* timed out */
  g_assert_not_reached ();
  return G_SOURCE_REMOVE;
}

G_GNUC_NULL_TERMINATED
static GFile *
file_new_build_filename (const gchar *first, ...)
{
  g_autoptr(GPtrArray) parts = g_ptr_array_new ();
  g_autofree gchar *path = NULL;
  va_list args;

  va_start (args, first);
  g_ptr_array_add (parts, (gchar *)first);
  while ((first = va_arg (args, const gchar *)))
    g_ptr_array_add (parts, (gchar *)first);
  g_ptr_array_add (parts, NULL);

  path = g_build_filenamev ((gchar **)(gpointer)parts->pdata);

  return g_file_new_for_path (path);
}

static gboolean
begin_test_basic (gpointer data)
{
  g_autoptr(GError) error = NULL;
  BasicState *state = data;
  GFile *current;
  gboolean r;

  g_assert (state != NULL);
  g_assert (state->iter != NULL);
  g_assert (state->iter->data != NULL);
  g_assert (G_IS_FILE (state->iter->data));

  current = state->iter->data;

  if (state->mode == MODE_CREATE)
    {
      if (g_hash_table_contains (state->created, current))
        {
          state->iter = state->iter->next;
          state->did_action = FALSE;

          if (state->iter == NULL)
            {
              state->mode = MODE_DELETE;
              state->iter = state->dirs.tail;
            }
        }
      else if (!state->did_action)
        {
          state->did_action = TRUE;
          r = g_file_make_directory (current, NULL, &error);
          g_assert_no_error (error);
          g_assert_cmpint (r, ==, TRUE);
        }
    }
  else if (state->mode == MODE_DELETE)
    {
      if (g_hash_table_contains (state->deleted, current))
        {
          state->iter = state->iter->prev;
          state->did_action = FALSE;

          if (state->iter == NULL)
            {
              g_main_loop_quit (state->main_loop);
              return G_SOURCE_REMOVE;
            }
        }
      else if (!state->did_action)
        {
          state->did_action = TRUE;
          g_file_delete (current, NULL, &error);
          g_assert_no_error (error);
        }
    }
  else
    g_assert_not_reached ();

  return G_SOURCE_CONTINUE;
}

static void
monitor_changed_cb (DzlRecursiveFileMonitor *monitor,
                    GFile                   *file,
                    GFile                   *other_file,
                    GFileMonitorEvent        event,
                    gpointer                 data)
{
  BasicState *state = data;

  if (event == G_FILE_MONITOR_EVENT_CREATED)
    g_hash_table_insert (state->created, g_object_ref (file), NULL);
  else if (event == G_FILE_MONITOR_EVENT_DELETED)
    g_hash_table_insert (state->deleted, g_object_ref (file), NULL);
}

static void
started_cb (GObject      *object,
            GAsyncResult *result,
            gpointer      user_data)
{
  DzlRecursiveFileMonitor *monitor = (DzlRecursiveFileMonitor *)object;
  BasicState *state = user_data;
  g_autoptr(GError) error = NULL;
  gboolean r;

  g_assert (DZL_IS_RECURSIVE_FILE_MONITOR (monitor));

  r = dzl_recursive_file_monitor_start_finish (monitor, result, &error);
  g_assert_no_error (error);
  g_assert_cmpint (r, ==, TRUE);

  /*
   * Now start the async test processing. We use a very
   * low priority idle to ensure other things process before us.
   */

  state->iter = state->dirs.head;
  state->did_action = FALSE;
  state->mode = MODE_CREATE;
  g_idle_add_full (G_MAXINT, begin_test_basic, state, NULL);
}

static void
test_basic (void)
{
  g_autoptr(GFile) dir = g_file_new_for_path ("recursive-dir");
  BasicState state = { 0 };
  gint r;

  state.main_loop = g_main_loop_new (NULL, FALSE);
  g_queue_init (&state.dirs);
  state.created = g_hash_table_new_full (g_file_hash,
                                         (GEqualFunc) g_file_equal,
                                         g_object_unref,
                                         NULL);
  state.deleted = g_hash_table_new_full (g_file_hash,
                                         (GEqualFunc) g_file_equal,
                                         g_object_unref,
                                         NULL);

  /* Cleanup any previously failed run */
  if (g_file_test ("recursive-dir", G_FILE_TEST_EXISTS))
    {
      g_autoptr(DzlDirectoryReaper) reaper = dzl_directory_reaper_new ();

      dzl_directory_reaper_add_directory (reaper, dir, 0);
      dzl_directory_reaper_execute (reaper, NULL, NULL);

      r = g_rmdir ("recursive-dir");
      g_assert_cmpint (r, ==, 0);
    }

  /* Create our root directory to use */
  r = g_mkdir ("recursive-dir", 0750);
  g_assert_cmpint (r, ==, 0);

  /* Build our list of directories to create/test */
  for (guint i = 0; layer1[i]; i++)
    {
      g_autoptr(GFile) file1 = file_new_build_filename ("recursive-dir", layer1[i], NULL);

      g_queue_push_tail (&state.dirs, g_object_ref (file1));

      for (guint j = 0; layer2[j]; j++)
        {
          g_autoptr(GFile) file2 = g_file_get_child (file1, layer2[j]);
          g_queue_push_tail (&state.dirs, g_steal_pointer (&file2));
        }
    }

  state.monitor = dzl_recursive_file_monitor_new (dir);
  g_signal_connect (state.monitor, "changed", G_CALLBACK (monitor_changed_cb), &state);

  /* Add a timeout to avoid infinite running */
  g_timeout_add_seconds (3, failed_timeout, &state);

  dzl_recursive_file_monitor_start_async (state.monitor, NULL, started_cb, &state);

  g_main_loop_run (state.main_loop);

  dzl_recursive_file_monitor_cancel (state.monitor);

  g_clear_pointer (&state.main_loop, g_main_loop_unref);
  g_clear_pointer (&state.created, g_hash_table_unref);
  g_clear_pointer (&state.deleted, g_hash_table_unref);
  g_queue_foreach (&state.dirs, (GFunc)g_object_unref, NULL);
  g_queue_clear (&state.dirs);
  g_clear_object (&state.monitor);
}

gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/RecursiveFileMonitor/basic", test_basic);
  return g_test_run ();
}
