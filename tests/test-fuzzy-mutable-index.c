#include <dazzle.h>
#include <stdlib.h>

static gint
compare_match (gconstpointer a,
               gconstpointer b)
{
  const DzlFuzzyMutableIndexMatch *ma = a;
  const DzlFuzzyMutableIndexMatch *mb = b;

  if (ma->score < mb->score)
    return 1;
  else if (ma->score > mb->score)
    return -1;
  return 0;
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_autofree gchar *path = g_build_filename (TEST_DATA_DIR, "test-fuzzy-mutable-index.txt", NULL);
  DzlFuzzyMutableIndex *fuzzy;
  GFileInputStream *file_stream;
  GDataInputStream *data_stream;
  gint64 begin;
  gint64 end;
  GFile *file;
  gchar *line;

  if (argc == 1)
    {
      g_printerr ("usage: %s FUZZY_NEEDLE\n"
                  "Search fuzzy index for needle\n", argv[0]);
      return EXIT_FAILURE;
    }

  fuzzy = dzl_fuzzy_mutable_index_new_with_free_func (FALSE, g_free);
  file = g_file_new_for_path (path);
  file_stream = g_file_read (file, NULL, NULL);
  data_stream = g_data_input_stream_new (G_INPUT_STREAM (file_stream));

  dzl_fuzzy_mutable_index_begin_bulk_insert (fuzzy);
  while (NULL != (line = g_data_input_stream_read_line (data_stream, NULL, NULL, NULL)))
    dzl_fuzzy_mutable_index_insert (fuzzy, line, line);
  dzl_fuzzy_mutable_index_end_bulk_insert (fuzzy);

  if (argc > 1)
   {
     guint i;

     for (i = 1; i < argc; i++)
       {
         const gchar *keyword;
         GArray *matches;

         g_printerr ("Searching for \"%s\"\n", argv [i]);

         if (g_utf8_validate (argv[i], -1, NULL))
           keyword = argv[i];
         else
           continue;

         begin = g_get_monotonic_time();
         matches = dzl_fuzzy_mutable_index_match (fuzzy, keyword, 0);
         end = g_get_monotonic_time();

         g_array_sort (matches, compare_match);

         for (guint j = 0; j < matches->len; j++)
           {
             const DzlFuzzyMutableIndexMatch *match = &g_array_index (matches, DzlFuzzyMutableIndexMatch, j);
             g_print ("  %0.4f : %s\n", match->score, match->key);
             g_assert_cmpstr (match->key, ==, match->value);
           }

         g_print ("Completed query in: %ld msec.\n", (glong)((end - begin) / 1000L));

         g_array_unref (matches);
       }
    }

  dzl_fuzzy_mutable_index_unref (fuzzy);
  g_clear_object (&file_stream);
  g_clear_object (&data_stream);
  g_clear_object (&file);

  return EXIT_SUCCESS;
}
