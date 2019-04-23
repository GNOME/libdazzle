/* test-pattern-spec.c
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
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
test_basic (void)
{
  static const struct {
    const gchar *needle;
    const gchar *haystack;
    gboolean     result;
  } tests[] = {
    { "blue", "red blue purple pink green black cyan", TRUE },
    { "Blue", "red blue purple pink green black cyan", FALSE },
    { "blue", "red Blue purple pink green black cyan", TRUE },
    { "DzlPatternSpec", "DzlPatternSpec_autoptr", TRUE },
    { "dzlpattern", "DzlPatternSpec_autoptr", TRUE },
    { "dzlpattern", "dzl_pattern_spec", FALSE },
    { "dzl pattern", "dzl_pattern_spec", TRUE },
    { "org", "org.freedesktop.DBus", TRUE },
    { "org Db", "org.freedesktop.DBus", FALSE },
    { "org DB", "org.freedesktop.DBus", TRUE },
  };

  for (guint i = 0; i < G_N_ELEMENTS (tests); i++)
    {
      g_autoptr(DzlPatternSpec) spec = NULL;
      gboolean r;

      spec = dzl_pattern_spec_new (tests[i].needle);
      r = dzl_pattern_spec_match (spec, tests[i].haystack);
      g_assert_cmpint (r, ==, tests[i].result);
    }
}

gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/PatternSpec/basic", test_basic);
  return g_test_run ();
}
