/* dzl-preferences-bin-private.h
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

#ifndef DZL_PREFERENCES_BIN_PRIVATE_H
#define DZL_PREFERENCES_BIN_PRIVATE_H

#include "prefs/dzl-preferences-bin.h"

G_BEGIN_DECLS

void     _dzl_preferences_bin_set_map (DzlPreferencesBin *self,
                                       GHashTable        *map);
gboolean _dzl_preferences_bin_matches (DzlPreferencesBin *self,
                                       DzlPatternSpec    *spec);

G_END_DECLS

#endif /* DZL_PREFERENCES_BIN_PRIVATE_H */
