/* dzl-read-only-list-model.h
 *
 * Copyright © 2018 Christian Hergert <chergert@redhat.com>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <gio/gio.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_READ_ONLY_LIST_MODEL (dzl_read_only_list_model_get_type())

DZL_AVAILABLE_IN_3_30
G_DECLARE_FINAL_TYPE (DzlReadOnlyListModel, dzl_read_only_list_model, DZL, READ_ONLY_LIST_MODEL, GObject)

DZL_AVAILABLE_IN_3_30
GListModel *dzl_read_only_list_model_new (GListModel *base_model);

G_END_DECLS
