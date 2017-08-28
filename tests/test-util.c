/* test-util.c
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

#include "util/dzl-util-private.h"

static void
test_action_parsing (void)
{
  struct {
    const gchar *input;
    const gchar *expected_prefix;
    const gchar *expected_name;
    const gchar *expected_target;
  } simple_tests[] = {
    { "app.foobar", "app", "foobar", NULL },
    { "win.foo", "win", "foo", NULL },
    { "win.foo::1", "win", "foo", "'1'" },
    { "win.foo(1)", "win", "foo", "1" },
  };

  for (guint i = 0; i < G_N_ELEMENTS (simple_tests); i++)
    {
      g_autofree gchar *prefix = NULL;
      g_autofree gchar *name = NULL;
      g_autoptr(GVariant) target = NULL;

      if (!dzl_g_action_name_parse_full (simple_tests[i].input, &prefix, &name, &target))
        g_error ("Failed to parse %s", simple_tests[i].input);

      g_assert_cmpstr (prefix, ==, simple_tests[i].expected_prefix);
      g_assert_cmpstr (name, ==, simple_tests[i].expected_name);

      if (simple_tests[i].expected_target)
        {
          g_autoptr(GVariant) expected_target = g_variant_parse (NULL,
                                                                 simple_tests[i].expected_target,
                                                                 NULL, NULL, NULL);
          if (!g_variant_equal (expected_target, target))
            {
              g_printerr ("Expected: %s\n", g_variant_print (expected_target, TRUE));
              g_printerr ("Actual: %s\n", g_variant_print (target, TRUE));
              g_assert_not_reached ();
            }
        }
    }
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Util/Action/parse", test_action_parsing);
  return g_test_run ();
}
