/* dzl-graph-view.c
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <dazzle.h>
#include <glib/gi18n.h>

#include "dzl-graph-view.h"

typedef struct
{
  DzlGraphModel   *model;
  DzlSignalGroup  *model_signals;
  GPtrArray       *renderers;
  cairo_surface_t *surface;
  guint            tick_handler;
  gdouble          x_offset;
  guint            missed_count;
  guint            surface_dirty : 1;
} DzlGraphViewPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (DzlGraphView, dzl_graph_view, GTK_TYPE_DRAWING_AREA)

enum {
  PROP_0,
  PROP_TABLE,
  LAST_PROP
};

static GParamSpec *properties [LAST_PROP];

GtkWidget *
dzl_graph_view_new (void)
{
  return g_object_new (DZL_TYPE_GRAPH_VIEW, NULL);
}

static void
dzl_graph_view_clear_surface (DzlGraphView *self)
{
  DzlGraphViewPrivate *priv = dzl_graph_view_get_instance_private (self);

  g_assert (DZL_IS_GRAPH_VIEW (self));

  priv->surface_dirty = TRUE;
}

/**
 * dzl_graph_view_get_model:
 *
 * Gets the #DzlGraphView:model property.
 *
 * Returns: (transfer none) (nullable): An #DzlGraphModel or %NULL.
 */
DzlGraphModel *
dzl_graph_view_get_model (DzlGraphView *self)
{
  DzlGraphViewPrivate *priv = dzl_graph_view_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_GRAPH_VIEW (self), NULL);

  return priv->model;
}

void
dzl_graph_view_set_model (DzlGraphView  *self,
                          DzlGraphModel *model)
{
  DzlGraphViewPrivate *priv = dzl_graph_view_get_instance_private (self);

  g_return_if_fail (DZL_IS_GRAPH_VIEW (self));
  g_return_if_fail (!model || DZL_IS_GRAPH_MODEL (model));

  if (g_set_object (&priv->model, model))
    {
      dzl_signal_group_set_target (priv->model_signals, model);
      gtk_widget_queue_allocate (GTK_WIDGET (self));
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_TABLE]);
    }
}

void
dzl_graph_view_add_renderer (DzlGraphView     *self,
                             DzlGraphRenderer *renderer)
{
  DzlGraphViewPrivate *priv = dzl_graph_view_get_instance_private (self);

  g_return_if_fail (DZL_IS_GRAPH_VIEW (self));
  g_return_if_fail (DZL_IS_GRAPH_RENDERER (renderer));

  g_ptr_array_add (priv->renderers, g_object_ref (renderer));
  dzl_graph_view_clear_surface (self);
}

static gboolean
dzl_graph_view_tick_cb (GtkWidget     *widget,
                        GdkFrameClock *frame_clock,
                        gpointer       user_data)
{
  DzlGraphView *self = (DzlGraphView *)widget;
  DzlGraphViewPrivate *priv = dzl_graph_view_get_instance_private (self);
  GtkAllocation alloc;
  gint64 frame_time;
  gint64 end_time;
  gint64 timespan;
  gdouble x_offset;

  g_assert (DZL_IS_GRAPH_VIEW (self));

  if ((priv->surface == NULL) || (priv->model == NULL) || !gtk_widget_get_visible (widget))
    goto remove_handler;

  /*
   * If we've missed drawings for the last 10 tick callbacks, chances are we're
   * visible, but not being displayed to the user because we're not top-most.
   * Disable ourselves in that case too so that we don't spin the frame-clock.
   *
   * We'll be re-enabled when the next ensure_surface() is called (upon a real
   * draw by the system).
   *
   * The missed_count is reset on draw().
   */
  if (priv->missed_count > 10)
    goto remove_handler;
  else
    priv->missed_count++;

  timespan = dzl_graph_view_model_get_timespan (priv->model);
  if (timespan == 0)
    goto remove_handler;

  gtk_widget_get_allocation (widget, &alloc);

  frame_time = g_get_monotonic_time ();
  end_time = dzl_graph_view_model_get_end_time (priv->model);

  x_offset = -((frame_time - end_time) / (gdouble)timespan);

  if (x_offset != priv->x_offset)
    {
      priv->x_offset = x_offset;
      gtk_widget_queue_draw (widget);
    }

  return G_SOURCE_CONTINUE;

remove_handler:
  if (priv->tick_handler != 0)
    {
      gtk_widget_remove_tick_callback (widget, priv->tick_handler);
      priv->tick_handler = 0;
    }

  return G_SOURCE_REMOVE;
}

static void
dzl_graph_view_ensure_surface (DzlGraphView *self)
{
  DzlGraphViewPrivate *priv = dzl_graph_view_get_instance_private (self);
  GtkAllocation alloc;
  DzlGraphModelIter iter;
  gint64 begin_time;
  gint64 end_time;
  gdouble y_begin;
  gdouble y_end;
  cairo_t *cr;
  gsize i;

  g_assert (DZL_IS_GRAPH_VIEW (self));

  gtk_widget_get_allocation (GTK_WIDGET (self), &alloc);

  if (priv->surface == NULL)
    {
      priv->surface_dirty = TRUE;
      priv->surface = gdk_window_create_similar_surface (gtk_widget_get_window (GTK_WIDGET (self)),
                                                         CAIRO_CONTENT_COLOR_ALPHA,
                                                         alloc.width,
                                                         alloc.height);
    }

  if (priv->model == NULL)
    return;

  if (priv->surface_dirty)
    {
      priv->surface_dirty = FALSE;

      cr = cairo_create (priv->surface);

      cairo_save (cr);
      cairo_rectangle (cr, 0, 0, alloc.width, alloc.height);
      cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
      cairo_fill (cr);
      cairo_restore (cr);

      g_object_get (priv->model,
                    "value-min", &y_begin,
                    "value-max", &y_end,
                    NULL);

      dzl_graph_view_model_get_iter_last (priv->model, &iter);
      end_time = dzl_graph_view_model_iter_get_timestamp (&iter);
      begin_time = end_time - dzl_graph_view_model_get_timespan (priv->model);

      for (i = 0; i < priv->renderers->len; i++)
        {
          DzlGraphRenderer *renderer;

          renderer = g_ptr_array_index (priv->renderers, i);

          cairo_save (cr);
          dzl_graph_view_renderer_render (renderer, priv->model, begin_time, end_time, y_begin, y_end, cr, &alloc);
          cairo_restore (cr);
        }

      cairo_destroy (cr);
    }

  if (priv->tick_handler == 0)
    priv->tick_handler = gtk_widget_add_tick_callback (GTK_WIDGET (self),
                                                       dzl_graph_view_tick_cb,
                                                       self,
                                                       NULL);
}

static gboolean
dzl_graph_view_draw (GtkWidget *widget,
                     cairo_t   *cr)
{
  DzlGraphView *self = (DzlGraphView *)widget;
  DzlGraphViewPrivate *priv = dzl_graph_view_get_instance_private (self);
  GtkStyleContext *style_context;
  GtkAllocation alloc;

  g_assert (DZL_IS_GRAPH_VIEW (self));

  priv->missed_count = 0;

  gtk_widget_get_allocation (widget, &alloc);

  style_context = gtk_widget_get_style_context (widget);

  dzl_graph_view_ensure_surface (self);

  gtk_style_context_save (style_context);
  gtk_style_context_add_class (style_context, "view");
  gtk_render_background (style_context, cr, 0, 0, alloc.width, alloc.height);
  gtk_style_context_restore (style_context);

  cairo_save (cr);
  cairo_set_source_surface (cr, priv->surface, priv->x_offset * alloc.width, 0);
  cairo_rectangle (cr, 0, 0, alloc.width, alloc.height);
  cairo_fill (cr);
  cairo_restore (cr);

  return GDK_EVENT_PROPAGATE;
}

static void
dzl_graph_view_size_allocate (GtkWidget     *widget,
                              GtkAllocation *alloc)
{
  DzlGraphView *self = (DzlGraphView *)widget;
  DzlGraphViewPrivate *priv = dzl_graph_view_get_instance_private (self);
  GtkAllocation old_alloc;

  g_assert (DZL_IS_GRAPH_VIEW (self));
  g_assert (alloc != NULL);

  gtk_widget_get_allocation (widget, &old_alloc);

  if ((old_alloc.width != alloc->width) || (old_alloc.height != alloc->height))
    g_clear_pointer (&priv->surface, cairo_surface_destroy);

  GTK_WIDGET_CLASS (dzl_graph_view_parent_class)->size_allocate (widget, alloc);
}

static void
dzl_graph_view__model__changed (DzlGraphView  *self,
                                DzlGraphModel *model)
{
  DzlGraphViewPrivate *priv = dzl_graph_view_get_instance_private (self);

  g_assert (DZL_IS_GRAPH_VIEW (self));
  g_assert (DZL_IS_GRAPH_MODEL (model));

  priv->x_offset = 0;

  dzl_graph_view_clear_surface (self);
}

static void
dzl_graph_view__model__notify_timespan (DzlGraphView  *self,
                                        GParamSpec    *pspec,
                                        DzlGraphModel *model)
{
  g_assert (DZL_IS_GRAPH_VIEW (self));
  g_assert (DZL_IS_GRAPH_MODEL (model));

  /* Avoid this in a number of scenarios */
  if (gtk_widget_get_visible (GTK_WIDGET (self)) &&
      gtk_widget_get_child_visible (GTK_WIDGET (self)))
    gtk_widget_queue_draw (GTK_WIDGET (self));
}

static void
dzl_graph_view_destroy (GtkWidget *widget)
{
  DzlGraphView *self = (DzlGraphView *)widget;
  DzlGraphViewPrivate *priv = dzl_graph_view_get_instance_private (self);

  if (priv->tick_handler != 0)
    {
      gtk_widget_remove_tick_callback (widget, priv->tick_handler);
      priv->tick_handler = 0;
    }

  g_clear_pointer (&priv->surface, cairo_surface_destroy);

  GTK_WIDGET_CLASS (dzl_graph_view_parent_class)->destroy (widget);
}

static void
dzl_graph_view_finalize (GObject *object)
{
  DzlGraphView *self = (DzlGraphView *)object;
  DzlGraphViewPrivate *priv = dzl_graph_view_get_instance_private (self);

  g_clear_object (&priv->model);
  g_clear_object (&priv->model_signals);
  g_clear_pointer (&priv->renderers, g_ptr_array_unref);

  G_OBJECT_CLASS (dzl_graph_view_parent_class)->finalize (object);
}

static void
dzl_graph_view_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  DzlGraphView *self = DZL_GRAPH_VIEW (object);

  switch (prop_id)
    {
    case PROP_TABLE:
      g_value_set_object (value, dzl_graph_view_get_model (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_graph_view_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  DzlGraphView *self = DZL_GRAPH_VIEW (object);

  switch (prop_id)
    {
    case PROP_TABLE:
      dzl_graph_view_set_model (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_graph_view_class_init (DzlGraphViewClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = dzl_graph_view_finalize;
  object_class->get_property = dzl_graph_view_get_property;
  object_class->set_property = dzl_graph_view_set_property;

  widget_class->destroy = dzl_graph_view_destroy;
  widget_class->draw = dzl_graph_view_draw;
  widget_class->size_allocate = dzl_graph_view_size_allocate;

  properties [PROP_TABLE] =
    g_param_spec_object ("model",
                         "Table",
                         "The data model for the graph.",
                         DZL_TYPE_GRAPH_MODEL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);

  gtk_widget_class_set_css_name (widget_class, "dzlgraphview");
}

static void
dzl_graph_view_init (DzlGraphView *self)
{
  DzlGraphViewPrivate *priv = dzl_graph_view_get_instance_private (self);

  priv->renderers = g_ptr_array_new_with_free_func (g_object_unref);

  priv->model_signals = dzl_signal_group_new (DZL_TYPE_GRAPH_MODEL);

  dzl_signal_group_connect_object (priv->model_signals,
                                   "notify::value-max",
                                   G_CALLBACK (gtk_widget_queue_allocate),
                                   self,
                                   G_CONNECT_SWAPPED);

  dzl_signal_group_connect_object (priv->model_signals,
                                   "notify::value-min",
                                   G_CALLBACK (gtk_widget_queue_allocate),
                                   self,
                                   G_CONNECT_SWAPPED);

  dzl_signal_group_connect_object (priv->model_signals,
                                   "notify::timespan",
                                   G_CALLBACK (dzl_graph_view__model__notify_timespan),
                                   self,
                                   G_CONNECT_SWAPPED);

  dzl_signal_group_connect_object (priv->model_signals,
                                   "changed",
                                   G_CALLBACK (dzl_graph_view__model__changed),
                                   self,
                                   G_CONNECT_SWAPPED);
}
