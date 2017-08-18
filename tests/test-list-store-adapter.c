/* test-list-store-adapter.c
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
test_list_adapter (void)
{
  g_autoptr(GListStore) store = NULL;
  g_autoptr(DzlListStoreAdapter) adapter = NULL;
  GtkTreeIter iter;
  gint n_children;

  store = g_list_store_new (G_TYPE_OBJECT);
  adapter = dzl_list_store_adapter_new (G_LIST_MODEL (store));

  n_children = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (adapter), NULL);
  g_assert_cmpint (n_children, ==, 0);

  for (guint i = 0; i < 100; i++)
    {
      g_autoptr(GObject) obj = g_object_new (G_TYPE_OBJECT, NULL);
      g_object_set_data (obj, "ID", GINT_TO_POINTER (i));
      g_list_store_append (store, obj);

      n_children = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (adapter), NULL);
      g_assert_cmpint (n_children, ==, i + 1);
    }

  n_children = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (adapter), NULL);
  g_assert_cmpint (n_children, ==, 100);

  for (gint i = 99; i > 0; i -= 2)
    g_list_store_remove (store, i);

  n_children = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (adapter), NULL);
  g_assert_cmpint (n_children, ==, 50);

  for (guint i = 0; i < 50; i++)
    {
      g_autoptr(GObject) obj = g_list_model_get_item (G_LIST_MODEL (store), i);
      gint id = GPOINTER_TO_INT (g_object_get_data (obj, "ID"));
      g_assert_cmpint (id, ==, i * 2);
    }

  for (guint i = 10; i > 0; i--)
    {
      g_autoptr(GObject) obj = g_object_new (G_TYPE_OBJECT, NULL);
      g_object_set_data (obj, "ID", GINT_TO_POINTER (i - 1));
      g_list_store_insert (store, 20, obj);
    }

  for (guint i = 0; i < 60; i++)
    {
      g_autoptr(GObject) obj = g_list_model_get_item (G_LIST_MODEL (store), i);
      gint id = GPOINTER_TO_INT (g_object_get_data (obj, "ID"));

      if (i < 20)
        g_assert_cmpint (id, ==, i * 2);
      else if (i >= 30)
        g_assert_cmpint (id, ==, (i-10) * 2);
      else
        g_assert_cmpint (id, ==, i - 20);
    }

  gtk_tree_model_get_iter_first (GTK_TREE_MODEL (adapter), &iter);

  for (guint i = 0; i < 60; i++)
    {
      g_autoptr(GObject) obj = NULL;
      g_autoptr(GObject) obj2 = NULL;

      obj = g_list_model_get_item (G_LIST_MODEL (store), i);
      gtk_tree_model_get (GTK_TREE_MODEL (adapter), &iter, 0, &obj2, -1);

      g_assert (obj == obj2);

      gtk_tree_model_iter_next (GTK_TREE_MODEL (adapter), &iter);
    }
}

gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/ListStoreAdapter/basic", test_list_adapter);
  return g_test_run ();
}
