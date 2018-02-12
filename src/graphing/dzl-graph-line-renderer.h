/* dzl-graph-line-renderer.h
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

#ifndef DZL_GRAPH_LINE_RENDERER_H
#define DZL_GRAPH_LINE_RENDERER_H

#include <gdk/gdk.h>

#include "dzl-version-macros.h"

#include "dzl-graph-renderer.h"

G_BEGIN_DECLS

#define DZL_TYPE_GRAPH_LINE_RENDERER (dzl_graph_view_line_renderer_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlGraphLineRenderer, dzl_graph_view_line_renderer, DZL, GRAPH_LINE_RENDERER, GObject)

DZL_AVAILABLE_IN_ALL
DzlGraphLineRenderer *dzl_graph_view_line_renderer_new (void);
DZL_AVAILABLE_IN_ALL
void            dzl_graph_view_line_renderer_set_stroke_color      (DzlGraphLineRenderer *self,
                                                        const gchar    *stroke_color);
DZL_AVAILABLE_IN_ALL
void            dzl_graph_view_line_renderer_set_stroke_color_rgba (DzlGraphLineRenderer *self,
                                                        const GdkRGBA  *stroke_color_rgba);
DZL_AVAILABLE_IN_ALL
const GdkRGBA  *dzl_graph_view_line_renderer_get_stroke_color_rgba (DzlGraphLineRenderer *self);

G_END_DECLS

#endif /* DZL_GRAPH_LINE_RENDERER_H */
