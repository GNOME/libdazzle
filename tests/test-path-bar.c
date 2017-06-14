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
  GtkWidget *headerbar;
  GtkWidget *pathbar;
  DzlPath *path;
  DzlPathElement *ele0;
  DzlPathElement *ele1;
  DzlPathElement *ele2;
  DzlPathElement *ele3;

  gtk_init (&argc, &argv);

  load_css ();

  window = g_object_new (GTK_TYPE_WINDOW,
                         "default-width", 600,
                         "default-height", 200,
                         "title", "Test Path Bar",
                         NULL);
  headerbar = g_object_new (GTK_TYPE_HEADER_BAR,
                            "show-close-button", TRUE,
                            "visible", TRUE,
                            NULL);
  gtk_window_set_titlebar (GTK_WINDOW (window), headerbar);

  pathbar = g_object_new (DZL_TYPE_PATH_BAR,
                          "hexpand", TRUE,
                          "visible", TRUE,
                          NULL);
  gtk_container_add (GTK_CONTAINER (headerbar), pathbar);

  path = dzl_path_new ();
  g_assert (dzl_path_is_empty (path));
  g_assert_cmpint (dzl_path_get_length (path), ==, 0);
  g_assert (NULL == dzl_path_get_elements (path));

  ele1 = dzl_path_element_new ("home", "user-home-symbolic", "Home");
  dzl_path_append (path, ele1);
  g_assert (NULL != dzl_path_get_elements (path));
  g_assert (dzl_path_get_elements (path)->data == ele1);

  ele2 = dzl_path_element_new ("christian", NULL, "Xtian");
  dzl_path_append (path, ele2);
  g_assert (dzl_path_get_elements (path)->next->data == ele2);
  g_assert (dzl_path_get_elements (path)->next->next == NULL);

  ele3 = dzl_path_element_new ("music", NULL, "Music");
  dzl_path_append (path, ele3);
  g_assert (dzl_path_get_elements (path)->data == ele1);
  g_assert (dzl_path_get_elements (path)->next->data == ele2);
  g_assert (dzl_path_get_elements (path)->next->next->data == ele3);
  g_assert (dzl_path_get_elements (path)->next->next->next == NULL);

  ele0 = dzl_path_element_new ("computer", "computer-symbolic", "Computer");
  dzl_path_prepend (path, ele0);
  g_assert (dzl_path_get_elements (path)->data == ele0);
  g_assert (dzl_path_get_elements (path)->next->data == ele1);

  dzl_path_bar_set_path (DZL_PATH_BAR (pathbar), path);

  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (GTK_WINDOW (window));

  g_object_add_weak_pointer (G_OBJECT (pathbar), (gpointer *)&pathbar);

  gtk_main ();

  g_object_add_weak_pointer (G_OBJECT (path), (gpointer *)&path);
  g_object_add_weak_pointer (G_OBJECT (ele0), (gpointer *)&ele0);
  g_object_add_weak_pointer (G_OBJECT (ele1), (gpointer *)&ele1);
  g_object_add_weak_pointer (G_OBJECT (ele2), (gpointer *)&ele2);
  g_object_add_weak_pointer (G_OBJECT (ele3), (gpointer *)&ele3);

  /* ensure we clear path references */
  if (pathbar)
    dzl_path_bar_set_path (DZL_PATH_BAR (pathbar), NULL);

  g_object_unref (path);
  g_object_unref (ele0);
  g_object_unref (ele1);
  g_object_unref (ele2);
  g_object_unref (ele3);

  g_assert (path == NULL);
  g_assert (ele0 == NULL);
  g_assert (ele1 == NULL);
  g_assert (ele2 == NULL);
  g_assert (ele3 == NULL);

  return 0;
}
