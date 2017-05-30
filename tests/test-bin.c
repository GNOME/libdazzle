#include <dazzle.h>

gint
main (gint argc,
      gchar *argv[])
{
  GtkWindow *win;

  gtk_init (&argc, &argv);

  win = g_object_new (GTK_TYPE_WINDOW,
                      "title", "Test Bin",
                      NULL);
  gtk_container_add (GTK_CONTAINER (win),
                     g_object_new (DZL_TYPE_BIN,
                                   "visible", TRUE,
                                   "child", g_object_new (GTK_TYPE_LABEL,
                                                          "label", "Some label for the bin",
                                                          "visible", TRUE,
                                                          NULL),
                                   NULL)
                     );

  g_signal_connect (win, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (win);
  gtk_main ();

  return 0;
}
