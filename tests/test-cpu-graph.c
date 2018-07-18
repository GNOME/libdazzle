#include <dazzle.h>
#include <stdlib.h>

int
main (int argc,
      char *argv[])
{
  guint samples = 2;
  guint seconds = 30;
  const GOptionEntry entries[] = {
    { "samples", 'm', 0, G_OPTION_ARG_INT, &samples, "Number of samples per second", "2" },
    { "seconds", 's', 0, G_OPTION_ARG_INT, &seconds, "Number of seconds to display", "60" },
    { NULL }
  };
  gint64 timespan;
  guint max_samples;
  GOptionContext *context;
  GtkWindow *window;
  GtkBox *box;
  GtkCssProvider *provider;
  GError *error = NULL;

  context = g_option_context_new ("- a simple cpu graph");
  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  g_option_context_add_main_entries (context, entries, NULL);

  if (!g_option_context_parse (context, &argc, &argv, &error))
    {
      g_printerr ("%s\n", error->message);
      return EXIT_FAILURE;
    }

  g_print ("%d samples per second over %d seconds.\n",
           samples, seconds);

  timespan = (gint64)seconds * G_USEC_PER_SEC;
  max_samples = seconds * samples;

  provider = dzl_css_provider_new ("resource:///org/gnome/dazzle/themes");
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (), GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref (provider);

  window = g_object_new (GTK_TYPE_WINDOW,
                         "default-width", 600,
                         "default-height", 325,
                         "title", "CPU Graph",
                         NULL);

  box = g_object_new (GTK_TYPE_BOX,
                      "orientation", GTK_ORIENTATION_VERTICAL,
                      "visible", TRUE,
                      "spacing", 3,
                      NULL);
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (box));

  for (int i = 0; i < 3; i++)
    {
      GtkWidget *graph = dzl_cpu_graph_new_full (timespan, max_samples);

      gtk_widget_set_vexpand (graph, TRUE);
      gtk_widget_set_visible (graph, TRUE);

      gtk_container_add (GTK_CONTAINER (box), graph);
    }

  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (window);
  gtk_main ();

  return 0;
}
