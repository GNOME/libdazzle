#include <dazzle.h>

static const gchar *
build_path (const gchar *name)
{
  g_autofree gchar *path = NULL;
  const gchar *ret;

  path = g_build_filename (TEST_DATA_DIR, "menus", name, NULL);
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
  GtkWindow *window;
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

  window = g_object_new (GTK_TYPE_WINDOW,
                         "title", "Test Window",
                         NULL);
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);

  widget = gtk_menu_bar_new_from_model (G_MENU_MODEL (top));
  gtk_widget_set_halign (widget, GTK_ALIGN_START);
  gtk_widget_set_valign (widget, GTK_ALIGN_START);
  gtk_container_add (GTK_CONTAINER (window), widget);
  gtk_widget_show (widget);

  gtk_window_present (window);

  gtk_main ();

  return 0;
}
