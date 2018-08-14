/* dzl-graph-column.h
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

#ifndef DZL_GRAPH_COLUMN_H
#define DZL_GRAPH_COLUMN_H

#include <glib-object.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_GRAPH_COLUMN (dzl_graph_view_column_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlGraphColumn, dzl_graph_view_column, DZL, GRAPH_COLUMN, GObject)

struct _DzlGraphColumnClass
{
  GObjectClass parent;
};

DZL_AVAILABLE_IN_ALL
DzlGraphColumn *dzl_graph_view_column_new      (const gchar    *name,
                                                GType           value_type);
DZL_AVAILABLE_IN_ALL
const gchar    *dzl_graph_view_column_get_name (DzlGraphColumn *self);
DZL_AVAILABLE_IN_ALL
void            dzl_graph_view_column_set_name (DzlGraphColumn *self,
                                                const gchar    *name);

G_END_DECLS

#endif /* DZL_GRAPH_COLUMN_H */
