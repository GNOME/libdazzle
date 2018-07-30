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

#include "config.h"

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
 *
 * You must subclass DzlListBoxRow for your rows.
 */

#include "util/dzl-macros.h"
#include "widgets/dzl-list-box.h"
#include "widgets/dzl-list-box-private.h"
#include "widgets/dzl-list-box-row.h"

#define RECYCLE_MAX_DEFAULT 25

typedef struct
{
  DzlListBoxAttachFunc  attach_func;
  gpointer              attach_data;
  GListModel           *model;
  gchar                *property_name;
  GType                 row_type;
  guint                 recycle_max;
  GQueue                trashed_rows;
  guint                 destroying : 1;
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

gboolean
_dzl_list_box_cache (DzlListBox    *self,
                     DzlListBoxRow *row)
{
  DzlListBoxPrivate *priv = dzl_list_box_get_instance_private (self);

  g_assert (DZL_IS_LIST_BOX (self));
  g_assert (DZL_IS_LIST_BOX_ROW (row));

  if (gtk_widget_get_parent (GTK_WIDGET (row)) != GTK_WIDGET (self))
    {
      g_warning ("Attempt to cache row not belonging to list box");
      return FALSE;
    }

  if (gtk_widget_in_destruction (GTK_WIDGET (self)))
    return FALSE;

  if (priv->trashed_rows.length < priv->recycle_max)
    {
      g_autoptr(GtkWidget) held = g_object_ref (GTK_WIDGET (row));

      gtk_list_box_unselect_row (GTK_LIST_BOX (self), GTK_LIST_BOX_ROW (row));
      gtk_container_remove (GTK_CONTAINER (self), GTK_WIDGET (row));
      g_object_set (held, priv->property_name, NULL, NULL);
      g_object_force_floating (G_OBJECT (held));
      g_queue_push_head (&priv->trashed_rows, g_steal_pointer (&held));

      return TRUE;
    }

  return FALSE;
}

static GtkWidget *
dzl_list_box_create_row (gpointer item,
                         gpointer user_data)
{
  DzlListBox *self = user_data;
  DzlListBoxPrivate *priv = dzl_list_box_get_instance_private (self);
  GtkListBoxRow *row;

  g_assert (G_IS_OBJECT (item));
  g_assert (DZL_IS_LIST_BOX (self));

  if (priv->trashed_rows.length > 0)
    {
      row = g_queue_pop_tail (&priv->trashed_rows);

      g_assert (DZL_IS_LIST_BOX_ROW (row));
      g_assert (priv->property_name != NULL);
      g_assert (item != NULL);

      g_object_set (row, priv->property_name, item, NULL);
    }
  else
    {
      row = g_object_new (priv->row_type,
                          "visible", TRUE,
                          priv->property_name, item,
                          NULL);
    }

  g_return_val_if_fail (DZL_IS_LIST_BOX_ROW (row), NULL);

  if (priv->attach_func)
    priv->attach_func (self, DZL_LIST_BOX_ROW (row), priv->attach_data);

  return GTK_WIDGET (row);
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
  GList *rows;

  g_assert (DZL_IS_LIST_BOX (self));

  priv->destroying = TRUE;
  priv->recycle_max = 0;

  rows = priv->trashed_rows.head;

  priv->trashed_rows.head = NULL;
  priv->trashed_rows.tail = NULL;
  priv->trashed_rows.length = 0;

  g_list_foreach (rows, (GFunc)gtk_widget_destroy, NULL);
  g_list_free (rows);

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

  object_class->constructed = dzl_list_box_constructed;
  object_class->finalize = dzl_list_box_finalize;
  object_class->get_property = dzl_list_box_get_property;
  object_class->set_property = dzl_list_box_set_property;

  widget_class->destroy = dzl_list_box_destroy;

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
  priv->recycle_max = RECYCLE_MAX_DEFAULT;

  g_queue_init (&priv->trashed_rows);
}

GtkWidget *
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

/**
 * dzl_list_box_set_recycle_max:
 * @self: a #DzlListBox
 * @recycle_max: max number of rows to cache
 *
 * Sets the max number of rows to cache for reuse.  Set to 0 to return
 * to the default.
 *
 * Since: 3.28
 */
void
dzl_list_box_set_recycle_max (DzlListBox *self,
                              guint       recycle_max)
{
  DzlListBoxPrivate *priv = dzl_list_box_get_instance_private (self);

  g_return_if_fail (DZL_IS_LIST_BOX (self));

  if (recycle_max == 0)
    priv->recycle_max = RECYCLE_MAX_DEFAULT;
  else
    priv->recycle_max = recycle_max;
}

/* Like gtk_container_forall() but also calls for all cached rows */
void
_dzl_list_box_forall (DzlListBox  *self,
                      GtkCallback  callback,
                      gpointer     user_data)
{
  DzlListBoxPrivate *priv = dzl_list_box_get_instance_private (self);
  const GList *iter;

  g_assert (DZL_IS_LIST_BOX (self));
  g_assert (callback != NULL);

  gtk_container_foreach (GTK_CONTAINER (self), callback, user_data);

  iter = priv->trashed_rows.head;

  while (iter != NULL)
    {
      GtkWidget *widget = iter->data;
      iter = iter->next;
      callback (widget, user_data);
    }
}

void
_dzl_list_box_set_attach_func (DzlListBox           *self,
                               DzlListBoxAttachFunc  func,
                               gpointer              user_data)
{
  DzlListBoxPrivate *priv = dzl_list_box_get_instance_private (self);

  g_return_if_fail (DZL_IS_LIST_BOX (self));

  priv->attach_func = func;
  priv->attach_data = user_data;
}
