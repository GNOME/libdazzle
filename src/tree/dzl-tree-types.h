/* dzl-tree-types.h
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
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

#ifndef DZL_TREE_TYPES_H
#define DZL_TREE_TYPES_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_TREE         (dzl_tree_get_type())
#define DZL_TYPE_TREE_NODE    (dzl_tree_node_get_type())
#define DZL_TYPE_TREE_BUILDER (dzl_tree_builder_get_type())

typedef enum
{
  DZL_TREE_DROP_INTO   = 0,
  DZL_TREE_DROP_BEFORE = 1,
  DZL_TREE_DROP_AFTER  = 2,
} DzlTreeDropPosition;

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlTree,        dzl_tree,         DZL, TREE,         GtkTreeView)
DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlTreeBuilder, dzl_tree_builder, DZL, TREE_BUILDER, GInitiallyUnowned)
DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE     (DzlTreeNode,    dzl_tree_node,    DZL, TREE_NODE,    GInitiallyUnowned)

typedef gint (*DzlTreeNodeCompareFunc) (DzlTreeNode *a,
                                        DzlTreeNode *b,
                                        gpointer     user_data);

G_END_DECLS

#endif /* DZL_TREE_TYPES_H */
