/* dzl-tree-builder.h
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

#ifndef DZL_TREE_BUILDER_H
#define DZL_TREE_BUILDER_H

#include <glib-object.h>

#include "tree/dzl-tree-node.h"
#include "tree/dzl-tree-types.h"

G_BEGIN_DECLS

struct _DzlTreeBuilderClass
{
  GInitiallyUnownedClass parent_class;

  void     (*added)           (DzlTreeBuilder *builder,
                               GtkWidget      *tree);
  void     (*removed)         (DzlTreeBuilder *builder,
                               GtkWidget      *tree);
  void     (*build_node)      (DzlTreeBuilder *builder,
                               DzlTreeNode    *node);
  gboolean (*node_activated)  (DzlTreeBuilder *builder,
                               DzlTreeNode    *node);
  void     (*node_selected)   (DzlTreeBuilder *builder,
                               DzlTreeNode    *node);
  void     (*node_unselected) (DzlTreeBuilder *builder,
                               DzlTreeNode    *node);
  void     (*node_popup)      (DzlTreeBuilder *builder,
                               DzlTreeNode    *node,
                               GMenu          *menu);
};

DzlTree *dzl_tree_builder_get_tree (DzlTreeBuilder *builder);

G_END_DECLS

#endif /* DZL_TREE_BUILDER_H */
