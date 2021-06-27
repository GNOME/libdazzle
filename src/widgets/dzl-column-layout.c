/* dzl-column-layout.c
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

#include "config.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define G_LOG_DOMAIN "dzl-column-layout"

#include "util/dzl-macros.h"
#include "widgets/dzl-column-layout.h"

typedef struct
{
  GtkWidget      *widget;
  GtkAllocation   alloc;
  GtkRequisition  req;
  GtkRequisition  min_req;
  gint            priority;
} DzlColumnLayoutChild;

typedef struct
{
  GArray *children;
  gint    column_width;
  gint    column_spacing;
  gint    row_spacing;
  guint   max_columns;
} DzlColumnLayoutPrivate;

#define COLUMN_WIDTH_DEFAULT   500
#define COLUMN_SPACING_DEFAULT 24
#define ROW_SPACING_DEFAULT    24

G_DEFINE_TYPE_WITH_PRIVATE (DzlColumnLayout, dzl_column_layout, GTK_TYPE_CONTAINER)

enum {
  PROP_0,
  PROP_COLUMN_WIDTH,
  PROP_COLUMN_SPACING,
  PROP_MAX_COLUMNS,
  PROP_ROW_SPACING,
  LAST_PROP
};

enum {
  CHILD_PROP_0,
  CHILD_PROP_PRIORITY,
  LAST_CHILD_PROP
};

static GParamSpec *properties [LAST_PROP];
static GParamSpec *child_properties [LAST_CHILD_PROP];

static void
dzl_column_layout_layout (DzlColumnLayout *self,
                          gint             width,
                          gint             height,
                          gint            *tallest_column)
{
  DzlColumnLayoutPrivate *priv = dzl_column_layout_get_instance_private (self);
  gint real_tallest_column = 0;
  gint total_height = 0;
  gint n_columns = 0;
  gint border_width;
  gint column;
  guint i;

  g_assert (DZL_IS_COLUMN_LAYOUT (self));
  g_assert (width > 0);
  g_assert (tallest_column != NULL);

  /*
   * We want to layout the children in a series of columns, but try to
   * fill up each column before spilling into the next column.
   *
   * We can determine the number of columns we can support by the width
   * of our allocation, and determine the max-height of each column
   * by dividing the total height of all children by the number of
   * columns. There is the chance that non-uniform sizing will mess up
   * the height a bit here, but in practice it's mostly okay.
   *
   * The order of children is sorted by the priority, so that we know
   * we can allocate them serially as we walk the array.
   *
   * We keep allocating children until we will go over the height of
   * the column.
   */

  border_width = gtk_container_get_border_width (GTK_CONTAINER (self));
  total_height = border_width * 2;

  for (i = 0; i < priv->children->len; i++)
    {
      DzlColumnLayoutChild *child;

      child = &g_array_index (priv->children, DzlColumnLayoutChild, i);

      gtk_widget_get_preferred_height_for_width (child->widget,
                                                 priv->column_width,
                                                 &child->min_req.height,
                                                 &child->req.height);

      if (i != 0)
        total_height += priv->row_spacing;
      total_height += child->req.height;
    }

  if (total_height <= height)
    {
      n_columns = 1;
    }
  else
    {
      int estimated_cols;
      int leftover;

      estimated_cols = round(width / priv->column_width);
      leftover = (width % priv->column_width) - (border_width * 2) - (priv->column_spacing * (estimated_cols - 1));

      if (leftover >= 0)
        n_columns = estimated_cols;
      else
        n_columns = MAX(1, (estimated_cols - 1));
    }

  if (priv->max_columns > 0)
    n_columns = MIN (n_columns, (gint)priv->max_columns);

  for (column = 0, i = 0; column < n_columns; column++)
    {
      GtkAllocation alloc;
      gint j = 0;

      alloc.x = border_width + (priv->column_width * column) + (column * priv->column_spacing);
      alloc.y = border_width;
      alloc.width = priv->column_width;
      alloc.height = (height != 0) ? (height - (border_width * 2)) : total_height / n_columns;

      for (; i < priv->children->len; i++)
        {
          DzlColumnLayoutChild *child;
          gint child_height;

          child = &g_array_index (priv->children, DzlColumnLayoutChild, i);

          /*
           * Ignore this child if it is not visible.
           */
          if (!gtk_widget_get_visible (child->widget) ||
              !gtk_widget_get_child_visible (child->widget))
            continue;

          /*
           * If we are discovering height, and this is the last item in the
           * first column, and we only have one column, then we will just
           * make this "vexpand".
           */
          if (priv->max_columns == 1 && i == priv->children->len - 1)
            {
              if (height == 0)
                child_height = child->min_req.height;
              else
                child_height = alloc.height;
            }
          else
            child_height = child->req.height;

          /*
           * If the child requisition is taller than the space we have left in
           * this column, we need to spill over to the next column.
           */
          if ((j != 0) && (child_height > alloc.height) && (column < (n_columns - 1)))
            break;

          child->alloc.x = alloc.x;
          child->alloc.y = alloc.y;
          child->alloc.width = priv->column_width;
          child->alloc.height = child_height;

#if 0
          g_print ("Allocating child to: [%d] %d,%d %dx%d\n",
                   column,
                   child->alloc.x,
                   child->alloc.y,
                   child->alloc.width,
                   child->alloc.height);
#endif

          alloc.y += child_height + priv->row_spacing;
          alloc.height -= child_height + priv->row_spacing;

          if (alloc.y > real_tallest_column)
            real_tallest_column = alloc.y;

          j++;
        }
    }

  real_tallest_column += border_width;

  *tallest_column = real_tallest_column;
}

static GtkSizeRequestMode
dzl_column_layout_get_request_mode (GtkWidget *widget)
{
  return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

static void
dzl_column_layout_get_preferred_width (GtkWidget *widget,
                                       gint      *min_width,
                                       gint      *nat_width)
{
  DzlColumnLayout *self = (DzlColumnLayout *)widget;
  DzlColumnLayoutPrivate *priv = dzl_column_layout_get_instance_private (self);
  gint border_width;
  gint n_columns = 3;

  g_assert (DZL_IS_COLUMN_LAYOUT (self));
  g_assert (min_width != NULL);
  g_assert (nat_width != NULL);

  border_width = gtk_container_get_border_width (GTK_CONTAINER (self));

  /*
   * By default we try to natural size up to 3 columns. Otherwise, we
   * use the max_columns. It would be nice if we could deal with this
   * in a better way, but that is going to take a bunch more solving.
   */

  if (priv->max_columns > 0)
    n_columns = priv->max_columns;

  *nat_width = (priv->column_width * n_columns) + (priv->column_spacing * (n_columns - 1)) + (border_width * 2);
  *min_width = priv->column_width + (border_width * 2);
}

static void
dzl_column_layout_get_preferred_height_for_width (GtkWidget *widget,
                                                  gint       width,
                                                  gint      *min_height,
                                                  gint      *nat_height)
{
  DzlColumnLayout *self = (DzlColumnLayout *)widget;
  gint tallest_column = 0;

  g_assert (DZL_IS_COLUMN_LAYOUT (self));
  g_assert (min_height != NULL);
  g_assert (nat_height != NULL);

  dzl_column_layout_layout (self, width, 0, &tallest_column);

  *min_height = *nat_height = tallest_column;
}

static void
dzl_column_layout_size_allocate (GtkWidget     *widget,
                                 GtkAllocation *allocation)
{
  DzlColumnLayout *self = (DzlColumnLayout *)widget;
  DzlColumnLayoutPrivate *priv = dzl_column_layout_get_instance_private (self);
  gint tallest_column = 0;
  guint i;

  g_assert (DZL_IS_COLUMN_LAYOUT (self));
  g_assert (allocation != NULL);

  gtk_widget_set_allocation (widget, allocation);

  dzl_column_layout_layout (self, allocation->width, allocation->height, &tallest_column);

  /*
   * If we are on a RTL language, flip all our allocations around so
   * we move from right to left. This is easier than adding all the
   * complexity to directions during layout time.
   */
  if (GTK_TEXT_DIR_RTL == gtk_widget_get_direction (widget))
    {
      for (i = 0; i < priv->children->len; i++)
        {
          DzlColumnLayoutChild *child;

          child = &g_array_index (priv->children, DzlColumnLayoutChild, i);
          child->alloc.x = allocation->x + allocation->width - child->alloc.x - child->alloc.width;
        }
    }

  for (i = 0; i < priv->children->len; i++)
    {
      DzlColumnLayoutChild *child;

      child = &g_array_index (priv->children, DzlColumnLayoutChild, i);
      gtk_widget_size_allocate (child->widget, &child->alloc);
    }
}

static gint
dzl_column_layout_child_compare (gconstpointer a,
                                 gconstpointer b)
{
  const DzlColumnLayoutChild *child_a = a;
  const DzlColumnLayoutChild *child_b = b;

  return child_a->priority - child_b->priority;
}

static void
dzl_column_layout_add (GtkContainer *container,
                       GtkWidget    *widget)
{
  DzlColumnLayout *self = (DzlColumnLayout *)container;
  DzlColumnLayoutPrivate *priv = dzl_column_layout_get_instance_private (self);
  DzlColumnLayoutChild child = { 0 };

  g_assert (DZL_IS_COLUMN_LAYOUT (self));
  g_assert (GTK_IS_WIDGET (widget));

  child.widget = g_object_ref_sink (widget);
  child.priority = 0;

  g_array_append_val (priv->children, child);
  g_array_sort (priv->children, dzl_column_layout_child_compare);

  gtk_widget_set_parent (widget, GTK_WIDGET (self));
  gtk_widget_queue_resize (GTK_WIDGET (self));
}

static void
dzl_column_layout_remove (GtkContainer *container,
                          GtkWidget    *widget)
{
  DzlColumnLayout *self = (DzlColumnLayout *)container;
  DzlColumnLayoutPrivate *priv = dzl_column_layout_get_instance_private (self);
  guint i;

  g_assert (GTK_IS_CONTAINER (container));
  g_assert (GTK_IS_WIDGET (widget));

  for (i = 0; i < priv->children->len; i++)
    {
      DzlColumnLayoutChild *child;

      child = &g_array_index (priv->children, DzlColumnLayoutChild, i);

      if (child->widget == widget)
        {
          gtk_widget_unparent (child->widget);
          g_array_remove_index (priv->children, i);
          gtk_widget_queue_resize (GTK_WIDGET (self));
          return;
        }
    }
}

static void
dzl_column_layout_forall (GtkContainer *container,
                          gboolean      include_internals,
                          GtkCallback   callback,
                          gpointer      user_data)
{
  DzlColumnLayout *self = (DzlColumnLayout *)container;
  DzlColumnLayoutPrivate *priv = dzl_column_layout_get_instance_private (self);
  gint i;

  g_assert (GTK_IS_CONTAINER (container));
  g_assert (callback != NULL);

  /*
   * We walk backwards in the array to be safe against callback destorying
   * the widget (and causing it to be removed).
   */

  for (i = priv->children->len; i > 0; i--)
    {
      DzlColumnLayoutChild *child;

      child = &g_array_index (priv->children, DzlColumnLayoutChild, i - 1);
      callback (child->widget, user_data);
    }
}

static DzlColumnLayoutChild *
dzl_column_layout_find_child (DzlColumnLayout *self,
                              GtkWidget       *widget)
{
  DzlColumnLayoutPrivate *priv = dzl_column_layout_get_instance_private (self);

  g_assert (DZL_IS_COLUMN_LAYOUT (self));
  g_assert (GTK_IS_WIDGET (widget));

  for (guint i = 0; i < priv->children->len; i++)
    {
      DzlColumnLayoutChild *child;

      child = &g_array_index (priv->children, DzlColumnLayoutChild, i);

      if (child->widget == widget)
        return child;
    }

  g_assert_not_reached ();

  return NULL;
}

gint
dzl_column_layout_get_column_width (DzlColumnLayout *self)
{
  DzlColumnLayoutPrivate *priv = dzl_column_layout_get_instance_private (self);
  g_return_val_if_fail (DZL_IS_COLUMN_LAYOUT (self), 0);
  return priv->column_width;
}

void
dzl_column_layout_set_column_width (DzlColumnLayout *self,
                                    gint             column_width)
{
  DzlColumnLayoutPrivate *priv = dzl_column_layout_get_instance_private (self);

  g_return_if_fail (DZL_IS_COLUMN_LAYOUT (self));
  g_return_if_fail (column_width >= 0);

  if (priv->column_width != column_width)
    {
      priv->column_width = column_width;
      gtk_widget_queue_resize (GTK_WIDGET (self));
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_COLUMN_WIDTH]);
    }
}

gint
dzl_column_layout_get_column_spacing (DzlColumnLayout *self)
{
  DzlColumnLayoutPrivate *priv = dzl_column_layout_get_instance_private (self);
  g_return_val_if_fail (DZL_IS_COLUMN_LAYOUT (self), 0);
  return priv->column_spacing;
}

void
dzl_column_layout_set_column_spacing (DzlColumnLayout *self,
                                      gint             column_spacing)
{
  DzlColumnLayoutPrivate *priv = dzl_column_layout_get_instance_private (self);

  g_return_if_fail (DZL_IS_COLUMN_LAYOUT (self));
  g_return_if_fail (column_spacing >= 0);

  if (priv->column_spacing != column_spacing)
    {
      priv->column_spacing = column_spacing;
      gtk_widget_queue_resize (GTK_WIDGET (self));
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_COLUMN_SPACING]);
    }
}

gint
dzl_column_layout_get_row_spacing (DzlColumnLayout *self)
{
  DzlColumnLayoutPrivate *priv = dzl_column_layout_get_instance_private (self);
  g_return_val_if_fail (DZL_IS_COLUMN_LAYOUT (self), 0);
  return priv->row_spacing;
}

void
dzl_column_layout_set_row_spacing (DzlColumnLayout *self,
                                   gint             row_spacing)
{
  DzlColumnLayoutPrivate *priv = dzl_column_layout_get_instance_private (self);

  g_return_if_fail (DZL_IS_COLUMN_LAYOUT (self));
  g_return_if_fail (row_spacing >= 0);

  if (priv->row_spacing != row_spacing)
    {
      priv->row_spacing = row_spacing;
      gtk_widget_queue_resize (GTK_WIDGET (self));
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_ROW_SPACING]);
    }
}

static void
dzl_column_layout_get_child_property (GtkContainer *container,
                                      GtkWidget    *widget,
                                      guint         prop_id,
                                      GValue       *value,
                                      GParamSpec   *pspec)
{
  DzlColumnLayout *self = (DzlColumnLayout *)container;
  DzlColumnLayoutChild *child = dzl_column_layout_find_child (self, widget);

  switch (prop_id)
    {
    case CHILD_PROP_PRIORITY:
      g_value_set_int (value, child->priority);
      break;

    default:
      GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, prop_id, pspec);
    }
}

static void
dzl_column_layout_set_child_property (GtkContainer *container,
                                      GtkWidget    *widget,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  DzlColumnLayout *self = (DzlColumnLayout *)container;
  DzlColumnLayoutChild *child = dzl_column_layout_find_child (self, widget);

  switch (prop_id)
    {
    case CHILD_PROP_PRIORITY:
      child->priority = g_value_get_int (value);
      gtk_widget_queue_allocate (GTK_WIDGET (self));
      break;

    default:
      GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, prop_id, pspec);
    }
}

static void
dzl_column_layout_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  DzlColumnLayout *self = DZL_COLUMN_LAYOUT(object);

  switch (prop_id)
    {
    case PROP_COLUMN_SPACING:
      g_value_set_int (value, dzl_column_layout_get_column_spacing (self));
      break;

    case PROP_COLUMN_WIDTH:
      g_value_set_int (value, dzl_column_layout_get_column_width (self));
      break;

    case PROP_MAX_COLUMNS:
      g_value_set_uint (value, dzl_column_layout_get_max_columns (self));
      break;

    case PROP_ROW_SPACING:
      g_value_set_int (value, dzl_column_layout_get_row_spacing (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
dzl_column_layout_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  DzlColumnLayout *self = DZL_COLUMN_LAYOUT(object);

  switch (prop_id)
    {
    case PROP_COLUMN_SPACING:
      dzl_column_layout_set_column_spacing (self, g_value_get_int (value));
      break;

    case PROP_COLUMN_WIDTH:
      dzl_column_layout_set_column_width (self, g_value_get_int (value));
      break;

    case PROP_MAX_COLUMNS:
      dzl_column_layout_set_max_columns (self, g_value_get_uint (value));
      break;

    case PROP_ROW_SPACING:
      dzl_column_layout_set_row_spacing (self, g_value_get_int (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
dzl_column_layout_finalize (GObject *object)
{
  DzlColumnLayout *self = (DzlColumnLayout *)object;
  DzlColumnLayoutPrivate *priv = dzl_column_layout_get_instance_private (self);

  g_clear_pointer (&priv->children, g_array_unref);

  G_OBJECT_CLASS (dzl_column_layout_parent_class)->finalize (object);
}

static void
dzl_column_layout_class_init (DzlColumnLayoutClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->finalize = dzl_column_layout_finalize;
  object_class->get_property = dzl_column_layout_get_property;
  object_class->set_property = dzl_column_layout_set_property;

  properties [PROP_COLUMN_SPACING] =
    g_param_spec_int ("column-spacing",
                      "Column Spacing",
                      "The spacing between columns",
                      0,
                      G_MAXINT,
                      COLUMN_SPACING_DEFAULT,
                      (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_COLUMN_WIDTH] =
    g_param_spec_int ("column-width",
                      "Column Width",
                      "The width of the columns",
                      0,
                      G_MAXINT,
                      COLUMN_WIDTH_DEFAULT,
                      (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_MAX_COLUMNS] =
    g_param_spec_uint ("max-columns",
                       "Max Columns",
                       "Max Columns",
                       0,
                       G_MAXINT,
                       0,
                       (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_ROW_SPACING] =
    g_param_spec_int ("row-spacing",
                      "Row Spacing",
                      "The spacing between rows",
                      0,
                      G_MAXINT,
                      ROW_SPACING_DEFAULT,
                      (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);

  widget_class->get_preferred_height_for_width = dzl_column_layout_get_preferred_height_for_width;
  widget_class->get_preferred_width = dzl_column_layout_get_preferred_width;
  widget_class->get_request_mode = dzl_column_layout_get_request_mode;
  widget_class->size_allocate = dzl_column_layout_size_allocate;

  container_class->add = dzl_column_layout_add;
  container_class->forall = dzl_column_layout_forall;
  container_class->remove = dzl_column_layout_remove;
  container_class->get_child_property = dzl_column_layout_get_child_property;
  container_class->set_child_property = dzl_column_layout_set_child_property;

  child_properties [CHILD_PROP_PRIORITY] =
    g_param_spec_int ("priority",
                      "Priority",
                      "The sort priority of the child",
                      G_MININT,
                      G_MAXINT,
                      0,
                      (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gtk_container_class_install_child_properties (container_class, LAST_CHILD_PROP, child_properties);
}

static void
dzl_column_layout_init (DzlColumnLayout *self)
{
  DzlColumnLayoutPrivate *priv = dzl_column_layout_get_instance_private (self);

  gtk_widget_set_has_window (GTK_WIDGET (self), FALSE);

  priv->children = g_array_new (FALSE, TRUE, sizeof (DzlColumnLayoutChild));

  priv->column_width = COLUMN_WIDTH_DEFAULT;
  priv->column_spacing = COLUMN_SPACING_DEFAULT;
  priv->row_spacing = ROW_SPACING_DEFAULT;
}

GtkWidget *
dzl_column_layout_new (void)
{
  return g_object_new (DZL_TYPE_COLUMN_LAYOUT, NULL);
}

guint
dzl_column_layout_get_max_columns (DzlColumnLayout *self)
{
  DzlColumnLayoutPrivate *priv = dzl_column_layout_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_COLUMN_LAYOUT (self), 0);

  return priv->max_columns;
}

void
dzl_column_layout_set_max_columns (DzlColumnLayout *self,
                                   guint            max_columns)
{
  DzlColumnLayoutPrivate *priv = dzl_column_layout_get_instance_private (self);

  g_return_if_fail (DZL_IS_COLUMN_LAYOUT (self));

  if (priv->max_columns != max_columns)
    {
      priv->max_columns = max_columns;
      gtk_widget_queue_resize (GTK_WIDGET (self));
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_MAX_COLUMNS]);
    }
}
