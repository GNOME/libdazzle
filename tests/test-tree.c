#include <dazzle.h>

static void
build_children_cb (DzlTreeBuilder *builder,
                   DzlTreeNode    *node)
{
  GFile *file;

  g_assert (DZL_IS_TREE_BUILDER (builder));
  g_assert (DZL_IS_TREE_NODE (node));

  file = G_FILE (dzl_tree_node_get_item (node));
  g_assert (G_IS_FILE (file));

  if (g_file_query_file_type (file, 0, NULL) == G_FILE_TYPE_DIRECTORY)
    {
      static const GdkRGBA dim = { 0, 0, 0, 0.5 };
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
          const gchar *name = g_file_info_get_name (info);
          g_autoptr(GFile) child = g_file_get_child (file, name);
          DzlTreeNode *child_node;

          child_node = dzl_tree_node_new ();
          dzl_tree_node_set_item (child_node, G_OBJECT (child));
          dzl_tree_node_set_text (child_node, g_file_info_get_name (info));
          dzl_tree_node_set_gicon (child_node, g_file_info_get_icon (info));
          dzl_tree_node_set_reset_on_collapse (child_node, TRUE);

          if (*name == '.')
            dzl_tree_node_set_foreground_rgba (child_node, &dim);

          if (g_file_info_get_file_type (info) == G_FILE_TYPE_DIRECTORY)
            dzl_tree_node_set_children_possible (child_node, TRUE);

          dzl_tree_node_append (node, child_node);
        }

      g_file_enumerator_close (enumerator, NULL, NULL);
    }
}

static gboolean
node_draggable_cb (DzlTreeBuilder *builder,
                   DzlTreeNode    *node)
{
  return TRUE;
}

static gboolean
node_droppable_cb (DzlTreeBuilder   *builder,
                   DzlTreeNode      *node,
                   GtkSelectionData *data)
{
  /* XXX: Check that uri/node is not a parent of node */
  return TRUE;
}

static gboolean
node_drag_data_get_cb (DzlTreeBuilder   *builder,
                       DzlTreeNode      *node,
                       GtkSelectionData *data)
{
  g_autofree gchar *uri = NULL;
  gchar *uris[] = { NULL, NULL };
  GFile *file;

  g_assert (DZL_IS_TREE_BUILDER (builder));
  g_assert (DZL_IS_TREE_NODE (node));

  if (gtk_selection_data_get_target (data) != gdk_atom_intern_static_string ("text/uri-list"))
    return FALSE;

  file = G_FILE (dzl_tree_node_get_item (node));
  uris[0] = uri = g_file_get_uri (file);
  if (gtk_selection_data_set_uris (data, uris))
    g_print ("Set uri to %s\n", uri);

  return TRUE;
}

static gboolean
drag_data_received_cb (DzlTreeBuilder      *builder,
                       DzlTreeNode         *node,
                       DzlTreeDropPosition  position,
                       GdkDragAction        action,
                       GtkSelectionData    *data)
{
  g_assert (DZL_IS_TREE_BUILDER (builder));
  g_assert (DZL_IS_TREE_NODE (node));
  g_assert (data != NULL);

  g_print ("Drag data received: action = %d\n", action);

  if (gtk_selection_data_get_target (data) == gdk_atom_intern ("text/uri-list", FALSE))
    {
      g_auto(GStrv) uris = NULL;
      g_autofree gchar *str = NULL;
      g_autofree gchar *dst = NULL;
      GFile *file;

      /*
       * We get a node inside the parent when dropping onto a parent.
       * So we really want to try to get the file for the parent node.
       */

      node = dzl_tree_node_get_parent (node);
      file = G_FILE (dzl_tree_node_get_item (node));

      g_assert (DZL_IS_TREE_NODE (node));
      g_assert (G_IS_FILE (file));

      uris = gtk_selection_data_get_uris (data);
      str = g_strjoinv (" ", uris);
      dst = g_file_get_uri (file);

      g_print ("Dropping uris: %s onto %s\n", str, dst);

      return TRUE;
    }

  return FALSE;
}

static gboolean
drag_node_delete_cb (DzlTreeBuilder *builder,
                     DzlTreeNode    *node)
{
  g_assert (DZL_IS_TREE_BUILDER (builder));
  g_assert (DZL_IS_TREE_NODE (node));

  /* This is called when GTK_ACTION_MOVE is used and we need
   * to cleanup the old node which is not gone.
   */
  g_print ("Delete node %s\n", dzl_tree_node_get_text (node));

  return FALSE;
}

static gboolean
drag_node_received_cb (DzlTreeBuilder      *builder,
                       DzlTreeNode         *drag_node,
                       DzlTreeNode         *drop_node,
                       DzlTreeDropPosition  position,
                       GdkDragAction        action,
                       GtkSelectionData    *data,
                       gpointer             user_data)
{
  g_assert (DZL_IS_TREE_BUILDER (builder));
  g_assert (DZL_IS_TREE_NODE (drag_node));
  g_assert (DZL_IS_TREE_NODE (drop_node));
  g_assert (data != NULL);

  g_print ("Drop %s onto %s with pos %d and action %d\n",
           dzl_tree_node_get_text (drag_node),
           dzl_tree_node_get_text (drop_node),
           position, action);

  /* Pretend we succeeded */

  return TRUE;
}

gint
main (gint   argc,
      gchar *argv[])
{
  static const GtkTargetEntry drag_targets[] = {
    { (gchar *)"GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0 },
    { (gchar *)"text/uri-list", 0, 0 },
  };
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
  gtk_tree_view_enable_model_drag_source (GTK_TREE_VIEW (tree),
                                          GDK_BUTTON1_MASK,
                                          drag_targets, G_N_ELEMENTS (drag_targets),
                                          GDK_ACTION_COPY | GDK_ACTION_MOVE);
  gtk_tree_view_enable_model_drag_dest (GTK_TREE_VIEW (tree),
                                        drag_targets, G_N_ELEMENTS (drag_targets),
                                        GDK_ACTION_COPY | GDK_ACTION_MOVE);
  gtk_container_add (GTK_CONTAINER (scroller), tree);

  builder = dzl_tree_builder_new ();
  g_signal_connect (builder, "build-children", G_CALLBACK (build_children_cb), NULL);
  g_signal_connect (builder, "drag-data-get", G_CALLBACK (node_drag_data_get_cb), NULL);
  g_signal_connect (builder, "drag-data-received", G_CALLBACK (drag_data_received_cb), NULL);
  g_signal_connect (builder, "drag-node-received", G_CALLBACK (drag_node_received_cb), NULL);
  g_signal_connect (builder, "drag-node-delete", G_CALLBACK (drag_node_delete_cb), NULL);
  g_signal_connect (builder, "node-draggable", G_CALLBACK (node_draggable_cb), NULL);
  g_signal_connect (builder, "node-droppable", G_CALLBACK (node_droppable_cb), NULL);
  dzl_tree_add_builder (DZL_TREE (tree), builder);

  root = g_object_ref_sink (dzl_tree_node_new ());
  dzl_tree_node_set_item (root, G_OBJECT (home));
  dzl_tree_set_root (DZL_TREE (tree), root);

  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (GTK_WINDOW (window));
  gtk_main ();

  return 0;
}
