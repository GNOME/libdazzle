#include <dazzle.h>

static void
test_dzl_trie_insert (void)
{
   DzlTrie *trie;

   trie = dzl_trie_new(NULL);
   dzl_trie_insert(trie, "a", (gchar *)"a");
   g_assert_cmpstr("a", ==, dzl_trie_lookup(trie, "a"));
   dzl_trie_insert(trie, "b", (gchar *)"b");
   g_assert_cmpstr("b", ==, dzl_trie_lookup(trie, "b"));
   dzl_trie_insert(trie, "c", (gchar *)"c");
   g_assert_cmpstr("c", ==, dzl_trie_lookup(trie, "c"));
   dzl_trie_insert(trie, "d", (gchar *)"d");
   g_assert_cmpstr("d", ==, dzl_trie_lookup(trie, "d"));
   dzl_trie_insert(trie, "e", (gchar *)"e");
   g_assert_cmpstr("e", ==, dzl_trie_lookup(trie, "e"));
   dzl_trie_insert(trie, "f", (gchar *)"f");
   g_assert_cmpstr("f", ==, dzl_trie_lookup(trie, "f"));
   dzl_trie_insert(trie, "g", (gchar *)"g");
   g_assert_cmpstr("g", ==, dzl_trie_lookup(trie, "g"));
   dzl_trie_destroy(trie);
}

static gboolean
traverse_cb (DzlTrie        *trie,
             const gchar *key,
             gpointer     value,
             gpointer     user_data)
{
   guint *count = user_data;

   (*count)++;

   return FALSE;
}

static void
test_dzl_trie_gauntlet (void)
{
   g_autofree gchar *path = NULL;
   gboolean ret;
   GTimer *timer;
   GError *error = NULL;
   gchar *content;
   gchar **words;
   guint word_count = 0;
   guint count = 0;
   guint i;
   guint j;
   DzlTrie *trie;

   path = g_build_filename (TEST_DATA_DIR, "words.txt", NULL);
   ret = g_file_get_contents(path, &content, NULL, &error);
   g_assert_no_error(error);
   if (!ret) {
      g_assert(ret);
   }

   words = g_strsplit(content, "\n", -1);
   trie = dzl_trie_new(NULL);

   g_free(content);
   content = NULL;

   g_print("\ninsert,read,traverse,remove,free\n");

   timer = g_timer_new();

   for (i = 0; words[i]; i++) {
      dzl_trie_insert(trie, words[i], words[i]);
   }

   word_count = i;

   g_timer_stop(timer);
   g_print("%lf", g_timer_elapsed(timer, NULL));
   g_timer_reset(timer);

   for (j = 0; j < 4; j++) {
      for (i = 0; words[i]; i++) {
         gchar *s = dzl_trie_lookup(trie, words[i]);
         g_assert_cmpstr(words[i], ==, s);
      }
   }

   g_timer_stop(timer);
   g_print(",%lf", g_timer_elapsed(timer, NULL));
   g_timer_reset(timer);

   dzl_trie_traverse(trie, NULL,
                 G_PRE_ORDER, G_TRAVERSE_LEAVES, -1,
                 traverse_cb, &count);
   g_assert_cmpint(count, ==, word_count);

   g_timer_stop(timer);
   g_print(",%lf", g_timer_elapsed(timer, NULL));
   g_timer_reset(timer);

   for (i = 0; words[i]; i++) {
      if (i % 2 == 0) {
         g_assert(dzl_trie_remove(trie, words[i]));
      }
   }

   for (i = 0; words[i]; i++) {
      if (i % 2 != 0) {
         g_assert_cmpstr(words[i], ==, dzl_trie_lookup(trie, words[i]));
      } else {
         g_assert(!dzl_trie_lookup(trie, words[i]));
      }
   }

   g_timer_stop(timer);
   g_print(",%lf", g_timer_elapsed(timer, NULL));
   g_timer_reset(timer);

   dzl_trie_destroy(trie);
   trie = NULL;

   g_timer_stop(timer);
   g_print(",%lf\n", g_timer_elapsed(timer, NULL));

   g_strfreev(words);
   words = NULL;

   g_timer_destroy (timer);
}

gint
main (gint   argc,
      gchar *argv[])
{
   g_test_init(&argc, &argv, NULL);
   g_test_add_func("/Dazzle/Trie/insert", test_dzl_trie_insert);
   g_test_add_func("/Dazzle/Trie/gauntlet", test_dzl_trie_gauntlet);
   return g_test_run();
}
