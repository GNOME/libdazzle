/* dzl-tree.h
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

#ifndef DZL_TREE_H
#define DZL_TREE_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

#include "tree/dzl-tree-builder.h"
#include "tree/dzl-tree-node.h"
#include "tree/dzl-tree-types.h"

G_BEGIN_DECLS

/**
 * DzlTreeFindFunc:
 *
 * Callback to check @child, a child of @node, matches a lookup
 * request. Returns %TRUE if @child matches, %FALSE if not.
 *
 * Returns: %TRUE if @child matched
 */
typedef gboolean (*DzlTreeFindFunc) (DzlTree     *tree,
                                     DzlTreeNode *node,
                                     DzlTreeNode *child,
                                     gpointer     user_data);

/**
 * DzlTreeFilterFunc:
 *
 * Callback to check if @node should be visible.
 *
 * Returns: %TRUE if @node should be visible.
 */
typedef gboolean (*DzlTreeFilterFunc) (DzlTree     *tree,
                                       DzlTreeNode *node,
                                       gpointer     user_data);

struct _DzlTreeClass
{
	GtkTreeViewClass parent_class;

  void (*action)         (DzlTree     *self,
                          const gchar *action_group,
                          const gchar *action_name,
                          const gchar *param);
  void (*populate_popup) (DzlTree     *self,
                          GtkWidget   *widget);

  /*< private >*/
  gpointer _padding[12];
};

DZL_AVAILABLE_IN_ALL
void          dzl_tree_add_builder      (DzlTree           *self,
                                         DzlTreeBuilder    *builder);
DZL_AVAILABLE_IN_ALL
void          dzl_tree_remove_builder   (DzlTree           *self,
                                         DzlTreeBuilder    *builder);
DZL_AVAILABLE_IN_ALL
DzlTreeNode   *dzl_tree_find_item       (DzlTree           *self,
                                         GObject           *item);
DZL_AVAILABLE_IN_ALL
DzlTreeNode   *dzl_tree_find_custom     (DzlTree           *self,
                                         GEqualFunc         equal_func,
                                         gpointer           key);
DZL_AVAILABLE_IN_ALL
DzlTreeNode   *dzl_tree_get_selected    (DzlTree           *self);
DZL_AVAILABLE_IN_ALL
void          dzl_tree_unselect_all     (DzlTree           *self);
DZL_AVAILABLE_IN_ALL
void          dzl_tree_rebuild          (DzlTree           *self);
DZL_AVAILABLE_IN_ALL
void          dzl_tree_set_root         (DzlTree           *self,
                                         DzlTreeNode       *node);
DZL_AVAILABLE_IN_ALL
DzlTreeNode   *dzl_tree_get_root        (DzlTree           *self);
DZL_AVAILABLE_IN_ALL
void          dzl_tree_set_show_icons   (DzlTree           *self,
                                         gboolean           show_icons);
DZL_AVAILABLE_IN_ALL
gboolean      dzl_tree_get_show_icons   (DzlTree           *self);
DZL_AVAILABLE_IN_ALL
void          dzl_tree_scroll_to_node   (DzlTree           *self,
                                         DzlTreeNode       *node);
DZL_AVAILABLE_IN_ALL
void          dzl_tree_expand_to_node   (DzlTree           *self,
                                         DzlTreeNode       *node);
DZL_AVAILABLE_IN_ALL
DzlTreeNode   *dzl_tree_find_child_node (DzlTree           *self,
                                         DzlTreeNode       *node,
                                         DzlTreeFindFunc    find_func,
                                         gpointer           user_data);
DZL_AVAILABLE_IN_ALL
void          dzl_tree_set_filter       (DzlTree           *self,
                                         DzlTreeFilterFunc  filter_func,
                                         gpointer           filter_data,
                                         GDestroyNotify     filter_data_destroy);
DZL_AVAILABLE_IN_ALL
GMenuModel   *dzl_tree_get_context_menu (DzlTree           *self);
DZL_AVAILABLE_IN_ALL
void          dzl_tree_set_context_menu (DzlTree           *self,
                                         GMenuModel        *context_menu);

G_END_DECLS

#endif /* DZL_TREE_H */
