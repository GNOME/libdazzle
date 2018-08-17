/* dzl-read-only-list-model.c
 *
 * Copyright © 2018 Christian Hergert <chergert@redhat.com>
 *
 * This file is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#define G_LOG_DOMAIN "dzl-read-only-list-model"

#include "util/dzl-read-only-list-model.h"

struct _DzlReadOnlyListModel
{
  GObject     parent_instance;
  GListModel *base_model;
};

static GType
dzl_read_only_list_model_get_item_type (GListModel *model)
{
  DzlReadOnlyListModel *self = (DzlReadOnlyListModel *)model;

  g_assert (DZL_IS_READ_ONLY_LIST_MODEL (self));

  if (self->base_model != NULL)
    return g_list_model_get_item_type (self->base_model);

  return G_TYPE_OBJECT;
}

static guint
dzl_read_only_list_model_get_n_items (GListModel *model)
{
  DzlReadOnlyListModel *self = (DzlReadOnlyListModel *)model;

  g_assert (DZL_IS_READ_ONLY_LIST_MODEL (self));

  if (self->base_model != NULL)
    return g_list_model_get_n_items (self->base_model);

  return 0;
}

static gpointer
dzl_read_only_list_model_get_item (GListModel *model,
                                   guint       position)
{
  DzlReadOnlyListModel *self = (DzlReadOnlyListModel *)model;

  g_assert (DZL_IS_READ_ONLY_LIST_MODEL (self));

  if (self->base_model != NULL)
    return g_list_model_get_item (self->base_model, position);

  g_critical ("No item at position %u", position);

  return NULL;
}

static void
list_model_iface_init (GListModelInterface *iface)
{
  iface->get_n_items = dzl_read_only_list_model_get_n_items;
  iface->get_item = dzl_read_only_list_model_get_item;
  iface->get_item_type = dzl_read_only_list_model_get_item_type;
}

G_DEFINE_TYPE_WITH_CODE (DzlReadOnlyListModel, dzl_read_only_list_model, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, list_model_iface_init))

enum {
  PROP_0,
  PROP_BASE_MODEL,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
dzl_read_only_list_model_items_changed_cb (DzlReadOnlyListModel *self,
                                           guint                 position,
                                           guint                 removed,
                                           guint                 added,
                                           GListModel           *base_model)
{
  g_assert (DZL_IS_READ_ONLY_LIST_MODEL (self));
  g_assert (G_IS_LIST_MODEL (base_model));

  g_list_model_items_changed (G_LIST_MODEL (self), position, removed, added);
}

static void
dzl_read_only_list_model_set_base_model (DzlReadOnlyListModel *self,
                                         GListModel           *base_model)
{
  g_assert (DZL_IS_READ_ONLY_LIST_MODEL (self));

  if (base_model == NULL)
    return;

  self->base_model = g_object_ref (base_model);

  g_signal_connect_object (self->base_model,
                           "items-changed",
                           G_CALLBACK (dzl_read_only_list_model_items_changed_cb),
                           self,
                           G_CONNECT_SWAPPED);
}

static void
dzl_read_only_list_model_dispose (GObject *object)
{
  DzlReadOnlyListModel *self = (DzlReadOnlyListModel *)object;

  g_clear_object (&self->base_model);

  G_OBJECT_CLASS (dzl_read_only_list_model_parent_class)->dispose (object);
}

static void
dzl_read_only_list_model_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  DzlReadOnlyListModel *self = DZL_READ_ONLY_LIST_MODEL (object);

  switch (prop_id)
    {
    case PROP_BASE_MODEL:
      dzl_read_only_list_model_set_base_model (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_read_only_list_model_class_init (DzlReadOnlyListModelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = dzl_read_only_list_model_dispose;
  object_class->set_property = dzl_read_only_list_model_set_property;

  /**
   * DzlReadOnlyListModel:base-model:
   *
   * The "base-model" property is the #GListModel that will be wrapped.
   *
   * This base model is not accessible after creation so that API creators can
   * be sure the consumer cannot mutate the underlying model. That is useful
   * when you want to give a caller access to a #GListModel without the ability
   * to introspect on the type and mutate it without your knowledge (such as
   * with #GListStore).
   *
   * Since: 3.30
   */
  properties [PROP_BASE_MODEL] =
    g_param_spec_object ("base-model",
                         "Base Model",
                         "The list model to be wrapped as read-only",
                         G_TYPE_LIST_MODEL,
                         (G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));
  
  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_read_only_list_model_init (DzlReadOnlyListModel *self)
{
}

/**
 * dzl_read_only_list_model_new:
 * @base_model: a #GListModel
 *
 * Creates a new #DzlReadOnlyListModel which is a read-only wrapper around
 * @base_model. This is useful when you want to give API consumers access to
 * a #GListModel but without the ability to mutate the underlying list.
 *
 * Returns: (transfer full): a #DzlReadOnlyListModel
 *
 * Since: 3.30
 */
GListModel *
dzl_read_only_list_model_new (GListModel *base_model)
{
  g_return_val_if_fail (G_IS_LIST_MODEL (base_model), NULL);

  return g_object_new (DZL_TYPE_READ_ONLY_LIST_MODEL,
                       "base-model", base_model,
                       NULL);
}
