/* dzl-graph-view.h
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

#ifndef DZL_GRAPH_VIEW_H
#define DZL_GRAPH_VIEW_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

#include "dzl-graph-model.h"
#include "dzl-graph-renderer.h"

G_BEGIN_DECLS

#define DZL_TYPE_GRAPH_VIEW (dzl_graph_view_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlGraphView, dzl_graph_view, DZL, GRAPH_VIEW, GtkDrawingArea)

struct _DzlGraphViewClass
{
  GtkDrawingAreaClass parent_class;

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
  gpointer _reserved5;
  gpointer _reserved6;
  gpointer _reserved7;
  gpointer _reserved8;
};

DZL_AVAILABLE_IN_ALL
GtkWidget     *dzl_graph_view_new          (void);
DZL_AVAILABLE_IN_ALL
void           dzl_graph_view_set_model    (DzlGraphView     *self,
                                            DzlGraphModel    *model);
DZL_AVAILABLE_IN_ALL
DzlGraphModel *dzl_graph_view_get_model    (DzlGraphView     *self);
DZL_AVAILABLE_IN_ALL
void           dzl_graph_view_add_renderer (DzlGraphView     *self,
                                            DzlGraphRenderer *renderer);

G_END_DECLS

#endif /* DZL_GRAPH_VIEW_H */
