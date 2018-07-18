/* dzl-cpu-graph.h
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

#ifndef DZL_CPU_GRAPH_H
#define DZL_CPU_GRAPH_H

#include "dzl-version-macros.h"

#include "graphing/dzl-graph-view.h"

G_BEGIN_DECLS

#define DZL_TYPE_CPU_GRAPH (dzl_cpu_graph_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlCpuGraph, dzl_cpu_graph, DZL, CPU_GRAPH, DzlGraphView)

DZL_AVAILABLE_IN_3_30
GtkWidget *dzl_cpu_graph_new_full (gint64 timespan,
                                   guint  max_samples);

G_END_DECLS

#endif /* DZL_CPU_GRAPH_H */
