#include <dazzle.h>

gint
main (gint argc,
      gchar *argv[])
{
  GtkWidget *window;
  GtkWidget *state;

  gtk_init (&argc, &argv);

  window = g_object_new (GTK_TYPE_WINDOW,
                         "default-width", 800,
                         "default-height", 600,
                         "title", "Test Empty State",
                         NULL);

  state = g_object_new (DZL_TYPE_EMPTY_STATE,
                        "icon-name", "face-sick-symbolic",
                        "pixel-size", 192,
                        "visible", TRUE,
                        "title", "No Recordings",
                        "subtitle", "Click <a href=\"action:something\">record</a> to begin a new recording",
                        NULL);
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (state));

  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (GTK_WINDOW (window));

  gtk_main ();

  return 0;
}
