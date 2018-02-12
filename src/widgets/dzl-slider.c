/* dzl-slider.c
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

#define G_LOG_DOMAIN "dzl-slider"

#include "config.h"

#include <glib/gi18n.h>

#include "animation/dzl-animation.h"
#include "widgets/dzl-slider.h"
#include "util/dzl-util-private.h"

typedef struct
{
  GtkWidget         *widget;
  GdkWindow         *window;
  GtkAllocation      allocation;
  DzlSliderPosition  position : 3;
} DzlSliderChild;

typedef struct
{
  GtkAdjustment     *h_adj;
  GtkAdjustment     *v_adj;

  DzlAnimation      *h_anim;
  DzlAnimation      *v_anim;

  GPtrArray         *children;

  DzlSliderPosition  position : 3;
} DzlSliderPrivate;

static void buildable_iface_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (DzlSlider, dzl_slider, GTK_TYPE_CONTAINER,
                         G_ADD_PRIVATE (DzlSlider)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, buildable_iface_init))

enum {
  PROP_0,
  PROP_POSITION,
  LAST_PROP
};

enum {
  CHILD_PROP_0,
  CHILD_PROP_POSITION,
};

#define ANIMATION_MODE     DZL_ANIMATION_EASE_IN_QUAD
#define ANIMATION_DURATION 150

static GParamSpec *properties [LAST_PROP];

static void
dzl_slider_child_free (DzlSliderChild *child)
{
  g_slice_free (DzlSliderChild, child);
}

static void
dzl_slider_compute_margin (DzlSlider *self,
                           gint      *x_margin,
                           gint      *y_margin)
{
  DzlSliderPrivate *priv = dzl_slider_get_instance_private (self);
  gdouble x_ratio;
  gdouble y_ratio;
  gsize i;
  gint real_top_margin = 0;
  gint real_bottom_margin = 0;
  gint real_left_margin = 0;
  gint real_right_margin = 0;

  g_assert (DZL_IS_SLIDER (self));

  for (i = 0; i < priv->children->len; i++)
    {
      DzlSliderChild *child;
      gint margin;

      child = g_ptr_array_index (priv->children, i);

      switch (child->position)
        {
        case DZL_SLIDER_NONE:
          break;

        case DZL_SLIDER_BOTTOM:
          gtk_widget_get_preferred_height (child->widget, NULL, &margin);
          real_bottom_margin = MAX (real_bottom_margin, margin);
          break;

        case DZL_SLIDER_TOP:
          gtk_widget_get_preferred_height (child->widget, NULL, &margin);
          real_top_margin = MAX (real_top_margin, margin);
          break;

        case DZL_SLIDER_LEFT:
          gtk_widget_get_preferred_width (child->widget, NULL, &margin);
          real_left_margin = MAX (real_left_margin, margin);
          break;

        case DZL_SLIDER_RIGHT:
          gtk_widget_get_preferred_width (child->widget, NULL, &margin);
          real_right_margin = MAX (real_right_margin, margin);
          break;

        default:
          g_assert_not_reached ();
          break;
        }
    }

  x_ratio = gtk_adjustment_get_value (priv->h_adj);
  y_ratio = gtk_adjustment_get_value (priv->v_adj);

  if (x_ratio < 0.0)
    *x_margin = x_ratio * real_left_margin;
  else if (x_ratio > 0.0)
    *x_margin = x_ratio * real_right_margin;
  else
    *x_margin = 0;

  if (y_ratio < 0.0)
    *y_margin = y_ratio * real_bottom_margin;
  else if (y_ratio > 0.0)
    *y_margin = y_ratio * real_top_margin;
  else
    *y_margin = 0;
}

static void
dzl_slider_compute_child_allocation (DzlSlider      *self,
                                     DzlSliderChild *child,
                                     GtkAllocation  *window_allocation,
                                     GtkAllocation  *child_allocation)
{
  GtkAllocation real_window_allocation;
  GtkAllocation real_child_allocation;
  gint nat_height;
  gint nat_width;
  gint x_margin;
  gint y_margin;

  g_assert (DZL_IS_SLIDER (self));
  g_assert (child != NULL);
  g_assert (GTK_IS_WIDGET (child->widget));

  gtk_widget_get_allocation (GTK_WIDGET (self), &real_window_allocation);

  dzl_slider_compute_margin (self, &x_margin, &y_margin);

  if (child->position == DZL_SLIDER_NONE)
    {
      real_child_allocation.y = y_margin;
      real_child_allocation.x = x_margin;
      real_child_allocation.width = real_window_allocation.width;
      real_child_allocation.height = real_window_allocation.height;
    }
  else if (child->position == DZL_SLIDER_TOP)
    {
      gtk_widget_get_preferred_height (child->widget, NULL, &nat_height);

      real_window_allocation.y = real_window_allocation.y - nat_height + y_margin;
      real_window_allocation.height = nat_height;

      real_child_allocation.y = 0;
      real_child_allocation.x = 0;
      real_child_allocation.height = nat_height;
      real_child_allocation.width = real_window_allocation.width;
    }
  else if (child->position == DZL_SLIDER_BOTTOM)
    {
      gtk_widget_get_preferred_height (child->widget, NULL, &nat_height);

      real_window_allocation.y = real_window_allocation.y + real_window_allocation.height + y_margin;
      real_window_allocation.height = nat_height;

      real_child_allocation.y = 0;
      real_child_allocation.x = 0;
      real_child_allocation.height = nat_height;
      real_child_allocation.width = real_window_allocation.width;
    }
  else if (child->position == DZL_SLIDER_RIGHT)
    {
      gtk_widget_get_preferred_width (child->widget, NULL, &nat_width);

      real_window_allocation.x = real_window_allocation.x + real_window_allocation.width + x_margin;
      real_window_allocation.width = nat_width;

      real_child_allocation.y = 0;
      real_child_allocation.x = 0;
      real_child_allocation.height = real_window_allocation.height;
      real_child_allocation.width = nat_width;
    }
  else if (child->position == DZL_SLIDER_LEFT)
    {
      gtk_widget_get_preferred_width (child->widget, NULL, &nat_width);

      real_window_allocation.x = real_window_allocation.x - nat_width + x_margin;
      real_window_allocation.width = nat_width;

      real_child_allocation.y = 0;
      real_child_allocation.x = 0;
      real_child_allocation.height = real_window_allocation.height;
      real_child_allocation.width = nat_width;
    }
  else
    {
      g_assert_not_reached ();
    }

  if (window_allocation)
    *window_allocation = real_window_allocation;

  if (child_allocation)
    *child_allocation = real_child_allocation;
}

static GdkWindow *
dzl_slider_create_child_window (DzlSlider      *self,
                                DzlSliderChild *child)
{
  GtkWidget *widget = (GtkWidget *)self;
  GdkWindow *window;
  GtkAllocation allocation;
  GdkWindowAttr attributes;
  gint attributes_mask;

  g_assert (DZL_IS_SLIDER (self));
  g_assert (child != NULL);

  dzl_slider_compute_child_allocation (self, child, &allocation, NULL);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.width = allocation.width;
  attributes.height = allocation.height;
  attributes.x = allocation.x;
  attributes.y = allocation.y;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;
  attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;

  window = gdk_window_new (gtk_widget_get_window (widget), &attributes, attributes_mask);
  gtk_widget_register_window (widget, window);

  gtk_widget_set_parent_window (child->widget, window);

  return window;
}

static void
dzl_slider_add (GtkContainer *container,
                GtkWidget    *widget)
{
  DzlSlider *self = (DzlSlider *)container;
  DzlSliderPrivate *priv = dzl_slider_get_instance_private (self);
  DzlSliderChild *child;

  g_assert (DZL_IS_SLIDER (self));
  g_assert (GTK_IS_WIDGET (widget));

  child = g_slice_new0 (DzlSliderChild);
  child->position = DZL_SLIDER_NONE;
  child->widget = g_object_ref (widget);

  g_ptr_array_add (priv->children, child);

  gtk_widget_set_parent (widget, GTK_WIDGET (self));

  if (gtk_widget_get_realized (GTK_WIDGET (self)))
    child->window = dzl_slider_create_child_window (self, child);
}

static void
dzl_slider_remove (GtkContainer *container,
                   GtkWidget    *widget)
{
  DzlSlider *self = (DzlSlider *)container;
  DzlSliderPrivate *priv = dzl_slider_get_instance_private (self);
  DzlSliderChild *child;
  gsize i;

  g_assert (DZL_IS_SLIDER (self));
  g_assert (GTK_IS_WIDGET (widget));

  for (i = 0; i < priv->children->len; i++)
    {
      child = g_ptr_array_index (priv->children, i);

      if (child->widget == widget)
        {
          gtk_widget_unparent (widget);
          g_ptr_array_remove_index (priv->children, i);
          gtk_widget_queue_allocate (GTK_WIDGET (self));
          break;
        }
    }
}

static void
dzl_slider_size_allocate (GtkWidget     *widget,
                          GtkAllocation *allocation)
{
  DzlSlider *self = (DzlSlider *)widget;
  DzlSliderPrivate *priv = dzl_slider_get_instance_private (self);
  guint i;

  g_assert (DZL_IS_SLIDER (self));
  g_assert (allocation != NULL);

  gtk_widget_set_allocation (widget, allocation);

  for (i = 0; i < priv->children->len; i++)
    {
      DzlSliderChild *child = g_ptr_array_index (priv->children, i);

      if (gtk_widget_get_mapped (child->widget))
        {
          GtkAllocation window_allocation;
          GtkAllocation child_allocation;

          dzl_slider_compute_child_allocation (self, child, &window_allocation, &child_allocation);

          gdk_window_move_resize (child->window,
                                  window_allocation.x,
                                  window_allocation.y,
                                  window_allocation.width,
                                  window_allocation.height);

          /* raise the window edges */
          if (child->position != DZL_SLIDER_NONE)
            gdk_window_show (child->window);

          gtk_widget_size_allocate (child->widget, &child_allocation);
        }
    }
}

static void
dzl_slider_forall (GtkContainer *container,
                   gboolean      include_internals,
                   GtkCallback   callback,
                   gpointer      callback_data)
{
  DzlSlider *self = (DzlSlider *)container;
  DzlSliderPrivate *priv = dzl_slider_get_instance_private (self);
  GtkWidget **children;
  guint len;
  guint i;

  g_assert (DZL_IS_SLIDER (self));

  /*
   * We need to be widget re-entrant safe, meaning that the callback could
   * remove a child during callback(), using gtk_widget_destroy or similar. So
   * we create a local array containing a ref'd copy of all of the widgets in
   * case the callback removes widgets.
   */

  len = priv->children->len;
  children = g_new0 (GtkWidget *, len);

  for (i = 0; i < len; i++)
    {
      DzlSliderChild *child = g_ptr_array_index (priv->children, i);

      children [i] = g_object_ref (child->widget);
    }

  for (i = 0; i < len; i++)
    {
      callback (children [i], callback_data);
      g_object_unref (children [i]);
    }

  g_free (children);
}

static DzlSliderChild *
dzl_slider_get_child (DzlSlider *self,
                      GtkWidget *widget)
{
  DzlSliderPrivate *priv = dzl_slider_get_instance_private (self);
  gsize i;

  g_assert (DZL_IS_SLIDER (self));
  g_assert (GTK_IS_WIDGET (widget));
  g_assert (gtk_widget_get_parent (widget) == GTK_WIDGET (self));

  for (i = 0; i < priv->children->len; i++)
    {
      DzlSliderChild *child;

      child = g_ptr_array_index (priv->children, i);

      if (child->widget == widget)
        return child;
    }

  g_assert_not_reached ();

  return NULL;
}

static DzlSliderPosition
dzl_slider_child_get_position (DzlSlider *self,
                               GtkWidget *widget)
{
  DzlSliderChild *child;

  g_assert (DZL_IS_SLIDER (self));
  g_assert (GTK_IS_WIDGET (widget));

  child = dzl_slider_get_child (self, widget);

  return child->position;
}

static void
dzl_slider_child_set_position (DzlSlider         *self,
                               GtkWidget         *widget,
                               DzlSliderPosition  position)
{
  DzlSliderChild *child;

  g_assert (DZL_IS_SLIDER (self));
  g_assert (GTK_IS_WIDGET (widget));
  g_assert (position >= DZL_SLIDER_NONE);
  g_assert (position <= DZL_SLIDER_LEFT);

  child = dzl_slider_get_child (self, widget);

  if (position != child->position)
    {
      child->position = position;
      gtk_container_child_notify (GTK_CONTAINER (self), widget, "position");
      gtk_widget_queue_allocate (GTK_WIDGET (self));
    }
}

static void
dzl_slider_get_child_property (GtkContainer *container,
                               GtkWidget    *child,
                               guint         prop_id,
                               GValue       *value,
                               GParamSpec   *pspec)
{
  DzlSlider *self = (DzlSlider *)container;

  switch (prop_id)
    {
    case CHILD_PROP_POSITION:
      g_value_set_enum (value, dzl_slider_child_get_position (self, child));
      break;

    default:
      GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, prop_id, pspec);
    }
}

static void
dzl_slider_set_child_property (GtkContainer *container,
                               GtkWidget    *child,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  DzlSlider *self = (DzlSlider *)container;

  switch (prop_id)
    {
    case CHILD_PROP_POSITION:
      dzl_slider_child_set_position (self, child, g_value_get_enum (value));
      break;

    default:
      GTK_CONTAINER_WARN_INVALID_CHILD_PROPERTY_ID (container, prop_id, pspec);
    }
}

static void
dzl_slider_get_preferred_height (GtkWidget *widget,
                                 gint      *min_height,
                                 gint      *nat_height)
{
  DzlSlider *self = (DzlSlider *)widget;
  DzlSliderPrivate *priv = dzl_slider_get_instance_private (self);
  gint real_min_height = 0;
  gint real_nat_height = 0;
  gsize i;

  g_assert (DZL_IS_SLIDER (self));

  for (i = 0; i < priv->children->len; i++)
    {
      DzlSliderChild *child;
      gint child_min_height = 0;
      gint child_nat_height = 0;

      child = g_ptr_array_index (priv->children, i);

      if ((child->position == DZL_SLIDER_NONE) && gtk_widget_get_visible (child->widget))
        {
          gtk_widget_get_preferred_height (child->widget, &child_min_height, &child_nat_height);
          real_min_height = MAX (real_min_height, child_min_height);
          real_nat_height = MAX (real_nat_height, child_nat_height);
        }
    }

  *min_height = real_min_height;
  *nat_height = real_nat_height;
}

static void
dzl_slider_get_preferred_width (GtkWidget *widget,
                                gint      *min_width,
                                gint      *nat_width)
{
  DzlSlider *self = (DzlSlider *)widget;
  DzlSliderPrivate *priv = dzl_slider_get_instance_private (self);
  gint real_min_width = 0;
  gint real_nat_width = 0;
  gsize i;

  g_assert (DZL_IS_SLIDER (self));

  for (i = 0; i < priv->children->len; i++)
    {
      DzlSliderChild *child;
      gint child_min_width = 0;
      gint child_nat_width = 0;

      child = g_ptr_array_index (priv->children, i);

      if ((child->position == DZL_SLIDER_NONE) && gtk_widget_get_visible (child->widget))
        {
          gtk_widget_get_preferred_width (child->widget, &child_min_width, &child_nat_width);
          real_min_width = MAX (real_min_width, child_min_width);
          real_nat_width = MAX (real_nat_width, child_nat_width);
        }
    }

  *min_width = real_min_width;
  *nat_width = real_nat_width;
}

static void
dzl_slider_realize (GtkWidget *widget)
{
  DzlSlider *self = (DzlSlider *)widget;
  DzlSliderPrivate *priv = dzl_slider_get_instance_private (self);
  GdkWindow *window;
  gsize i;

  g_assert (DZL_IS_SLIDER (self));

  gtk_widget_set_realized (widget, TRUE);

  window = gtk_widget_get_parent_window (widget);
  gtk_widget_set_window (widget, g_object_ref (window));

  for (i = 0; i < priv->children->len; i++)
    {
      DzlSliderChild *child;

      child = g_ptr_array_index (priv->children, i);

      if (child->window == NULL)
        child->window = dzl_slider_create_child_window (self, child);
    }
}

static void
dzl_slider_unrealize (GtkWidget *widget)
{
  DzlSlider *self = (DzlSlider *)widget;
  DzlSliderPrivate *priv = dzl_slider_get_instance_private (self);
  gsize i;

  g_assert (DZL_IS_SLIDER (self));

  for (i = 0; i < priv->children->len; i++)
    {
      DzlSliderChild *child;

      child = g_ptr_array_index (priv->children, i);

      if (child->window != NULL)
        {
          gtk_widget_set_parent_window (child->widget, NULL);
          gtk_widget_unregister_window (widget, child->window);
          gdk_window_destroy (child->window);
          child->window = NULL;
        }
    }

  GTK_WIDGET_CLASS (dzl_slider_parent_class)->unrealize (widget);
}

static void
dzl_slider_map (GtkWidget *widget)
{
  DzlSlider *self = (DzlSlider *)widget;
  DzlSliderPrivate *priv = dzl_slider_get_instance_private (self);
  gsize i;

  g_assert (DZL_IS_SLIDER (self));

  GTK_WIDGET_CLASS (dzl_slider_parent_class)->map (widget);

  for (i = 0; i < priv->children->len; i++)
    {
      DzlSliderChild *child;

      child = g_ptr_array_index (priv->children, i);

      if ((child->window != NULL) &&
          gtk_widget_get_visible (child->widget) &&
          gtk_widget_get_child_visible (child->widget))
        gdk_window_show (child->window);
    }
}

static void
dzl_slider_unmap (GtkWidget *widget)
{
  DzlSlider *self = (DzlSlider *)widget;
  DzlSliderPrivate *priv = dzl_slider_get_instance_private (self);
  gsize i;

  g_assert (DZL_IS_SLIDER (self));

  for (i = 0; i < priv->children->len; i++)
    {
      DzlSliderChild *child;

      child = g_ptr_array_index (priv->children, i);

      if ((child->window != NULL) && gdk_window_is_visible (child->window))
        gdk_window_hide (child->window);
    }

  GTK_WIDGET_CLASS (dzl_slider_parent_class)->unmap (widget);
}

static void
dzl_slider_add_child (GtkBuildable *buildable,
                      GtkBuilder   *builder,
                      GObject      *child,
                      const gchar  *type)
{
  DzlSliderPosition position = DZL_SLIDER_NONE;
  DzlSlider *self = (DzlSlider *)buildable;

  g_assert (DZL_IS_SLIDER (self));
  g_assert (GTK_IS_BUILDABLE (buildable));
  g_assert (GTK_IS_BUILDER (builder));
  g_assert (G_IS_OBJECT (child));

  if (!GTK_IS_WIDGET (child))
    {
      g_warning ("Child \"%s\" must be of type GtkWidget.",
                 G_OBJECT_TYPE_NAME (child));
      return;
    }

  if (type == NULL)
    position = DZL_SLIDER_NONE;
  else if (g_str_equal (type, "bottom"))
    position = DZL_SLIDER_BOTTOM;
  else if (g_str_equal (type, "top"))
    position = DZL_SLIDER_TOP;
  else if (g_str_equal (type, "left"))
    position = DZL_SLIDER_LEFT;
  else if (g_str_equal (type, "right"))
    position = DZL_SLIDER_RIGHT;
  else
    g_warning ("Unknown child type \"%s\"", type);

  dzl_slider_add_slider (self, GTK_WIDGET (child), position);
}

static void
dzl_slider_finalize (GObject *object)
{
  DzlSlider *self = (DzlSlider *)object;
  DzlSliderPrivate *priv = dzl_slider_get_instance_private (self);

  g_clear_object (&priv->h_adj);
  g_clear_object (&priv->v_adj);
  g_clear_pointer (&priv->children, g_ptr_array_unref);

  dzl_clear_weak_pointer (&priv->h_anim);
  dzl_clear_weak_pointer (&priv->v_anim);

  G_OBJECT_CLASS (dzl_slider_parent_class)->finalize (object);
}

static void
dzl_slider_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  DzlSlider *self = DZL_SLIDER (object);

  switch (prop_id)
    {
    case PROP_POSITION:
      g_value_set_enum (value, dzl_slider_get_position (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_slider_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  DzlSlider *self = DZL_SLIDER (object);

  switch (prop_id)
    {
    case PROP_POSITION:
      dzl_slider_set_position (self, g_value_get_enum (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
buildable_iface_init (GtkBuildableIface *iface)
{
  iface->add_child = dzl_slider_add_child;
}

static void
dzl_slider_class_init (DzlSliderClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->finalize = dzl_slider_finalize;
  object_class->get_property = dzl_slider_get_property;
  object_class->set_property = dzl_slider_set_property;

  widget_class->get_preferred_height = dzl_slider_get_preferred_height;
  widget_class->get_preferred_width = dzl_slider_get_preferred_width;
  widget_class->map = dzl_slider_map;
  widget_class->realize = dzl_slider_realize;
  widget_class->size_allocate = dzl_slider_size_allocate;
  widget_class->unmap = dzl_slider_unmap;
  widget_class->unrealize = dzl_slider_unrealize;

  container_class->add = dzl_slider_add;
  container_class->forall = dzl_slider_forall;
  container_class->get_child_property = dzl_slider_get_child_property;
  container_class->remove = dzl_slider_remove;
  container_class->set_child_property = dzl_slider_set_child_property;

  properties [PROP_POSITION] =
    g_param_spec_enum ("position",
                       "Position",
                       "Which slider child is visible.",
                       DZL_TYPE_SLIDER_POSITION,
                       DZL_SLIDER_NONE,
                       (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);

  gtk_container_class_install_child_property (container_class,
                                              CHILD_PROP_POSITION,
                                              g_param_spec_enum ("position",
                                                                 "Position",
                                                                 "Position",
                                                                 DZL_TYPE_SLIDER_POSITION,
                                                                 DZL_SLIDER_NONE,
                                                                 G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

static void
dzl_slider_init (DzlSlider *self)
{
  DzlSliderPrivate *priv = dzl_slider_get_instance_private (self);

  priv->position = DZL_SLIDER_NONE;
  priv->children = g_ptr_array_new_with_free_func ((GDestroyNotify)dzl_slider_child_free);

  priv->v_adj = g_object_new (GTK_TYPE_ADJUSTMENT,
                              "lower", -1.0,
                              "upper", 1.0,
                              "value", 0.0,
                              NULL);
  g_signal_connect_object (priv->v_adj,
                           "value-changed",
                           G_CALLBACK (gtk_widget_queue_allocate),
                           self,
                           G_CONNECT_SWAPPED);

  priv->h_adj = g_object_new (GTK_TYPE_ADJUSTMENT,
                              "lower", -1.0,
                              "upper", 1.0,
                              "value", 0.0,
                              NULL);
  g_signal_connect_object (priv->h_adj,
                           "value-changed",
                           G_CALLBACK (gtk_widget_queue_allocate),
                           self,
                           G_CONNECT_SWAPPED);

  gtk_widget_set_has_window (GTK_WIDGET (self), FALSE);
}

GType
dzl_slider_position_get_type (void)
{
  static GType type_id;
  static const GEnumValue values[] = {
    { DZL_SLIDER_NONE, "DZL_SLIDER_NONE", "none" },
    { DZL_SLIDER_TOP, "DZL_SLIDER_TOP", "top" },
    { DZL_SLIDER_RIGHT, "DZL_SLIDER_RIGHT", "right" },
    { DZL_SLIDER_BOTTOM, "DZL_SLIDER_BOTTOM", "bottom" },
    { DZL_SLIDER_LEFT, "DZL_SLIDER_LEFT", "left" },
    { 0 }
  };

  if (g_once_init_enter (&type_id))
    {
      GType _type_id;

      _type_id = g_enum_register_static ("DzlSliderPosition", values);
      g_once_init_leave (&type_id, _type_id);
    }

  return type_id;
}

GtkWidget *
dzl_slider_new (void)
{
  return g_object_new (DZL_TYPE_SLIDER, NULL);
}

DzlSliderPosition
dzl_slider_get_position (DzlSlider *self)
{
  DzlSliderPrivate *priv = dzl_slider_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SLIDER (self), DZL_SLIDER_NONE);

  return priv->position;
}

void
dzl_slider_set_position (DzlSlider         *self,
                         DzlSliderPosition  position)
{
  DzlSliderPrivate *priv = dzl_slider_get_instance_private (self);

  g_return_if_fail (DZL_IS_SLIDER (self));
  g_return_if_fail (position >= DZL_SLIDER_NONE);
  g_return_if_fail (position <= DZL_SLIDER_LEFT);

  if (priv->position != position)
    {
      GdkFrameClock *frame_clock;
      DzlAnimation *anim;
      gdouble v_value;
      gdouble h_value;

      priv->position = position;

      if (priv->h_anim)
        dzl_animation_stop (priv->h_anim);
      dzl_clear_weak_pointer (&priv->h_anim);

      if (priv->v_anim)
        dzl_animation_stop (priv->v_anim);
      dzl_clear_weak_pointer (&priv->v_anim);

      switch (position)
        {
        case DZL_SLIDER_NONE:
          h_value = 0.0;
          v_value = 0.0;
          break;

        case DZL_SLIDER_TOP:
          h_value = 0.0;
          v_value = 1.0;
          break;

        case DZL_SLIDER_RIGHT:
          h_value = -1.0;
          v_value = 0.0;
          break;

        case DZL_SLIDER_BOTTOM:
          h_value = 0.0;
          v_value = -1.0;
          break;

        case DZL_SLIDER_LEFT:
          h_value = 1.0;
          v_value = 0.0;
          break;

        default:
          g_return_if_reached ();
        }

      frame_clock = gtk_widget_get_frame_clock (GTK_WIDGET (self));

      anim = dzl_object_animate (priv->h_adj,
                                 ANIMATION_MODE,
                                 ANIMATION_DURATION,
                                 frame_clock,
                                 "value", h_value,
                                 NULL);
      dzl_set_weak_pointer (&priv->h_anim, anim);

      anim = dzl_object_animate (priv->v_adj,
                                 ANIMATION_MODE,
                                 ANIMATION_DURATION,
                                 frame_clock,
                                 "value", v_value,
                                 NULL);
      dzl_set_weak_pointer (&priv->v_anim, anim);

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_POSITION]);
      gtk_widget_queue_allocate (GTK_WIDGET (self));
    }
}

void
dzl_slider_add_slider (DzlSlider         *self,
                       GtkWidget         *widget,
                       DzlSliderPosition  position)
{
  g_return_if_fail (DZL_IS_SLIDER (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (position >= DZL_SLIDER_NONE);
  g_return_if_fail (position <= DZL_SLIDER_LEFT);

  gtk_container_add_with_properties (GTK_CONTAINER (self), widget,
                                     "position", position,
                                     NULL);
}
