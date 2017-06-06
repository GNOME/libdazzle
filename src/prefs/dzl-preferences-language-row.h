/* dzl-preferences-language-row.h
 *
 * Copyright (C) 2015-2017 Christian Hergert <christian@hergert.me>
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

#ifndef DZL_PREFERENCES_LANGUAGE_ROW_H
#define DZL_PREFERENCES_LANGUAGE_ROW_H

#include "prefs/dzl-preferences-bin.h"

G_BEGIN_DECLS

#define DZL_TYPE_PREFERENCES_LANGUAGE_ROW (dzl_preferences_language_row_get_type())

G_DECLARE_FINAL_TYPE (DzlPreferencesLanguageRow, dzl_preferences_language_row, DZL, PREFERENCES_LANGUAGE_ROW, DzlPreferencesBin)

G_END_DECLS

#endif /* DZL_PREFERENCES_LANGUAGE_ROW_H */
