/* test-graph-model.c
 *
 * Copyright 2018 Christian Hergert <chergert@redhat.com>
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
test_basic (void)
{
  g_autoptr(DzlGraphModel) model = dzl_graph_view_model_new ();
  g_autoptr(DzlGraphColumn) column = dzl_graph_view_column_new ("foo", G_TYPE_INT64);
  DzlGraphModelIter iter;
  GValue value = { 0 };
  gint idx;

  g_value_init (&value, G_TYPE_INT64);
  g_value_set_int64 (&value, 1234);

  idx = dzl_graph_view_model_add_column (model, column);

  for (guint i = 0; i < 100; i++)
    {
      gint64 v = 0;

      g_value_set_int64 (&value, i * 99);

      dzl_graph_view_model_push (model, &iter, i + 1);
      dzl_graph_view_model_iter_set_value (&iter, idx, &value);

      dzl_graph_view_model_iter_get (&iter, 0, &v, -1);
      g_assert_cmpint (v, ==, (i * 99));
    }

  g_value_unset (&value);
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/GraphModel/basic", test_basic);
  return g_test_run ();
}
