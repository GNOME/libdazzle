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

#include "dzl-version-macros.h"

#include "tree/dzl-tree-types.h"

G_BEGIN_DECLS

DZL_AVAILABLE_IN_ALL
DzlTreeNode    *dzl_tree_node_new                   (void);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_append                (DzlTreeNode            *node,
                                                     DzlTreeNode            *child);
DZL_AVAILABLE_IN_3_28
void            dzl_tree_node_insert                (DzlTreeNode            *self,
                                                     DzlTreeNode            *child,
                                                     guint                   position);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_insert_sorted         (DzlTreeNode            *node,
                                                     DzlTreeNode            *child,
                                                     DzlTreeNodeCompareFunc  compare_func,
                                                     gpointer                user_data);
DZL_AVAILABLE_IN_ALL
gboolean        dzl_tree_node_is_root               (DzlTreeNode            *node);
DZL_AVAILABLE_IN_ALL
const gchar    *dzl_tree_node_get_icon_name         (DzlTreeNode            *node);
DZL_AVAILABLE_IN_ALL
GObject        *dzl_tree_node_get_item              (DzlTreeNode            *node);
DZL_AVAILABLE_IN_ALL
DzlTreeNode    *dzl_tree_node_get_parent            (DzlTreeNode            *node);
DZL_AVAILABLE_IN_ALL
GtkTreePath    *dzl_tree_node_get_path              (DzlTreeNode            *node);
DZL_AVAILABLE_IN_ALL
gboolean        dzl_tree_node_get_iter              (DzlTreeNode            *node,
                                                     GtkTreeIter            *iter);
DZL_AVAILABLE_IN_3_28
guint           dzl_tree_node_n_children            (DzlTreeNode            *self);
DZL_AVAILABLE_IN_3_28
DzlTreeNode    *dzl_tree_node_nth_child             (DzlTreeNode            *self,
                                                     guint                   nth);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_prepend               (DzlTreeNode            *node,
                                                     DzlTreeNode            *child);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_remove                (DzlTreeNode            *node,
                                                     DzlTreeNode            *child);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_set_icon_name         (DzlTreeNode            *node,
                                                     const gchar            *icon_name);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_set_item              (DzlTreeNode            *node,
                                                     GObject                *item);
DZL_AVAILABLE_IN_ALL
gboolean        dzl_tree_node_expand                (DzlTreeNode            *node,
                                                     gboolean                expand_ancestors);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_collapse              (DzlTreeNode            *node);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_select                (DzlTreeNode            *node);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_get_area              (DzlTreeNode            *node,
                                                     GdkRectangle           *area);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_invalidate            (DzlTreeNode            *node);
DZL_AVAILABLE_IN_3_28
void            dzl_tree_node_rebuild               (DzlTreeNode            *self);
DZL_AVAILABLE_IN_ALL
gboolean        dzl_tree_node_get_expanded          (DzlTreeNode            *node);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_show_popover          (DzlTreeNode            *node,
                                                     GtkPopover             *popover);
DZL_AVAILABLE_IN_ALL
const gchar    *dzl_tree_node_get_text              (DzlTreeNode            *node);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_set_text              (DzlTreeNode            *node,
                                                     const gchar            *text);
DZL_AVAILABLE_IN_ALL
DzlTree        *dzl_tree_node_get_tree              (DzlTreeNode            *node);
DZL_AVAILABLE_IN_ALL
gboolean        dzl_tree_node_get_children_possible (DzlTreeNode            *self);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_set_children_possible (DzlTreeNode            *self,
                                                     gboolean                children_possible);
DZL_AVAILABLE_IN_ALL
gboolean        dzl_tree_node_get_use_markup        (DzlTreeNode            *self);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_set_use_markup        (DzlTreeNode            *self,
                                                     gboolean                use_markup);
DZL_AVAILABLE_IN_ALL
GIcon          *dzl_tree_node_get_gicon             (DzlTreeNode            *self);
DZL_AVAILABLE_IN_3_28
void            dzl_tree_node_set_gicon             (DzlTreeNode            *self,
                                                     GIcon                  *icon);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_add_emblem            (DzlTreeNode            *self,
                                                     const gchar            *emblem_name);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_remove_emblem         (DzlTreeNode            *self,
                                                     const gchar            *emblem_name);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_clear_emblems         (DzlTreeNode            *self);
DZL_AVAILABLE_IN_ALL
gboolean        dzl_tree_node_has_emblem            (DzlTreeNode            *self,
                                                     const gchar            *emblem_name);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_set_emblems           (DzlTreeNode            *self,
                                                     const gchar * const    *emblems);
DZL_AVAILABLE_IN_ALL
gboolean        dzl_tree_node_get_use_dim_label     (DzlTreeNode            *self);
DZL_AVAILABLE_IN_ALL
void            dzl_tree_node_set_use_dim_label     (DzlTreeNode            *self,
                                                     gboolean                use_dim_label);
DZL_AVAILABLE_IN_3_28
gboolean        dzl_tree_node_get_reset_on_collapse (DzlTreeNode            *self);
DZL_AVAILABLE_IN_3_28
void            dzl_tree_node_set_reset_on_collapse (DzlTreeNode            *self,
                                                     gboolean                reset_on_collapse);
DZL_AVAILABLE_IN_3_28
const GdkRGBA  *dzl_tree_node_get_foreground_rgba   (DzlTreeNode            *self);
DZL_AVAILABLE_IN_3_28
void            dzl_tree_node_set_foreground_rgba   (DzlTreeNode            *self,
                                                     const GdkRGBA          *foreground_rgba);

G_END_DECLS

#endif /* DZL_TREE_NODE_H */
