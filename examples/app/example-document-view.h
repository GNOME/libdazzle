#pragma once

#include <gtk/gtk.h>

#include "example-document.h"

G_BEGIN_DECLS

#define EXAMPLE_TYPE_DOCUMENT_VIEW (example_document_view_get_type())

G_DECLARE_FINAL_TYPE (ExampleDocumentView, example_document_view, EXAMPLE, DOCUMENT_VIEW, GtkBin)

GtkWidget *example_document_view_new (void);

G_END_DECLS
