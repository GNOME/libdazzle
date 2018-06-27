/* dzl-empty-state.h
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
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

#ifndef DZL_EMPTY_STATE_H
#define DZL_EMPTY_STATE_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_EMPTY_STATE (dzl_empty_state_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlEmptyState, dzl_empty_state, DZL, EMPTY_STATE, GtkBin)

struct _DzlEmptyStateClass
{
  GtkBinClass parent_class;
};

DZL_AVAILABLE_IN_3_30
GtkWidget    *dzl_empty_state_new          (void);
DZL_AVAILABLE_IN_ALL
const gchar  *dzl_empty_state_get_icon_name (DzlEmptyState *self);
DZL_AVAILABLE_IN_ALL
void          dzl_empty_state_set_icon_name (DzlEmptyState *self,
                                             const gchar   *icon_name);
DZL_AVAILABLE_IN_ALL
void          dzl_empty_state_set_resource  (DzlEmptyState *self,
                                             const gchar   *resource);
DZL_AVAILABLE_IN_ALL
const gchar  *dzl_empty_state_get_title     (DzlEmptyState *self);
DZL_AVAILABLE_IN_ALL
void          dzl_empty_state_set_title     (DzlEmptyState *self,
                                             const gchar   *title);
DZL_AVAILABLE_IN_ALL
const gchar  *dzl_empty_state_get_subtitle  (DzlEmptyState *self);
DZL_AVAILABLE_IN_ALL
void          dzl_empty_state_set_subtitle  (DzlEmptyState *self,
                                             const gchar   *title);

G_END_DECLS

#endif /* DZL_EMPTY_STATE_H */
