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

#include "dzl-version-macros.h"

#include "dzl-graph-column.h"

G_BEGIN_DECLS

#define DZL_TYPE_GRAPH_MODEL (dzl_graph_view_model_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlGraphModel, dzl_graph_view_model, DZL, GRAPH_MODEL, GObject)

struct _DzlGraphModelClass
{
  GObjectClass parent;
};

typedef struct
{
  gpointer data[8];
} DzlGraphModelIter;

DZL_AVAILABLE_IN_ALL
DzlGraphModel *dzl_graph_view_model_new                (void);
DZL_AVAILABLE_IN_ALL
guint          dzl_graph_view_model_add_column         (DzlGraphModel     *self,
                                                        DzlGraphColumn    *column);
DZL_AVAILABLE_IN_ALL
guint          dzl_graph_view_model_get_n_columns      (DzlGraphModel     *self);
DZL_AVAILABLE_IN_ALL
GTimeSpan      dzl_graph_view_model_get_timespan       (DzlGraphModel     *self);
DZL_AVAILABLE_IN_ALL
void           dzl_graph_view_model_set_timespan       (DzlGraphModel     *self,
                                                        GTimeSpan          timespan);
DZL_AVAILABLE_IN_ALL
gint64         dzl_graph_view_model_get_end_time       (DzlGraphModel     *self);
DZL_AVAILABLE_IN_ALL
guint          dzl_graph_view_model_get_max_samples    (DzlGraphModel     *self);
DZL_AVAILABLE_IN_ALL
void           dzl_graph_view_model_set_max_samples    (DzlGraphModel     *self,
                                                        guint              n_rows);
DZL_AVAILABLE_IN_ALL
void           dzl_graph_view_model_push               (DzlGraphModel     *self,
                                                        DzlGraphModelIter *iter,
                                                        gint64             timestamp);
DZL_AVAILABLE_IN_ALL
gboolean       dzl_graph_view_model_get_iter_first     (DzlGraphModel     *self,
                                                        DzlGraphModelIter *iter);
DZL_AVAILABLE_IN_ALL
gboolean       dzl_graph_view_model_get_iter_last      (DzlGraphModel     *self,
                                                        DzlGraphModelIter *iter);
DZL_AVAILABLE_IN_ALL
gboolean       dzl_graph_view_model_iter_next          (DzlGraphModelIter *iter);
DZL_AVAILABLE_IN_ALL
void           dzl_graph_view_model_iter_get           (DzlGraphModelIter *iter,
                                                        gint               first_column,
                                                        ...);
DZL_AVAILABLE_IN_ALL
void           dzl_graph_view_model_iter_get_value     (DzlGraphModelIter *iter,
                                                        guint              column,
                                                        GValue            *value);
DZL_AVAILABLE_IN_ALL
gint64         dzl_graph_view_model_iter_get_timestamp (DzlGraphModelIter *iter);
DZL_AVAILABLE_IN_ALL
void           dzl_graph_view_model_iter_set           (DzlGraphModelIter *iter,
                                                        gint               first_column,
                                                        ...);
DZL_AVAILABLE_IN_3_30
void           dzl_graph_view_model_iter_set_value     (DzlGraphModelIter *iter,
                                                        guint              column,
                                                        const GValue      *value);

G_END_DECLS

#endif /* DZL_GRAPH_MODEL_H */
