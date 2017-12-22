/* dzl-tree-private.h
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

#ifndef DZL_TREE_PRIVATE_H
#define DZL_TREE_PRIVATE_H

#include "dzl-tree-types.h"

G_BEGIN_DECLS

void          _dzl_tree_invalidate                      (DzlTree                *tree,
                                                         DzlTreeNode            *node);
GtkTreePath  *_dzl_tree_get_path                        (DzlTree                *tree,
                                                         GList                  *list);
void          _dzl_tree_build_children                  (DzlTree                *self,
                                                         DzlTreeNode            *node);
void          _dzl_tree_build_node                      (DzlTree                *self,
                                                         DzlTreeNode            *node);
void          _dzl_tree_insert                          (DzlTree                *self,
                                                         DzlTreeNode            *node,
                                                         DzlTreeNode            *child,
                                                         guint                   position);
void          _dzl_tree_append                          (DzlTree                *self,
                                                         DzlTreeNode            *node,
                                                         DzlTreeNode            *child);
void          _dzl_tree_prepend                         (DzlTree                *self,
                                                         DzlTreeNode            *node,
                                                         DzlTreeNode            *child);
void          _dzl_tree_insert_sorted                   (DzlTree                *self,
                                                         DzlTreeNode            *node,
                                                         DzlTreeNode            *child,
                                                         DzlTreeNodeCompareFunc  compare_func,
                                                         gpointer                user_data);
void          _dzl_tree_remove                          (DzlTree                *self,
                                                         DzlTreeNode            *node);
DzlTreeNode  *_dzl_tree_get_drop_node                   (DzlTree                *self,
                                                         DzlTreeDropPosition    *pos);
GPtrArray    *_dzl_tree_get_builders                    (DzlTree                *self);
GdkDragAction _dzl_tree_get_drag_action                 (DzlTree                *self);
gboolean      _dzl_tree_get_iter                        (DzlTree                *self,
                                                         DzlTreeNode            *node,
                                                         GtkTreeIter            *iter);
GtkTreeStore *_dzl_tree_get_store                       (DzlTree                *self);
void          _dzl_tree_node_set_tree                   (DzlTreeNode            *node,
                                                         DzlTree                *tree);
void          _dzl_tree_node_set_parent                 (DzlTreeNode            *node,
                                                         DzlTreeNode            *parent);
const gchar  *_dzl_tree_node_get_expanded_icon          (DzlTreeNode            *node);
gboolean      _dzl_tree_node_get_needs_build_children   (DzlTreeNode            *node);
void          _dzl_tree_node_set_needs_build_children   (DzlTreeNode            *node,
                                                         gboolean                needs_build_children);
void          _dzl_tree_node_add_dummy_child            (DzlTreeNode            *node);
void          _dzl_tree_node_remove_dummy_child         (DzlTreeNode            *node);
gboolean      _dzl_tree_node_is_dummy                   (DzlTreeNode            *self);
void          _dzl_tree_builder_set_tree                (DzlTreeBuilder         *builder,
                                                         DzlTree                *tree);
void          _dzl_tree_builder_added                   (DzlTreeBuilder         *builder,
                                                         DzlTree                *tree);
void          _dzl_tree_builder_removed                 (DzlTreeBuilder         *builder,
                                                         DzlTree                *tree);
void          _dzl_tree_builder_build_node              (DzlTreeBuilder         *builder,
                                                         DzlTreeNode            *node);
void          _dzl_tree_builder_build_children          (DzlTreeBuilder         *builder,
                                                         DzlTreeNode            *node);
gboolean      _dzl_tree_builder_drag_data_get           (DzlTreeBuilder         *builder,
                                                         DzlTreeNode            *node,
                                                         GtkSelectionData       *data);
gboolean      _dzl_tree_builder_drag_data_received      (DzlTreeBuilder         *builder,
                                                         DzlTreeNode            *drop_node,
                                                         DzlTreeDropPosition     position,
                                                         GdkDragAction           action,
                                                         GtkSelectionData       *data);
gboolean      _dzl_tree_builder_drag_node_received      (DzlTreeBuilder         *builder,
                                                         DzlTreeNode            *drag_node,
                                                         DzlTreeNode            *drop_node,
                                                         DzlTreeDropPosition     position,
                                                         GdkDragAction           action,
                                                         GtkSelectionData       *data);
gboolean      _dzl_tree_builder_drag_node_delete        (DzlTreeBuilder         *builder,
                                                         DzlTreeNode            *node);
gboolean      _dzl_tree_builder_node_activated          (DzlTreeBuilder         *builder,
                                                         DzlTreeNode            *node);
void          _dzl_tree_builder_node_popup              (DzlTreeBuilder         *builder,
                                                         DzlTreeNode            *node,
                                                         GMenu                  *menu);
void          _dzl_tree_builder_node_selected           (DzlTreeBuilder         *builder,
                                                         DzlTreeNode            *node);
void          _dzl_tree_builder_node_unselected         (DzlTreeBuilder         *builder,
                                                         DzlTreeNode            *node);
void          _dzl_tree_builder_node_collapsed          (DzlTreeBuilder         *builder,
                                                         DzlTreeNode            *node);
void          _dzl_tree_builder_node_expanded           (DzlTreeBuilder         *builder,
                                                         DzlTreeNode            *node);
gboolean      _dzl_tree_builder_node_draggable          (DzlTreeBuilder         *builder,
                                                         DzlTreeNode            *node);
gboolean      _dzl_tree_builder_node_droppable          (DzlTreeBuilder         *builder,
                                                         DzlTreeNode            *node,
                                                         GtkSelectionData       *data);
GtkTreeStore *_dzl_tree_store_new                       (DzlTree                *tree);

G_END_DECLS

#endif /* DZL_TREE_PRIVATE_H */
