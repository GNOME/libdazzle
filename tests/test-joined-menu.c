#include <dazzle.h>

static void
load_css (void)
{
  g_autoptr(GtkCssProvider) provider = NULL;

  provider = dzl_css_provider_new ("resource:///org/gnome/dazzle/themes");
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

gint
main (gint argc,
      gchar *argv[])
{
  GtkWidget *window;
  GtkWidget *menu_button;
  g_autoptr(DzlMenuManager) manager = NULL;
  g_autoptr(DzlJoinedMenu) joined = NULL;
  g_autoptr(GError) error = NULL;
  g_autofree gchar *filename1 = g_build_filename (TEST_DATA_DIR, "menus/joined1.ui", NULL);
  g_autofree gchar *filename2 = g_build_filename (TEST_DATA_DIR, "menus/joined2.ui", NULL);
  GMenu *menu1;
  GMenu *menu2;

  gtk_init (&argc, &argv);

  load_css ();

  manager = dzl_menu_manager_new ();

  dzl_menu_manager_add_filename (manager, filename1, &error);
  g_assert_no_error (error);
  menu1 = dzl_menu_manager_get_menu_by_id (manager, "document-menu");

  {
    g_autofree gchar *icon = NULL;
    g_autoptr(GMenuModel) section = NULL;
    gboolean r;

    section = g_menu_model_get_item_link (G_MENU_MODEL (menu1), 0, "section");
    g_assert (section != NULL);
    g_assert_cmpint (g_menu_model_get_n_items (section), ==, 3);

    r = g_menu_model_get_item_attribute (section, 0, "verb-icon-name", "s", &icon);
    g_assert_true (r);
    g_assert_cmpstr (icon, ==, "document-open-symbolic");
  }

  dzl_menu_manager_add_filename (manager, filename2, &error);
  g_assert_no_error (error);
  menu2 = dzl_menu_manager_get_menu_by_id (manager, "frame-menu");

  joined = dzl_joined_menu_new ();
  dzl_joined_menu_append_menu (joined, G_MENU_MODEL (menu1));
  dzl_joined_menu_append_menu (joined, G_MENU_MODEL (menu2));

  {
    g_autofree gchar *icon = NULL;
    g_autoptr(GMenuModel) section = NULL;
    gboolean r;

    section = g_menu_model_get_item_link (G_MENU_MODEL (joined), 0, "section");
    g_assert (section != NULL);
    g_assert_cmpint (g_menu_model_get_n_items (section), ==, 3);

    r = g_menu_model_get_item_attribute (section, 0, "verb-icon-name", "s", &icon);
    g_assert_true (r);
    g_assert_cmpstr (icon, ==, "document-open-symbolic");
  }

  window = g_object_new (GTK_TYPE_WINDOW,
                         "default-width", 100,
                         "default-height", 100,
                         "border-width", 32,
                         "title", "Joined Menu Test",
                         NULL);
  menu_button = g_object_new (DZL_TYPE_MENU_BUTTON,
                              "show-arrow", TRUE,
                              "show-accels", TRUE,
                              "show-icons", TRUE,
                              "icon-name", "document-open-symbolic",
                              "model", joined,
                              "visible", TRUE,
                              NULL);
  gtk_container_add (GTK_CONTAINER (window), menu_button);
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (GTK_WINDOW (window));

  gtk_main ();

  return 0;
}
