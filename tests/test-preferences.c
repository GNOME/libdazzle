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
  dzl_preferences_add_table_row (prefs, "appearance", "basic",
                                 g_object_new (GTK_TYPE_LABEL,
                                               "label", "Key",
                                               "visible", TRUE,
                                               "width-chars", 15,
                                               "xalign", 0.0f,
                                               NULL),
                                 g_object_new (GTK_TYPE_LABEL,
                                               "hexpand", TRUE,
                                               "xalign", 0.0f,
                                               "label", "Value",
                                               "visible", TRUE,
                                               NULL),
                                 NULL);
  dzl_preferences_add_table_row (prefs, "appearance", "basic",
                                 g_object_new (GTK_TYPE_LABEL,
                                               "label", "Key 2",
                                               "visible", TRUE,
                                               "xalign", 0.0f,
                                               NULL),
                                 g_object_new (GTK_TYPE_LABEL,
                                               "xalign", 0.0f,
                                               "label", "Value 2",
                                               "visible", TRUE,
                                               NULL),
                                 NULL);

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

static void
load_css (void)
{
  g_autoptr(GtkCssProvider) provider = NULL;

  provider = dzl_css_provider_new ("resource:///org/gnome/dazzle/themes");
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

gint
main (gint   argc,
      gchar *argv[])
{
  GtkWidget *window;
  GtkWidget *headerbar;
  GtkWidget *toggle;
  GtkWidget *view;

  gtk_init (&argc, &argv);

  load_css ();

  window = g_object_new (GTK_TYPE_WINDOW,
                         "title", "Preferences Test",
                         "default-width", 800,
                         "default-height", 600,
                         NULL);

  headerbar = g_object_new (GTK_TYPE_HEADER_BAR,
                            "show-close-button", TRUE,
                            "visible", TRUE,
                            NULL);
  gtk_window_set_titlebar (GTK_WINDOW (window), headerbar);

  view = g_object_new (DZL_TYPE_PREFERENCES_VIEW,
                       "visible", TRUE,
                       NULL);
  gtk_container_add (GTK_CONTAINER (window), view);

  toggle = g_object_new (GTK_TYPE_SWITCH,
                         "visible", TRUE,
                         NULL);
  g_object_bind_property (view, "use-sidebar", toggle, "active",
                          G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);
  gtk_container_add (GTK_CONTAINER (headerbar), toggle);

  add_preferences (DZL_PREFERENCES (view));

  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (GTK_WINDOW (window));
  return gtk_main (), 0;
}
