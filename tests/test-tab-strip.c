#include <dazzle.h>

gint
main (gint   argc,
      gchar *argv[])
{
  GtkBuilder *builder = NULL;
  GtkWindow *window = NULL;
  GError *error = NULL;
  g_autofree gchar *path = g_build_filename (TEST_DATA_DIR, "test-tab-strip.ui", NULL);

  gtk_init (&argc, &argv);

  g_type_ensure (DZL_TYPE_TAB_STRIP);

  builder = gtk_builder_new ();
  gtk_builder_add_from_file (builder, path, &error);
  g_assert_no_error (error);

  window = GTK_WINDOW (gtk_builder_get_object (builder, "window"));
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);

  gtk_window_present (window);
  gtk_main ();

  g_clear_object (&builder);

  return 0;
}
