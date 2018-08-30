/* dzl-list-model-filter.h
 *
 * Copyright (C) 2016 Christian Hergert <christian@hergert.me>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <gio/gio.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_LIST_MODEL_FILTER (dzl_list_model_filter_get_type())

typedef gboolean (*DzlListModelFilterFunc) (GObject  *object,
                                            gpointer  user_data);

DZL_AVAILABLE_IN_3_30
G_DECLARE_FINAL_TYPE (DzlListModelFilter, dzl_list_model_filter, DZL, LIST_MODEL_FILTER, GObject)

DZL_AVAILABLE_IN_3_30
DzlListModelFilter *dzl_list_model_filter_new             (GListModel             *child_model);
DZL_AVAILABLE_IN_3_30
GListModel         *dzl_list_model_filter_get_child_model (DzlListModelFilter     *self);
DZL_AVAILABLE_IN_3_30
void                dzl_list_model_filter_invalidate      (DzlListModelFilter     *self);
DZL_AVAILABLE_IN_3_30
void                dzl_list_model_filter_set_filter_func (DzlListModelFilter     *self,
                                                           DzlListModelFilterFunc  filter_func,
                                                           gpointer                filter_func_data,
                                                           GDestroyNotify          filter_func_data_destroy);

G_END_DECLS
