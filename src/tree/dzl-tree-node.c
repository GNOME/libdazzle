/* dzl-tree-node.c
 *
 * Copyright (C) 2011-2017 Christian Hergert <christian@hergert.me>
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

#define G_LOG_DOMAIN "dzl-tree-node"

#include "config.h"

#include <glib/gi18n.h>
#include <string.h>

#include "tree/dzl-tree.h"
#include "tree/dzl-tree-node.h"
#include "tree/dzl-tree-private.h"
#include "util/dzl-macros.h"

struct _DzlTreeNode
{
  GInitiallyUnowned  parent_instance;

  DzlTreeNode       *parent;

  GObject           *item;
  gchar             *text;
  DzlTree           *tree;

  GIcon             *gicon;
  GList             *emblems;
  GQuark             icon_name;
  GQuark             expanded_icon_name;

  GdkRGBA            foreground_rgba;

  guint              children_possible : 1;
  guint              is_dummy : 1;
  guint              foreground_rgba_set : 1;
  guint              needs_build_children : 1;
  guint              reset_on_collapse : 1;
  guint              use_dim_label : 1;
  guint              use_markup : 1;
};

typedef struct
{
  DzlTreeNode *self;
  GtkPopover *popover;
} PopupRequest;

G_DEFINE_TYPE (DzlTreeNode, dzl_tree_node, G_TYPE_INITIALLY_UNOWNED)

enum {
  PROP_0,
  PROP_CHILDREN_POSSIBLE,
  PROP_EXPANDED_ICON_NAME,
  PROP_ICON_NAME,
  PROP_GICON,
  PROP_ITEM,
  PROP_PARENT,
  PROP_RESET_ON_COLLAPSE,
  PROP_TEXT,
  PROP_TREE,
  PROP_USE_DIM_LABEL,
  PROP_USE_MARKUP,
  LAST_PROP
};

static GParamSpec *properties [LAST_PROP];

/**
 * dzl_tree_node_new:
 *
 * Creates a new #DzlTreeNode instance. This is handy for situations where you
 * do not want to subclass #DzlTreeNode.
 *
 * Returns: (transfer full): A #DzlTreeNode
 */
DzlTreeNode *
dzl_tree_node_new (void)
{
  return g_object_new (DZL_TYPE_TREE_NODE, NULL);
}

/**
 * dzl_tree_node_get_tree:
 * @node: (in): A #DzlTreeNode.
 *
 * Fetches the #DzlTree instance that owns the node.
 *
 * Returns: (transfer none): A #DzlTree.
 */
DzlTree *
dzl_tree_node_get_tree (DzlTreeNode *node)
{
  g_return_val_if_fail (DZL_IS_TREE_NODE (node), NULL);

  while (node != NULL)
    {
      if (node->tree != NULL)
        return node->tree;
      node = node->parent;
    }

  return NULL;
}

/**
 * dzl_tree_node_set_tree:
 * @node: (in): A #DzlTreeNode.
 * @tree: (in): A #DzlTree.
 *
 * Internal method to set the nodes tree.
 */
void
_dzl_tree_node_set_tree (DzlTreeNode *node,
                         DzlTree     *tree)
{
  g_return_if_fail (DZL_IS_TREE_NODE (node));
  g_return_if_fail (!tree || DZL_IS_TREE (tree));

  dzl_set_weak_pointer (&node->tree, tree);
}

/**
 * dzl_tree_node_insert:
 * @self: a #DzlTreeNode
 * @child: a #DzlTreeNode
 * @position: the position for the child
 *
 * Inserts @child as a child of @self at @position.
 *
 * Since: 3.28
 */
void
dzl_tree_node_insert (DzlTreeNode *self,
                      DzlTreeNode *child,
                      guint        position)
{
  g_return_if_fail (DZL_IS_TREE_NODE (self));
  g_return_if_fail (DZL_IS_TREE_NODE (child));

  _dzl_tree_insert (self->tree, self, child, position);
}

/**
 * dzl_tree_node_insert_sorted:
 * @node: A #DzlTreeNode.
 * @child: A #DzlTreeNode.
 * @compare_func: (scope call): A compare func to compare nodes.
 * @user_data: user data for @compare_func.
 *
 * Inserts a @child as a child of @node, sorting it among the other children.
 */
void
dzl_tree_node_insert_sorted (DzlTreeNode            *node,
                             DzlTreeNode            *child,
                             DzlTreeNodeCompareFunc  compare_func,
                             gpointer                user_data)
{
  g_return_if_fail (DZL_IS_TREE_NODE (node));
  g_return_if_fail (DZL_IS_TREE_NODE (child));
  g_return_if_fail (compare_func != NULL);

  _dzl_tree_insert_sorted (node->tree, node, child, compare_func, user_data);
}

/**
 * dzl_tree_node_append:
 * @node: A #DzlTreeNode.
 * @child: A #DzlTreeNode.
 *
 * Appends @child to the list of children owned by @node.
 */
void
dzl_tree_node_append (DzlTreeNode *node,
                     DzlTreeNode *child)
{
  g_return_if_fail (DZL_IS_TREE_NODE (node));

  _dzl_tree_append (node->tree, node, child);
}

/**
 * dzl_tree_node_prepend:
 * @node: A #DzlTreeNode.
 * @child: A #DzlTreeNode.
 *
 * Prepends @child to the list of children owned by @node.
 */
void
dzl_tree_node_prepend (DzlTreeNode *node,
                      DzlTreeNode *child)
{
  g_return_if_fail (DZL_IS_TREE_NODE (node));

  _dzl_tree_prepend (node->tree, node, child);
}

/**
 * dzl_tree_node_remove:
 * @node: A #DzlTreeNode.
 * @child: A #DzlTreeNode.
 *
 * Removes @child from the list of children owned by @node.
 */
void
dzl_tree_node_remove (DzlTreeNode *node,
                     DzlTreeNode *child)
{
  g_return_if_fail (DZL_IS_TREE_NODE (node));
  g_return_if_fail (DZL_IS_TREE_NODE (child));

  _dzl_tree_remove (node->tree, child);
}

/**
 * dzl_tree_node_get_path:
 * @node: (in): A #DzlTreeNode.
 *
 * Gets a #GtkTreePath for @node.
 *
 * Returns: (nullable) (transfer full): A #GtkTreePath if successful; otherwise %NULL.
 */
GtkTreePath *
dzl_tree_node_get_path (DzlTreeNode *node)
{
  DzlTreeNode *toplevel;
  GtkTreePath *path;
  GList *list = NULL;

  g_return_val_if_fail (DZL_IS_TREE_NODE (node), NULL);

  if ((node->parent == NULL) || (node->tree == NULL))
    return NULL;

  do
    {
      list = g_list_prepend (list, node);
    }
  while ((node = node->parent));

  toplevel = list->data;

  g_assert (toplevel);
  g_assert (toplevel->tree);

  path = _dzl_tree_get_path (toplevel->tree, list);

  g_list_free (list);

  return path;
}

gboolean
dzl_tree_node_get_iter (DzlTreeNode *self,
                        GtkTreeIter *iter)
{
  gboolean ret = FALSE;

  g_return_val_if_fail (DZL_IS_TREE_NODE (self), FALSE);
  g_return_val_if_fail (iter != NULL, FALSE);

  if (self->tree != NULL)
    ret = _dzl_tree_get_iter (self->tree, self, iter);

  return ret;
}

/**
 * dzl_tree_node_get_parent:
 * @node: (in): A #DzlTreeNode.
 *
 * Retrieves the parent #DzlTreeNode for @node.
 *
 * Returns: (transfer none): A #DzlTreeNode.
 */
DzlTreeNode *
dzl_tree_node_get_parent (DzlTreeNode *node)
{
  g_return_val_if_fail (DZL_IS_TREE_NODE (node), NULL);

  return node->parent;
}

static void
dzl_tree_node_queue_draw (DzlTreeNode *self)
{
  g_assert (DZL_IS_TREE_NODE (self));

  if (self->tree != NULL)
    gtk_widget_queue_draw (GTK_WIDGET (self->tree));
}

void
dzl_tree_node_set_gicon (DzlTreeNode *self,
                         GIcon       *gicon)
{
  g_return_if_fail (DZL_IS_TREE_NODE (self));
  g_return_if_fail (!gicon || G_IS_ICON (gicon));

  if (g_set_object (&self->gicon, gicon))
    g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_GICON]);
}

/**
 * dzl_tree_node_get_gicon:
 *
 * Fetch the GIcon, re-render if necessary
 *
 * Returns: (transfer none): An #GIcon or %NULL.
 */
GIcon *
dzl_tree_node_get_gicon (DzlTreeNode *self)
{
  const gchar *icon_name;

  g_return_val_if_fail (DZL_IS_TREE_NODE (self), NULL);

  icon_name = dzl_tree_node_get_icon_name (self);

  if G_UNLIKELY (self->gicon == NULL && icon_name != NULL)
    {
      g_autoptr(GIcon) base = NULL;
      g_autoptr(GIcon) icon = NULL;

      base = g_themed_icon_new (icon_name);
      icon = g_emblemed_icon_new (base, NULL);

      for (GList *iter = self->emblems; iter != NULL; iter = iter->next)
        {
          const gchar *emblem_icon_name = iter->data;
          g_autoptr(GIcon) emblem_base = NULL;
          g_autoptr(GEmblem) emblem = NULL;

          emblem_base = g_themed_icon_new (emblem_icon_name);
          emblem = g_emblem_new (emblem_base);

          g_emblemed_icon_add_emblem (G_EMBLEMED_ICON (icon), emblem);
        }

      if (g_set_object (&self->gicon, icon))
        g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_GICON]);
    }

  return self->gicon;
}

/**
 * dzl_tree_node_get_icon_name:
 *
 * Fetches the icon-name of the icon to display, or NULL for no icon.
 */
const gchar *
dzl_tree_node_get_icon_name (DzlTreeNode *node)
{
  g_return_val_if_fail (DZL_IS_TREE_NODE (node), NULL);

  return g_quark_to_string (node->icon_name);
}

/**
 * dzl_tree_node_set_icon_name:
 * @node: A #DzlTreeNode.
 * @icon_name: (nullable): The icon name.
 *
 * Sets the icon name of the node. This is displayed in the pixbuf
 * cell of the DzlTree.
 */
void
dzl_tree_node_set_icon_name (DzlTreeNode *node,
                             const gchar *icon_name)
{
  GQuark value = 0;

  g_return_if_fail (DZL_IS_TREE_NODE (node));

  if (icon_name != NULL)
    value = g_quark_from_string (icon_name);

  if (value != node->icon_name)
    {
      node->icon_name = value;
      g_clear_object (&node->gicon);
      g_object_notify_by_pspec (G_OBJECT (node), properties [PROP_ICON_NAME]);
      g_object_notify_by_pspec (G_OBJECT (node), properties [PROP_GICON]);
      dzl_tree_node_queue_draw (node);
    }
}

/**
 * dzl_tree_node_add_emblem:
 * @self: An #DzlTreeNode
 * @emblem_name: the icon-name of the emblem
 *
 * Adds an emplem to be rendered on top of the node.
 *
 * Use dzl_tree_node_remove_emblem() to remove an emblem.
 */
void
dzl_tree_node_add_emblem (DzlTreeNode *self,
                          const gchar *emblem)
{
  g_return_if_fail (DZL_IS_TREE_NODE (self));

  for (GList *iter = self->emblems; iter != NULL; iter = iter->next)
    {
      const gchar *iter_icon_name = iter->data;

      if (g_strcmp0 (iter_icon_name, emblem) == 0)
        return;
    }

  self->emblems = g_list_prepend (self->emblems, g_strdup (emblem));
  g_clear_object (&self->gicon);
  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_GICON]);
  dzl_tree_node_queue_draw (self);
}

void
dzl_tree_node_remove_emblem (DzlTreeNode *self,
                             const gchar *emblem_name)
{
  g_return_if_fail (DZL_IS_TREE_NODE (self));

  for (GList *iter = self->emblems; iter != NULL; iter = iter->next)
    {
      gchar *iter_icon_name = iter->data;

      if (g_strcmp0 (iter_icon_name, emblem_name) == 0)
        {
          g_free (iter_icon_name);
          self->emblems = g_list_delete_link (self->emblems, iter);
          g_clear_object (&self->gicon);
          g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_GICON]);
          dzl_tree_node_queue_draw (self);
          return;
        }
    }
}

/**
 * dzl_tree_node_clear_emblems:
 *
 * Removes all emblems from @self.
 */
void
dzl_tree_node_clear_emblems (DzlTreeNode *self)
{
  g_return_if_fail (DZL_IS_TREE_NODE (self));

  g_list_free_full (self->emblems, g_free);
  self->emblems = NULL;
  g_clear_object (&self->gicon);
  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_GICON]);
  dzl_tree_node_queue_draw (self);
}

/**
 * dzl_tree_node_has_emblem:
 * @self: An #DzlTreeNode
 * @emblem_name: a string containing the emblem name
 *
 * Checks to see if @emblem_name has been added to the #DzlTreeNode.
 *
 * Returns: %TRUE if @emblem_name is used by @self
 */
gboolean
dzl_tree_node_has_emblem (DzlTreeNode *self,
                          const gchar *emblem_name)
{
  g_return_val_if_fail (DZL_IS_TREE_NODE (self), FALSE);

  for (GList *iter = self->emblems; iter != NULL; iter = iter->next)
    {
      const gchar *iter_icon_name = iter->data;

      if (g_strcmp0 (iter_icon_name, emblem_name) == 0)
        return TRUE;
    }

  return FALSE;
}

void
dzl_tree_node_set_emblems (DzlTreeNode         *self,
                           const gchar * const *emblems)
{
  g_return_if_fail (DZL_IS_TREE_NODE (self));

  if (self->emblems != NULL)
    {
      g_list_free_full (self->emblems, g_free);
      self->emblems = NULL;
    }

  if (emblems != NULL)
    {
      guint len = g_strv_length ((gchar **)emblems);

      for (guint i = len; i > 0; i--)
        self->emblems = g_list_prepend (self->emblems, g_strdup (emblems[i-1]));
    }

  g_clear_object (&self->gicon);
  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_GICON]);
  dzl_tree_node_queue_draw (self);
}

/**
 * dzl_tree_node_set_item:
 * @node: (in): A #DzlTreeNode.
 * @item: (in): A #GObject.
 *
 * An optional object to associate with the node. This is handy to save needing
 * to subclass the #DzlTreeNode class.
 */
void
dzl_tree_node_set_item (DzlTreeNode *node,
                        GObject     *item)
{
  g_return_if_fail (DZL_IS_TREE_NODE (node));
  g_return_if_fail (!item || G_IS_OBJECT (item));

  if (g_set_object (&node->item, item))
    g_object_notify_by_pspec (G_OBJECT (node), properties [PROP_ITEM]);
}

void
_dzl_tree_node_set_parent (DzlTreeNode *node,
                           DzlTreeNode *parent)
{
  g_return_if_fail (DZL_IS_TREE_NODE (node));
  g_return_if_fail (node->parent == NULL);
  g_return_if_fail (!parent || DZL_IS_TREE_NODE (parent));

  dzl_set_weak_pointer (&node->parent, parent);
}

const gchar *
dzl_tree_node_get_text (DzlTreeNode *node)
{
  g_return_val_if_fail (DZL_IS_TREE_NODE (node), NULL);

  return node->text;
}

/**
 * dzl_tree_node_set_text:
 * @node: A #DzlTreeNode.
 * @text: (nullable): The node text.
 *
 * Sets the text of the node. This is displayed in the text
 * cell of the DzlTree.
 */
void
dzl_tree_node_set_text (DzlTreeNode *node,
                        const gchar *text)
{
  g_return_if_fail (DZL_IS_TREE_NODE (node));

  if (g_strcmp0 (text, node->text) != 0)
    {
      g_free (node->text);
      node->text = g_strdup (text);
      g_object_notify_by_pspec (G_OBJECT (node), properties [PROP_TEXT]);
    }
}

gboolean
dzl_tree_node_get_use_markup (DzlTreeNode *self)
{
  g_return_val_if_fail (DZL_IS_TREE_NODE (self), FALSE);

  return self->use_markup;
}

void
dzl_tree_node_set_use_markup (DzlTreeNode *self,
                             gboolean    use_markup)
{
  g_return_if_fail (DZL_IS_TREE_NODE (self));

  use_markup = !!use_markup;

  if (self->use_markup != use_markup)
    {
      self->use_markup = use_markup;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_USE_MARKUP]);
    }
}

/**
 * dzl_tree_node_get_item:
 * @node: (in): A #DzlTreeNode.
 *
 * Gets a #GObject for the node, if one was set.
 *
 * Returns: (transfer none): A #GObject or %NULL.
 */
GObject *
dzl_tree_node_get_item (DzlTreeNode *node)
{
  g_return_val_if_fail (DZL_IS_TREE_NODE (node), NULL);

  return node->item;
}

gboolean
dzl_tree_node_expand (DzlTreeNode *node,
                      gboolean     expand_ancestors)
{
  DzlTree *tree;
  GtkTreePath *path;
  gboolean ret;

  g_return_val_if_fail (DZL_IS_TREE_NODE (node), FALSE);

  tree = dzl_tree_node_get_tree (node);
  path = dzl_tree_node_get_path (node);
  ret = gtk_tree_view_expand_row (GTK_TREE_VIEW (tree), path, FALSE);
  if (expand_ancestors)
    gtk_tree_view_expand_to_path (GTK_TREE_VIEW (tree), path);
  gtk_tree_path_free (path);

  return ret;
}

void
dzl_tree_node_collapse (DzlTreeNode *node)
{
  DzlTree *tree;
  GtkTreePath *path;

  g_return_if_fail (DZL_IS_TREE_NODE (node));

  tree = dzl_tree_node_get_tree (node);
  path = dzl_tree_node_get_path (node);
  gtk_tree_view_collapse_row (GTK_TREE_VIEW (tree), path);
  gtk_tree_path_free (path);
}

void
dzl_tree_node_select (DzlTreeNode *node)
{
  DzlTree *tree;
  GtkTreePath *path;
  GtkTreeSelection *selection;

  g_return_if_fail (DZL_IS_TREE_NODE (node));

  tree = dzl_tree_node_get_tree (node);
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
  path = dzl_tree_node_get_path (node);
  gtk_tree_selection_select_path (selection, path);
  gtk_tree_path_free (path);
}

void
dzl_tree_node_get_area (DzlTreeNode  *node,
                        GdkRectangle *area)
{
  DzlTree *tree;
  GtkTreeViewColumn *column;
  GtkTreePath *path;

  g_return_if_fail (DZL_IS_TREE_NODE (node));
  g_return_if_fail (area != NULL);

  tree = dzl_tree_node_get_tree (node);
  path = dzl_tree_node_get_path (node);
  column = gtk_tree_view_get_column (GTK_TREE_VIEW (tree), 0);
  gtk_tree_view_get_cell_area (GTK_TREE_VIEW (tree), path, column, area);
  gtk_tree_path_free (path);
}

void
dzl_tree_node_invalidate (DzlTreeNode *self)
{
  g_return_if_fail (DZL_IS_TREE_NODE (self));

  if (self->tree != NULL)
    _dzl_tree_invalidate (self->tree, self);
}

gboolean
dzl_tree_node_get_expanded (DzlTreeNode *self)
{
  gboolean ret = TRUE;

  g_return_val_if_fail (DZL_IS_TREE_NODE (self), FALSE);

  if ((self->tree != NULL) && (self->parent != NULL))
    {
      GtkTreePath *path = dzl_tree_node_get_path (self);

      if (path != NULL)
        {
          ret = gtk_tree_view_row_expanded (GTK_TREE_VIEW (self->tree), path);
          gtk_tree_path_free (path);
        }
    }

  return ret;
}

static void
dzl_tree_node_finalize (GObject *object)
{
  DzlTreeNode *self = DZL_TREE_NODE (object);

  g_clear_object (&self->item);
  g_clear_object (&self->gicon);
  g_clear_pointer (&self->text, g_free);

  g_list_free_full (self->emblems, g_free);
  self->emblems = NULL;

  dzl_clear_weak_pointer (&self->tree);
  dzl_clear_weak_pointer (&self->parent);

  G_OBJECT_CLASS (dzl_tree_node_parent_class)->finalize (object);
}

static void
dzl_tree_node_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  DzlTreeNode *node = DZL_TREE_NODE (object);

  switch (prop_id)
    {
    case PROP_CHILDREN_POSSIBLE:
      g_value_set_boolean (value, dzl_tree_node_get_children_possible (node));
      break;

    case PROP_EXPANDED_ICON_NAME:
      g_value_set_string (value, _dzl_tree_node_get_expanded_icon (node));
      break;

    case PROP_ICON_NAME:
      g_value_set_string (value, g_quark_to_string (node->icon_name));
      break;

    case PROP_ITEM:
      g_value_set_object (value, node->item);
      break;

    case PROP_GICON:
      g_value_set_object (value, node->gicon);
      break;

    case PROP_PARENT:
      g_value_set_object (value, node->parent);
      break;

    case PROP_RESET_ON_COLLAPSE:
      g_value_set_boolean (value, node->reset_on_collapse);
      break;

    case PROP_TEXT:
      g_value_set_string (value, node->text);
      break;

    case PROP_TREE:
      g_value_set_object (value, dzl_tree_node_get_tree (node));
      break;

    case PROP_USE_DIM_LABEL:
      g_value_set_boolean (value, node->use_dim_label);
      break;

    case PROP_USE_MARKUP:
      g_value_set_boolean (value, node->use_markup);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_tree_node_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  DzlTreeNode *node = DZL_TREE_NODE (object);

  switch (prop_id)
    {
    case PROP_CHILDREN_POSSIBLE:
      dzl_tree_node_set_children_possible (node, g_value_get_boolean (value));
      break;

    case PROP_EXPANDED_ICON_NAME:
      node->expanded_icon_name = g_quark_from_string (g_value_get_string (value));
      break;

    case PROP_GICON:
      dzl_tree_node_set_gicon (node, g_value_get_object (value));
      break;

    case PROP_ICON_NAME:
      dzl_tree_node_set_icon_name (node, g_value_get_string (value));
      break;

    case PROP_ITEM:
      dzl_tree_node_set_item (node, g_value_get_object (value));
      break;

    case PROP_RESET_ON_COLLAPSE:
      dzl_tree_node_set_reset_on_collapse (node, g_value_get_boolean (value));
      break;

    case PROP_TEXT:
      dzl_tree_node_set_text (node, g_value_get_string (value));
      break;

    case PROP_USE_DIM_LABEL:
      dzl_tree_node_set_use_dim_label (node, g_value_get_boolean (value));
      break;

    case PROP_USE_MARKUP:
      dzl_tree_node_set_use_markup (node, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_tree_node_class_init (DzlTreeNodeClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_tree_node_finalize;
  object_class->get_property = dzl_tree_node_get_property;
  object_class->set_property = dzl_tree_node_set_property;

  /**
   * DzlTreeNode:children-possible:
   *
   * This property allows for more lazy loading of nodes.
   *
   * When a node becomes visible, we normally build its children nodes
   * so that we know if we need an expansion arrow. However, that can
   * be expensive when rendering directories with lots of subdirectories.
   *
   * Using this, you can always show an arrow without building the children
   * and simply hide the arrow if there were in fact no children (upon
   * expansion).
   */
  properties [PROP_CHILDREN_POSSIBLE] =
    g_param_spec_boolean ("children-possible",
                          "Children Possible",
                          "Allows for lazy creation of children nodes.",
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_EXPANDED_ICON_NAME] =
    g_param_spec_string ("expanded-icon-name",
                         "Expanded Icon Name",
                         "The icon-name to use when the row is expanded",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * DzlTreeNode:icon-name:
   *
   * An icon-name to display on the row.
   */
  properties[PROP_ICON_NAME] =
    g_param_spec_string ("icon-name",
                         "Icon Name",
                         "The icon name to display.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);
  /**
   * DzlTreeNode:gicon:
   *
   * The cached GIcon to display.
   */
  properties[PROP_GICON] =
    g_param_spec_object ("gicon",
                         "GIcon",
                         "The GIcon object",
                         G_TYPE_ICON,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  /**
   * DzlTreeNode:item:
   *
   * An optional #GObject to associate with the node.
   */
  properties[PROP_ITEM] =
    g_param_spec_object ("item",
                         "Item",
                         "Optional object to associate with node.",
                         G_TYPE_OBJECT,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  /**
   * DzlTreeNode:parent:
   *
   * The parent of the node.
   */
  properties [PROP_PARENT] =
    g_param_spec_object ("parent",
                         "Parent",
                         "The parent node.",
                         DZL_TYPE_TREE_NODE,
                         G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  /**
   * DzlTreeNode:reset-on-collapse:
   *
   * The "reset-on-collapse" property denotes that all children should be
   * removed from the node when it's row is collapsed. It will also set
   * #DzlTreeNode:needs-build to %TRUE so the next expansion rebuilds the
   * children. This is useful for situations where you want to ensure the nodes
   * are up to date (refreshed) on every expansion.
   *
   * Since: 3.28
   */
  properties [PROP_RESET_ON_COLLAPSE] =
    g_param_spec_boolean ("reset-on-collapse",
                          "Reset on Collapse",
                          "Reset by clearing children on collapse, requiring a rebuild on next expand",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  /**
   * DzlTreeNode:tree:
   *
   * The tree the node belongs to.
   */
  properties [PROP_TREE] =
    g_param_spec_object ("tree",
                         "Tree",
                         "The DzlTree the node belongs to.",
                         DZL_TYPE_TREE,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

  /**
   * DzlTreeNode:text:
   *
   * Text to display on the tree node.
   */
  properties [PROP_TEXT] =
    g_param_spec_string ("text",
                         "Text",
                         "The text of the node.",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  /**
   * DzlTreeNode:use-markup:
   *
   * If the "text" property includes #GMarkup.
   */
  properties [PROP_USE_MARKUP] =
    g_param_spec_boolean ("use-markup",
                          "Use Markup",
                          "If text should be translated as markup.",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  properties [PROP_USE_DIM_LABEL] =
    g_param_spec_boolean ("use-dim-label",
                          "Use Dim Label",
                          "If text should be rendered with a dim label.",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
dzl_tree_node_init (DzlTreeNode *node)
{
  node->needs_build_children = TRUE;
}

static gboolean
dzl_tree_node_show_popover_timeout_cb (gpointer data)
{
  PopupRequest *popreq = data;
  GdkRectangle rect;
  GtkAllocation alloc;
  DzlTree *tree;

  g_assert (popreq);
  g_assert (DZL_IS_TREE_NODE (popreq->self));
  g_assert (GTK_IS_POPOVER (popreq->popover));

  if (!(tree = dzl_tree_node_get_tree (popreq->self)))
    goto cleanup;

  dzl_tree_node_get_area (popreq->self, &rect);
  gtk_widget_get_allocation (GTK_WIDGET (tree), &alloc);

  if ((rect.x + rect.width) > (alloc.x + alloc.width))
    rect.width = (alloc.x + alloc.width) - rect.x;

  /*
   * FIXME: Wouldn't this be better placed in a theme?
   */
  switch (gtk_popover_get_position (popreq->popover))
    {
    case GTK_POS_BOTTOM:
    case GTK_POS_TOP:
      rect.y += 3;
      rect.height -= 6;
      break;
    case GTK_POS_RIGHT:
    case GTK_POS_LEFT:
      rect.x += 3;
      rect.width -= 6;
      break;

    default:
      break;
    }

  gtk_popover_set_relative_to (popreq->popover, GTK_WIDGET (tree));
  gtk_popover_set_pointing_to (popreq->popover, &rect);
  gtk_popover_popup (popreq->popover);

cleanup:
  g_clear_object (&popreq->self);
  g_clear_object (&popreq->popover);
  g_slice_free (PopupRequest, popreq);

  return G_SOURCE_REMOVE;
}

void
dzl_tree_node_show_popover (DzlTreeNode *self,
                           GtkPopover *popover)
{
  GdkRectangle cell_area;
  GdkRectangle visible_rect;
  PopupRequest *popreq;
  DzlTree *tree;

  g_return_if_fail (DZL_IS_TREE_NODE (self));
  g_return_if_fail (GTK_IS_POPOVER (popover));

  tree = dzl_tree_node_get_tree (self);
  gtk_tree_view_get_visible_rect (GTK_TREE_VIEW (tree), &visible_rect);
  dzl_tree_node_get_area (self, &cell_area);
  gtk_tree_view_convert_bin_window_to_tree_coords (GTK_TREE_VIEW (tree),
                                                   cell_area.x,
                                                   cell_area.y,
                                                   &cell_area.x,
                                                   &cell_area.y);

  popreq = g_slice_new0 (PopupRequest);
  popreq->self = g_object_ref (self);
  popreq->popover = g_object_ref (popover);

  /*
   * If the node is not on screen, we need to animate until we get there.
   */
  if ((cell_area.y < visible_rect.y) ||
      ((cell_area.y + cell_area.height) >
       (visible_rect.y + visible_rect.height)))
    {
      GtkTreePath *path;

      path = dzl_tree_node_get_path (self);
      gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (tree), path, NULL, FALSE, 0, 0);
      g_clear_pointer (&path, gtk_tree_path_free);

      /*
       * FIXME: Time period comes from gtk animation duration.
       *        Not curently available in pubic API.
       *        We need to be greater than the max timeout it
       *        could take to move, since we must have it
       *        on screen by then.
       *
       *        One alternative might be to check the result
       *        and if we are still not on screen, then just
       *        pin it to a row-height from the top or bottom.
       */
      g_timeout_add (300,
                     dzl_tree_node_show_popover_timeout_cb,
                     popreq);

      return;
    }

  dzl_tree_node_show_popover_timeout_cb (g_steal_pointer (&popreq));
}

gboolean
_dzl_tree_node_is_dummy (DzlTreeNode *self)
{
  g_return_val_if_fail (DZL_IS_TREE_NODE (self), FALSE);

  return self->is_dummy;
}

gboolean
_dzl_tree_node_get_needs_build_children (DzlTreeNode *self)
{
  g_assert (DZL_IS_TREE_NODE (self));

  return self->needs_build_children;
}

void
_dzl_tree_node_set_needs_build_children (DzlTreeNode *self,
                                         gboolean     needs_build_children)
{
  g_assert (DZL_IS_TREE_NODE (self));

  self->needs_build_children = !!needs_build_children;

  if (!needs_build_children)
    self->is_dummy = FALSE;
}

void
_dzl_tree_node_add_dummy_child (DzlTreeNode *self)
{
  GtkTreeStore *model;
  DzlTreeNode *dummy;
  GtkTreeIter iter;
  GtkTreeIter parent;

  g_assert (DZL_IS_TREE_NODE (self));

  model = _dzl_tree_get_store (self->tree);
  dzl_tree_node_get_iter (self, &parent);
  dummy = g_object_ref_sink (dzl_tree_node_new ());
  _dzl_tree_node_set_tree (dummy, self->tree);
  _dzl_tree_node_set_parent (dummy, self);
  dummy->is_dummy = TRUE;
  gtk_tree_store_insert_with_values (model, &iter, &parent, -1,
                                     0, dummy,
                                     -1);
  g_clear_object (&dummy);
}

void
_dzl_tree_node_remove_dummy_child (DzlTreeNode *self)
{
  GtkTreeStore *model;
  GtkTreeIter iter;
  GtkTreeIter children;

  g_assert (DZL_IS_TREE_NODE (self));

  if (self->parent == NULL)
    return;

  model = _dzl_tree_get_store (self->tree);

  if (dzl_tree_node_get_iter (self, &iter) &&
      gtk_tree_model_iter_children (GTK_TREE_MODEL (model), &children, &iter))
    {
      while (gtk_tree_store_remove (model, &children))
        { /* Do nothing */ }
    }
}

gboolean
dzl_tree_node_get_children_possible (DzlTreeNode *self)
{
  g_return_val_if_fail (DZL_IS_TREE_NODE (self), FALSE);

  return self->children_possible;
}

/**
 * dzl_tree_node_set_children_possible:
 * @self: A #DzlTreeNode.
 * @children_possible: If the node has children.
 *
 * If the node has not yet been built, setting this to %TRUE will add a
 * dummy child node. This dummy node will be removed when when the node
 * is built by the registered #DzlTreeBuilder instances.
 */
void
dzl_tree_node_set_children_possible (DzlTreeNode *self,
                                     gboolean     children_possible)
{
  g_return_if_fail (DZL_IS_TREE_NODE (self));

  children_possible = !!children_possible;

  if (children_possible != self->children_possible)
    {
      self->children_possible = children_possible;

      if (self->tree != NULL && self->needs_build_children)
        {
          if (self->children_possible)
            _dzl_tree_node_add_dummy_child (self);
          else
            _dzl_tree_node_remove_dummy_child (self);
        }
    }
}

gboolean
dzl_tree_node_get_use_dim_label (DzlTreeNode *self)
{
  g_return_val_if_fail (DZL_IS_TREE_NODE (self), FALSE);

  return self->use_dim_label;
}

void
dzl_tree_node_set_use_dim_label (DzlTreeNode *self,
                                gboolean    use_dim_label)
{
  g_return_if_fail (DZL_IS_TREE_NODE (self));

  use_dim_label = !!use_dim_label;

  if (use_dim_label != self->use_dim_label)
    {
      self->use_dim_label = use_dim_label;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_USE_DIM_LABEL]);
    }
}

gboolean
dzl_tree_node_is_root (DzlTreeNode *node)
{
  g_return_val_if_fail (DZL_IS_TREE_NODE (node), FALSE);

  return node->parent == NULL;
}

const gchar *
_dzl_tree_node_get_expanded_icon (DzlTreeNode *node)
{
  g_return_val_if_fail (DZL_IS_TREE_NODE (node), NULL);

  return g_quark_to_string (node->expanded_icon_name);
}

gboolean
dzl_tree_node_get_reset_on_collapse (DzlTreeNode *self)
{
  g_return_val_if_fail (DZL_IS_TREE_NODE (self), FALSE);

  return self->reset_on_collapse;
}

void
dzl_tree_node_set_reset_on_collapse (DzlTreeNode *self,
                                     gboolean     reset_on_collapse)
{
  g_return_if_fail (DZL_IS_TREE_NODE (self));

  reset_on_collapse = !!reset_on_collapse;

  if (reset_on_collapse != self->reset_on_collapse)
    {
      self->reset_on_collapse = reset_on_collapse;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_RESET_ON_COLLAPSE]);
    }
}

guint
dzl_tree_node_n_children (DzlTreeNode *self)
{
  g_return_val_if_fail (DZL_IS_TREE_NODE (self), 0);

  if (!self->needs_build_children && self->tree != NULL)
    {
      GtkTreeIter iter;
      GtkTreeModel *model;

      model = GTK_TREE_MODEL (_dzl_tree_get_store (self->tree));

      if (dzl_tree_node_get_iter (self, &iter))
        return gtk_tree_model_iter_n_children (model, &iter);
    }

  return 0;
}

/**
 * dzl_tree_node_nth_child:
 * @self: a #DzlTreeNode
 * @nth: the index of the child
 *
 * Gets the @nth child of @self or %NULL if it does not exist.
 *
 * Returns: (transfer full) (nullable): a #DzlTreeNode or %NULL
 */
DzlTreeNode *
dzl_tree_node_nth_child (DzlTreeNode *self,
                         guint        nth)
{
  g_return_val_if_fail (DZL_IS_TREE_NODE (self), NULL);
  g_return_val_if_fail (!self->needs_build_children, NULL);

  if (self->tree != NULL)
    {
      GtkTreeIter parent;
      GtkTreeIter iter;
      GtkTreeModel *model;

      model = GTK_TREE_MODEL (_dzl_tree_get_store (self->tree));

      if (dzl_tree_node_get_iter (self, &parent) &&
          gtk_tree_model_iter_nth_child (model, &iter, &parent, nth))
        {
          g_autoptr(DzlTreeNode) node = NULL;

          gtk_tree_model_get (model, &iter, 0, &node, -1);
          g_assert (DZL_IS_TREE_NODE (node));

          /* Don't hand back a dummy node */
          if (_dzl_tree_node_is_dummy (node))
            return NULL;

          return g_steal_pointer (&node);
        }
    }

  return NULL;
}

/**
 * dzl_tree_node_get_foreground_rgba:
 * @self: a #DzlTreeNode
 *
 * Gets the foreground-rgba to use for row text.
 *
 * If %NULL, the default foreground color should be used.
 *
 * Returns: (nullable) (transfer none): A #GdkRGBA or %NULL
 *
 * Since: 3.28
 */
const GdkRGBA *
dzl_tree_node_get_foreground_rgba (DzlTreeNode *self)
{
  g_return_val_if_fail (DZL_IS_TREE_NODE (self), NULL);

  if (self->foreground_rgba_set)
    return &self->foreground_rgba;

  return NULL;
}

/**
 * dzl_tree_node_set_foreground_rgba:
 * @self: a #DzlTreeNode
 * @foreground_rgba: (nullable): A #GdkRGBA or %NULL
 *
 * Sets the foreground-rgba to be used by the row text.
 *
 * If @foreground_rgba is %NULL, the value is reset to the default.
 *
 * Since: 3.28
 */
void
dzl_tree_node_set_foreground_rgba (DzlTreeNode   *self,
                                   const GdkRGBA *foreground_rgba)
{
  g_return_if_fail (DZL_IS_TREE_NODE (self));

  if (foreground_rgba != NULL)
    self->foreground_rgba = *foreground_rgba;
  else
    memset (&self->foreground_rgba, 0, sizeof self->foreground_rgba);

  self->foreground_rgba_set = !!foreground_rgba;
}

/**
 * dzl_tree_node_rebuild:
 * @self: a #DzlTreeNode
 *
 * Rebuilds a node, without invalidating children nodes. If you want to
 * ensure that children are also rebuilt, use dzl_tree_node_invalidate().
 *
 * Since: 3.28
 */
void
dzl_tree_node_rebuild (DzlTreeNode *self)
{
  DzlTree *tree;

  g_return_if_fail (DZL_IS_TREE_NODE (self));

  tree = dzl_tree_node_get_tree (self);

  if (tree != NULL)
    _dzl_tree_build_node (tree, self);
}
