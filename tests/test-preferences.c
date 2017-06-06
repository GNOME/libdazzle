#include <dazzle.h>

static const gchar *themes[] = {
  "Applications",
  "Cursor",
  "Icons",
  "System",
  NULL
};

static void
add_preferences (DzlPreferences *prefs)
{
  dzl_preferences_add_page (prefs, "appearance", "Appearance", 0);
  dzl_preferences_add_page (prefs, "desktop", "Desktop", 1);
  dzl_preferences_add_page (prefs, "extensions", "Extensions", 2);
  dzl_preferences_add_page (prefs, "fonts", "Fonts", 3);
  dzl_preferences_add_page (prefs, "keyboard", "Keyboard & Mouse", 4);
  dzl_preferences_add_page (prefs, "power", "Power", 5);
  dzl_preferences_add_page (prefs, "startup", "Startup Applications", 6);
  dzl_preferences_add_page (prefs, "topbar", "Top Bar", 7);
  dzl_preferences_add_page (prefs, "windows", "Windows", 7);
  dzl_preferences_add_page (prefs, "workspaces", "Workspaces", 8);

  dzl_preferences_add_group (prefs, "appearance", "basic", NULL, 0);
  dzl_preferences_add_switch (prefs, "appearance", "basic", "com.example", "foo", NULL, NULL, "Global Dark Theme", "Applications need to be restarted for this change to take place", "dark theme", 0);
  dzl_preferences_add_switch (prefs, "appearance", "basic", "com.example", "foo", NULL, NULL, "Animations", NULL, "animations", 1);

  dzl_preferences_add_list_group (prefs, "appearance", "themes", "Themes", GTK_SELECTION_NONE, 10);

  for (guint i = 0; themes[i]; i++)
    dzl_preferences_add_custom (prefs, "appearance", "themes",
                                g_object_new (GTK_TYPE_LABEL,
                                              "label", themes[i],
                                              "visible", TRUE,
                                              "xalign", 0.0f,
                                              NULL),
                                themes[i], i);

  dzl_preferences_add_group (prefs, "appearance", "install", NULL, 20);
  dzl_preferences_add_custom (prefs, "appearance", "install",
                              g_object_new (GTK_TYPE_BUTTON,
                                            "label", "Install from File…",
                                            "halign", GTK_ALIGN_END,
                                            "visible", TRUE,
                                            NULL),
                              NULL, 0);

  dzl_preferences_set_page (prefs, "appearance", NULL);
}

gint
main (gint   argc,
      gchar *argv[])
{
  GtkWidget *window;
  GtkWidget *view;

  gtk_init (&argc, &argv);

  window = g_object_new (GTK_TYPE_WINDOW,
                         "title", "Preferences Test",
                         "default-width", 800,
                         "default-height", 600,
                         NULL);

  view = g_object_new (DZL_TYPE_PREFERENCES_VIEW,
                       "visible", TRUE,
                       NULL);
  gtk_container_add (GTK_CONTAINER (window), view);

  add_preferences (DZL_PREFERENCES (view));

  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (GTK_WINDOW (window));
  return gtk_main (), 0;
}
