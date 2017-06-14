#include <dazzle.h>

static const gchar *words[] = {
  "Autotools",
  "Meson",
  "CMake",
  "waf",
  "scons",
  "shell",
  NULL
};

static void
load_css (void)
{
  g_autoptr(GtkCssProvider) provider = NULL;

  provider = dzl_css_provider_new ("resource:///org/gnome/dazzle/themes");
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

int
main (int argc,
      char *argv[])
{
  GtkWidget *window;
  GtkWidget *box;
  GtkWidget *pill;

  gtk_init (&argc, &argv);

  load_css ();

  window = g_object_new (GTK_TYPE_WINDOW,
                         "title", "Test PillBox",
                         "border-width", 24,
                         NULL);
  box = g_object_new (GTK_TYPE_BOX,
                      "spacing", 3,
                      "visible", TRUE,
                      NULL);
  gtk_container_add (GTK_CONTAINER (window), box);

  for (guint i = 0; words[i]; i++)
    {
      pill = g_object_new (DZL_TYPE_PILL_BOX,
                           "label", words[i],
                           "visible", TRUE,
                           NULL);
      gtk_container_add (GTK_CONTAINER (box), pill);
    }

  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (GTK_WINDOW (window));

  gtk_main ();

  return 0;
}
