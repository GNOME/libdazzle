#include "config.h"

#include <dazzle.h>
#include <glib/gi18n.h>

#include "example-window.h"

struct _ExampleWindow
{
  GtkWindow   parent_instance;
  DzlDockBin *dockbin;
};

G_DEFINE_TYPE (ExampleWindow, example_window, GTK_TYPE_WINDOW)

static const DzlShortcutEntry shortcuts[] = {
  { "com.example.window.NewDoc", "<control>n", N_("Editing"), N_("Documents"), N_("New Document"), N_("Create a new document") },
  { "com.example.window.CloseDoc", "<control>w", N_("Editing"), N_("Documents"), N_("Close Document"), N_("Close the current document") },
};

static void
new_document_cb (ExampleWindow *self,
                 gpointer       user_data)
{
  g_assert (EXAMPLE_IS_WINDOW (self));

}

static void
close_document_cb (ExampleWindow *self,
                   gpointer       user_data)
{
  g_assert (EXAMPLE_IS_WINDOW (self));

}

static void
example_window_class_init (ExampleWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/example/ui/example-window.ui");
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, dockbin);
}

static void
example_window_init (ExampleWindow *self)
{
  DzlShortcutController *controller;

  gtk_widget_init_template (GTK_WIDGET (self));
  dzl_shortcut_manager_add_shortcut_entries (NULL, shortcuts, G_N_ELEMENTS (shortcuts), GETTEXT_PACKAGE);

  controller = dzl_shortcut_controller_find (GTK_WIDGET (self));

  dzl_shortcut_manager_set_theme_name (NULL, "default");

  dzl_shortcut_controller_add_command_callback (controller,
                                                "com.example.window.NewDoc",
                                                "<control>n",
                                                (GtkCallback) new_document_cb, NULL, NULL);

  dzl_shortcut_controller_add_command_callback (controller,
                                                "com.example.window.CloseDoc",
                                                "<control>w",
                                                (GtkCallback) close_document_cb, NULL, NULL);

  g_signal_connect_swapped (self,
                            "key-press-event",
                            G_CALLBACK (dzl_shortcut_manager_handle_event),
                            dzl_shortcut_manager_get_default ());
}

GtkWidget *
example_window_new (void)
{
  return g_object_new (EXAMPLE_TYPE_WINDOW, NULL);
}
