#include <dazzle.h>

gint
main (gint argc,
      gchar *argv[])
{
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *label;

  gtk_init (&argc, &argv);

  window = g_object_new (GTK_TYPE_WINDOW,
                         "title", "Box Test",
                         NULL);
  box = g_object_new (DZL_TYPE_BOX,
                      "spacing", 12,
                      "visible", TRUE,
                      NULL);
  gtk_container_add (GTK_CONTAINER (window), box);

  for (guint i = 0; i < 10; i += 2)
    {
      g_autofree gchar *str = g_strdup_printf ("%u", i);

      label = g_object_new (GTK_TYPE_LABEL,
                            "label", str,
                            "visible", TRUE,
                            NULL);
      gtk_container_add_with_properties (GTK_CONTAINER (box), label,
                                         "position", i,
                                         NULL);
    }

  for (guint i = 1; i < 10; i += 2)
    {
      g_autofree gchar *str = g_strdup_printf ("%u", i);

      label = g_object_new (GTK_TYPE_LABEL,
                            "label", str,
                            "visible", TRUE,
                            NULL);
      gtk_container_add_with_properties (GTK_CONTAINER (box), label,
                                         "position", i,
                                         NULL);
    }

  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (GTK_WINDOW (window));
  gtk_main ();

  return 0;
}
