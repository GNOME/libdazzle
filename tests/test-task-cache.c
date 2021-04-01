#include <dazzle.h>

static GMainLoop *main_loop;
static DzlTaskCache *cache;
static GObject *foo;

static void
populate_callback (DzlTaskCache  *self,
                   gconstpointer  key,
                   GTask         *task,
                   gpointer       user_data)
{
  foo = g_object_new (G_TYPE_OBJECT, NULL);
  g_object_add_weak_pointer (G_OBJECT (foo), (gpointer *)&foo);
  g_task_return_pointer (task, foo, g_object_unref);
}

static void
get_foo_cb (GObject      *object,
            GAsyncResult *result,
            gpointer      user_data)
{
  GError *error = NULL;
  GObject *ret;

  ret = dzl_task_cache_get_finish (cache, result, &error);
  g_assert_no_error (error);
  g_assert_nonnull (ret);
  g_assert_true (ret == foo);

  g_assert_true (dzl_task_cache_evict (cache, "foo"));
  g_object_unref (ret);

  g_main_loop_quit (main_loop);
}

static void
test_task_cache (void)
{
  main_loop = g_main_loop_new (NULL, FALSE);
  cache = dzl_task_cache_new (g_str_hash,
                              g_str_equal,
                              (GBoxedCopyFunc)g_strdup,
                              (GBoxedFreeFunc)g_free,
                              g_object_ref,
                              g_object_unref,
                              100 /* msec */,
                              populate_callback, NULL, NULL);

  g_assert_null (dzl_task_cache_peek (cache, "foo"));
  g_assert_false (dzl_task_cache_evict (cache, "foo"));

  dzl_task_cache_get_async (cache, "foo", TRUE, NULL, get_foo_cb, NULL);

  g_main_loop_run (main_loop);
  g_main_loop_unref (main_loop);

  g_assert_null (foo);
}

static void
populate_callback_raw_value (DzlTaskCache  *self,
                             gconstpointer  key,
                             GTask         *task,
                             gpointer       user_data)
{
  g_task_return_pointer (task, GINT_TO_POINTER ((gint) TRUE), NULL);
}

static void
get_foo_raw_value_cb (GObject      *object,
                      GAsyncResult *result,
                      gpointer      user_data)
{
  GError *error = NULL;
  gboolean value;
  gpointer ret;

  ret = dzl_task_cache_get_finish (cache, result, &error);
  g_assert_no_error (error);
  g_assert_nonnull (ret);

  value = (gboolean) GPOINTER_TO_INT (ret);
  g_assert_true (value);

  g_assert_true (dzl_task_cache_evict (cache, "foo"));

  g_main_loop_quit (main_loop);
}

static void
test_task_cache_raw_value (void)
{
  main_loop = g_main_loop_new (NULL, FALSE);
  cache = dzl_task_cache_new (g_str_hash,
                              g_str_equal,
                              (GBoxedCopyFunc)g_strdup,
                              (GBoxedFreeFunc)g_free,
                              NULL,
                              NULL,
                              100 /* msec */,
                              populate_callback_raw_value, NULL, NULL);

  g_assert_null (dzl_task_cache_peek (cache, "foo"));
  g_assert_false (dzl_task_cache_evict (cache, "foo"));

  dzl_task_cache_get_async (cache, "foo", TRUE, NULL, get_foo_raw_value_cb, NULL);

  g_main_loop_run (main_loop);
  g_main_loop_unref (main_loop);
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/TaskCache/basic", test_task_cache);
  g_test_add_func ("/Dazzle/TaskCache/raw-value", test_task_cache_raw_value);
  return g_test_run ();
}
