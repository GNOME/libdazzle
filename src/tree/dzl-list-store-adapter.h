/* dzl-list-store-adapter.h
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
 *
 * This file is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DZL_LIST_STORE_ADAPTER_H
#define DZL_LIST_STORE_ADAPTER_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_LIST_STORE_ADAPTER (dzl_list_store_adapter_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlListStoreAdapter, dzl_list_store_adapter, DZL, LIST_STORE_ADAPTER, GObject)

struct _DzlListStoreAdapterClass
{
  GObjectClass parent_class;
};

DZL_AVAILABLE_IN_ALL
DzlListStoreAdapter *dzl_list_store_adapter_new       (GListModel          *model);
DZL_AVAILABLE_IN_ALL
GListModel          *dzl_list_store_adapter_get_model (DzlListStoreAdapter *self);
DZL_AVAILABLE_IN_ALL
void                 dzl_list_store_adapter_set_model (DzlListStoreAdapter *self,
                                                       GListModel          *model);

G_END_DECLS

#endif /* DZL_LIST_STORE_ADAPTER_H */
