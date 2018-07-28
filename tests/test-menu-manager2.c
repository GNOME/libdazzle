#include <dazzle.h>

#define assert_item_at_index(menu, i, label) \
G_STMT_START { \
  g_autofree gchar *str = NULL; \
  gboolean r = g_menu_model_get_item_attribute (G_MENU_MODEL (menu), i, "label", "s", &str); \
  g_assert (r); \
  g_assert_cmpstr (str, ==, label); \
} G_STMT_END

static void
test_menu_manager (void)
{
  g_autoptr(DzlMenuManager) manager = dzl_menu_manager_new ();
  g_autoptr(GMenu) menu1 = g_menu_new ();
  g_autoptr(GMenu) menu2 = g_menu_new ();
  g_autoptr(GMenuItem) item1 = g_menu_item_new ("item1", "item1");
  g_autoptr(GMenuItem) item2 = g_menu_item_new ("item2", "item2");
  g_autoptr(GMenuItem) item3 = g_menu_item_new ("item3", "item3");
  g_autoptr(GMenuItem) item4 = g_menu_item_new ("item4", "item4");
  g_autoptr(GMenuItem) item5 = g_menu_item_new ("item5", "item5");
  g_autoptr(GMenuItem) item6 = g_menu_item_new ("item6", "item6");
  GMenu *merged;
  guint n_items;

  g_menu_item_set_attribute (item1, "after", "s", "item3");
  g_menu_item_set_attribute (item1, "before", "s", "item5");

  g_menu_append_item (menu1, item1);
  g_menu_append_item (menu1, item2);
  g_menu_append_item (menu1, item3);

  dzl_menu_manager_merge (manager, "menu", G_MENU_MODEL (menu1));

  merged = dzl_menu_manager_get_menu_by_id (manager, "menu");

  assert_item_at_index (merged, 0, "item2");
  assert_item_at_index (merged, 1, "item3");
  assert_item_at_index (merged, 2, "item1");

  g_menu_item_set_attribute (item4, "after", "s", "item2");
  g_menu_item_set_attribute (item4, "before", "s", "item3");

  g_menu_append_item (menu2, item4);
  g_menu_append_item (menu2, item5);
  g_menu_append_item (menu2, item6);

  dzl_menu_manager_merge (manager, "menu", G_MENU_MODEL (menu2));

  n_items = g_menu_model_get_n_items (G_MENU_MODEL (merged));
  g_assert_cmpint (n_items, ==, 6);

  assert_item_at_index (merged, 0, "item2");
  assert_item_at_index (merged, 1, "item4");
  assert_item_at_index (merged, 2, "item3");
  assert_item_at_index (merged, 3, "item1");
  assert_item_at_index (merged, 4, "item5");
  assert_item_at_index (merged, 5, "item6");

  g_object_run_dispose (G_OBJECT (manager));
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/MenuManager/basic", test_menu_manager);
  return g_test_run ();
}
