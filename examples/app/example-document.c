#include "example-document.h"

struct _ExampleDocument
{
  GtkTextBuffer parent_instance;
  gchar *title;
};

enum {
  PROP_0,
  PROP_TITLE,
  N_PROPS
};

G_DEFINE_TYPE (ExampleDocument, example_document, GTK_TYPE_TEXT_BUFFER)

static guint last_untitled;
static GParamSpec *properties [N_PROPS];

static void
example_document_constructed (GObject *object)
{
  ExampleDocument *self = (ExampleDocument *)object;

  if (self->title == NULL)
    self->title = g_strdup_printf ("Untitled Document %u", ++last_untitled);

  G_OBJECT_CLASS (example_document_parent_class)->constructed (object);
}

static void
example_document_finalize (GObject *object)
{
  ExampleDocument *self = (ExampleDocument *)object;

  g_clear_pointer (&self->title, g_free);

  G_OBJECT_CLASS (example_document_parent_class)->finalize (object);
}

static void
example_document_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  ExampleDocument *self = EXAMPLE_DOCUMENT (object);

  switch (prop_id)
    {
    case PROP_TITLE:
      g_value_set_string (value, self->title);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
example_document_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  ExampleDocument *self = EXAMPLE_DOCUMENT (object);

  switch (prop_id)
    {
    case PROP_TITLE:
      g_free (self->title);
      self->title = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
example_document_class_init (ExampleDocumentClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = example_document_finalize;
  object_class->constructed = example_document_constructed;
  object_class->get_property = example_document_get_property;
  object_class->set_property = example_document_set_property;

  properties [PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "The title of the document",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
example_document_init (ExampleDocument *self)
{
}

ExampleDocument *
example_document_new (void)
{
  return g_object_new (EXAMPLE_TYPE_DOCUMENT, NULL);
}
