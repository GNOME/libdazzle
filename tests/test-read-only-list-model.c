/* test-read-only-list-model.c
 *
 * Copyright © 2018 Christian Hergert <chergert@redhat.com>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <dazzle.h>

static void
on_items_changed_cb (GListModel *model,
                     guint       position,
                     guint       removed,
                     guint       added,
                     guint      *count)
{
  g_assert (DZL_IS_READ_ONLY_LIST_MODEL (model));
  g_assert (added == 1);
  g_assert (removed == 0);

  (*count)++;
}

static void
test_basic (void)
{
  g_autoptr(GListStore) store = g_list_store_new (G_TYPE_OBJECT);
  g_autoptr(GListModel) wrapper = dzl_read_only_list_model_new (G_LIST_MODEL (store));
  g_autoptr(GObject) obj1 = g_object_new (G_TYPE_OBJECT, NULL);
  g_autoptr(GObject) obj2 = g_object_new (G_TYPE_OBJECT, NULL);
  g_autoptr(GObject) obj3 = g_object_new (G_TYPE_OBJECT, NULL);
  g_autoptr(GObject) obj4 = g_object_new (G_TYPE_OBJECT, NULL);
  g_autoptr(GObject) obj5 = g_object_new (G_TYPE_OBJECT, NULL);
  g_autoptr(GObject) read1 = NULL;
  g_autoptr(GObject) read2 = NULL;
  g_autoptr(GObject) read3 = NULL;
  g_autoptr(GObject) read4 = NULL;
  g_autoptr(GObject) read5 = NULL;
  guint count = 0;

  g_signal_connect (wrapper,
                    "items-changed",
                    G_CALLBACK (on_items_changed_cb),
                    &count);

  g_list_store_append (store, obj3);
  g_list_store_insert (store, 0, obj2);
  g_list_store_append (store, obj4);
  g_list_store_insert (store, 0, obj1);
  g_list_store_append (store, obj5);

  g_assert_cmpint (5, ==, count);
  g_assert_cmpint (5, ==, g_list_model_get_n_items (wrapper));
  g_assert (G_TYPE_OBJECT == g_list_model_get_item_type (wrapper));

  read1 = g_list_model_get_item (wrapper, 0);
  read2 = g_list_model_get_item (wrapper, 1);
  read3 = g_list_model_get_item (wrapper, 2);
  read4 = g_list_model_get_item (wrapper, 3);
  read5 = g_list_model_get_item (wrapper, 4);

  g_assert (read1 == obj1);
  g_assert (read2 == obj2);
  g_assert (read3 == obj3);
  g_assert (read4 == obj4);
  g_assert (read5 == obj5);
}

gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/ReadOnlyListModel/basic", test_basic);
  return g_test_run ();
}
