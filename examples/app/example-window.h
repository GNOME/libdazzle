#pragma once

#include <dazzle.h>

#include "example-document-view.h"

G_BEGIN_DECLS

#define EXAMPLE_TYPE_WINDOW (example_window_get_type())

G_DECLARE_FINAL_TYPE (ExampleWindow, example_window, EXAMPLE, WINDOW, DzlApplicationWindow)

GtkWidget           *example_window_new                       (void);
ExampleDocumentView *example_window_get_current_document_view (ExampleWindow *self);

G_END_DECLS
