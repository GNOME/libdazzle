#include <dazzle.h>

static void
test_basic (void)
{
  g_autoptr(GCancellable) root = g_cancellable_new ();
  g_autoptr(GCancellable) a = g_cancellable_new ();
  g_autoptr(GCancellable) b = g_cancellable_new ();
  g_autoptr(GCancellable) a1 = g_cancellable_new ();
  g_autoptr(GCancellable) a2 = g_cancellable_new ();

  dzl_cancellable_chain (root, a);
  dzl_cancellable_chain (root, b);

  dzl_cancellable_chain (a, a1);
  dzl_cancellable_chain (a, a2);

  g_cancellable_cancel (a2);

  g_assert_cmpint (TRUE, ==, g_cancellable_is_cancelled (a2));
  g_assert_cmpint (TRUE, ==, g_cancellable_is_cancelled (a));
  g_assert_cmpint (TRUE, ==, g_cancellable_is_cancelled (root));

  g_assert_cmpint (FALSE, ==, g_cancellable_is_cancelled (a1));
  g_assert_cmpint (FALSE, ==, g_cancellable_is_cancelled (b));
}

static void
test_root (void)
{
  g_autoptr(GCancellable) root = g_cancellable_new ();
  g_autoptr(GCancellable) a = g_cancellable_new ();
  g_autoptr(GCancellable) b = g_cancellable_new ();
  g_autoptr(GCancellable) a1 = g_cancellable_new ();
  g_autoptr(GCancellable) a2 = g_cancellable_new ();

  dzl_cancellable_chain (root, a);
  dzl_cancellable_chain (root, b);
  dzl_cancellable_chain (a, a1);
  dzl_cancellable_chain (a, a2);

  g_cancellable_cancel (root);

  g_assert_cmpint (TRUE, ==, g_cancellable_is_cancelled (root));

  g_assert_cmpint (FALSE, ==, g_cancellable_is_cancelled (a));
  g_assert_cmpint (FALSE, ==, g_cancellable_is_cancelled (a1));
  g_assert_cmpint (FALSE, ==, g_cancellable_is_cancelled (a2));
  g_assert_cmpint (FALSE, ==, g_cancellable_is_cancelled (b));

  g_clear_object (&root);
  g_clear_object (&a);
  g_clear_object (&b);
  g_clear_object (&a1);
  g_clear_object (&a2);
}

static void
test_weak (void)
{
  g_autoptr(GCancellable) root = g_cancellable_new ();
  g_autoptr(GCancellable) a = g_cancellable_new ();
  g_autoptr(GCancellable) b = g_cancellable_new ();
  g_autoptr(GCancellable) a1 = g_cancellable_new ();
  g_autoptr(GCancellable) a2 = g_cancellable_new ();

  dzl_cancellable_chain (root, a);
  dzl_cancellable_chain (root, b);
  dzl_cancellable_chain (a, a1);
  dzl_cancellable_chain (a, a2);

  g_clear_object (&root);
  g_clear_object (&a);
  g_clear_object (&b);
  g_clear_object (&a1);
  g_clear_object (&a2);
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/Cancellable/basic", test_basic);
  g_test_add_func ("/Dazzle/Cancellable/root", test_root);
  g_test_add_func ("/Dazzle/Cancellable/weak", test_weak);
  return g_test_run ();
}
