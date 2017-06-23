#include <dazzle.h>

static GtkWidget *
create_child_func (gpointer item,
                   gpointer user_data)
{
  GFileInfo *file_info = G_FILE_INFO (item);
  g_autofree gchar *display_name = NULL;
  GObject *icon = NULL;
  GtkListBoxRow *row;
  GtkWidget *label;
  GFile *parent = user_data;
  GtkBox *box;
  GtkImage *image;

  g_assert (!file_info || G_IS_FILE_INFO (file_info));
  g_assert (!parent || G_IS_FILE (parent));

  if (file_info == NULL)
    {
      if (parent == NULL)
        display_name = g_strdup ("Computer");
      else
        display_name = g_file_get_basename (parent);
    }
  else
    {
      display_name = g_strdup (g_file_info_get_display_name (file_info));
      icon = g_file_info_get_attribute_object (file_info, G_FILE_ATTRIBUTE_STANDARD_SYMBOLIC_ICON);
    }


  box = g_object_new (GTK_TYPE_BOX,
                      "visible", TRUE,
                      NULL);

  image = g_object_new (GTK_TYPE_IMAGE,
                        "visible", TRUE,
                        "gicon", icon,
                        NULL);
  gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (image));

  label = g_object_new (GTK_TYPE_LABEL,
                        "label", display_name,
                        "halign", GTK_ALIGN_START,
                        "hexpand", TRUE,
                        "visible", TRUE,
                        NULL);
  gtk_container_add (GTK_CONTAINER (box), label);

  row = g_object_new (GTK_TYPE_LIST_BOX_ROW,
                      "visible", TRUE,
                      "child", box,
                      NULL);

  if (parent != NULL)
    g_object_set_data_full (G_OBJECT (row), "FILE",
                            g_file_get_child (parent, g_file_info_get_name (file_info)),
                            g_object_unref);

  return GTK_WIDGET (row);
}

static void
row_activated (DzlStackList  *stack_list,
               GtkListBoxRow *row,
               gpointer       user_data)
{
  GFile *file = G_FILE (g_object_get_data (G_OBJECT (row), "FILE"));
  GFile *parent = g_file_get_parent (file);
  g_autoptr(GListModel) model = dzl_directory_model_new (file);
  g_autofree gchar *name = g_file_get_basename (file);
  g_autoptr(GFileInfo) file_info = g_file_info_new ();

  g_file_info_set_name (file_info, name);
  g_file_info_set_display_name (file_info, name);

  dzl_stack_list_push (stack_list,
                       create_child_func (file_info, parent),
                       model,
                       create_child_func,
                       g_object_ref (file),
                       g_object_unref);
}

static void
load_css (void)
{
  g_autoptr(GtkCssProvider) provider = NULL;

  provider = dzl_css_provider_new ("resource:///org/gnome/dazzle/themes");
  gtk_style_context_add_provider_for_screen (gdk_screen_get_default (),
                                             GTK_STYLE_PROVIDER (provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_autoptr(GListModel) file_system_model = NULL;
  g_autoptr(GFile) root = NULL;
  DzlStackList *stack_list;
  GtkWidget *window;
  GtkWidget *header;

  gtk_init (&argc, &argv);

  load_css ();

  root = g_file_new_for_path ("/");
  file_system_model = dzl_directory_model_new (root);

  window = g_object_new (GTK_TYPE_WINDOW,
                         "default-width", 250,
                         "default-height", 600,
                         NULL);

  header = g_object_new (GTK_TYPE_HEADER_BAR,
                         "title", "Stack List Test",
                         "show-close-button", TRUE,
                         "visible", TRUE,
			 NULL);
  gtk_window_set_titlebar (GTK_WINDOW (window), header);

  stack_list = g_object_new (DZL_TYPE_STACK_LIST,
                             "visible", TRUE,
                             NULL);
  gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (stack_list));

  dzl_stack_list_push (stack_list,
                       create_child_func (NULL, root),
                       file_system_model,
                       create_child_func,
                       g_steal_pointer (&root),
                       g_object_unref);

  g_signal_connect (stack_list,
                    "row-activated",
                    G_CALLBACK (row_activated),
                    NULL);

  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (GTK_WINDOW (window));

  gtk_main ();

  return 0;
}
