#include <dazzle.h>
#include <stdlib.h>

static GRand *grand;

static gint
compare_direct (gconstpointer a,
                gconstpointer b,
                gpointer      data)
{
  if (a < b)
    return -1;
  else if (a > b)
    return 1;
  return 0;
}

#define assert_cmpptr(a,eq,b) \
  G_STMT_START { \
    if (!((a) eq (b))) { \
      g_error (#a "(%p) " #eq " " #b "(%p)\n", (a), (b)); \
    } \
  } G_STMT_END

static void
test_insert_sorted (void)
{
  GtkListStore *store = gtk_list_store_new (1, G_TYPE_POINTER);
  GtkTreeIter iter;
  gboolean r;
  gpointer last = NULL;
  guint count = 0;

  for (guint i = 0; i < 1000; i++)
    {
      gpointer value = GINT_TO_POINTER (g_rand_int (grand));

      dzl_gtk_list_store_insert_sorted (store, &iter, value, 0, compare_direct, NULL);
      gtk_list_store_set (store, &iter, 0, value, -1);
    }

  r = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (store), &iter);
  g_assert_cmpint (r, ==, TRUE);

  do
    {
      gpointer value = NULL;

      gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, 0, &value, -1);
      assert_cmpptr (value, >=, last);
      last = value;

      count++;
    }
  while (gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter));

  g_assert_cmpint (count, ==, 1000);

  g_object_unref (store);
}

gint
main (gint argc,
      gchar *argv[])
{
  grand = g_rand_new ();
  gtk_init (&argc, &argv);
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/GtkListStore/insert_sorted", test_insert_sorted);
  return g_test_run ();
}
