#include <dazzle.h>

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

gint
main (gint   argc,
      gchar *argv[])
{
  gtk_init (&argc, &argv);
  g_test_init (&argc, &argv, NULL);
  g_test_add_func ("/Dazzle/ShortcutTheme/basic", test_shortcut_theme_basic);
  return g_test_run ();
}
