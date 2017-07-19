#include <dazzle.h>

typedef struct
{
  GtkWindow              *window;
  GtkHeaderBar           *header;
  GtkSearchEntry         *search;
  DzlShortcutController  *root_controller;
  DzlShortcutController  *search_controller;
  GtkStack               *stack;
  GtkStackSwitcher       *stack_switcher;
  DzlShortcutThemeEditor *editor;
  GtkButton              *shortcuts;
  GtkLabel               *message;
} App;

static const DzlShortcutEntry entries[] = {
  { "a.b.c.a", 0, "<Control>l", "Editor", "Navigation", "Move to next error" },
  { "a.b.c.b", 0, "<Control>k", "Editor", "Navigation", "Move to previous error" },
};

static void
on_shortcuts_clicked (GtkButton *button,
                      App       *app)
{
  DzlShortcutsWindow *window;

  g_assert (GTK_IS_BUTTON (button));
  g_assert (app != NULL);

  window = g_object_new (DZL_TYPE_SHORTCUTS_WINDOW,
                         "transient-for", app->window,
                         "modal", TRUE,
                         NULL);

  dzl_shortcut_manager_add_shortcuts_to_window (NULL, window);

  gtk_window_present (GTK_WINDOW (window));
}

static void
on_current_chord_notify (DzlShortcutController *controller,
                         GParamSpec            *pspec,
                         App                   *app)
{
  g_autofree gchar *str = NULL;
  const DzlShortcutChord *chord;

  g_assert (DZL_IS_SHORTCUT_CONTROLLER (controller));
  g_assert (app != NULL);
  g_assert (GTK_IS_LABEL (app->message));

  chord = dzl_shortcut_controller_get_current_chord (controller);
  str = dzl_shortcut_chord_get_label (chord);

  gtk_label_set_label (app->message, str);
}

static void
test_callback (GtkWidget *widget,
               gpointer   user_data)
{
  g_assert (GTK_IS_WIDGET (widget));
  g_assert (user_data == NULL);

  g_print ("test_callback\n");
}

static void
a_b_c_a (GtkWidget *widget,
         gpointer   user_data)
{
  g_assert (GTK_IS_WIDGET (widget));
  g_assert (user_data == NULL);

  g_print ("a_b_c_a\n");
}

gint
main (gint argc,
      gchar *argv[])
{
  DzlShortcutManager *manager;
  g_autoptr(DzlShortcutTheme) theme = NULL;
  App app = { 0 };
  GtkWidget *box;
  GtkWidget *separator;

#if GTK_CHECK_VERSION(3,89,0)
  gtk_init ();
#else
  gtk_init (&argc, &argv);
#endif

  manager = dzl_shortcut_manager_get_default ();

  dzl_shortcut_manager_append_search_path (manager, TEST_DATA_DIR"/keythemes");
  g_initable_init (G_INITABLE (manager), NULL, NULL);

  app.window = g_object_new (GTK_TYPE_WINDOW,
                             "default-width", 800,
                             "default-height", 600,
                             "title", "Shortcuts Test",
                             NULL);
  app.root_controller = dzl_shortcut_controller_new (GTK_WIDGET (app.window));
  dzl_shortcut_controller_add_command_callback (app.root_controller, "a.b.c.a", NULL, 0, a_b_c_a, NULL, NULL);

  g_signal_connect (app.root_controller,
                    "notify::current-chord",
                    G_CALLBACK (on_current_chord_notify),
                    &app);

  theme = g_object_ref (dzl_shortcut_manager_get_theme (manager));

  app.header = g_object_new (GTK_TYPE_HEADER_BAR,
                             "show-close-button", TRUE,
                             "visible", TRUE,
                             NULL);

  app.search = g_object_new (GTK_TYPE_SEARCH_ENTRY,
                             "placeholder-text", "ctrl+y ctrl+y to focus",
                             "width-chars", 30,
                             "visible", TRUE,
                             NULL);

  gtk_window_set_titlebar (app.window, GTK_WIDGET (app.header));
  gtk_container_add (GTK_CONTAINER (app.header), GTK_WIDGET (app.search));

  /*
   * This is basically like adding to a binding set, but it allows registering
   * a "command" to the signal so that users can override the command in the
   * context.
   *
   * We also define info so that it can be displayed in the shortcuts window.
   * However that work still needs to be done.
   */
  app.search_controller = dzl_shortcut_controller_new (GTK_WIDGET (app.search));
  dzl_shortcut_controller_add_command_signal (app.search_controller,
                                              "com.example.foo.search",
                                              "<ctrl>y|<ctrl>y",
                                              DZL_SHORTCUT_PHASE_GLOBAL,
                                              "grab-focus",
                                              0);
  dzl_shortcut_controller_add_command_callback (app.search_controller,
                                                "com.example.foo.test",
                                                "<ctrl>x|<ctrl>r",
                                                DZL_SHORTCUT_PHASE_GLOBAL,
                                                test_callback, NULL, NULL);

  app.shortcuts = g_object_new (GTK_TYPE_BUTTON,
                                "label", "Shortcuts",
                                "visible", TRUE,
                                NULL);
  g_signal_connect (app.shortcuts,
                    "clicked",
                    G_CALLBACK (on_shortcuts_clicked),
                    &app);
  gtk_container_add (GTK_CONTAINER (app.header), GTK_WIDGET (app.shortcuts));

  box = g_object_new (GTK_TYPE_BOX,
                      "orientation", GTK_ORIENTATION_VERTICAL,
                      "visible", TRUE,
                      NULL);
  app.stack = g_object_new (GTK_TYPE_STACK,
                            "expand", TRUE,
                            "visible", TRUE,
                            NULL);
  app.stack_switcher = g_object_new (GTK_TYPE_STACK_SWITCHER,
                                     "margin-top", 6,
                                     "margin-bottom", 6,
                                     "orientation", GTK_ORIENTATION_HORIZONTAL,
                                     "halign", GTK_ALIGN_CENTER,
                                     "hexpand", TRUE,
                                     "stack", app.stack,
                                     "visible", FALSE,
                                     NULL);

  app.editor = g_object_new (DZL_TYPE_SHORTCUT_THEME_EDITOR,
                             "theme", dzl_shortcut_manager_get_theme (NULL),
                             "width-request", 600,
                             "halign", GTK_ALIGN_CENTER,
                             "margin", 32,
                             "vexpand", TRUE,
                             "visible", TRUE,
                             NULL);
  gtk_container_add_with_properties (GTK_CONTAINER (app.stack), GTK_WIDGET (app.editor),
                                     "name", "editor",
                                     "title", "Shortcuts Editor",
                                     "position", 0,
                                     NULL);

  separator = g_object_new (GTK_TYPE_SEPARATOR,
                            "visible", TRUE,
                            NULL);

  app.message = g_object_new (GTK_TYPE_LABEL,
                              "label", "Ready.",
                              "margin", 6,
                              "xalign", 0.0f,
                              "visible", TRUE,
                              NULL);

  gtk_container_add (GTK_CONTAINER (app.window), GTK_WIDGET (box));
  gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (app.stack_switcher));
  gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (app.stack));
  gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (separator));
  gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (app.message));

  dzl_shortcut_manager_add_action (manager, "build-manager.build", "Projects", "Building", "Build Project", NULL);
  dzl_shortcut_manager_add_action (manager, "build-manager.rebuild", "Projects", "Building", "Rebuild Project", NULL);
  dzl_shortcut_manager_add_action (manager, "build-manager.clean", "Projects", "Building", "Clean Project", NULL);

  dzl_shortcut_manager_add_action (manager, "run-manager.run", "Projects", "Running", "Run Project", NULL);
  dzl_shortcut_manager_add_action (manager, "run-manager.run-with-handler::'profiler'", "Projects", "Running", "Run Profiler", NULL);
  dzl_shortcut_manager_add_action (manager, "run-manager.run-with-handler::'debugger'", "Projects", "Running", "Run Debugger", NULL);

  dzl_shortcut_manager_add_action (manager, "win.new-terminal", "Terminal", "Terminal", "New terminal on host", NULL);
  dzl_shortcut_manager_add_action (manager, "win.new-terminal-in-runtime", "Terminal", "Terminal", "New terminal in build runtime", NULL);

  dzl_shortcut_manager_add_command (manager, "org.gnome.builder.cut", "Editor", "Editing", "Cut Selection", NULL);
  dzl_shortcut_manager_add_command (manager, "org.gnome.builder.copy", "Editor", "Editing", "Copy Selection", NULL);
  dzl_shortcut_manager_add_command (manager, "org.gnome.builder.paste", "Editor", "Editing", "Paste Selection", NULL);
  dzl_shortcut_manager_add_command (manager, "org.gnome.builder.delete", "Editor", "Editing", "Delete Selection", NULL);

  dzl_shortcut_theme_set_accel_for_action (theme, "build-manager.build", "F7", 0);
  dzl_shortcut_theme_set_accel_for_action (theme, "build-manager.rebuild", "<Control>F7", 0);
  dzl_shortcut_theme_set_accel_for_action (theme, "build-manager.clean", "F8", 0);

  dzl_shortcut_theme_set_accel_for_action (theme, "win.new-terminal", "<primary><shift>t", 0);
  dzl_shortcut_theme_set_accel_for_action (theme, "win.new-terminal-in-runtime", "<primary><shift><alt>t", 0);

  dzl_shortcut_theme_set_accel_for_command (theme, "org.gnome.builder.cut", "<Primary>x", 0);
  dzl_shortcut_theme_set_accel_for_command (theme, "org.gnome.builder.copy", "<Primary>c", 0);
  dzl_shortcut_theme_set_accel_for_command (theme, "org.gnome.builder.paste", "<Primary>v", 0);
  dzl_shortcut_theme_set_accel_for_command (theme, "org.gnome.builder.delete", "Delete", 0);

  dzl_shortcut_theme_set_accel_for_command (theme, "com.example.foo.test", "<Control>r|<Control>r", 0);

  dzl_shortcut_theme_set_accel_for_action (theme, "run-manager.run", "F3", 0);
  dzl_shortcut_theme_set_accel_for_action (theme, "run-manager.run-with-handler::'debugger'", "<Shift>F3", 0);

  dzl_shortcut_manager_add_shortcut_entries (manager, entries, G_N_ELEMENTS (entries), NULL);

  dzl_shortcut_manager_set_theme_name (manager, "test");

  g_signal_emit_by_name (manager, "changed");

  g_signal_connect_swapped (app.window,
                            "key-press-event",
                            G_CALLBACK (dzl_shortcut_manager_handle_event),
                            manager);
  g_signal_connect (app.window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (app.window);

  {
    guint len = g_list_model_get_n_items (G_LIST_MODEL (manager));

    for (guint i = 0; i < len; i++)
      {
        g_autoptr(DzlShortcutTheme) t = g_list_model_get_item (G_LIST_MODEL (manager), i);
        const gchar *name = dzl_shortcut_theme_get_name (t);

        g_print ("Theme = %s\n", name);
      }
  }

  gtk_main ();

  return 0;
}
