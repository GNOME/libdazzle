/* dzl-tree-node.h
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

#ifndef DZL_TREE_NODE_H
#define DZL_TREE_NODE_H

#include "tree/dzl-tree-types.h"

G_BEGIN_DECLS

DzlTreeNode    *dzl_tree_node_new                   (void);
void            dzl_tree_node_append                (DzlTreeNode            *node,
                                                     DzlTreeNode            *child);
void            dzl_tree_node_insert_sorted         (DzlTreeNode            *node,
                                                     DzlTreeNode            *child,
                                                     DzlTreeNodeCompareFunc  compare_func,
                                                     gpointer                user_data);
gboolean        dzl_tree_node_is_root               (DzlTreeNode            *node);
const gchar    *dzl_tree_node_get_icon_name         (DzlTreeNode            *node);
GObject        *dzl_tree_node_get_item              (DzlTreeNode            *node);
DzlTreeNode    *dzl_tree_node_get_parent            (DzlTreeNode            *node);
GtkTreePath    *dzl_tree_node_get_path              (DzlTreeNode            *node);
gboolean        dzl_tree_node_get_iter              (DzlTreeNode            *node,
                                                     GtkTreeIter            *iter);
void            dzl_tree_node_prepend               (DzlTreeNode            *node,
                                                     DzlTreeNode            *child);
void            dzl_tree_node_remove                (DzlTreeNode            *node,
                                                     DzlTreeNode            *child);
void            dzl_tree_node_set_icon_name         (DzlTreeNode            *node,
                                                     const gchar            *icon_name);
void            dzl_tree_node_set_item              (DzlTreeNode            *node,
                                                     GObject                *item);
gboolean        dzl_tree_node_expand                (DzlTreeNode            *node,
                                                     gboolean                expand_ancestors);
void            dzl_tree_node_collapse              (DzlTreeNode            *node);
void            dzl_tree_node_select                (DzlTreeNode            *node);
void            dzl_tree_node_get_area              (DzlTreeNode            *node,
                                                     GdkRectangle           *area);
void            dzl_tree_node_invalidate            (DzlTreeNode            *node);
gboolean        dzl_tree_node_get_expanded          (DzlTreeNode            *node);
void            dzl_tree_node_show_popover          (DzlTreeNode            *node,
                                                     GtkPopover             *popover);
const gchar    *dzl_tree_node_get_text              (DzlTreeNode            *node);
void            dzl_tree_node_set_text              (DzlTreeNode            *node,
                                                     const gchar            *text);
DzlTree        *dzl_tree_node_get_tree              (DzlTreeNode            *node);
gboolean        dzl_tree_node_get_children_possible (DzlTreeNode            *self);
void            dzl_tree_node_set_children_possible (DzlTreeNode            *self,
                                                     gboolean                children_possible);
gboolean        dzl_tree_node_get_use_markup        (DzlTreeNode            *self);
void            dzl_tree_node_set_use_markup        (DzlTreeNode            *self,
                                                     gboolean                use_markup);
GIcon          *dzl_tree_node_get_gicon             (DzlTreeNode            *self);
void            dzl_tree_node_add_emblem            (DzlTreeNode            *self,
                                                     const gchar            *emblem_name);
void            dzl_tree_node_remove_emblem         (DzlTreeNode            *self,
                                                     const gchar            *emblem_name);
void            dzl_tree_node_clear_emblems         (DzlTreeNode            *self);
gboolean        dzl_tree_node_has_emblem            (DzlTreeNode            *self,
                                                     const gchar            *emblem_name);
void            dzl_tree_node_set_emblems           (DzlTreeNode            *self,
                                                     const gchar * const    *emblems);
gboolean        dzl_tree_node_get_use_dim_label     (DzlTreeNode            *self);
void            dzl_tree_node_set_use_dim_label     (DzlTreeNode            *self,
                                                     gboolean                use_dim_label);

G_END_DECLS

#endif /* DZL_TREE_NODE_H */
