/* dzl-preferences-flow-box.h
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

#ifndef DZL_PREFERENCES_FLOW_BOX_H
#define DZL_PREFERENCES_FLOW_BOX_H

#include "dzl-version-macros.h"

#include "widgets/dzl-column-layout.h"

G_BEGIN_DECLS

#define DZL_TYPE_PREFERENCES_FLOW_BOX (dzl_preferences_flow_box_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlPreferencesFlowBox, dzl_preferences_flow_box, DZL, PREFERENCES_FLOW_BOX, DzlColumnLayout)

DZL_AVAILABLE_IN_3_30
GtkWidget *dzl_preferences_flow_box_new (void);

G_END_DECLS

#endif /* DZL_PREFERENCES_FLOW_BOX_H */
