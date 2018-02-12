/* dzl-preferences-entry.h
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

#ifndef DZL_PREFERENCES_ENTRY_H
#define DZL_PREFERENCES_ENTRY_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

#include "prefs/dzl-preferences-bin.h"

G_BEGIN_DECLS

#define DZL_TYPE_PREFERENCES_ENTRY (dzl_preferences_entry_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlPreferencesEntry, dzl_preferences_entry, DZL, PREFERENCES_ENTRY, DzlPreferencesBin)

struct _DzlPreferencesEntryClass
{
  DzlPreferencesBinClass parent_class;
};

DZL_AVAILABLE_IN_ALL
GtkWidget *dzl_preferences_entry_get_entry_widget (DzlPreferencesEntry *self);
DZL_AVAILABLE_IN_ALL
GtkWidget *dzl_preferences_entry_get_title_widget (DzlPreferencesEntry *self);

G_END_DECLS

#endif /* DZL_PREFERENCES_ENTRY_H */
