#include <dazzle.h>
#include <string.h>

#define TEST_TYPE_ITEM (test_item_get_type())

struct _TestItem
{
  GObject p;
  guint n;
};

G_DECLARE_FINAL_TYPE (TestItem, test_item, TEST, ITEM, GObject)
G_DEFINE_TYPE (TestItem, test_item, G_TYPE_OBJECT)

static void
test_item_class_init (TestItemClass *klass)
{
}

static void
test_item_init (TestItem *self)
{
}

static TestItem *
test_item_new (guint n)
{
  TestItem *item;

  item = g_object_new (TEST_TYPE_ITEM, NULL);
  item->n = n;

  return item;
}

static gboolean
filter_func1 (GObject  *object,
              gpointer  user_data)
{
  return (TEST_ITEM (object)->n & 1) == 0;
}

static gboolean
filter_func2 (GObject  *object,
              gpointer  user_data)
{
  return (TEST_ITEM (object)->n & 1) == 1;
}

static void
test_basic (void)
{
  GListStore *model;
  DzlListModelFilter *filter;
  TestItem *item;
  guint i;

  model = g_list_store_new (TEST_TYPE_ITEM);
  g_assert (model);

  filter = dzl_list_model_filter_new (G_LIST_MODEL (model));
  g_assert (filter);

  /* Test requesting past boundary */
  g_assert_null (g_list_model_get_item (G_LIST_MODEL (filter), 0));
  g_assert_null (g_list_model_get_item (G_LIST_MODEL (filter), 1));

  for (i = 0; i < 1000; i++)
    {
      g_autoptr(TestItem) val = test_item_new (i);

      g_list_store_append (model, val);
    }

  /* Test requesting past boundary */
  g_assert_null (g_list_model_get_item (G_LIST_MODEL (filter), 1000));

  g_assert_cmpint (1000, ==, g_list_model_get_n_items (G_LIST_MODEL (model)));
  g_assert_cmpint (1000, ==, g_list_model_get_n_items (G_LIST_MODEL (filter)));

  g_assert_cmpint (1000, ==, g_list_model_get_n_items (G_LIST_MODEL (filter)));
  dzl_list_model_filter_set_filter_func (filter, filter_func1, NULL, NULL);
  g_assert_cmpint (500, ==, g_list_model_get_n_items (G_LIST_MODEL (filter)));

  for (i = 0; i < 500; i++)
    {
      g_autoptr(TestItem) ele = g_list_model_get_item (G_LIST_MODEL (filter), i);

      g_assert_nonnull (ele);
      g_assert (TEST_IS_ITEM (ele));
      g_assert (filter_func1 (G_OBJECT (ele), NULL));
    }

  for (i = 0; i < 1000; i += 2)
    g_list_store_remove (model, 998 - i);

  g_assert_cmpint (500, ==, g_list_model_get_n_items (G_LIST_MODEL (model)));
  g_assert_cmpint (0, ==, g_list_model_get_n_items (G_LIST_MODEL (filter)));

  dzl_list_model_filter_set_filter_func (filter, NULL, NULL, NULL);
  g_assert_cmpint (500, ==, g_list_model_get_n_items (G_LIST_MODEL (filter)));

  dzl_list_model_filter_set_filter_func (filter, filter_func2, NULL, NULL);
  g_assert_cmpint (500, ==, g_list_model_get_n_items (G_LIST_MODEL (filter)));

  {
    g_autoptr(TestItem) freeme = test_item_new (1001);
    g_list_store_append (model, freeme);
  }

  for (i = 0; i < 500; i++)
    g_list_store_remove (model, 0);

  g_assert_cmpint (1, ==, g_list_model_get_n_items (G_LIST_MODEL (model)));
  g_assert_cmpint (1, ==, g_list_model_get_n_items (G_LIST_MODEL (filter)));

  dzl_list_model_filter_set_filter_func (filter, NULL, NULL, NULL);
  g_assert_cmpint (1, ==, g_list_model_get_n_items (G_LIST_MODEL (model)));
  g_assert_cmpint (1, ==, g_list_model_get_n_items (G_LIST_MODEL (filter)));

  item = g_list_model_get_item (G_LIST_MODEL (filter), 0);
  g_assert (item);
  g_assert (TEST_IS_ITEM (item));
  g_assert_cmpint (item->n, ==, 1001);
  g_clear_object (&item);

  g_clear_object (&model);
  g_clear_object (&filter);
}

static guint last_n_added = 0;
static guint last_n_removed = 0;
static guint last_changed_position = 0;

static void
model_items_changed_cb (DzlListModelFilter *filter,
                        guint               position,
                        guint               n_removed,
                        guint               n_added,
                        GListModel         *model)
{
  last_n_added = n_added;
  last_n_removed = n_removed;
  last_changed_position = position;
}


static void
filter_items_changed_cb (DzlListModelFilter *filter,
                         guint               position,
                         guint               n_removed,
                         guint               n_added,
                         GListModel         *model)
{
  g_assert_cmpint (n_added, ==, last_n_added);
  g_assert_cmpint (n_removed, ==, last_n_removed);
  g_assert_cmpint (position, ==, last_changed_position);
}

static void
test_items_changed (void)
{
  DzlListModelFilter *filter;
  GListStore *model;
  guint i;

  model = g_list_store_new (TEST_TYPE_ITEM);
  g_assert (model);

  g_signal_connect (model, "items-changed", G_CALLBACK (model_items_changed_cb), NULL);

  filter = dzl_list_model_filter_new (G_LIST_MODEL (model));
  g_assert (filter);

  g_signal_connect_after (filter, "items-changed", G_CALLBACK (filter_items_changed_cb), model);

  /* The filter model is not filtered, so it must mirror whatever
   * the child model does. In this case, test if the position of
   * items-changed match.
   */
  for (i = 0; i < 100; i++)
    {
      g_autoptr (TestItem) val = test_item_new (i);
      g_list_store_append (model, val);
    }

  g_assert_cmpint (100, ==, g_list_model_get_n_items (G_LIST_MODEL (model)));
  g_assert_cmpint (100, ==, g_list_model_get_n_items (G_LIST_MODEL (filter)));

  for (i = 0; i < 100; i++)
    g_list_store_remove (model, 0);

  g_clear_object (&model);
  g_clear_object (&filter);
}


static guint items_to_remove = 0;
static gboolean items_changed_emitted = FALSE;

static void
filter_items_removed_cb (DzlListModelFilter *filter,
                         guint               position,
                         guint               n_removed,
                         guint               n_added,
                         GListModel         *model)
{
  items_changed_emitted = TRUE;

  g_assert_cmpint (n_removed, ==, items_to_remove);
}

static void
test_remove_all (void)
{
  DzlListModelFilter *filter;
  GListStore *model;
  guint i;

  model = g_list_store_new (TEST_TYPE_ITEM);
  g_assert (model);

  filter = dzl_list_model_filter_new (G_LIST_MODEL (model));
  g_assert (filter);

  /* The filter model is not filtered, so it must mirror whatever
   * the child model does. In this case, test if the position of
   * items-changed match.
   */
  for (i = 0; i < 100; i++)
    {
      g_autoptr (TestItem) val = test_item_new (i);

      g_list_store_append (model, val);

      items_to_remove++;
    }

  g_assert_cmpint (100, ==, g_list_model_get_n_items (G_LIST_MODEL (model)));
  g_assert_cmpint (100, ==, g_list_model_get_n_items (G_LIST_MODEL (filter)));

  g_signal_connect_after (filter, "items-changed", G_CALLBACK (filter_items_removed_cb), model);

  g_list_store_remove_all (model);

  g_assert_true (items_changed_emitted);

  g_clear_object (&model);
  g_clear_object (&filter);
}

gint
main (gint argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/ListModelFilter/basic", test_basic);
  g_test_add_func ("/Dazzle/ListModelFilter/items-changed", test_items_changed);
  g_test_add_func ("/Dazzle/ListModelFilter/remove-all", test_remove_all);
  return g_test_run ();
}
