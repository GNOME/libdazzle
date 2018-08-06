#include <dazzle.h>

#include "shortcuts/dzl-shortcut-private.h"

static void
test_shortcut_theme_basic (void)
{
  DzlShortcutTheme *theme;
  DzlShortcutContext *context;
  g_autofree gchar *path = NULL;
  GError *error = NULL;
  gboolean r;

  theme = dzl_shortcut_theme_new (NULL);

  path = g_build_filename (TEST_DATA_DIR, "keythemes", "test.keytheme", NULL);

  if (!dzl_shortcut_theme_load_from_path (theme, path, NULL, &error))
    g_error ("%s", error->message);

  g_assert_cmpstr ("test", ==, dzl_shortcut_theme_get_name (theme));
  g_assert_cmpstr ("Test", ==, dzl_shortcut_theme_get_title (theme));
  g_assert_cmpstr ("Test theme", ==, dzl_shortcut_theme_get_subtitle (theme));

  context = dzl_shortcut_theme_find_context_by_name (theme, "GtkWindow");
  g_assert (context != NULL);
  g_assert (DZL_IS_SHORTCUT_CONTEXT (context));

  r = dzl_shortcut_theme_save_to_path (theme, "saved.keytheme", NULL, &error);
  g_assert (r);
  g_assert_no_error (error);

  g_object_add_weak_pointer (G_OBJECT (context), (gpointer *)&context);
  g_object_add_weak_pointer (G_OBJECT (theme), (gpointer *)&theme);
  g_object_unref (theme);
  g_assert (theme == NULL);
  g_assert (context == NULL);
}

static void
key_callback (GtkWidget *widget,
              gpointer   data)
{
  gboolean *did_cb = data;
  *did_cb = TRUE;
  g_assert (GTK_IS_LABEL (widget));
}

static void
test_shortcut_theme_manager (void)
{
  DzlShortcutController *controller;
  DzlShortcutManager *manager;
  DzlShortcutTheme *theme;
  GtkWidget *window;
  GtkWidget *label;
  GdkEventKey *event;
  GError *error = NULL;
  gboolean did_cb = FALSE;
  gboolean r;

  manager = dzl_shortcut_manager_get_default ();
  g_assert (DZL_IS_SHORTCUT_MANAGER (manager));

  theme = dzl_shortcut_manager_get_theme_by_name (manager, NULL);
  g_assert (DZL_IS_SHORTCUT_THEME (theme));
  g_assert (theme == dzl_shortcut_manager_get_theme_by_name (manager, "internal"));
  g_assert_cmpstr (dzl_shortcut_theme_get_parent_name (theme), ==, NULL);

  theme = dzl_shortcut_manager_get_theme_by_name (manager, "default");
  g_assert (theme == NULL);

  g_assert (G_IS_INITABLE (manager));
  r = g_initable_init (G_INITABLE (manager), NULL, &error);
  g_assert_no_error (error);
  g_assert_cmpint (r, ==, TRUE);

  theme = dzl_shortcut_manager_get_theme_by_name (manager, "default");
  g_assert (DZL_IS_SHORTCUT_THEME (theme));
  g_assert_cmpstr ("internal", ==, dzl_shortcut_theme_get_parent_name (theme));

  /* Add a command and make sure we can resolve it */
  window = gtk_offscreen_window_new ();
  label = gtk_label_new (NULL);
  gtk_container_add (GTK_CONTAINER (window), label);
  gtk_widget_show (label);
  gtk_widget_show (window);
  controller = dzl_shortcut_controller_find (label);
  g_assert (DZL_IS_SHORTCUT_CONTROLLER (controller));
  dzl_shortcut_controller_add_command_callback (controller, "useless.command.here", "<Control>a", DZL_SHORTCUT_PHASE_GLOBAL, key_callback, &did_cb, NULL);
  event = dzl_gdk_synthesize_event_keyval (gtk_widget_get_window (label), GDK_KEY_a);
  event->state |= GDK_CONTROL_MASK;
  r = dzl_shortcut_manager_handle_event (NULL, event, window);
  g_assert_cmpint (did_cb, ==, TRUE);
  g_assert_cmpint (r, ==, GDK_EVENT_STOP);
}

gint
main (gint   argc,
      gchar *argv[])
{
  gtk_init (&argc, &argv);
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/ShortcutTheme/basic", test_shortcut_theme_basic);
  g_test_add_func ("/Dazzle/ShortcutTheme/manager", test_shortcut_theme_manager);
  return g_test_run ();
}
