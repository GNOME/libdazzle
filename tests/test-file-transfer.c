#include <dazzle.h>
#include <glib/gstdio.h>

static void
write_file (const gchar *path)
{
  g_autoptr(GFile) file = g_file_new_for_path (path);
  g_autoptr(GOutputStream) stream = NULL;
  g_autoptr(GError) error = NULL;
  gsize len = 0;

  stream = G_OUTPUT_STREAM (g_file_create (file, G_FILE_CREATE_NONE, NULL, &error));
  g_assert_no_error (error);
  g_assert (G_IS_OUTPUT_STREAM (stream));
  g_output_stream_write_all (stream, "some-data", strlen ("some-data"), &len, NULL, &error);
  g_assert_no_error (error);
  g_assert_cmpint (len, ==, strlen ("some-data"));
}

static void
test_basic (void)
{
  g_autoptr(DzlFileTransfer) xfer = dzl_file_transfer_new ();
  g_autoptr(GFile) root = g_file_new_for_path ("test-file-transfer-data");
  g_autoptr(GFile) copy = g_file_new_for_path ("test-file-transfer-copy");
  g_autoptr(GError) error = NULL;
  g_autoptr(DzlDirectoryReaper) reaper = dzl_directory_reaper_new ();
  gboolean r;

  dzl_directory_reaper_add_directory (reaper, root, 0);
  dzl_directory_reaper_add_directory (reaper, copy, 0);
  dzl_directory_reaper_add_file (reaper, root, 0);
  dzl_directory_reaper_add_file (reaper, copy, 0);
  dzl_directory_reaper_execute (reaper, NULL, NULL);
  g_assert (!g_file_query_exists (root, NULL));
  g_assert (!g_file_query_exists (copy, NULL));

  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/a", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/a/1", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/a/1/a", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/a/1/b", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/a/1/c", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/a/2", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/a/2/a", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/a/2/b", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/a/2/c", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/b", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/b/1", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/b/1/a", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/b/1/b", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/b/1/c", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/c", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/c/1", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/c/1/a", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/c/1/b", 0750));
  g_assert_cmpint (0, ==, g_mkdir ("test-file-transfer-data/c/1/c", 0750));

  write_file ("test-file-transfer-data/z");
  write_file ("test-file-transfer-data/a/z");
  write_file ("test-file-transfer-data/b/1/c/z");
  write_file ("test-file-transfer-data/c/1/c/z");

  dzl_file_transfer_set_flags (xfer, DZL_FILE_TRANSFER_FLAGS_MOVE);
  dzl_file_transfer_add (xfer, root, copy);
  r = dzl_file_transfer_execute (xfer, G_PRIORITY_DEFAULT, NULL, &error);
  g_assert_no_error (error);
  g_assert_cmpint (r, ==, TRUE);

  dzl_directory_reaper_execute (reaper, NULL, NULL);
  g_assert (!g_file_query_exists (root, NULL));
  g_assert (!g_file_query_exists (copy, NULL));
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/FileTransfer/basic", test_basic);
  return g_test_run ();
}
