#include <dazzle.h>
#include <glib/gstdio.h>

static const gchar *layer1[] = { "a", "b", "c", "d", "e", "f", "g", "h", "i", NULL };
static const gchar *layer2[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", NULL };
static GMainLoop *main_loop;
static GHashTable *created;
static GHashTable *deleted;

static void
sync_for_changes (void)
{
  GMainContext *context = g_main_loop_get_context (main_loop);
  gint64 enter = g_get_monotonic_time ();

  /*
   * we need to spin a little bit while we wait for things
   * to complete.
   */

  for (;;)
    {
      gint64 now;

      if (g_main_context_pending (context))
        g_main_context_iteration (context, FALSE);

      now = g_get_monotonic_time ();

      /* Spin at least a milliseconds */
      if ((now - enter) > (G_USEC_PER_SEC / 1000))
        break;
    }

  g_main_context_iteration (context, FALSE);
}

static void
monitor_changed_cb (DzlRecursiveFileMonitor *monitor,
                    GFile                   *file,
                    GFile                   *other_file,
                    GFileMonitorEvent        event,
                    gpointer                 data)
{
  sync_for_changes ();

  if (event == G_FILE_MONITOR_EVENT_CREATED)
    g_hash_table_insert (created, g_object_ref (file), NULL);
  else if (event == G_FILE_MONITOR_EVENT_DELETED)
    g_hash_table_insert (deleted, g_object_ref (file), NULL);
}

static void
test_basic (void)
{
  g_autoptr(DzlRecursiveFileMonitor) monitor = NULL;
  g_autoptr(GFile) dir = g_file_new_for_path ("recursive-dir");
  g_autoptr(GPtrArray) dirs = NULL;
  gint r;

  main_loop = g_main_loop_new (NULL, FALSE);

  created = g_hash_table_new_full (g_file_hash, (GEqualFunc) g_file_equal, g_object_unref, g_object_unref);
  deleted = g_hash_table_new_full (g_file_hash, (GEqualFunc) g_file_equal, g_object_unref, g_object_unref);

  if (g_file_test ("recursive-dir", G_FILE_TEST_EXISTS))
    {
      g_autoptr(DzlDirectoryReaper) reaper = dzl_directory_reaper_new ();

      dzl_directory_reaper_add_directory (reaper, dir, 0);
      dzl_directory_reaper_execute (reaper, NULL, NULL);

      r = g_rmdir ("recursive-dir");
      g_assert_cmpint (r, ==, 0);
    }

  r = g_mkdir ("recursive-dir", 0750);
  g_assert_cmpint (r, ==, 0);

  monitor = dzl_recursive_file_monitor_new (dir);
  g_assert (monitor != NULL);
  sync_for_changes ();

  g_signal_connect (monitor,
                    "changed",
                    G_CALLBACK (monitor_changed_cb),
                    NULL);

  /* Make a bunch of directories while we monitor. We'll add files to
   * the directories afterwards, and then ensure we got notified. This
   * allows us to ensure that we track changes as we add dirs.
   */
  dirs = g_ptr_array_new_with_free_func (g_object_unref);

  for (guint i = 0; layer1[i]; i++)
    {
      g_autofree gchar *first = g_build_filename ("recursive-dir", layer1[i], NULL);
      g_autoptr(GFile) file1 = g_file_new_for_path (first);

      r = g_mkdir (first, 0750);
      g_assert_cmpint (r, ==, 0);
      sync_for_changes ();

      g_ptr_array_add (dirs, g_object_ref (file1));

      g_assert (g_hash_table_contains (created, file1));

      for (guint j = 0; layer2[j]; j++)
        {
          g_autofree gchar *second = g_build_filename (first, layer2[j], NULL);
          g_autoptr(GFile) file2 = g_file_new_for_path (second);

          r = g_mkdir (second, 0750);
          g_assert_cmpint (r, ==, 0);
          sync_for_changes ();

          g_assert (g_hash_table_contains (created, file2));

          g_ptr_array_add (dirs, g_object_ref (file2));
        }
    }

  for (guint i = dirs->len; i > 0; i--)
    {
      GFile *file = g_ptr_array_index (dirs, i - 1);

      r = g_file_delete (file, NULL, NULL);
      g_assert_cmpint (r, ==, TRUE);
      sync_for_changes ();

      g_assert (g_hash_table_contains (deleted, file));
    }
}

gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/RecursiveFileMonitor/basic", test_basic);
  return g_test_run ();
}
