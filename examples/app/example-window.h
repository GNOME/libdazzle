#ifndef EXAMPLE_WINDOW_H
#define EXAMPLE_WINDOW_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EXAMPLE_TYPE_WINDOW (example_window_get_type())

G_DECLARE_FINAL_TYPE (ExampleWindow, example_window, EXAMPLE, WINDOW, GtkWindow)

GtkWidget *example_window_new (void);

G_END_DECLS

#endif /* EXAMPLE_WINDOW_H */
