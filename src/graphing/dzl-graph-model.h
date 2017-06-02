/* dzl-graph-model.h
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

#ifndef DZL_GRAPH_MODEL_H
#define DZL_GRAPH_MODEL_H

#include <glib-object.h>

#include "dzl-graph-column.h"

G_BEGIN_DECLS

#define DZL_TYPE_GRAPH_MODEL (dzl_graph_view_model_get_type())

G_DECLARE_DERIVABLE_TYPE (DzlGraphModel, dzl_graph_view_model, DZL, GRAPH_MODEL, GObject)

struct _DzlGraphModelClass
{
  GObjectClass parent;
};

typedef struct
{
  gpointer data[8];
} DzlGraphModelIter;

DzlGraphModel   *dzl_graph_view_model_new                (void);
guint      dzl_graph_view_model_add_column         (DzlGraphModel     *self,
                                        DzlGraphColumn    *column);
GTimeSpan  dzl_graph_view_model_get_timespan       (DzlGraphModel     *self);
void       dzl_graph_view_model_set_timespan       (DzlGraphModel     *self,
                                        GTimeSpan    timespan);
gint64     dzl_graph_view_model_get_end_time       (DzlGraphModel     *self);
guint      dzl_graph_view_model_get_max_samples    (DzlGraphModel     *self);
void       dzl_graph_view_model_set_max_samples    (DzlGraphModel     *self,
                                        guint        n_rows);
void       dzl_graph_view_model_push               (DzlGraphModel     *self,
                                        DzlGraphModelIter *iter,
                                        gint64       timestamp);
gboolean   dzl_graph_view_model_get_iter_first     (DzlGraphModel     *self,
                                        DzlGraphModelIter *iter);
gboolean   dzl_graph_view_model_get_iter_last      (DzlGraphModel     *self,
                                        DzlGraphModelIter *iter);
gboolean   dzl_graph_view_model_iter_next          (DzlGraphModelIter *iter);
void       dzl_graph_view_model_iter_get           (DzlGraphModelIter *iter,
                                        gint         first_column,
                                        ...);
void       dzl_graph_view_model_iter_get_value     (DzlGraphModelIter *iter,
                                        guint        column,
                                        GValue      *value);
gint64     dzl_graph_view_model_iter_get_timestamp (DzlGraphModelIter *iter);
void       dzl_graph_view_model_iter_set           (DzlGraphModelIter *iter,
                                        gint         first_column,
                                        ...);

G_END_DECLS

#endif /* DZL_GRAPH_MODEL_H */
