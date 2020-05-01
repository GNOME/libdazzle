#include <glib/gi18n.h>

#include "example-window.h"

struct _ExampleWindow
{
  DzlApplicationWindow parent_instance;
  DzlDockBin *dockbin;
  GtkHeaderBar *header_bar;
  GtkNotebook *notebook;
  DzlEmptyState *empty_state;
  GtkStack *stack;
};

G_DEFINE_TYPE (ExampleWindow, example_window, DZL_TYPE_APPLICATION_WINDOW)

static const DzlShortcutEntry shortcuts[] = {
  { "com.example.window.NewDoc", 0, NULL, N_("Editing"), N_("Documents"), N_("New Document"), N_("Create a new document") },
  { "com.example.window.CloseDoc", 0, NULL, N_("Editing"), N_("Documents"), N_("Close Document"), N_("Close the current document") },
  { "com.example.window.Fullscreen", 0, "F11", N_("Editing"), N_("General"), N_("Fullscreen"), N_("Toggle window fullscreen") },
};

ExampleDocumentView *
example_window_get_current_document_view (ExampleWindow *self)
{
  gint page;

  g_assert (EXAMPLE_IS_WINDOW (self));

  page = gtk_notebook_get_current_page (self->notebook);

  if (page == -1)
    return NULL;

  return EXAMPLE_DOCUMENT_VIEW (gtk_notebook_get_nth_page (self->notebook, page));
}

static GtkWidget *
new_label (ExampleDocument *document)
{
  GtkWidget *box;
  GtkWidget *label;
  GtkWidget *button;

  box = g_object_new (GTK_TYPE_BOX,
                      "visible", TRUE,
                      "spacing", 3,
                      NULL);
  label = g_object_new (GTK_TYPE_LABEL,
                        "xalign", 0.0f,
                        "visible", TRUE,
                        NULL);
  g_object_bind_property (document, "title", label, "label", G_BINDING_SYNC_CREATE);
  button = g_object_new (GTK_TYPE_BUTTON,
                         "action-name", "win.close-document",
                         "visible", TRUE,
                         "child", g_object_new (GTK_TYPE_IMAGE,
                                                "visible", TRUE,
                                                "icon-name", "window-close-symbolic",
                                                "icon-size", GTK_ICON_SIZE_MENU,
                                                NULL),
                         NULL);
  dzl_gtk_widget_add_style_class (button, "flat");
  dzl_gtk_widget_add_style_class (button, "image-button");
  gtk_container_add (GTK_CONTAINER (box), label);
  gtk_container_add (GTK_CONTAINER (box), button);

  return box;
}

static void
new_document_cb (GSimpleAction *action,
                 GVariant      *param,
                 gpointer       user_data)
{
  ExampleWindow *self = user_data;
  g_autoptr(ExampleDocument) document = NULL;
  GtkWidget *view;
  GtkWidget *label;
  gint page;

  g_assert (EXAMPLE_IS_WINDOW (self));

  document = example_document_new ();
  view = g_object_new (EXAMPLE_TYPE_DOCUMENT_VIEW,
                       "visible", TRUE,
                       "document", document,
                       NULL);
  label = new_label (document);

  page = gtk_notebook_append_page (self->notebook, view, label);
  gtk_notebook_set_current_page (self->notebook, page);
  gtk_stack_set_visible_child (self->stack, GTK_WIDGET (self->notebook));
  gtk_widget_grab_focus (view);
}

static void
close_document_cb (GSimpleAction *action,
                   GVariant      *param,
                   gpointer       user_data)
{
  ExampleWindow *self = user_data;
  ExampleDocumentView *view;

  g_assert (EXAMPLE_IS_WINDOW (self));

  view = example_window_get_current_document_view (self);

  if (view != NULL)
    gtk_widget_destroy (GTK_WIDGET (view));

  if (gtk_notebook_get_n_pages (self->notebook) == 0)
    {
      gtk_stack_set_visible_child (self->stack, GTK_WIDGET (self->empty_state));
      gtk_header_bar_set_subtitle (self->header_bar, NULL);
    }
}

static const GActionEntry actions[] = {
  { "new-document", new_document_cb },
  { "close-document", close_document_cb },
};

static void
on_page_changed (GtkNotebook   *notebook,
                 GParamSpec    *pspec,
                 ExampleWindow *self)
{
  gint page = gtk_notebook_get_current_page (notebook);
  g_autofree gchar *subtitle = NULL;

  if (page >= 0)
    {
      GtkWidget *view;
      g_autoptr(ExampleDocument) document = NULL;

      view = gtk_notebook_get_nth_page (self->notebook, page);
      g_object_get (view, "document", &document, NULL);
      g_object_get (document, "title", &subtitle, NULL);
    }

  gtk_header_bar_set_subtitle (self->header_bar, subtitle);
}

static void
on_titlebar_animation (ExampleWindow *self,
                       GParamSpec    *pspec)
{
  g_autoptr(GEnumClass) enum_class = NULL;
  DzlTitlebarAnimation mode;
  GEnumValue *value;

  g_assert (EXAMPLE_IS_WINDOW (self));

  mode = dzl_application_window_get_titlebar_animation (DZL_APPLICATION_WINDOW (self));

  enum_class = g_type_class_ref (DZL_TYPE_TITLEBAR_ANIMATION);
  value = g_enum_get_value (enum_class, mode);
  g_print ("Titlebar Animation: %s\n", value->value_nick);
}

static void
example_window_class_init (ExampleWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/example/ui/example-window.ui");
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, dockbin);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, empty_state);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, header_bar);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, notebook);
  gtk_widget_class_bind_template_child (widget_class, ExampleWindow, stack);
}

static void
example_window_init (ExampleWindow *self)
{
  DzlShortcutController *controller;
  g_autoptr(GPropertyAction) left = NULL;
  g_autoptr(GPropertyAction) right = NULL;

  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect (self,
                    "notify::titlebar-animation",
                    G_CALLBACK (on_titlebar_animation),
                    NULL);

  g_signal_connect (self->notebook,
                    "notify::page",
                    G_CALLBACK (on_page_changed),
                    self);

  dzl_shortcut_manager_add_shortcut_entries (NULL, shortcuts, G_N_ELEMENTS (shortcuts), NULL);

  controller = dzl_shortcut_controller_find (GTK_WIDGET (self));

  dzl_shortcut_controller_add_command_action (controller, "com.example.window.NewDoc", NULL, 0, "win.new-document");
  dzl_shortcut_controller_add_command_action (controller, "com.example.window.CloseDoc", NULL, 0, "win.close-document");
  dzl_shortcut_controller_add_command_action (controller, "com.example.window.Fullscreen", NULL, 0, "win.fullscreen");

  g_signal_connect_swapped (self,
                            "key-press-event",
                            G_CALLBACK (dzl_shortcut_manager_handle_event),
                            dzl_shortcut_manager_get_default ());

  g_action_map_add_action_entries (G_ACTION_MAP (self), actions, G_N_ELEMENTS (actions), self);

  left = g_property_action_new ("show-left-panel", self->dockbin, "left-visible");
  right = g_property_action_new ("show-right-panel", self->dockbin, "right-visible");

  g_action_map_add_action (G_ACTION_MAP (self), G_ACTION (left));
  g_action_map_add_action (G_ACTION_MAP (self), G_ACTION (right));
}

GtkWidget *
example_window_new (void)
{
  return g_object_new (EXAMPLE_TYPE_WINDOW, NULL);
}
