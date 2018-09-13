/* dzl-preferences-group.h
 *
 * Copyright (C) 2015-2017 Christian Hergert <chergert@redhat.com>
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

#ifndef DZL_PREFERENCES_GROUP_H
#define DZL_PREFERENCES_GROUP_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

#include "search/dzl-pattern-spec.h"

G_BEGIN_DECLS

#define DZL_TYPE_PREFERENCES_GROUP (dzl_preferences_group_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlPreferencesGroup, dzl_preferences_group, DZL, PREFERENCES_GROUP, GtkBin)

DZL_AVAILABLE_IN_ALL
void         dzl_preferences_group_add            (DzlPreferencesGroup *self,
                                                   GtkWidget           *widget);
DZL_AVAILABLE_IN_ALL
const gchar  *dzl_preferences_group_get_title      (DzlPreferencesGroup *self);
DZL_AVAILABLE_IN_ALL
gint          dzl_preferences_group_get_priority   (DzlPreferencesGroup *self);
DZL_AVAILABLE_IN_ALL
void          dzl_preferences_group_set_map        (DzlPreferencesGroup *self,
                                                    GHashTable          *map);
DZL_AVAILABLE_IN_ALL
guint         dzl_preferences_group_refilter       (DzlPreferencesGroup *self,
                                                    DzlPatternSpec      *spec);
DZL_AVAILABLE_IN_3_32
GtkSizeGroup *dzl_preferences_group_get_size_group (DzlPreferencesGroup *self,
                                                    guint                column);


G_END_DECLS

#endif /* DZL_PREFERENCES_GROUP_H */
