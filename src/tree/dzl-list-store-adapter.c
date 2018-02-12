/* dzl-list-store-adapter.c
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
 *
 * This file is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define G_LOG_DOMAIN "dzl-list-store-adapter"

#include "config.h"

#include "bindings/dzl-signal-group.h"
#include "tree/dzl-list-store-adapter.h"

typedef struct
{
  DzlSignalGroup *signals;
  GListModel *model;
  gint length;
  GType type;
} DzlListStoreAdapterPrivate;

enum {
  PROP_0,
  PROP_MODEL,
  N_PROPS
};

static void tree_model_iface_init (GtkTreeModelIface *iface);

G_DEFINE_TYPE_WITH_CODE (DzlListStoreAdapter, dzl_list_store_adapter, G_TYPE_OBJECT,
                         G_ADD_PRIVATE (DzlListStoreAdapter)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_MODEL, tree_model_iface_init))

static GParamSpec *properties [N_PROPS];

static GtkTreeModelFlags
dzl_list_store_adapter_get_flags (GtkTreeModel *model)
{
  return GTK_TREE_MODEL_ITERS_PERSIST | GTK_TREE_MODEL_LIST_ONLY;
}

static gint
dzl_list_store_adapter_get_n_columns (GtkTreeModel *model)
{
  return 1;
}

static GType
dzl_list_store_adapter_get_column_type (GtkTreeModel *model,
                                        gint          column)
{
  DzlListStoreAdapter *self = DZL_LIST_STORE_ADAPTER (model);
  DzlListStoreAdapterPrivate *priv = dzl_list_store_adapter_get_instance_private (self);

  if (column == 0)
    return priv->type;

  return G_TYPE_INVALID;
}

static gboolean
dzl_list_store_adapter_get_iter (GtkTreeModel *model,
                                 GtkTreeIter  *iter,
                                 GtkTreePath  *path)
{
  DzlListStoreAdapter *self = (DzlListStoreAdapter *)model;
  DzlListStoreAdapterPrivate *priv = dzl_list_store_adapter_get_instance_private (self);
  gint pos;

  g_assert (DZL_IS_LIST_STORE_ADAPTER (self));
  g_assert (iter != NULL);
  g_assert (path != NULL);

  if (gtk_tree_path_get_depth (path) != 1)
    return FALSE;

  pos = gtk_tree_path_get_indices (path) [0];

  if (pos >= priv->length)
    return FALSE;

  iter->user_data = GINT_TO_POINTER (pos);

  return TRUE;
}

static GtkTreePath *
dzl_list_store_adapter_get_path (GtkTreeModel *model,
                                 GtkTreeIter  *iter)
{
  return gtk_tree_path_new_from_indices (GPOINTER_TO_INT (iter->user_data), -1);
}

static void
dzl_list_store_adapter_get_value (GtkTreeModel *model,
                                  GtkTreeIter  *iter,
                                  gint          column,
                                  GValue       *value)
{
  DzlListStoreAdapter *self = DZL_LIST_STORE_ADAPTER (model);
  DzlListStoreAdapterPrivate *priv = dzl_list_store_adapter_get_instance_private (self);

  g_value_init (value, priv->type);
  g_value_take_object (value,
                       g_list_model_get_item (priv->model,
                                              GPOINTER_TO_INT (iter->user_data)));
}

static gboolean
dzl_list_store_adapter_iter_next (GtkTreeModel *model,
                                  GtkTreeIter  *iter)
{
  DzlListStoreAdapter *self = DZL_LIST_STORE_ADAPTER (model);
  DzlListStoreAdapterPrivate *priv = dzl_list_store_adapter_get_instance_private (self);
  gint pos = GPOINTER_TO_INT (iter->user_data) + 1;

  iter->user_data = GINT_TO_POINTER (pos);

  return pos < priv->length;
}

static gboolean
dzl_list_store_adapter_iter_previous (GtkTreeModel *model,
                                      GtkTreeIter  *iter)
{
  gint pos = GPOINTER_TO_INT (iter->user_data) - 1;

  iter->user_data = GINT_TO_POINTER (pos);

  return pos >= 0;
}

static gboolean
dzl_list_store_adapter_iter_children (GtkTreeModel *model,
                                      GtkTreeIter  *iter,
                                      GtkTreeIter  *parent)
{
  if (parent != NULL)
    return FALSE;

  iter->user_data = NULL;

  return TRUE;
}

static gboolean
dzl_list_store_adapter_iter_has_child (GtkTreeModel *model,
                                       GtkTreeIter  *iter)
{
  return iter == NULL;
}

static gint
dzl_list_store_adapter_iter_n_children (GtkTreeModel *model,
                                        GtkTreeIter  *iter)
{
  DzlListStoreAdapter *self = DZL_LIST_STORE_ADAPTER (model);
  DzlListStoreAdapterPrivate *priv = dzl_list_store_adapter_get_instance_private (self);

  if (iter == NULL)
    return priv->length;

  return 0;
}

static gboolean
dzl_list_store_adapter_iter_parent (GtkTreeModel *model,
                                    GtkTreeIter  *iter,
                                    GtkTreeIter  *child)
{
  return FALSE;
}

static gboolean
dzl_list_store_adapter_iter_nth_child (GtkTreeModel *model,
                                       GtkTreeIter  *iter,
                                       GtkTreeIter  *parent,
                                       gint          nth)
{
  DzlListStoreAdapter *self = DZL_LIST_STORE_ADAPTER (model);
  DzlListStoreAdapterPrivate *priv = dzl_list_store_adapter_get_instance_private (self);

  if (parent == NULL && nth < priv->length)
    {
      iter->user_data = GINT_TO_POINTER (nth);
      return TRUE;
    }

  return FALSE;
}

static void
tree_model_iface_init (GtkTreeModelIface *iface)
{
  iface->get_flags = dzl_list_store_adapter_get_flags;
  iface->get_n_columns = dzl_list_store_adapter_get_n_columns;
  iface->get_column_type = dzl_list_store_adapter_get_column_type;
  iface->get_iter = dzl_list_store_adapter_get_iter;
  iface->get_path = dzl_list_store_adapter_get_path;
  iface->get_value = dzl_list_store_adapter_get_value;
  iface->iter_next = dzl_list_store_adapter_iter_next;
  iface->iter_previous = dzl_list_store_adapter_iter_previous;
  iface->iter_children = dzl_list_store_adapter_iter_children;
  iface->iter_has_child = dzl_list_store_adapter_iter_has_child;
  iface->iter_n_children = dzl_list_store_adapter_iter_n_children;
  iface->iter_nth_child = dzl_list_store_adapter_iter_nth_child;
  iface->iter_parent = dzl_list_store_adapter_iter_parent;
}

static void
dzl_list_store_adapter_items_changed (DzlListStoreAdapter *self,
                                      guint                position,
                                      guint                removed,
                                      guint                added,
                                      GListModel          *model)
{
  DzlListStoreAdapterPrivate *priv = dzl_list_store_adapter_get_instance_private (self);
  GtkTreePath *path;
  GtkTreeIter iter = { 0 };

  g_assert (DZL_IS_LIST_STORE_ADAPTER (self));
  g_assert (G_IS_LIST_MODEL (model));

  priv->length -= removed;
  priv->length += added;

  path = gtk_tree_path_new_from_indices (position, -1);

  for (guint i = 0; i < removed; i++)
    gtk_tree_model_row_deleted (GTK_TREE_MODEL (self), path);

  for (guint i = 0; i < added; i++)
    {
      iter.user_data = GINT_TO_POINTER (position + i);
      gtk_tree_model_row_inserted (GTK_TREE_MODEL (self), path, &iter);
      gtk_tree_path_next (path);
    }

  gtk_tree_path_free (path);
}

static void
dzl_list_store_adapter_bind (DzlListStoreAdapter *self,
                             GListModel          *model,
                             DzlSignalGroup      *signals)
{
  DzlListStoreAdapterPrivate *priv = dzl_list_store_adapter_get_instance_private (self);

  g_assert (DZL_IS_LIST_STORE_ADAPTER (self));
  g_assert (G_IS_LIST_MODEL (model));
  g_assert (DZL_IS_SIGNAL_GROUP (signals));

  priv->model = model;
  priv->type = g_list_model_get_item_type (model);
  priv->length = g_list_model_get_n_items (model);
}

static void
dzl_list_store_adapter_unbind (DzlListStoreAdapter *self,
                               DzlSignalGroup      *signals)
{
  DzlListStoreAdapterPrivate *priv = dzl_list_store_adapter_get_instance_private (self);

  g_assert (DZL_IS_LIST_STORE_ADAPTER (self));
  g_assert (DZL_IS_SIGNAL_GROUP (signals));

  priv->model = NULL;
  priv->length = 0;
  priv->type = G_TYPE_OBJECT;
}

static void
dzl_list_store_adapter_finalize (GObject *object)
{
  DzlListStoreAdapter *self = (DzlListStoreAdapter *)object;
  DzlListStoreAdapterPrivate *priv = dzl_list_store_adapter_get_instance_private (self);

  g_clear_object (&priv->signals);

  G_OBJECT_CLASS (dzl_list_store_adapter_parent_class)->finalize (object);
}

static void
dzl_list_store_adapter_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  DzlListStoreAdapter *self = DZL_LIST_STORE_ADAPTER (object);

  switch (prop_id)
    {
    case PROP_MODEL:
      g_value_set_object (value, dzl_list_store_adapter_get_model (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_list_store_adapter_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  DzlListStoreAdapter *self = DZL_LIST_STORE_ADAPTER (object);

  switch (prop_id)
    {
    case PROP_MODEL:
      dzl_list_store_adapter_set_model (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_list_store_adapter_class_init (DzlListStoreAdapterClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_list_store_adapter_finalize;
  object_class->get_property = dzl_list_store_adapter_get_property;
  object_class->set_property = dzl_list_store_adapter_set_property;

  properties [PROP_MODEL] =
    g_param_spec_object ("model",
                         "Model",
                         "The model to be adapted",
                         G_TYPE_LIST_MODEL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_list_store_adapter_init (DzlListStoreAdapter *self)
{
  DzlListStoreAdapterPrivate *priv = dzl_list_store_adapter_get_instance_private (self);

  priv->type = G_TYPE_OBJECT;
  priv->signals = dzl_signal_group_new (G_TYPE_LIST_MODEL);

  dzl_signal_group_connect_swapped (priv->signals,
                                    "items-changed",
                                    G_CALLBACK (dzl_list_store_adapter_items_changed),
                                    self);

  g_signal_connect_swapped (priv->signals,
                            "bind",
                            G_CALLBACK (dzl_list_store_adapter_bind),
                            self);

  g_signal_connect_swapped (priv->signals,
                            "unbind",
                            G_CALLBACK (dzl_list_store_adapter_unbind),
                            self);
}

DzlListStoreAdapter *
dzl_list_store_adapter_new (GListModel *model)
{
  return g_object_new (DZL_TYPE_LIST_STORE_ADAPTER,
                       "model", model,
                       NULL);
}

/**
 * dzl_list_store_adapter_get_model:
 * @self: A #DzlListStoreAdapter
 *
 * Gets the model being adapted.
 *
 * Returns: (transfer none): A #GListModel
 *
 * Since: 3.26
 */
GListModel *
dzl_list_store_adapter_get_model (DzlListStoreAdapter *self)
{
  DzlListStoreAdapterPrivate *priv = dzl_list_store_adapter_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_LIST_STORE_ADAPTER (self), NULL);

  return dzl_signal_group_get_target (priv->signals);
}

void
dzl_list_store_adapter_set_model (DzlListStoreAdapter *self,
                                  GListModel          *model)
{
  DzlListStoreAdapterPrivate *priv = dzl_list_store_adapter_get_instance_private (self);

  g_return_if_fail (DZL_IS_LIST_STORE_ADAPTER (self));
  g_return_if_fail (!model || G_IS_LIST_MODEL (model));

  dzl_signal_group_set_target (priv->signals, model);
  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_MODEL]);
}
