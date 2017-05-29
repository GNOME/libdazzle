/* dzl-list-box.c
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "dzl-list-box"

/*
 * This widget is just like GtkListBox, except that it allows you to
 * very simply re-use existing widgets instead of creating new widgets
 * all the time.
 *
 * It does not, however, try to keep the number of inflated widgets
 * low (that would require more work in GtkListBox directly).
 *
 * This mostly just avoids the overhead of reparsing the template XML
 * on every widget (re)creation.
 */

#include "dzl-list-box.h"

typedef struct
{
  GListModel *model;
  gchar      *property_name;
  GType       row_type;
  guint       recycle_max;
  GQueue      trashed_rows;
} DzlListBoxPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (DzlListBox, dzl_list_box, GTK_TYPE_LIST_BOX)

enum {
  PROP_0,
  PROP_PROPERTY_NAME,
  PROP_ROW_TYPE,
  PROP_ROW_TYPE_NAME,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static GtkWidget *
dzl_list_box_create_row (gpointer item,
                         gpointer user_data)
{
  DzlListBox *self = user_data;
  DzlListBoxPrivate *priv = dzl_list_box_get_instance_private (self);

  g_assert (G_IS_OBJECT (item));
  g_assert (DZL_IS_LIST_BOX (self));

  if (priv->trashed_rows.length > 0)
    {
      GtkListBoxRow *row = g_queue_pop_tail (&priv->trashed_rows);

      g_object_set (row, priv->property_name, item, NULL);
      g_object_force_floating (G_OBJECT (row));
      g_object_unref (row);

      return GTK_WIDGET (row);
    }

  return g_object_new (priv->row_type,
                       "visible", TRUE,
                       priv->property_name, item,
                       NULL);
}

static void
dzl_list_box_remove (GtkContainer *container,
                     GtkWidget    *widget)
{
  DzlListBox *self = (DzlListBox *)container;
  DzlListBoxPrivate *priv = dzl_list_box_get_instance_private (self);

  g_assert (DZL_IS_LIST_BOX (self));
  g_assert (GTK_IS_LIST_BOX_ROW (widget));

  g_object_ref (widget);

  GTK_CONTAINER_CLASS (dzl_list_box_parent_class)->remove (container, widget);

  if (priv->trashed_rows.length < priv->recycle_max)
    {
      g_object_set (widget, priv->property_name, NULL, NULL);
      g_queue_push_head (&priv->trashed_rows, g_steal_pointer (&widget));
    }

  g_clear_object (&widget);
}

static void
dzl_list_box_constructed (GObject *object)
{
  DzlListBox *self = (DzlListBox *)object;
  DzlListBoxPrivate *priv = dzl_list_box_get_instance_private (self);
  GObjectClass *row_class;
  GParamSpec *pspec;
  gboolean valid;

  G_OBJECT_CLASS (dzl_list_box_parent_class)->constructed (object);

  if (!g_type_is_a (priv->row_type, GTK_TYPE_LIST_BOX_ROW) || !priv->property_name)
    goto failure;

  row_class = g_type_class_ref (priv->row_type);
  pspec = g_object_class_find_property (row_class, priv->property_name);
  valid = (pspec != NULL) && g_type_is_a (pspec->value_type, G_TYPE_OBJECT);
  g_type_class_unref (row_class);

  if (valid)
    return;

failure:
  g_warning ("Invalid DzlListBox instantiated, will not work as expected");
  priv->row_type = G_TYPE_INVALID;
  g_clear_pointer (&priv->property_name, g_free);
}

static void
dzl_list_box_destroy (GtkWidget *widget)
{
  DzlListBox *self = (DzlListBox *)widget;
  DzlListBoxPrivate *priv = dzl_list_box_get_instance_private (self);

  g_assert (DZL_IS_LIST_BOX (self));

  g_queue_foreach (&priv->trashed_rows, (GFunc)g_object_unref, NULL);
  g_queue_clear (&priv->trashed_rows);

  GTK_WIDGET_CLASS (dzl_list_box_parent_class)->destroy (widget);
}

static void
dzl_list_box_finalize (GObject *object)
{
  DzlListBox *self = (DzlListBox *)object;
  DzlListBoxPrivate *priv = dzl_list_box_get_instance_private (self);

  g_clear_pointer (&priv->property_name, g_free);
  priv->row_type = G_TYPE_INVALID;

  G_OBJECT_CLASS (dzl_list_box_parent_class)->finalize (object);
}

static void
dzl_list_box_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  DzlListBox *self = DZL_LIST_BOX (object);
  DzlListBoxPrivate *priv = dzl_list_box_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_ROW_TYPE:
      g_value_set_gtype (value, priv->row_type);
      break;

    case PROP_PROPERTY_NAME:
      g_value_set_string (value, priv->property_name);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_list_box_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  DzlListBox *self = DZL_LIST_BOX (object);
  DzlListBoxPrivate *priv = dzl_list_box_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_ROW_TYPE:
      {
        GType gtype = g_value_get_gtype (value);

        if (gtype != G_TYPE_INVALID)
          priv->row_type = gtype;
      }
      break;

    case PROP_ROW_TYPE_NAME:
      {
        const gchar *name = g_value_get_string (value);

        if (name != NULL)
          priv->row_type = g_type_from_name (name);
      }
      break;

    case PROP_PROPERTY_NAME:
      priv->property_name = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_list_box_class_init (DzlListBoxClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->constructed = dzl_list_box_constructed;
  object_class->finalize = dzl_list_box_finalize;
  object_class->get_property = dzl_list_box_get_property;
  object_class->set_property = dzl_list_box_set_property;

  widget_class->destroy = dzl_list_box_destroy;

  container_class->remove = dzl_list_box_remove;

  properties [PROP_ROW_TYPE] =
    g_param_spec_gtype ("row-type",
                        "Row Type",
                        "The GtkListBoxRow or subclass type to instantiate",
                        GTK_TYPE_LIST_BOX_ROW,
                        (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_ROW_TYPE_NAME] =
    g_param_spec_string ("row-type-name",
                         "Row Type Name",
                         "The name of the GType as a string",
                         NULL,
                         (G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_PROPERTY_NAME] =
    g_param_spec_string ("property-name",
                         "Property Name",
                         "The property in which to assign the model item",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_list_box_init (DzlListBox *self)
{
  DzlListBoxPrivate *priv = dzl_list_box_get_instance_private (self);

  priv->row_type = G_TYPE_INVALID;
  priv->recycle_max = 25;

  g_queue_init (&priv->trashed_rows);
}

DzlListBox *
dzl_list_box_new (GType        row_type,
                  const gchar *property_name)
{
  g_return_val_if_fail (g_type_is_a (row_type, GTK_TYPE_LIST_BOX_ROW), NULL);
  g_return_val_if_fail (property_name != NULL, NULL);

  return g_object_new (DZL_TYPE_LIST_BOX,
                       "property-name", property_name,
                       "row-type", row_type,
                       NULL);
}

/**
 * dzl_list_box_get_model:
 *
 * Returns: (nullable) (transfer none): A #GListModel or %NULL.
 */
GListModel *
dzl_list_box_get_model (DzlListBox *self)
{
  DzlListBoxPrivate *priv = dzl_list_box_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_LIST_BOX (self), NULL);

  return priv->model;
}

GType
dzl_list_box_get_row_type (DzlListBox *self)
{
  DzlListBoxPrivate *priv = dzl_list_box_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_LIST_BOX (self), G_TYPE_INVALID);

  return priv->row_type;
}

const gchar *
dzl_list_box_get_property_name (DzlListBox *self)
{
  DzlListBoxPrivate *priv = dzl_list_box_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_LIST_BOX (self), NULL);

  return priv->property_name;
}

void
dzl_list_box_set_model (DzlListBox *self,
                        GListModel *model)
{
  DzlListBoxPrivate *priv = dzl_list_box_get_instance_private (self);

  g_return_if_fail (DZL_IS_LIST_BOX (self));
  g_return_if_fail (priv->property_name != NULL);
  g_return_if_fail (priv->row_type != G_TYPE_INVALID);

  if (model == NULL)
    {
      gtk_list_box_bind_model (GTK_LIST_BOX (self), NULL, NULL, NULL, NULL);
      return;
    }

  gtk_list_box_bind_model (GTK_LIST_BOX (self),
                           model,
                           dzl_list_box_create_row,
                           self,
                           NULL);
}
