#include "example-document-view.h"

struct _ExampleDocumentView
{
  GtkBin parent_instance;

  GtkTextView *text_view;

  ExampleDocument *document;
};

enum {
  PROP_0,
  PROP_DOCUMENT,
  N_PROPS
};

G_DEFINE_TYPE (ExampleDocumentView, example_document_view, GTK_TYPE_BIN)

static GParamSpec *properties [N_PROPS];

static void
example_document_view_set_document (ExampleDocumentView *self,
                                    ExampleDocument     *document)
{
  g_assert (EXAMPLE_IS_DOCUMENT_VIEW (self));
  g_assert (!document || EXAMPLE_IS_DOCUMENT (document));

  if (g_set_object (&self->document, document))
    {
      gtk_text_view_set_buffer (self->text_view, GTK_TEXT_BUFFER (document));
    }
}

static void
example_document_view_grab_focus (GtkWidget *widget)
{
  ExampleDocumentView *self = EXAMPLE_DOCUMENT_VIEW (widget);

  gtk_widget_grab_focus (GTK_WIDGET (self->text_view));
}

static void
example_document_view_finalize (GObject *object)
{
  ExampleDocumentView *self = (ExampleDocumentView *)object;

  g_clear_object (&self->document);

  G_OBJECT_CLASS (example_document_view_parent_class)->finalize (object);
}

static void
example_document_view_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  ExampleDocumentView *self = EXAMPLE_DOCUMENT_VIEW (object);

  switch (prop_id)
    {
    case PROP_DOCUMENT:
      g_value_set_object (value, self->document);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
example_document_view_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  ExampleDocumentView *self = EXAMPLE_DOCUMENT_VIEW (object);

  switch (prop_id)
    {
    case PROP_DOCUMENT:
      example_document_view_set_document (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
example_document_view_class_init (ExampleDocumentViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = example_document_view_finalize;
  object_class->get_property = example_document_view_get_property;
  object_class->set_property = example_document_view_set_property;

  widget_class->grab_focus = example_document_view_grab_focus;

  properties [PROP_DOCUMENT] =
    g_param_spec_object ("document",
                         "Document",
                         "The document to be viewed",
                         EXAMPLE_TYPE_DOCUMENT,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/example/ui/example-document-view.ui");
  gtk_widget_class_bind_template_child (widget_class, ExampleDocumentView, text_view);
}

static void
example_document_view_init (ExampleDocumentView *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

GtkWidget *
example_document_view_new (void)
{
  return g_object_new (EXAMPLE_TYPE_DOCUMENT, NULL);
}
