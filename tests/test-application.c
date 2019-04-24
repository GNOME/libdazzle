#include <dazzle.h>
#include <stdlib.h>

static gint    g_argc;
static gchar **g_argv;

static gboolean
on_timeout (gpointer data)
{
  DzlApplication *app = data;
  g_application_release (G_APPLICATION (app));
  return G_SOURCE_REMOVE;
}

static void
on_activate (DzlApplication *app)
{
  g_timeout_add_full (G_PRIORITY_LOW, 10, on_timeout, g_object_ref (app), g_object_unref);
  g_application_hold (G_APPLICATION (app));
}

static void
test_app_basic (void)
{
  g_autoptr(DzlApplication) app = NULL;
  int ret;

  app = dzl_application_new ("org.gnome.FooTest", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (on_activate), NULL);
  ret = g_application_run (G_APPLICATION (app), g_argc, g_argv);
  g_assert_cmpint (ret, ==, EXIT_SUCCESS);
}

gint
main (gint    argc,
      gchar **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_argc = argc;
  g_argv = argv;

  g_test_add_func ("/Dazzle/Application/basic", test_app_basic);

  return g_test_run ();
}
