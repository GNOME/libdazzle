#include "example-application.h"

int
main (int argc,
      char *argv[])
{
  g_autoptr(ExampleApplication) app = NULL;
  gint ret;

  app = g_object_new (EXAMPLE_TYPE_APPLICATION,
                      "application-id", "org.gnome.Example",
                      "resource-base-path", "/org/gnome/example",
                      NULL);
  ret = g_application_run (G_APPLICATION (app), argc, argv);

  return ret;
}
