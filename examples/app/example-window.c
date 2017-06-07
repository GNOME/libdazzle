#include <dazzle.h>

#include "example-window.h"

struct _ExampleWindow
{
  GtkWindow   parent_instance;
  DzlDockBin *dockbin;
};

G_DEFINE_TYPE (ExampleWindow, example_window, GTK_TYPE_WINDOW)

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
  gtk_widget_init_template (GTK_WIDGET (self));
}

GtkWidget *
example_window_new (void)
{
  return g_object_new (EXAMPLE_TYPE_WINDOW, NULL);
}
