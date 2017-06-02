#include <dazzle.h>
#include <stdlib.h>

static gchar **
tokenize (const gchar *input)
{
  return g_strsplit_set (input, ",;: -_./\t\n", 0);
}

static void
test_desktop_index_key (DzlFuzzyIndexBuilder *builder,
                        GKeyFile             *key_file,
                        const gchar          *group,
                        const gchar          *key,
                        GVariant             *document)
{
  g_autofree gchar *str = NULL;
  g_auto(GStrv) words = NULL;

  g_assert (DZL_IS_FUZZY_INDEX_BUILDER (builder));
  g_assert (group);
  g_assert (key);
  g_assert (document);

  if (NULL == (str = g_key_file_get_string (key_file, group, key, NULL)))
    return;

  words = tokenize (str);

  for (guint i = 0; words[i] != NULL; i++)
  {
    if (*words[i])
      dzl_fuzzy_index_builder_insert (builder, words[i], document);
  }
}

static gboolean
test_desktop_index_file (DzlFuzzyIndexBuilder  *builder,
                         const gchar           *path,
                         GError               **error)
{
  g_autoptr(GKeyFile) key_file = NULL;
  g_autoptr(GVariant) document = NULL;

  g_assert (DZL_IS_FUZZY_INDEX_BUILDER (builder));
  g_assert (path != NULL);

  key_file = g_key_file_new ();

  if (!g_key_file_load_from_file (key_file, path, 0, error))
    return FALSE;

  if (!g_key_file_has_group (key_file, "Desktop Entry"))
    {
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_NOT_SUPPORTED,
                   "%s is not a desktop file",
                   path);
      return FALSE;
    }

  document = g_variant_take_ref (g_variant_new_string (path));

  test_desktop_index_key (builder, key_file, "Desktop Entry", "Name", document);
  test_desktop_index_key (builder, key_file, "Desktop Entry", "Comment", document);
  test_desktop_index_key (builder, key_file, "Desktop Entry", "Categories", document);
  test_desktop_index_key (builder, key_file, "Desktop Entry", "Keywords", document);

  return TRUE;
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_autoptr(GOptionContext) context = NULL;
  g_autoptr(DzlFuzzyIndexBuilder) builder = NULL;
  g_autoptr(GFile) outfile = NULL;
  g_autoptr(GError) error = NULL;

  context = g_option_context_new ("[DIRECTORIES...] - Index desktop info directories");

  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_printerr ("%s\n", error->message);
      return EXIT_FAILURE;
    }

  builder = dzl_fuzzy_index_builder_new ();

  dzl_fuzzy_index_builder_set_case_sensitive (builder, TRUE);

  for (guint i = 1; i < argc; i++)
    {
      const gchar *name;
      const gchar *directory = argv[i];
      g_autoptr(GDir) dir = g_dir_open (directory, 0, &error);

      if (dir == NULL)
        {
          g_printerr ("%s\n", error->message);
          g_clear_error (&error);
          continue;
        }

      while (NULL != (name = g_dir_read_name (dir)))
        {
          g_autofree gchar *path = NULL;

          if (!g_str_has_suffix (name, ".desktop"))
            continue;

          path = g_build_filename (directory, name, NULL);

          if (!test_desktop_index_file (builder, path, &error))
            {
              g_printerr ("%s\n", error->message);
              g_clear_error (&error);
              continue;
            }
        }
    }

  outfile = g_file_new_for_path ("desktop.index");

  if (!dzl_fuzzy_index_builder_write (builder, outfile, G_PRIORITY_DEFAULT, NULL, &error))
    g_printerr ("%s\n", error->message);

  g_print ("desktop.index.written\n");

  return EXIT_SUCCESS;
}
