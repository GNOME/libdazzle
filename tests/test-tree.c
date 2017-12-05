#include <dazzle.h>

static void
build_node_cb (DzlTreeBuilder *builder,
               DzlTreeNode    *node)
{
  GFile *file;

  g_assert (DZL_IS_TREE_BUILDER (builder));
  g_assert (DZL_IS_TREE_NODE (node));

  file = G_FILE (dzl_tree_node_get_item (node));
  g_assert (G_IS_FILE (file));

  if (g_file_query_file_type (file, 0, NULL) == G_FILE_TYPE_DIRECTORY)
    {
      g_autoptr(GFileEnumerator) enumerator = NULL;
      gpointer infoptr;

      enumerator = g_file_enumerate_children (file,
                                              G_FILE_ATTRIBUTE_STANDARD_NAME","
                                              G_FILE_ATTRIBUTE_STANDARD_ICON","
                                              G_FILE_ATTRIBUTE_STANDARD_TYPE,
                                              0, NULL, NULL);

      if (enumerator == NULL)
        return;

      while (!!(infoptr = g_file_enumerator_next_file (enumerator, NULL, NULL)))
        {
          g_autoptr(GFileInfo) info = infoptr;
          g_autoptr(GFile) child = g_file_get_child (file, g_file_info_get_name (info));
          DzlTreeNode *child_node;

          child_node = dzl_tree_node_new ();
          dzl_tree_node_set_item (child_node, G_OBJECT (child));
          dzl_tree_node_set_text (child_node, g_file_info_get_name (info));
          dzl_tree_node_set_gicon (child_node, g_file_info_get_icon (info));
          dzl_tree_node_set_reset_on_collapse (child_node, TRUE);

          if (g_file_info_get_file_type (info) == G_FILE_TYPE_DIRECTORY)
            dzl_tree_node_set_children_possible (child_node, TRUE);

          dzl_tree_node_append (node, child_node);
        }

      g_file_enumerator_close (enumerator, NULL, NULL);
    }
}

gint
main (gint   argc,
      gchar *argv[])
{
  g_autoptr(DzlTreeNode) root = NULL;
  g_autoptr(GFile) home = NULL;
  DzlTreeBuilder *builder;
  GtkWidget *window;
  GtkWidget *scroller;
  GtkWidget *tree;

  gtk_init (&argc, &argv);

  home = g_file_new_for_path (g_get_home_dir ());

  window = g_object_new (GTK_TYPE_WINDOW,
                         "default-width", 300,
                         "default-height", 700,
                         "title", "Tree Test",
                         "visible", TRUE,
                         NULL);

  scroller = g_object_new (GTK_TYPE_SCROLLED_WINDOW,
                           "visible", TRUE,
                           NULL);
  gtk_container_add (GTK_CONTAINER (window), scroller);

  tree = g_object_new (DZL_TYPE_TREE,
                       "show-icons", TRUE,
                       "headers-visible", FALSE,
                       "visible", TRUE,
                       NULL);
  gtk_container_add (GTK_CONTAINER (scroller), tree);

  builder = dzl_tree_builder_new ();
  g_signal_connect (builder, "build-node", G_CALLBACK (build_node_cb), NULL);
  dzl_tree_add_builder (DZL_TREE (tree), builder);

  root = dzl_tree_node_new ();
  dzl_tree_node_set_item (root, G_OBJECT (home));
  dzl_tree_set_root (DZL_TREE (tree), root);

  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (GTK_WINDOW (window));
  gtk_main ();

  return 0;
}
