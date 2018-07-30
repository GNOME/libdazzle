/* dzl-priority-box.c
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

/**
 * SECTION:dzlprioritybox:
 * @title: DzlPriorityBox
 *
 * This is like a #GtkBox but uses stable priorities to sort.
 */

#define G_LOG_DOMAIN "dzl-priority-box"

#include "config.h"

#include "widgets/dzl-priority-box.h"
#include "util/dzl-macros.h"

typedef struct
{
  GtkWidget *widget;
  gint       priority;
} DzlPriorityBoxChild;

typedef struct
{
  GArray *children;
} DzlPriorityBoxPrivate;

enum {
  CHILD_PROP_0,
  CHILD_PROP_PRIORITY,
  N_CHILD_PROPS
};

G_DEFINE_TYPE_WITH_PRIVATE (DzlPriorityBox, dzl_priority_box, GTK_TYPE_BOX)

static GParamSpec *child_properties [N_CHILD_PROPS];

static gint
sort_by_priority (gconstpointer a,
                  gconstpointer b)
{
  const DzlPriorityBoxChild *child_a = a;
  const DzlPriorityBoxChild *child_b = b;

  return child_a->priority - child_b->priority;
}

static void
dzl_priority_box_resort (DzlPriorityBox *self)
{
  DzlPriorityBoxPrivate *priv = dzl_priority_box_get_instance_private (self);
  guint i;

  g_assert (DZL_IS_PRIORITY_BOX (self));

  g_array_sort (priv->children, sort_by_priority);

  for (i = 0; i < priv->children->len; i++)
    {
      DzlPriorityBoxChild *child = &g_array_index (priv->children, DzlPriorityBoxChild, i);

      gtk_container_child_set (GTK_CONTAINER (self), child->widget,
                               "position", i,
                               NULL);
    }
}

static gint
dzl_priority_box_get_child_priority (DzlPriorityBox *self,
                                     GtkWidget      *widget)
{
  DzlPriorityBoxPrivate *priv = dzl_priority_box_get_instance_private (self);
  guint i;

  g_assert (DZL_IS_PRIORITY_BOX (self));
  g_assert (GTK_IS_WIDGET (widget));

  for (i = 0; i < priv->children->len; i++)
    {
      DzlPriorityBoxChild *child = &g_array_index (priv->children, DzlPriorityBoxChild, i);

      if (child->widget == widget)
        return child->priority;
    }

  g_warning ("No such child \"%s\" of \"%s\"",
             G_OBJECT_TYPE_NAME (widget),
             G_OBJECT_TYPE_NAME (self));

  return 0;
}

static void
dzl_priority_box_set_child_priority (DzlPriorityBox *self,
                                     GtkWidget      *widget,
                                     gint            priority)
{
  DzlPriorityBoxPrivate *priv = dzl_priority_box_get_instance_private (self);
  guint i;

  g_assert (DZL_IS_PRIORITY_BOX (self));
  g_assert (GTK_IS_WIDGET (widget));

  for (i = 0; i < priv->children->len; i++)
    {
      DzlPriorityBoxChild *child = &g_array_index (priv->children, DzlPriorityBoxChild, i);

      if (child->widget == widget)
        {
          child->priority = priority;
          dzl_priority_box_resort (self);
          return;
        }
    }

  g_warning ("No such child \"%s\" of \"%s\"",
             G_OBJECT_TYPE_NAME (widget),
             G_OBJECT_TYPE_NAME (self));
}

static void
dzl_priority_box_add (GtkContainer *container,
                      GtkWidget    *widget)
{
  DzlPriorityBox *self = (DzlPriorityBox *)container;
  DzlPriorityBoxPrivate *priv = dzl_priority_box_get_instance_private (self);
  DzlPriorityBoxChild child;

  g_assert (DZL_IS_PRIORITY_BOX (self));
  g_assert (GTK_IS_WIDGET (widget));

  child.widget = widget;
  child.priority = 0;

  g_array_append_val (priv->children, child);

  GTK_CONTAINER_CLASS (dzl_priority_box_parent_class)->add (container, widget);

  dzl_priority_box_resort (self);
}

static void
dzl_priority_box_remove (GtkContainer *container,
                         GtkWidget    *widget)
{
  DzlPriorityBox *self = (DzlPriorityBox *)container;
  DzlPriorityBoxPrivate *priv = dzl_priority_box_get_instance_private (self);
  guint i;

  g_assert (DZL_IS_PRIORITY_BOX (self));
  g_assert (GTK_IS_WIDGET (widget));

  for (i = 0; i < priv->children->len; i++)
    {
      DzlPriorityBoxChild *child;

      child = &g_array_index (priv->children, DzlPriorityBoxChild, i);

      if (child->widget == widget)
        {
          g_array_remove_index_fast (priv->children, i);
          break;
        }
    }

  GTK_CONTAINER_CLASS (dzl_priority_box_parent_class)->remove (container, widget);

  dzl_priority_box_resort (self);
}

static void
dzl_priority_box_finalize (GObject *object)
{
  DzlPriorityBox *self = (DzlPriorityBox *)object;
  DzlPriorityBoxPrivate *priv = dzl_priority_box_get_instance_private (self);

  g_clear_pointer (&priv->children, g_array_unref);

  G_OBJECT_CLASS (dzl_priority_box_parent_class)->finalize (object);
}

static void
dzl_priority_box_get_child_property (GtkContainer *container,
                                     GtkWidget    *child,
                                     guint         prop_id,
                                     GValue       *value,
                                     GParamSpec   *pspec)
{
  DzlPriorityBox *self = DZL_PRIORITY_BOX (container);

  switch (prop_id)
    {
    case CHILD_PROP_PRIORITY:
      g_value_set_int (value, dzl_priority_box_get_child_priority (self, child));
      break;

    default:
      GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, prop_id, pspec);
    }
}

static void
dzl_priority_box_set_child_property (GtkContainer *container,
                                     GtkWidget    *child,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  DzlPriorityBox *self = DZL_PRIORITY_BOX (container);

  switch (prop_id)
    {
    case CHILD_PROP_PRIORITY:
      dzl_priority_box_set_child_priority (self, child, g_value_get_int (value));
      break;

    default:
      GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, prop_id, pspec);
    }
}

static void
dzl_priority_box_class_init (DzlPriorityBoxClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->finalize = dzl_priority_box_finalize;

  container_class->add = dzl_priority_box_add;
  container_class->remove = dzl_priority_box_remove;
  container_class->get_child_property = dzl_priority_box_get_child_property;
  container_class->set_child_property = dzl_priority_box_set_child_property;

  child_properties [CHILD_PROP_PRIORITY] =
    g_param_spec_int ("priority",
                      "Priority",
                      "Priority",
                      G_MININT,
                      G_MAXINT,
                      0,
                      (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gtk_container_class_install_child_properties (container_class, N_CHILD_PROPS, child_properties);
}

static void
dzl_priority_box_init (DzlPriorityBox *self)
{
  DzlPriorityBoxPrivate *priv = dzl_priority_box_get_instance_private (self);

  priv->children = g_array_new (FALSE, FALSE, sizeof (DzlPriorityBoxChild));
}

GtkWidget *
dzl_priority_box_new (void)
{
  return g_object_new (DZL_TYPE_PRIORITY_BOX, NULL);
}
