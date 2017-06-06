/* dzl-preferences-view.h
 *
 * Copyright (C) 2015 Christian Hergert <chergert@redhat.com>
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

#ifndef DZL_PREFERENCES_VIEW_H
#define DZL_PREFERENCES_VIEW_H

#include <gtk/gtk.h>

#include "prefs/dzl-preferences.h"

G_BEGIN_DECLS

#define DZL_TYPE_PREFERENCES_VIEW (dzl_preferences_view_get_type())

G_DECLARE_FINAL_TYPE (DzlPreferencesView, dzl_preferences_view, DZL, PREFERENCES_VIEW, GtkBin)

GtkWidget *dzl_preferences_view_new (void);

G_END_DECLS

#endif /* DZL_PREFERENCES_VIEW_H */
