/* test-ring.c
 *
 * Copyright (C) 2010-2017 Christian Hergert <chris@dronelabs.com>
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

static void
test_index_conversion (void)
{
  g_autoptr(DzlRing) ring = dzl_ring_sized_new (sizeof (guint), 10, NULL);

  g_assert_cmpint (ring->len, ==, 10);
  g_assert_cmpint (ring->pos, ==, 0);

  g_assert_cmpint (0, ==, _dzl_ring_index (ring, 0));
  g_assert_cmpint (1, ==, _dzl_ring_index (ring, 1));
  g_assert_cmpint (2, ==, _dzl_ring_index (ring, 2));
  g_assert_cmpint (3, ==, _dzl_ring_index (ring, 3));
  g_assert_cmpint (4, ==, _dzl_ring_index (ring, 4));
  g_assert_cmpint (5, ==, _dzl_ring_index (ring, 5));
  g_assert_cmpint (6, ==, _dzl_ring_index (ring, 6));
  g_assert_cmpint (7, ==, _dzl_ring_index (ring, 7));
  g_assert_cmpint (8, ==, _dzl_ring_index (ring, 8));
  g_assert_cmpint (9, ==, _dzl_ring_index (ring, 9));
  g_assert_cmpint (0, ==, _dzl_ring_index (ring, 10));
  g_assert_cmpint (1, ==, _dzl_ring_index (ring, 11));
  g_assert_cmpint (9, ==, _dzl_ring_index (ring, -1));
  g_assert_cmpint (8, ==, _dzl_ring_index (ring, -2));
  g_assert_cmpint (7, ==, _dzl_ring_index (ring, -3));
  g_assert_cmpint (6, ==, _dzl_ring_index (ring, -4));
  g_assert_cmpint (5, ==, _dzl_ring_index (ring, -5));
  g_assert_cmpint (4, ==, _dzl_ring_index (ring, -6));
  g_assert_cmpint (3, ==, _dzl_ring_index (ring, -7));
  g_assert_cmpint (2, ==, _dzl_ring_index (ring, -8));
  g_assert_cmpint (1, ==, _dzl_ring_index (ring, -9));
  g_assert_cmpint (0, ==, _dzl_ring_index (ring, -10));
}

static void
test_DzlRing_ref (void)
{
  DzlRing *ring;

  ring = dzl_ring_sized_new (sizeof (gdouble), 60, NULL);
  g_assert (ring);
  dzl_ring_unref (ring);
}

static void
test_various_lengths (gint len)
{
  DzlRing *ring;
  gdouble d;

  ring = dzl_ring_sized_new (sizeof (gdouble), len, NULL);
  g_assert (ring);

  d = 3.;
  dzl_ring_append_val (ring, d);
  d = dzl_ring_get_index (ring, gdouble, -1);
  g_assert_cmpint (d, ==, 3.);
  d = dzl_ring_get_index (ring, gdouble, len-1);
  g_assert_cmpint (d, ==, 3.);
  d = dzl_ring_get_index (ring, gdouble, 0);
  g_assert_cmpint (d, ==, 0.);

  d = 4.;
  dzl_ring_append_val (ring, d);
  d = dzl_ring_get_index (ring, gdouble, -1);
  g_assert_cmpint (d, ==, 4.);
  d = dzl_ring_get_index (ring, gdouble, len-1);
  g_assert_cmpint (d, ==, 4.);
  d = dzl_ring_get_index (ring, gdouble, -2);
  g_assert_cmpint (d, ==, 3.);
  d = dzl_ring_get_index (ring, gdouble, len-2);
  g_assert_cmpint (d, ==, 3.);
  d = dzl_ring_get_index (ring, gdouble, 0);
  g_assert_cmpint (d, ==, 0.);

  d = 6.;
  dzl_ring_append_val (ring, d);
  d = dzl_ring_get_index (ring, gdouble, -1);
  g_assert_cmpint (d, ==, 6.);
  d = dzl_ring_get_index (ring, gdouble, len-1);
  g_assert_cmpint (d, ==, 6.);
  d = dzl_ring_get_index (ring, gdouble, -2);
  g_assert_cmpint (d, ==, 4.);
  d = dzl_ring_get_index (ring, gdouble, len-2);
  g_assert_cmpint (d, ==, 4.);
  d = dzl_ring_get_index (ring, gdouble, -3);
  g_assert_cmpint (d, ==, 3.);
  d = dzl_ring_get_index (ring, gdouble, len-3);
  g_assert_cmpint (d, ==, 3.);
  d = dzl_ring_get_index (ring, gdouble, -4);
  g_assert_cmpint (d, ==, 0.);
  d = dzl_ring_get_index (ring, gdouble, len-4);
  g_assert_cmpint (d, ==, 0.);
  d = dzl_ring_get_index (ring, gdouble, 0);
  g_assert_cmpint (d, ==, 0.);
  d = dzl_ring_get_index (ring, gdouble, len);
  g_assert_cmpint (d, ==, 0.);

  dzl_ring_unref (ring);
}

static void
test_DzlRing_with_double (void)
{
  for (guint i = 20; i < 100; i++)
    test_various_lengths (i);
}

static void
test_DzlRing_with_int (void)
{
  DzlRing *ring;
  gint i;

  ring = dzl_ring_sized_new (sizeof (gint), 10, NULL);
  i = dzl_ring_get_index (ring, gint, 0);
  g_assert_cmpint (i, ==, 0);
  i = 10;
  dzl_ring_append_val (ring, i);
  i = dzl_ring_get_index (ring, gint, -1);
  g_assert_cmpint (i, ==, 10);
  i = 20;
  dzl_ring_append_val (ring, i);
  i = dzl_ring_get_index (ring, gint, -1);
  g_assert_cmpint (i, ==, 20);
  i = dzl_ring_get_index (ring, gint, -2);
  g_assert_cmpint (i, ==, 10);
  i = dzl_ring_get_index (ring, gint, 0);
  g_assert_cmpint (i, ==, 0);
  dzl_ring_unref (ring);
}

static gboolean test_DzlRing_with_array_result = FALSE;

static void
test_DzlRing_with_array_cb (gpointer data)
{
  GArray **ar = data;
  test_DzlRing_with_array_result = TRUE;
  g_array_unref (*ar);
}

static void
test_DzlRing_with_array (void)
{
  DzlRing *ring;
  GArray *ar0 = NULL;
  GArray *ar1 = NULL;
  GArray *ar2 = NULL;
  gpointer tmp;

  ring = dzl_ring_sized_new (sizeof (GArray*), 2, test_DzlRing_with_array_cb);
  ar0 = g_array_new (FALSE, TRUE, sizeof (gdouble));
  ar1 = g_array_new (FALSE, TRUE, sizeof (gdouble));
  ar2 = g_array_new (FALSE, TRUE, sizeof (gdouble));

  dzl_ring_append_val (ring, ar0);
  dzl_ring_append_val (ring, ar1);
  tmp = dzl_ring_get_index (ring, GArray*, -1);
  g_assert (tmp == ar1);
  tmp = dzl_ring_get_index (ring, GArray*, -2);
  g_assert (tmp == ar0);

  /*
   * Make sure that ar0 was dispoased as we ran over it.
   */
  dzl_ring_append_val (ring, ar2);
  tmp = dzl_ring_get_index (ring, GArray*, -1);
  g_assert (tmp == ar2);
  tmp = dzl_ring_get_index (ring, GArray*, -2);
  g_assert (tmp == ar1);
  g_assert_cmpint (test_DzlRing_with_array_result, ==, TRUE);

  dzl_ring_unref (ring);
}

static void
test_DzlRing_foreach_cb (gpointer data,
                       gpointer user_data)
{
  gboolean *success = user_data;
  *success = FALSE;
}

static void
test_DzlRing_foreach_cb2 (gpointer data,
                        gpointer user_data)
{
  gboolean *success = user_data;
  gpointer *p = data;

  if (*p)
    {
      g_assert (*p == GINT_TO_POINTER (1234));
      *success = TRUE;
    }
}

static void
test_DzlRing_foreach (void)
{
  DzlRing *ring;
  gboolean success = TRUE;
  gpointer data = GINT_TO_POINTER (1234);

  ring = dzl_ring_sized_new (sizeof (gpointer), 20, NULL);
  dzl_ring_foreach (ring, test_DzlRing_foreach_cb, &success);
  g_assert (success);

  success = FALSE;
  dzl_ring_append_val (ring, data);
  dzl_ring_foreach (ring, test_DzlRing_foreach_cb2, &success);
  g_assert (success);

  dzl_ring_unref (ring);
}

gint
main (gint   argc,   /* IN */
      gchar *argv[]) /* IN */
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/Dazzle/Ring/index", test_index_conversion);
	g_test_add_func ("/Dazzle/Ring/ref", test_DzlRing_ref);
	g_test_add_func ("/Dazzle/Ring/with_double", test_DzlRing_with_double);
	g_test_add_func ("/Dazzle/Ring/with_int", test_DzlRing_with_int);
	g_test_add_func ("/Dazzle/Ring/with_array", test_DzlRing_with_array);
	g_test_add_func ("/Dazzle/Ring/foreach", test_DzlRing_foreach);

	return g_test_run ();
}
