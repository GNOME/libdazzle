#include <dazzle.h>

static void
test_intpair_basic (void)
{
  DzlIntPair *p;

  p = dzl_int_pair_new (0, 0);
#ifdef DZL_INT_PAIR_64
  /* Technically not ANSI as NULL is allowed to be non-zero, but all the
   * platforms we support, this is the case.
   */
  g_assert (p == NULL);
#else
  g_assert (p != NULL);
#endif

  p = dzl_int_pair_new (4, 5);
  g_assert (p != NULL);
  g_assert_cmpint (dzl_int_pair_first (p), ==, 4);
  g_assert_cmpint (dzl_int_pair_second (p), ==, 5);
  dzl_int_pair_free (p);

  p = dzl_int_pair_new (G_MAXINT, G_MAXINT-1);
  g_assert (p != NULL);
  g_assert_cmpint (dzl_int_pair_first (p), ==, G_MAXINT);
  g_assert_cmpint (dzl_int_pair_second (p), ==, G_MAXINT-1);
  dzl_int_pair_free (p);

  p = dzl_int_pair_new (G_MAXINT-1, G_MAXINT);
  g_assert (p != NULL);
  g_assert_cmpint (dzl_int_pair_first (p), ==, G_MAXINT-1);
  g_assert_cmpint (dzl_int_pair_second (p), ==, G_MAXINT);
  dzl_int_pair_free (p);
}

static void
test_uintpair_basic (void)
{
  DzlUIntPair *p;

  p = dzl_uint_pair_new (0, 0);
#ifdef DZL_INT_PAIR_64
  /* Technically not ANSI as NULL is allowed to be non-zero, but all the
   * platforms we support, this is the case.
   */
  g_assert (p == NULL);
#else
  g_assert (p != NULL);
#endif

  p = dzl_uint_pair_new (4, 5);
  g_assert (p != NULL);
  g_assert_cmpuint (dzl_uint_pair_first (p), ==, 4);
  g_assert_cmpuint (dzl_uint_pair_second (p), ==, 5);
  dzl_uint_pair_free (p);

  p = dzl_uint_pair_new (G_MAXUINT, G_MAXUINT-1);
  g_assert (p != NULL);
  g_assert_cmpuint (dzl_uint_pair_first (p), ==, G_MAXUINT);
  g_assert_cmpuint (dzl_uint_pair_second (p), ==, G_MAXUINT-1);
  dzl_uint_pair_free (p);

  p = dzl_uint_pair_new (G_MAXUINT-1, G_MAXUINT);
  g_assert (p != NULL);
  g_assert_cmpuint (dzl_uint_pair_first (p), ==, G_MAXUINT-1);
  g_assert_cmpuint (dzl_uint_pair_second (p), ==, G_MAXUINT);
  dzl_uint_pair_free (p);
}

gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/IntPair/basic", test_intpair_basic);
  g_test_add_func ("/Dazzle/UIntPair/basic", test_uintpair_basic);
  return g_test_run ();
}
