#include "example-application.h"
#include "example-window.h"

struct _ExampleApplication
{
  DzlApplication parent_instance;
};

G_DEFINE_TYPE (ExampleApplication, example_application, DZL_TYPE_APPLICATION)

static void
example_application_activate (GApplication *app)
{
  GtkWindow *window;

  window = gtk_application_get_active_window (GTK_APPLICATION (app));

  if (window == NULL)
    window = g_object_new (EXAMPLE_TYPE_WINDOW,
                           "application", app,
                           "default-width", 800,
                           "default-height", 600,
                           "title", "Example Window",
                           NULL);

  gtk_window_present (window);
}

static void
example_application_class_init (ExampleApplicationClass *klass)
{
  GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

  app_class->activate = example_application_activate;
}

static void
about_activate (GSimpleAction *action,
                GVariant      *variant,
                gpointer       user_data)
{
  GtkAboutDialog *dialog;

  dialog = g_object_new (GTK_TYPE_ABOUT_DIALOG,
                         "copyright", "Copyright 2017 Christian Hergert",
                         "logo-icon-name", "org.gnome.clocks",
                         "website", "https://wiki.gnome.org/Apps/Builder",
                         "version", DZL_VERSION_S,
                         NULL);

  gtk_window_present (GTK_WINDOW (dialog));
}

static void
quit_activate (GSimpleAction *action,
               GVariant      *variant,
               gpointer       user_data)
{
  g_application_quit (G_APPLICATION (user_data));
}

static void
shortcuts_activate (GSimpleAction *action,
                    GVariant      *variant,
                    gpointer       user_data)
{
  DzlShortcutsWindow *window;
  DzlShortcutManager *manager;

  manager = dzl_application_get_shortcut_manager (user_data);

  window = g_object_new (DZL_TYPE_SHORTCUTS_WINDOW, NULL);
  dzl_shortcut_manager_add_shortcuts_to_window (manager, window);
  gtk_window_present (GTK_WINDOW (window));
}

static void
example_application_init (ExampleApplication *self)
{
  static GActionEntry entries[] = {
    { "about", about_activate },
    { "quit", quit_activate },
    { "shortcuts", shortcuts_activate },
  };

  g_action_map_add_action_entries (G_ACTION_MAP (self), entries, G_N_ELEMENTS (entries), self);
}
