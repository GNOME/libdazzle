/* dzl-graph-renderer.h
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

#ifndef DZL_GRAPH_RENDERER_H
#define DZL_GRAPH_RENDERER_H

#include <glib-object.h>

#include "dzl-graph-model.h"
#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_GRAPH_RENDERER (dzl_graph_view_renderer_get_type ())

DZL_AVAILABLE_IN_ALL
G_DECLARE_INTERFACE (DzlGraphRenderer, dzl_graph_view_renderer, DZL, GRAPH_RENDERER, GObject)

struct _DzlGraphRendererInterface
{
  GTypeInterface parent;

  void (*render) (DzlGraphRenderer                  *self,
                  DzlGraphModel                     *table,
                  gint64                       x_begin,
                  gint64                       x_end,
                  gdouble                      y_begin,
                  gdouble                      y_end,
                  cairo_t                     *cr,
                  const cairo_rectangle_int_t *area);
};

DZL_AVAILABLE_IN_ALL
void dzl_graph_view_renderer_render (DzlGraphRenderer                  *self,
                                     DzlGraphModel                     *table,
                                     gint64                       x_begin,
                                     gint64                       x_end,
                                     gdouble                      y_begin,
                                     gdouble                      y_end,
                                     cairo_t                     *cr,
                                     const cairo_rectangle_int_t *area);

G_END_DECLS

#endif /* DZL_GRAPH_RENDERER_H */
