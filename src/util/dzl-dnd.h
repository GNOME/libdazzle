/* dzl-dnd.h
 *
 * Copyright (C) 2015 Dimitris Zenios <dimitris.zenios@gmail.com>
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

#ifndef DZL_DND_H
#define DZL_DND_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

DZL_DEPRECATED_FOR (gtk_selection_data_get_uris)
gchar **dzl_dnd_get_uri_list (GtkSelectionData *selection_data);

G_END_DECLS

#endif /* GB_RGBA_H */
