/* test-fuzzy-highlight.c
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
test_highlight (void)
{
  static const struct {
    const gchar *needle;
    const gchar *haystack;
    const gchar *expected;
  } tests[] = {
    { "with tab", "with <Tab>", "<b>with </b>&lt;<b>Tab</b>&gt;" },
    { "with t", "with <Tab>", "<b>with </b>&lt;<b>T</b>ab&gt;" },
    { "with tuff", "with &apos; stuff", "<b>with </b>&apos; s<b>tuff</b>" },
    { "gtkwdg", "gtk_widget_show", "<b>gtk</b>_<b>w</b>i<b>dg</b>et_show" },
  };

  for (guint i = 0; i < G_N_ELEMENTS (tests); i++)
    {
      g_autofree gchar *highlight = dzl_fuzzy_highlight (tests[i].haystack,
                                                         tests[i].needle,
                                                         FALSE);
      g_assert_cmpstr (highlight, ==, tests[i].expected);
    }
}

gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/Fuzzy/highlight", test_highlight);
  return g_test_run ();
}
