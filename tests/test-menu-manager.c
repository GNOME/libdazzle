#include <dazzle.h>

static const gchar *
build_path (const gchar *name)
{
  g_autofree gchar *path = NULL;
  const gchar *ret;

  path = g_build_filename (TEST_DATA_DIR, name, NULL);
  ret = g_intern_string (path);

  return ret;
}

gint
main (gint   argc,
      gchar *argv[])
{
  DzlMenuManager *manager;
  GMenu *menu;
  GtkWidget *widget;
  GError *error = NULL;
  GMenu *top;

  gtk_init (&argc, &argv);

  manager = dzl_menu_manager_new ();

  dzl_menu_manager_add_filename (manager, build_path ("menus.ui"), &error);
  g_assert_no_error (error);

  dzl_menu_manager_add_filename (manager, build_path ("menus-exten-1.ui"), &error);
  g_assert_no_error (error);

  dzl_menu_manager_add_filename (manager, build_path ("menus-exten-2.ui"), &error);
  g_assert_no_error (error);

  dzl_menu_manager_add_filename (manager, build_path ("menus-exten-3.ui"), &error);
  g_assert_no_error (error);

  dzl_menu_manager_add_filename (manager, build_path ("menus-exten-4.ui"), &error);
  g_assert_no_error (error);

  dzl_menu_manager_add_filename (manager, build_path ("menus-exten-5.ui"), &error);
  g_assert_no_error (error);

  top = g_menu_new ();

  menu = dzl_menu_manager_get_menu_by_id (manager, "menu-1");
  g_menu_append_submenu (top, "menu-1", G_MENU_MODEL (menu));

  menu = dzl_menu_manager_get_menu_by_id (manager, "menu-2");
  g_menu_append_submenu (top, "menu-2", G_MENU_MODEL (menu));

  menu = dzl_menu_manager_get_menu_by_id (manager, "menu-3");
  g_menu_append_submenu (top, "menu-3", G_MENU_MODEL (menu));

  menu = dzl_menu_manager_get_menu_by_id (manager, "menu-4");
  g_menu_append_submenu (top, "menu-4", G_MENU_MODEL (menu));

  widget = gtk_menu_new_from_model (G_MENU_MODEL (top));
  gtk_menu_popup_at_pointer (GTK_MENU (widget), NULL);

  gtk_main ();

  return 0;
}
