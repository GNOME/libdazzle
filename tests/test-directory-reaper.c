/* test-directory-reaper.c
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

#include <dazzle.h>
#include <glib/gstdio.h>
#include <errno.h>

typedef struct
{
  /* the test data path */
  const gchar *path;

  /* null for a directory, otherwise file contents */
  const gchar *data;
} FileInfo;

static void
test_reaper_basic (void)
{
  g_autoptr(DzlDirectoryReaper) reaper = dzl_directory_reaper_new ();
  g_autoptr(GFile) file = g_file_new_for_path ("reaper");
  g_autoptr(GError) error = NULL;
  gboolean r;
  static const FileInfo files[] = {
    /* directories first */
    { "a/b/c" },
    { "a/c/b" },
    { "a/d/e" },

    /* then files */
    { "a/b/c/f", "" },
    { "a/c/b/g", "" },
    { "a/d/e/h", "" },
  };

  /* make a special out-of-tree file so that we know we never
   * followed a symlink to delete out of tree.
   */
  g_assert_cmpint (0, ==, g_mkdir_with_parents ("out-of-tree", 0750));
  g_assert_true (g_file_set_contents ("out-of-tree/delete-check", "", 0, NULL));

  /*
   * Start out by creating some directories and files so that we can
   * test that they've been reaped correctly.
   */
  for (guint i = 0; i < G_N_ELEMENTS (files); i++)
    {
      const FileInfo *info = &files[i];
      g_autofree gchar *path = g_build_filename ("reaper", info->path, NULL);

      if (info->data == NULL)
        {
          r = g_mkdir_with_parents (path, 0750);
          g_assert_cmpint (r, ==, 0);
          continue;
        }

      r = g_file_set_contents (path, info->data, -1, &error);
      g_assert_no_error (error);
      g_assert_cmpint (r, ==, TRUE);
    }

#ifdef G_OS_UNIX
  /* Add a symlink to ../ so that we keep ourselves honest ;) */
  {
    g_autofree gchar *cwd = g_get_current_dir ();
    g_autofree gchar *name = g_build_filename ("reaper", "parent-link", NULL);

    if (symlink (cwd, name) != 0)
      g_error ("Failed to create symlink");
  }

  /* also create link to out-of-tree to ensure we don't follow
   * symlinks and delete files there.
   */
  g_assert_cmpint (0, ==, symlink ("../out-of-tree", "reaper/symlink-check"));
  g_assert_true (g_file_test ("reaper/symlink-check", G_FILE_TEST_IS_SYMLINK));
  g_assert_true (g_file_test ("reaper/symlink-check", G_FILE_TEST_EXISTS));
  g_assert_true (g_file_test ("out-of-tree/delete-check", G_FILE_TEST_IS_REGULAR));

  g_assert_cmpint (0, ==, symlink ("../../../out-of-tree", "reaper/a/b/d"));
  g_assert_true (g_file_test ("reaper/a/b/d", G_FILE_TEST_IS_SYMLINK));
  g_assert_true (g_file_test ("reaper/a/b/d", G_FILE_TEST_EXISTS));
#endif

  dzl_directory_reaper_add_directory (reaper, file, 0);

  r = dzl_directory_reaper_execute (reaper, NULL, &error);
  g_assert_no_error (error);
  g_assert_cmpint (r, ==, TRUE);

  g_assert_true (g_file_test ("out-of-tree", G_FILE_TEST_IS_DIR));
  g_assert_true (g_file_test ("out-of-tree/delete-check", G_FILE_TEST_IS_REGULAR));

  /* make sure reaper dir is empty */
  if (g_rmdir ("reaper") != 0)
    g_error ("Failed to remove 'reaper': %s", g_strerror (errno));

  /* now remove our delete check dir */
  g_assert_cmpint (0, ==, g_unlink ("out-of-tree/delete-check"));
  g_assert_cmpint (0, ==, g_rmdir ("out-of-tree"));
}

gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/DirectoryReaper/basic", test_reaper_basic);
  return g_test_run ();
}
