/* dzl-box.c
 *
 * Copyright (C) 2015 Christian Hergert <chergert@redhat.com>
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

#include "config.h"

#include <glib/gi18n.h>

#include "dzl-box.h"

typedef struct
{
  gint  max_width_request;
} DzlBoxPrivate;

enum {
  PROP_0,
  PROP_MAX_WIDTH_REQUEST,
  LAST_PROP
};

G_DEFINE_TYPE_WITH_PRIVATE (DzlBox, dzl_box, GTK_TYPE_BOX)

static GParamSpec *properties [LAST_PROP];

/**
 * dzl_box_get_nth_child:
 * @self: a #DzlBox
 * @nth: the index of the child starting from 0
 *
 * Gets the nth child of @self.
 *
 * Returns: (transfer none) (nullable): A #GtkWidget or %NULL
 */
GtkWidget *
dzl_box_get_nth_child (DzlBox *self,
                       guint   nth)
{
  GtkWidget *ret = NULL;
  GList *list;

  g_return_val_if_fail (GTK_IS_BOX (self), NULL);

  list = gtk_container_get_children (GTK_CONTAINER (self));
  ret = g_list_nth_data (list, nth);
  g_list_free (list);

  return ret;
}

static void
dzl_box_get_preferred_width (GtkWidget *widget,
                             gint      *min_width,
                             gint      *nat_width)
{
  DzlBox *self = (DzlBox *)widget;
  DzlBoxPrivate *priv = dzl_box_get_instance_private (self);

  g_assert (DZL_IS_BOX (self));

  GTK_WIDGET_CLASS (dzl_box_parent_class)->get_preferred_width (widget, min_width, nat_width);

  if (priv->max_width_request > 0)
    {
      if (*min_width > priv->max_width_request)
        *min_width = priv->max_width_request;

      if (*nat_width > priv->max_width_request)
        *nat_width = priv->max_width_request;
    }
}

static void
dzl_box_get_property (GObject    *object,
                      guint       prop_id,
                      GValue     *value,
                      GParamSpec *pspec)
{
  DzlBox *self = DZL_BOX (object);
  DzlBoxPrivate *priv = dzl_box_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_MAX_WIDTH_REQUEST:
      g_value_set_int (value, priv->max_width_request);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_box_set_property (GObject      *object,
                      guint         prop_id,
                      const GValue *value,
                      GParamSpec   *pspec)
{
  DzlBox *self = DZL_BOX (object);
  DzlBoxPrivate *priv = dzl_box_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_MAX_WIDTH_REQUEST:
      priv->max_width_request = g_value_get_int (value);
      gtk_widget_queue_resize (GTK_WIDGET (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_box_class_init (DzlBoxClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = dzl_box_get_property;
  object_class->set_property = dzl_box_set_property;

  widget_class->get_preferred_width = dzl_box_get_preferred_width;

  properties [PROP_MAX_WIDTH_REQUEST] =
    g_param_spec_int ("max-width-request",
                      "Max Width Request",
                      "Max Width Request",
                      -1,
                      G_MAXINT,
                      -1,
                      (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
dzl_box_init (DzlBox *self)
{
  DzlBoxPrivate *priv = dzl_box_get_instance_private (self);

  priv->max_width_request = -1;
}

GtkWidget *
dzl_box_new (void)
{
  return g_object_new (DZL_TYPE_BOX, NULL);
}
