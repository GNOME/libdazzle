/* test-mutable-scoring.c
 *
 * Copyright 2019 Christian Hergert <chergert@redhat.com>
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
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <dazzle.h>

static void
test_scoring_1 (void)
{
  g_autoptr(DzlFuzzyMutableIndex) index = dzl_fuzzy_mutable_index_new (FALSE);
  g_autoptr(GArray) all = NULL;
  DzlFuzzyMutableIndexMatch *m;

  static const gchar *words[] = {
    "plugins/flatpak/gs-flatpak.c",
    "plugins/flatpak/gs-flatpak.h",
    "plugins/flatpak/gs-flatpak-app.c",
    "plugins/flatpak/gs-flatpak-app.h",
    "plugins/flatpak/gs-flatpak-utils.c",
    "plugins/flatpak/gs-flatpak-utils.h",
    "plugins/flatpak/gs-plugin-flatpak.c",
    "plugins/flatpak/gs-flatpak-transaction.c",
    "plugins/flatpak/gs-flatpak-transaction.h",
  };

  dzl_fuzzy_mutable_index_begin_bulk_insert (index);
  for (guint i = 0; i < G_N_ELEMENTS (words); i++)
    dzl_fuzzy_mutable_index_insert (index, words[i], (gchar *)words[i]);
  dzl_fuzzy_mutable_index_end_bulk_insert (index);

  all = dzl_fuzzy_mutable_index_match (index, "plugin-flatpak", G_N_ELEMENTS (words));
  g_assert_cmpint (all->len, ==, G_N_ELEMENTS (words));

#if 0
  for (guint i = 0; i < all->len; i++)
    {
      m = &g_array_index (all, DzlFuzzyMutableIndexMatch, i);
      g_print ("%d: %s %lf\n", i, m->key, m->score);
    }
#endif

  m = &g_array_index (all, DzlFuzzyMutableIndexMatch, 0);
  g_assert_cmpstr (m->key, ==, "plugins/flatpak/gs-plugin-flatpak.c");
}

gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/FuzzyMutableIndex/scoring-1", test_scoring_1);
  return g_test_run ();
}
