/* dzl-tree-store.c
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define G_LOG_DOMAIN "dzl-tree-store"

#include "config.h"

#include "tree/dzl-tree-builder.h"
#include "tree/dzl-tree-node.h"
#include "tree/dzl-tree-private.h"
#include "tree/dzl-tree-store.h"
#include "util/dzl-macros.h"

struct _DzlTreeStore
{
  GtkTreeStore  parent_instance;

  /* Weak references */
  DzlTree      *tree;
};

static void dest_iface_init   (GtkTreeDragDestIface   *iface);
static void source_iface_init (GtkTreeDragSourceIface *iface);

G_DEFINE_TYPE_WITH_CODE (DzlTreeStore, dzl_tree_store, GTK_TYPE_TREE_STORE,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_DRAG_DEST, dest_iface_init)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_DRAG_SOURCE, source_iface_init))

static void
dzl_tree_store_dispose (GObject *object)
{
  DzlTreeStore *self = (DzlTreeStore *)object;

  dzl_clear_weak_pointer (&self->tree);

  G_OBJECT_CLASS (dzl_tree_store_parent_class)->dispose (object);
}

static void
dzl_tree_store_class_init (DzlTreeStoreClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = dzl_tree_store_dispose;
}

static void
dzl_tree_store_init (DzlTreeStore *self)
{
  GType types[] = { DZL_TYPE_TREE_NODE };

  gtk_tree_store_set_column_types (GTK_TREE_STORE (self), 1, types);
}

static gboolean
dzl_tree_store_row_draggable (GtkTreeDragSource *source,
                              GtkTreePath       *path)
{
  GtkTreeIter iter;

  g_assert (GTK_IS_TREE_DRAG_SOURCE (source));
  g_assert (path != NULL);

  if (gtk_tree_model_get_iter (GTK_TREE_MODEL (source), &iter, path))
    {
      g_autoptr(DzlTreeNode) node = NULL;
      GPtrArray *builders;
      DzlTree *tree;

      gtk_tree_model_get (GTK_TREE_MODEL (source), &iter, 0, &node, -1);
      g_assert (DZL_IS_TREE_NODE (node));

      tree = dzl_tree_node_get_tree (node);
      g_assert (DZL_IS_TREE (tree));

      builders = _dzl_tree_get_builders (tree);
      g_assert (builders != NULL);

      if (dzl_tree_node_is_root (node) || _dzl_tree_node_is_dummy (node))
        return FALSE;

      for (guint i = 0; i < builders->len; i++)
        {
          DzlTreeBuilder *builder = g_ptr_array_index (builders, i);

          if (_dzl_tree_builder_node_draggable (builder, node))
            return TRUE;
        }
    }

  return FALSE;
}

static gboolean
dzl_tree_store_drag_data_get (GtkTreeDragSource *source,
                              GtkTreePath       *path,
                              GtkSelectionData  *data)
{
  GtkTreeIter iter;

  g_assert (DZL_IS_TREE_STORE (source));
  g_assert (path != NULL);
  g_assert (data != NULL);

  if (gtk_tree_model_get_iter (GTK_TREE_MODEL (source), &iter, path))
    {
      g_autoptr(DzlTreeNode) node = NULL;
      GPtrArray *builders;
      DzlTree *tree;

      gtk_tree_model_get (GTK_TREE_MODEL (source), &iter, 0, &node, -1);
      g_assert (DZL_IS_TREE_NODE (node));

      tree = dzl_tree_node_get_tree (node);
      builders = _dzl_tree_get_builders (tree);

      for (guint i = 0; i < builders->len; i++)
        {
          DzlTreeBuilder *builder = g_ptr_array_index (builders, i);

          if (_dzl_tree_builder_drag_data_get (builder, node, data))
            return TRUE;
        }
    }

  return FALSE;
}

static gboolean
dzl_tree_store_row_drop_possible (GtkTreeDragDest  *dest,
                                  GtkTreePath      *path,
                                  GtkSelectionData *data)
{
  GtkTreeIter iter;

  g_assert (GTK_IS_TREE_DRAG_DEST (dest));
  g_assert (path != NULL);
  g_assert (data != NULL);

  if (gtk_tree_model_get_iter (GTK_TREE_MODEL (dest), &iter, path))
    {
      g_autoptr(DzlTreeNode) node = NULL;
      DzlTreeNode *effective;
      GPtrArray *builders;
      DzlTree *tree;

      gtk_tree_model_get (GTK_TREE_MODEL (dest), &iter, 0, &node, -1);
      g_assert (DZL_IS_TREE_NODE (node));

      tree = dzl_tree_node_get_tree (node);
      g_assert (DZL_IS_TREE (tree));

      builders = _dzl_tree_get_builders (tree);
      g_assert (builders != NULL);

      if (dzl_tree_node_is_root (node))
        return FALSE;

      effective = _dzl_tree_node_is_dummy (node)
                ?  dzl_tree_node_get_parent (node)
                : node;

      if (effective == NULL || dzl_tree_node_is_root (effective))
        return FALSE;

      g_assert (effective != NULL);
      g_assert (!_dzl_tree_node_is_dummy (effective));
      g_assert (!dzl_tree_node_is_root (effective));

      for (guint i = 0; i < builders->len; i++)
        {
          DzlTreeBuilder *builder = g_ptr_array_index (builders, i);

          if (_dzl_tree_builder_node_droppable (builder, effective, data))
            return TRUE;
        }
    }

  return FALSE;
}

static gboolean
dzl_tree_store_drag_data_received (GtkTreeDragDest  *dest,
                                   GtkTreePath      *path,
                                   GtkSelectionData *data)
{
  DzlTreeStore *self = (DzlTreeStore *)dest;
  g_autoptr(DzlTreeNode) drop_node = NULL;
  GPtrArray *builders;
  DzlTreeDropPosition pos = 0;
  GdkDragAction action;

  g_assert (GTK_IS_TREE_DRAG_DEST (self));
  g_assert (self->tree != NULL);
  g_assert (path != NULL);
  g_assert (data != NULL);

  builders = _dzl_tree_get_builders (self->tree);
  drop_node = _dzl_tree_get_drop_node (self->tree, &pos);
  action = _dzl_tree_get_drag_action (self->tree);

  /*
   * If we have a drag/drop of a node onto/adjacent another node, then
   * use the simplified drag_node_received API for the builders instead
   * of making them extract the node information themselves.
   */
  if (gtk_selection_data_get_target (data) == gdk_atom_intern_static_string ("GTK_TREE_MODEL_ROW"))
    {
      GtkTreePath *src_path = NULL;
      GtkTreeModel *model = NULL;

      if (gtk_tree_get_row_drag_data (data, &model, &src_path))
        {
          GtkTreeIter iter;
          gboolean found;

          found = gtk_tree_model_get_iter (model, &iter, src_path);
          g_clear_pointer (&src_path, gtk_tree_path_free);

          if (found)
            {
              g_autoptr(DzlTreeNode) drag_node = NULL;

              gtk_tree_model_get (model, &iter, 0, &drag_node, -1);
              g_assert (DZL_IS_TREE_NODE (drag_node));

              for (guint i = 0; i < builders->len; i++)
                {
                  DzlTreeBuilder *builder = g_ptr_array_index (builders, i);

                  g_assert (DZL_IS_TREE_BUILDER (builder));

                  if (_dzl_tree_builder_drag_node_received (builder, drag_node, drop_node, pos, action, data))
                    return TRUE;
                }
            }
        }
    }

  for (guint i = 0; i < builders->len; i++)
    {
      DzlTreeBuilder *builder = g_ptr_array_index (builders, i);

      g_assert (DZL_IS_TREE_BUILDER (builder));

      if (_dzl_tree_builder_drag_data_received (builder, drop_node, pos, action, data))
        return TRUE;
    }

  return FALSE;
}

static gboolean
dzl_tree_store_drag_data_delete (GtkTreeDragSource *source,
                                 GtkTreePath       *path)
{
  DzlTreeStore *self = (DzlTreeStore *)source;
  GtkTreeIter iter;
  GPtrArray *builders;

  g_assert (GTK_IS_TREE_DRAG_SOURCE (self));
  g_assert (self->tree != NULL);
  g_assert (path != NULL);

  builders = _dzl_tree_get_builders (self->tree);
  g_assert (builders != NULL);

  if (gtk_tree_model_get_iter (GTK_TREE_MODEL (source), &iter, path))
    {
      g_autoptr(DzlTreeNode) node = NULL;

      gtk_tree_model_get (GTK_TREE_MODEL (source), &iter, 0, &node, -1);
      g_assert (DZL_IS_TREE_NODE (node));

      for (guint i = 0; i < builders->len; i++)
        {
          DzlTreeBuilder *builder = g_ptr_array_index (builders, i);

          g_assert (DZL_IS_TREE_BUILDER (builder));

          if (_dzl_tree_builder_drag_node_delete (builder, node))
            return TRUE;
        }
    }

  return FALSE;
}

static void
dest_iface_init (GtkTreeDragDestIface *iface)
{
  iface->row_drop_possible = dzl_tree_store_row_drop_possible;
  iface->drag_data_received = dzl_tree_store_drag_data_received;
}

static void
source_iface_init (GtkTreeDragSourceIface *iface)
{
  iface->row_draggable = dzl_tree_store_row_draggable;
  iface->drag_data_get = dzl_tree_store_drag_data_get;
  iface->drag_data_delete = dzl_tree_store_drag_data_delete;
}

GtkTreeStore *
_dzl_tree_store_new (DzlTree *tree)
{
  DzlTreeStore *self;

  self = g_object_new (DZL_TYPE_TREE_STORE, NULL);
  dzl_set_weak_pointer (&self->tree, tree);

  return GTK_TREE_STORE (self);
}
