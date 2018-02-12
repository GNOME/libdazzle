/* dzl-widget-action-group.h
 *
 * Copyright (C) 2015 Christian Hergert <chergert@redhat.com>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DZL_WIDGET_ACTION_GROUP_H
#define DZL_WIDGET_ACTION_GROUP_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_WIDGET_ACTION_GROUP (dzl_widget_action_group_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlWidgetActionGroup, dzl_widget_action_group, DZL, WIDGET_ACTION_GROUP, GObject)

DZL_AVAILABLE_IN_ALL
GActionGroup *dzl_widget_action_group_new                (GtkWidget            *widget);
DZL_AVAILABLE_IN_ALL
void          dzl_widget_action_group_attach             (gpointer              widget,
                                                          const gchar          *group_name);
DZL_AVAILABLE_IN_ALL
void          dzl_widget_action_group_set_action_enabled (DzlWidgetActionGroup *self,
                                                          const gchar          *action_name,
                                                          gboolean              enabled);

G_END_DECLS

#endif /* DZL_WIDGET_ACTION_GROUP_H */
