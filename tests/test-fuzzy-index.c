/* test-writer.c
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
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

static GMainLoop *main_loop;

static void
test_index_builder_basic_cb (GObject      *object,
                             GAsyncResult *result,
                             gpointer      user_data)
{
  DzlFuzzyIndexBuilder *builder = (DzlFuzzyIndexBuilder *)object;
  GError **error = user_data;
  gboolean r;

  r = dzl_fuzzy_index_builder_write_finish (builder, result, error);
  g_assert (r == TRUE);
  g_main_loop_quit (main_loop);
}

static void
test_index_builder_basic (void)
{
  DzlFuzzyIndexBuilder *builder;
  gchar *contents = NULL;
  g_autoptr(GVariant) variant = NULL;
  GVariantDict dict;
  GVariant *v;
  GError *error = NULL;
  GFile *file;
  gboolean r;
  gsize len;

  main_loop = g_main_loop_new (NULL, FALSE);

  file = g_file_new_for_path ("index.gvariant");

  builder = dzl_fuzzy_index_builder_new ();

  dzl_fuzzy_index_builder_insert (builder, "foo", g_variant_new_int32 (1), 7);
  dzl_fuzzy_index_builder_insert (builder, "FOO", g_variant_new_int32 (2), 7);
  dzl_fuzzy_index_builder_insert (builder, "Foo", g_variant_new_int32 (3), 7);
  dzl_fuzzy_index_builder_insert (builder, "bar", g_variant_new_int32 (4), 7);
  dzl_fuzzy_index_builder_insert (builder, "baz", g_variant_new_int32 (5), 7);

  dzl_fuzzy_index_builder_write_async (builder,
                                       file,
                                       G_PRIORITY_LOW,
                                       NULL,
                                       test_index_builder_basic_cb,
                                       &error);

  g_main_loop_run (main_loop);
  g_assert_no_error (error);

  r = g_file_load_contents (file, NULL, &contents, &len, NULL, &error);
  g_assert_no_error (error);
  g_assert (r);

  variant = g_variant_new_from_data (G_VARIANT_TYPE_VARDICT,
                                     contents,
                                     len,
                                     FALSE,
                                     g_free,
                                     contents);
  g_assert (variant != NULL);
  g_variant_ref_sink (variant);

#if 0
  g_print ("%s\n", g_variant_print (variant, TRUE));
#endif

  g_variant_dict_init (&dict, variant);

  v = g_variant_dict_lookup_value (&dict, "version", G_VARIANT_TYPE_INT32);
  g_assert (v != NULL);
  g_variant_unref (v);

  v = g_variant_dict_lookup_value (&dict, "tables", G_VARIANT_TYPE_VARDICT);
  g_assert (v != NULL);
  g_variant_unref (v);

  v = g_variant_dict_lookup_value (&dict, "keys", G_VARIANT_TYPE_ARRAY);
  g_assert (v != NULL);
  g_variant_unref (v);

  v = g_variant_dict_lookup_value (&dict, "lookaside", G_VARIANT_TYPE_ARRAY);
  g_assert (v != NULL);
  g_variant_unref (v);

  v = g_variant_dict_lookup_value (&dict, "documents", G_VARIANT_TYPE_ARRAY);
  g_assert (v != NULL);
  g_variant_unref (v);

  g_variant_dict_clear (&dict);

  g_object_unref (builder);

  r = g_file_delete (file, NULL, &error);
  g_assert_no_error (error);
  g_assert (r);

  g_object_unref (file);

  while (g_main_context_pending (NULL))
    g_main_context_iteration (NULL, TRUE);

  g_main_loop_unref (main_loop);
}

static void
test_index_basic_query_cb (GObject      *object,
                           GAsyncResult *result,
                           gpointer      user_data)
{
  DzlFuzzyIndex *index = (DzlFuzzyIndex *)object;
  g_autoptr(GFile) file = user_data;
  g_autoptr(GListModel) matches = NULL;
  GError *error = NULL;
  guint n_items;
  guint i;

  matches = dzl_fuzzy_index_query_finish (index, result, &error);
  g_assert_no_error (error);
  g_assert (matches != NULL);

  n_items = g_list_model_get_n_items (matches);
  g_assert_cmpint (n_items, ==, 5);
  g_assert (DZL_TYPE_FUZZY_INDEX_MATCH == g_list_model_get_item_type (matches));

  for (i = 0; i < n_items; i++)
    {
      g_autoptr(DzlFuzzyIndexMatch) match = g_list_model_get_item (matches, i);
      const gchar *key;
      GVariant *doc;
      gfloat score;

      g_assert (DZL_IS_FUZZY_INDEX_MATCH (match));

      key = dzl_fuzzy_index_match_get_key (match);
      doc = dzl_fuzzy_index_match_get_document (match);
      score = dzl_fuzzy_index_match_get_score (match);

      if (FALSE)
        {
          g_autofree gchar *format = g_variant_print (doc, TRUE);
          g_print ("%f %s %s\n", score, key, format);
        }
    }

  g_main_loop_quit (main_loop);
}

static void
test_index_basic_load_cb (GObject      *object,
                          GAsyncResult *result,
                          gpointer      user_data)
{
  DzlFuzzyIndex *index = (DzlFuzzyIndex *)object;
  g_autoptr(GFile) file = user_data;
  GError *error = NULL;
  gboolean r;

  r = dzl_fuzzy_index_load_file_finish (index, result, &error);
  g_assert_no_error (error);
  g_assert_cmpint (r, ==, TRUE);

  dzl_fuzzy_index_query_async (index, "gtk", 0, NULL,
                               test_index_basic_query_cb,
                               g_object_ref (file));
}

static void
test_index_basic_cb (GObject      *object,
                     GAsyncResult *result,
                     gpointer      user_data)
{
  DzlFuzzyIndexBuilder *builder = (DzlFuzzyIndexBuilder *)object;
  g_autoptr(DzlFuzzyIndex) index = NULL;
  g_autoptr(GFile) file = user_data;
  GError *error = NULL;
  gboolean r;

  r = dzl_fuzzy_index_builder_write_finish (builder, result, &error);
  g_assert_no_error (error);
  g_assert_cmpint (r, ==, TRUE);

  index = dzl_fuzzy_index_new ();

  dzl_fuzzy_index_load_file_async (index,
                               file,
                               NULL,
                               test_index_basic_load_cb,
                               g_object_ref (file));
}

static void
test_index_basic (void)
{
  DzlFuzzyIndexBuilder *builder;
  GError *error = NULL;
  GFile *file;
  gboolean r;

  main_loop = g_main_loop_new (NULL, FALSE);
  file = g_file_new_for_path ("index.gvariant");
  builder = dzl_fuzzy_index_builder_new ();

  /*
   * We want to ensure we only get the highest scoring item for a
   * document (which are deduplicated in the index).
   */
  dzl_fuzzy_index_builder_insert (builder, "gtk_widget_show", g_variant_new_int32 (1), 7);
  dzl_fuzzy_index_builder_insert (builder, "gtk_widget_show_all", g_variant_new_int32 (1), 7);
  dzl_fuzzy_index_builder_insert (builder, "gtk_widget_hide", g_variant_new_int32 (2), 7);
  dzl_fuzzy_index_builder_insert (builder, "gtk_widget_hide_all", g_variant_new_int32 (2), 7);
  dzl_fuzzy_index_builder_insert (builder, "gtk_widget_get_parent", g_variant_new_int32 (3), 7);
  dzl_fuzzy_index_builder_insert (builder, "gtk_widget_get_name", g_variant_new_int32 (4), 7);
  dzl_fuzzy_index_builder_insert (builder, "gtk_widget_set_name", g_variant_new_int32 (5), 7);

  dzl_fuzzy_index_builder_write_async (builder,
                                       file,
                                       G_PRIORITY_LOW,
                                       NULL,
                                       test_index_basic_cb,
                                       g_object_ref (file));

  g_main_loop_run (main_loop);
  g_assert_no_error (error);

  g_object_unref (builder);

  r = g_file_delete (file, NULL, &error);
  g_assert_no_error (error);
  g_assert (r);

  g_object_unref (file);
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/Fuzzy/IndexBuilder/basic", test_index_builder_basic);
  g_test_add_func ("/Dazzle/Fuzzy/Index/basic", test_index_basic);
  return g_test_run ();
}
