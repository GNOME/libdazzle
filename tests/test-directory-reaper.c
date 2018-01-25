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

  /* Add a symlink to ../ so that we keep ourselves honest ;) */
  {
    g_autofree gchar *cwd = g_get_current_dir ();
    g_autofree gchar *name = g_build_filename ("reaper", "parent-link", NULL);

    if (symlink (cwd, name) != 0)
      g_error ("Failed to create symlink");
  }

  dzl_directory_reaper_add_directory (reaper, file, 0);

  r = dzl_directory_reaper_execute (reaper, NULL, &error);
  g_assert_no_error (error);
  g_assert_cmpint (r, ==, TRUE);

  if (g_rmdir ("reaper") != 0)
    g_error ("Failed to remove 'reaper': %s", g_strerror (errno));
}

gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/DirectoryReaper/basic", test_reaper_basic);
  return g_test_run ();
}
