#include <dazzle.h>

static void
ensure_menu_merging (DzlApplication *app)
{
  g_autofree gchar *id1 = NULL;
  g_autofree gchar *id2 = NULL;
  g_autofree gchar *id3 = NULL;
  g_autofree gchar *label1 = NULL;
  g_autoptr(GMenuModel) link1 = NULL;
  g_autoptr(GMenuModel) link2 = NULL;
  GMenu *menu;
  guint len;
  gboolean r;

  menu = dzl_application_get_menu_by_id (app, "app-menu");
  g_assert (G_IS_MENU (menu));

  len = g_menu_model_get_n_items (G_MENU_MODEL (menu));
  g_assert_cmpint (len, ==, 4);

  g_assert (NULL != g_menu_model_get_item_link (G_MENU_MODEL (menu), 0, G_MENU_LINK_SECTION));

  link1 = g_menu_model_get_item_link (G_MENU_MODEL (menu), 1, G_MENU_LINK_SECTION);
  g_assert (link1 != NULL);
  g_assert_cmpint (g_menu_model_get_n_items (link1), ==, 2);

  r = g_menu_model_get_item_attribute (link1, 0, "id", "s", &id1);
  g_assert_cmpint (r, ==, TRUE);
  g_assert_cmpstr (id1, ==, "section-2-item-1");

  r = g_menu_model_get_item_attribute (link1, 0, "label", "s", &label1);
  g_assert_cmpint (r, ==, TRUE);
  g_assert_cmpstr (label1, ==, "Item 2.1");

  r = g_menu_model_get_item_attribute (link1, 1, "id", "s", &id2);
  g_assert_cmpint (r, ==, TRUE);
  g_assert_cmpstr (id2, ==, "section-2-item-2");

  g_assert (NULL != g_menu_model_get_item_link (G_MENU_MODEL (menu), 2, G_MENU_LINK_SECTION));

  link2 = g_menu_model_get_item_link (G_MENU_MODEL (menu), 3, G_MENU_LINK_SECTION);
  g_assert (link2 != NULL);
  g_assert_cmpint (g_menu_model_get_n_items (link2), ==, 1);

  r = g_menu_model_get_item_attribute (link2, 0, "id", "s", &id3);
  g_assert_cmpint (r, ==, TRUE);
  g_assert_cmpstr (id3, ==, "section-4-item-1");
}

static void
ensure_keybinding_merging (DzlApplication *app)
{
  DzlShortcutManager *manager;
  DzlShortcutTheme *theme;

  g_assert (DZL_IS_APPLICATION (app));

  manager = dzl_application_get_shortcut_manager (app);
  g_assert (DZL_IS_SHORTCUT_MANAGER (manager));

  theme = dzl_shortcut_manager_get_theme_by_name (manager, "default");
  g_assert (DZL_IS_SHORTCUT_THEME (theme));

  theme = dzl_shortcut_manager_get_theme_by_name (manager, "secondary");
  g_assert (DZL_IS_SHORTCUT_THEME (theme));
  g_assert_cmpstr (dzl_shortcut_theme_get_parent_name (theme), ==, "default");
}

static void
on_activate (DzlApplication *app)
{
  ensure_menu_merging (app);
  ensure_keybinding_merging (app);
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_autoptr(DzlApplication) app = NULL;

  app = g_object_new (DZL_TYPE_APPLICATION,
                      "application-id", "org.gnome.Dazzle.Tests.Shortcuts",
                      "flags", G_APPLICATION_NON_UNIQUE,
                      NULL);

  /* Queue resource adding, which will happen for real during startup */
  dzl_application_add_resources (app, TEST_DATA_DIR"/shortcuts/0");
  dzl_application_add_resources (app, TEST_DATA_DIR"/shortcuts/1");
  dzl_application_add_resources (app, TEST_DATA_DIR"/shortcuts/2");

  g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);

  return g_application_run (G_APPLICATION (app), argc, argv);
}
