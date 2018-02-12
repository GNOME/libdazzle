/* dzl-preferences-page.h
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

#ifndef DZL_PREFERENCES_PAGE_H
#define DZL_PREFERENCES_PAGE_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

#include "prefs/dzl-preferences-group.h"
#include "search/dzl-pattern-spec.h"

G_BEGIN_DECLS

#define DZL_TYPE_PREFERENCES_PAGE (dzl_preferences_page_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlPreferencesPage, dzl_preferences_page, DZL, PREFERENCES_PAGE, GtkBin)

DZL_AVAILABLE_IN_ALL
void                 dzl_preferences_page_add_group (DzlPreferencesPage  *self,
                                                     DzlPreferencesGroup *group);
DZL_AVAILABLE_IN_ALL
DzlPreferencesGroup *dzl_preferences_page_get_group (DzlPreferencesPage  *self,
                                                     const gchar         *group_name);
DZL_AVAILABLE_IN_ALL
void                 dzl_preferences_page_refilter  (DzlPreferencesPage  *self,
                                                     DzlPatternSpec      *spec);
DZL_AVAILABLE_IN_ALL
void                 dzl_preferences_page_set_map   (DzlPreferencesPage  *self,
                                                     GHashTable          *map);

G_END_DECLS

#endif /* DZL_PREFERENCES_PAGE_H */
