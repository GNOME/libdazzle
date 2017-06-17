#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EXAMPLE_TYPE_DOCUMENT (example_document_get_type())

G_DECLARE_FINAL_TYPE (ExampleDocument, example_document, EXAMPLE, DOCUMENT, GtkTextBuffer)

ExampleDocument *example_document_new (void);

G_END_DECLS
