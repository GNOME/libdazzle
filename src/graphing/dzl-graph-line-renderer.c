/* dzl-graph-line-renderer.c
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

#include <glib/gi18n.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "dzl-graph-line-renderer.h"

struct _DzlGraphLineRenderer
{
  GObject parent_instance;

  GdkRGBA stroke_color;
  gdouble line_width;
  guint   column;
};

static void dzl_graph_view_line_renderer_init_renderer (DzlGraphRendererInterface *iface);

G_DEFINE_TYPE_WITH_CODE (DzlGraphLineRenderer, dzl_graph_view_line_renderer, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (DZL_TYPE_GRAPH_RENDERER,
                                                dzl_graph_view_line_renderer_init_renderer))

enum {
  PROP_0,
  PROP_COLUMN,
  PROP_LINE_WIDTH,
  PROP_STROKE_COLOR,
  PROP_STROKE_COLOR_RGBA,
  LAST_PROP
};

static GParamSpec *properties [LAST_PROP];

DzlGraphLineRenderer *
dzl_graph_view_line_renderer_new (void)
{
  return g_object_new (DZL_TYPE_GRAPH_LINE_RENDERER, NULL);
}

static gdouble
calc_x (DzlGraphModelIter *iter,
        gint64       begin,
        gint64       end,
        guint        width)
{
  gint64 timestamp;

  timestamp = dzl_graph_view_model_iter_get_timestamp (iter);

  g_assert_cmpint (timestamp, !=, 0);

  return ((timestamp - begin) / (gdouble)(end - begin) * width);
}

static gdouble
calc_y (DzlGraphModelIter *iter,
        gdouble      range_begin,
        gdouble      range_end,
        guint        height,
        guint        column)
{
  GValue value = G_VALUE_INIT;
  gdouble y;

  dzl_graph_view_model_iter_get_value (iter, column, &value);

  switch (G_VALUE_TYPE (&value))
    {
    case G_TYPE_DOUBLE:
      y = g_value_get_double (&value);
      break;

    case G_TYPE_UINT:
      y = g_value_get_uint (&value);
      break;

    case G_TYPE_UINT64:
      y = g_value_get_uint64 (&value);
      break;

    case G_TYPE_INT:
      y = g_value_get_int (&value);
      break;

    case G_TYPE_INT64:
      y = g_value_get_int64 (&value);
      break;

    default:
      y = 0.0;
      break;
    }

  y -= range_begin;
  y /= (range_end - range_begin);
  y = height - (y * height);

  return y;
}

static void
dzl_graph_view_line_renderer_render (DzlGraphRenderer                  *renderer,
                         DzlGraphModel                     *table,
                         gint64                       x_begin,
                         gint64                       x_end,
                         gdouble                      y_begin,
                         gdouble                      y_end,
                         cairo_t                     *cr,
                         const cairo_rectangle_int_t *area)
{
  DzlGraphLineRenderer *self = (DzlGraphLineRenderer *)renderer;
  DzlGraphModelIter iter;

  g_assert (DZL_IS_GRAPH_LINE_RENDERER (self));

  cairo_save (cr);

  if (dzl_graph_view_model_get_iter_first (table, &iter))
    {
      guint max_samples;
      gdouble chunk;
      gdouble last_x;
      gdouble last_y;

      max_samples = dzl_graph_view_model_get_max_samples (table);

      chunk = area->width / (gdouble)(max_samples - 1) / 2.0;

      last_x = calc_x (&iter, x_begin, x_end, area->width);
      last_y = calc_y (&iter, y_begin, y_end, area->height, self->column);

      cairo_move_to (cr, last_x, last_y);

      while (dzl_graph_view_model_iter_next (&iter))
        {
          gdouble x;
          gdouble y;

          x = calc_x (&iter, x_begin, x_end, area->width);
          y = calc_y (&iter, y_begin, y_end, area->height, self->column);

          cairo_curve_to (cr,
                          last_x + chunk,
                          last_y,
                          last_x + chunk,
                          y,
                          x,
                          y);

          last_x = x;
          last_y = y;
        }
    }

  cairo_set_line_width (cr, self->line_width);
  gdk_cairo_set_source_rgba (cr, &self->stroke_color);
  cairo_stroke (cr);

  cairo_restore (cr);
}

static void
dzl_graph_view_line_renderer_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  DzlGraphLineRenderer *self = DZL_GRAPH_LINE_RENDERER (object);

  switch (prop_id)
    {
    case PROP_COLUMN:
      g_value_set_uint (value, self->column);
      break;

    case PROP_LINE_WIDTH:
      g_value_set_double (value, self->line_width);
      break;

    case PROP_STROKE_COLOR:
      g_value_take_string (value, gdk_rgba_to_string (&self->stroke_color));
      break;

    case PROP_STROKE_COLOR_RGBA:
      g_value_set_boxed (value, &self->stroke_color);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_graph_view_line_renderer_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  DzlGraphLineRenderer *self = DZL_GRAPH_LINE_RENDERER (object);

  switch (prop_id)
    {
    case PROP_COLUMN:
      self->column = g_value_get_uint (value);
      break;

    case PROP_LINE_WIDTH:
      self->line_width = g_value_get_double (value);
      break;

    case PROP_STROKE_COLOR:
      dzl_graph_view_line_renderer_set_stroke_color (self, g_value_get_string (value));
      break;

    case PROP_STROKE_COLOR_RGBA:
      dzl_graph_view_line_renderer_set_stroke_color_rgba (self, g_value_get_boxed (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_graph_view_line_renderer_class_init (DzlGraphLineRendererClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = dzl_graph_view_line_renderer_get_property;
  object_class->set_property = dzl_graph_view_line_renderer_set_property;

  properties [PROP_COLUMN] =
    g_param_spec_uint ("column",
                       "Column",
                       "Column",
                       0, G_MAXUINT,
                       0,
                       (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_LINE_WIDTH] =
    g_param_spec_double ("line-width",
                         "Line Width",
                         "Line Width",
                         0.0, G_MAXDOUBLE,
                         1.0,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_STROKE_COLOR] =
    g_param_spec_string ("stroke-color",
                         "Stroke Color",
                         "Stroke Color",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_STROKE_COLOR_RGBA] =
    g_param_spec_boxed ("stroke-color-rgba",
                        "Stroke Color RGBA",
                        "Stroke Color RGBA",
                        GDK_TYPE_RGBA,
                        (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
dzl_graph_view_line_renderer_init (DzlGraphLineRenderer *self)
{
  self->line_width = 1.0;
}

static void
dzl_graph_view_line_renderer_init_renderer (DzlGraphRendererInterface *iface)
{
  iface->render = dzl_graph_view_line_renderer_render;
}

void
dzl_graph_view_line_renderer_set_stroke_color_rgba (DzlGraphLineRenderer *self,
                                        const GdkRGBA  *rgba)
{
  const GdkRGBA black = { 0, 0, 0, 1.0 };

  g_return_if_fail (DZL_IS_GRAPH_LINE_RENDERER (self));

  if (rgba == NULL)
    rgba = &black;

  if (!gdk_rgba_equal (rgba, &self->stroke_color))
    {
      self->stroke_color = *rgba;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_STROKE_COLOR_RGBA]);
    }
}

void
dzl_graph_view_line_renderer_set_stroke_color (DzlGraphLineRenderer *self,
                                   const gchar    *stroke_color)
{
  GdkRGBA rgba;

  g_return_if_fail (DZL_IS_GRAPH_LINE_RENDERER (self));

  if (stroke_color == NULL)
    stroke_color = "#000000";

  if (gdk_rgba_parse (&rgba, stroke_color))
    dzl_graph_view_line_renderer_set_stroke_color_rgba (self, &rgba);
}
