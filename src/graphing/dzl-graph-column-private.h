/* dzl-graph-column-private.h
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

#ifndef DZL_GRAPH_COLUMN_PRIVATE_H
#define DZL_GRAPH_COLUMN_PRIVATE_H

#include <glib-object.h>

#include "dzl-graph-column.h"

G_BEGIN_DECLS

void  _dzl_graph_view_column_get_value  (DzlGraphColumn *self,
                                         guint           index,
                                         GValue         *value);
void  _dzl_graph_view_column_set_value  (DzlGraphColumn *self,
                                         guint           index,
                                         const GValue   *value);
void  _dzl_graph_view_column_collect    (DzlGraphColumn *self,
                                         guint           index,
                                         va_list        *args);
void  _dzl_graph_view_column_lcopy      (DzlGraphColumn *self,
                                         guint           index,
                                         va_list        *args);
void  _dzl_graph_view_column_get        (DzlGraphColumn *column,
                                         guint           index,
                                         ...);
void  _dzl_graph_view_column_set        (DzlGraphColumn *column,
                                         guint           index,
                                         ...);
guint _dzl_graph_view_column_push       (DzlGraphColumn *column);
void  _dzl_graph_view_column_set_n_rows (DzlGraphColumn *column,
                                         guint           n_rows);

G_END_DECLS

#endif /* DZL_GRAPH_COLUMN_PRIVATE_H */
