/* dzl-tree-builder.c
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

#define G_LOG_DOMAIN "dzl-tree-builder"

#include <glib/gi18n.h>

#include "tree/dzl-tree.h"
#include "tree/dzl-tree-builder.h"

typedef struct
{
  DzlTree *tree;
} DzlTreeBuilderPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (DzlTreeBuilder, dzl_tree_builder, G_TYPE_INITIALLY_UNOWNED)

enum {
  PROP_0,
  PROP_TREE,
  LAST_PROP
};

enum {
  ADDED,
  REMOVED,
  BUILD_NODE,
  NODE_ACTIVATED,
  NODE_COLLAPSED,
  NODE_EXPANDED,
  NODE_POPUP,
  NODE_SELECTED,
  NODE_UNSELECTED,
  LAST_SIGNAL
};

static GParamSpec *properties [LAST_PROP];
static guint signals [LAST_SIGNAL];

gboolean
_dzl_tree_builder_node_activated (DzlTreeBuilder *builder,
                                  DzlTreeNode    *node)
{
  gboolean ret = FALSE;

  g_return_val_if_fail (DZL_IS_TREE_BUILDER(builder), FALSE);
  g_return_val_if_fail (DZL_IS_TREE_NODE(node), FALSE);

  g_signal_emit (builder, signals [NODE_ACTIVATED], 0, node, &ret);

  return ret;
}

void
_dzl_tree_builder_node_popup (DzlTreeBuilder *builder,
                              DzlTreeNode    *node,
                              GMenu          *menu)
{
  g_return_if_fail (DZL_IS_TREE_BUILDER (builder));
  g_return_if_fail (DZL_IS_TREE_NODE (node));
  g_return_if_fail (G_IS_MENU (menu));

  g_signal_emit (builder, signals [NODE_POPUP], 0, node, menu);
}

void
_dzl_tree_builder_node_selected (DzlTreeBuilder *builder,
                                 DzlTreeNode    *node)
{
  g_return_if_fail (DZL_IS_TREE_BUILDER (builder));
  g_return_if_fail (DZL_IS_TREE_NODE (node));

  g_signal_emit (builder, signals [NODE_SELECTED], 0, node);
}

void
_dzl_tree_builder_node_unselected (DzlTreeBuilder *builder,
                                   DzlTreeNode    *node)
{
  g_return_if_fail (DZL_IS_TREE_BUILDER (builder));
  g_return_if_fail (DZL_IS_TREE_NODE (node));

  g_signal_emit (builder, signals [NODE_UNSELECTED], 0, node);
}

void
_dzl_tree_builder_build_node (DzlTreeBuilder *builder,
                              DzlTreeNode    *node)
{
  g_return_if_fail (DZL_IS_TREE_BUILDER (builder));
  g_return_if_fail (DZL_IS_TREE_NODE (node));

  g_signal_emit (builder, signals [BUILD_NODE], 0, node);
}

void
_dzl_tree_builder_added (DzlTreeBuilder *builder,
                         DzlTree        *tree)
{
  g_return_if_fail (DZL_IS_TREE_BUILDER (builder));
  g_return_if_fail (DZL_IS_TREE (tree));

  g_signal_emit (builder, signals [ADDED], 0, tree);
}

void
_dzl_tree_builder_removed (DzlTreeBuilder *builder,
                           DzlTree        *tree)
{
  g_return_if_fail (DZL_IS_TREE_BUILDER (builder));
  g_return_if_fail (DZL_IS_TREE (tree));

  g_signal_emit (builder, signals [REMOVED], 0, tree);
}

void
_dzl_tree_builder_node_collapsed (DzlTreeBuilder *builder,
                                  DzlTreeNode    *node)
{
  g_return_if_fail (DZL_IS_TREE_BUILDER (builder));
  g_return_if_fail (DZL_IS_TREE_NODE (node));

  g_signal_emit (builder, signals [NODE_COLLAPSED], 0, node);
}

void
_dzl_tree_builder_node_expanded (DzlTreeBuilder *builder,
                                 DzlTreeNode    *node)
{
  g_return_if_fail (DZL_IS_TREE_BUILDER (builder));
  g_return_if_fail (DZL_IS_TREE_NODE (node));

  g_signal_emit (builder, signals [NODE_EXPANDED], 0, node);
}

void
_dzl_tree_builder_set_tree (DzlTreeBuilder *builder,
                            DzlTree        *tree)
{
  DzlTreeBuilderPrivate *priv = dzl_tree_builder_get_instance_private (builder);

  g_return_if_fail (DZL_IS_TREE_BUILDER (builder));
  g_return_if_fail (priv->tree == NULL || DZL_IS_TREE (priv->tree));
  g_return_if_fail (DZL_IS_TREE (tree));

  if (priv->tree != tree)
    {
      if (priv->tree != NULL)
        {
          g_object_remove_weak_pointer (G_OBJECT (priv->tree), (gpointer *)&priv->tree);
          priv->tree = NULL;
        }

      if (tree != NULL)
        {
          priv->tree = tree;
          g_object_add_weak_pointer (G_OBJECT (priv->tree), (gpointer *)&priv->tree);
        }

      g_object_notify_by_pspec (G_OBJECT (builder), properties [PROP_TREE]);
    }
}

/**
 * dzl_tree_builder_get_tree:
 * @builder: (in): A #DzlTreeBuilder.
 *
 * Gets the tree that owns the builder.
 *
 * Returns: (transfer none) (type DzlTree) (nullable): A #DzlTree or %NULL.
 */
DzlTree *
dzl_tree_builder_get_tree (DzlTreeBuilder *builder)
{
  DzlTreeBuilderPrivate *priv = dzl_tree_builder_get_instance_private (builder);

  g_return_val_if_fail (DZL_IS_TREE_BUILDER (builder), NULL);

  return priv->tree;
}

static void
dzl_tree_builder_finalize (GObject *object)
{
  DzlTreeBuilder *builder = DZL_TREE_BUILDER (object);
  DzlTreeBuilderPrivate *priv = dzl_tree_builder_get_instance_private (builder);

	if (priv->tree)
    {
      g_object_remove_weak_pointer (G_OBJECT (priv->tree), (gpointer *)&priv->tree);
      priv->tree = NULL;
    }

  G_OBJECT_CLASS (dzl_tree_builder_parent_class)->finalize (object);
}

static void
dzl_tree_builder_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  DzlTreeBuilder *builder = DZL_TREE_BUILDER (object);
  DzlTreeBuilderPrivate *priv = dzl_tree_builder_get_instance_private (builder);

  switch (prop_id)
    {
    case PROP_TREE:
      g_value_set_object (value, priv->tree);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_tree_builder_class_init (DzlTreeBuilderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_tree_builder_finalize;
  object_class->get_property = dzl_tree_builder_get_property;

  properties[PROP_TREE] =
    g_param_spec_object("tree",
                        "Tree",
                        "The DzlTree the builder belongs to.",
                        DZL_TYPE_TREE,
                        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, LAST_PROP, properties);

  signals [ADDED] =
    g_signal_new ("added",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DzlTreeBuilderClass, added),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  DZL_TYPE_TREE);
  g_signal_set_va_marshaller (signals [ADDED],
                              G_TYPE_FROM_CLASS (klass),
                              g_cclosure_marshal_VOID__OBJECTv);

  signals [BUILD_NODE] =
    g_signal_new ("build-node",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DzlTreeBuilderClass, build_node),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  DZL_TYPE_TREE_NODE);
  g_signal_set_va_marshaller (signals [BUILD_NODE],
                              G_TYPE_FROM_CLASS (klass),
                              g_cclosure_marshal_VOID__OBJECTv);

  signals [NODE_ACTIVATED] =
    g_signal_new ("node-activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DzlTreeBuilderClass, node_activated),
                  NULL, NULL, NULL,
                  G_TYPE_BOOLEAN,
                  1,
                  DZL_TYPE_TREE_NODE);

  signals [NODE_COLLAPSED] =
    g_signal_new ("node-collapsed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DzlTreeBuilderClass, node_collapsed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  DZL_TYPE_TREE_NODE);
  g_signal_set_va_marshaller (signals [NODE_COLLAPSED],
                              G_TYPE_FROM_CLASS (klass),
                              g_cclosure_marshal_VOID__OBJECTv);

  signals [NODE_EXPANDED] =
    g_signal_new ("node-expanded",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DzlTreeBuilderClass, node_expanded),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  DZL_TYPE_TREE_NODE);
  g_signal_set_va_marshaller (signals [NODE_EXPANDED],
                              G_TYPE_FROM_CLASS (klass),
                              g_cclosure_marshal_VOID__OBJECTv);

  signals [NODE_POPUP] =
    g_signal_new ("node-popup",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DzlTreeBuilderClass, node_popup),
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  2,
                  DZL_TYPE_TREE_NODE,
                  G_TYPE_MENU);

  signals [NODE_SELECTED] =
    g_signal_new ("node-selected",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DzlTreeBuilderClass, node_selected),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  DZL_TYPE_TREE_NODE);
  g_signal_set_va_marshaller (signals [NODE_SELECTED],
                              G_TYPE_FROM_CLASS (klass),
                              g_cclosure_marshal_VOID__OBJECTv);

  signals [NODE_UNSELECTED] =
    g_signal_new ("node-unselected",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DzlTreeBuilderClass, node_unselected),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  DZL_TYPE_TREE_NODE);
  g_signal_set_va_marshaller (signals [NODE_UNSELECTED],
                              G_TYPE_FROM_CLASS (klass),
                              g_cclosure_marshal_VOID__OBJECTv);

  signals [REMOVED] =
    g_signal_new ("removed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (DzlTreeBuilderClass, removed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE,
                  1,
                  DZL_TYPE_TREE);
  g_signal_set_va_marshaller (signals [REMOVED],
                              G_TYPE_FROM_CLASS (klass),
                              g_cclosure_marshal_VOID__OBJECTv);
}

static void
dzl_tree_builder_init (DzlTreeBuilder *builder)
{
}

DzlTreeBuilder *
dzl_tree_builder_new (void)
{
  return g_object_new (DZL_TYPE_TREE_BUILDER, NULL);
}
