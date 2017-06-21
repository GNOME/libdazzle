#include <dazzle.h>

static void
swap_orientation (GtkWidget *button,
                  GtkWidget *paned)
{
  GtkOrientation orientation;

  orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (paned));
  gtk_orientable_set_orientation (GTK_ORIENTABLE (paned), !orientation);
}

gint
main (gint   argc,
      gchar *argv[])
{
  GtkBuilder *builder = NULL;
  GtkWindow *window = NULL;
  GError *error = NULL;
  GObject *button;
  GObject *paned;
  g_autofree gchar *path = g_build_filename (TEST_DATA_DIR, "test-multi-paned.ui", NULL);

  gtk_init (&argc, &argv);

  g_type_ensure (DZL_TYPE_MULTI_PANED);

  builder = gtk_builder_new ();
  gtk_builder_add_from_file (builder, path, &error);
  g_assert_no_error (error);

  paned = gtk_builder_get_object (builder, "paned");
  button = gtk_builder_get_object (builder, "swap");
  g_signal_connect (button, "clicked", G_CALLBACK (swap_orientation), paned);

  window = GTK_WINDOW (gtk_builder_get_object (builder, "window"));
  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);

  gtk_window_present (window);
  gtk_main ();

  g_clear_object (&builder);

  return 0;
}
