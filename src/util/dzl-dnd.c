/* dzl-dnd.c
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

#define G_LOG_DOMAIN "dzl-dnd"

#include "config.h"

#include "dzl-dnd.h"

/**
 * SECTION:dzl-dnd
 * @title: Drag-and-Drop Utilities
 * @short_description: Helper functions to use with GTK's drag-and-drop system
 */

/**
 * dzl_dnd_get_uri_list:
 * @selection_data: the #GtkSelectionData from drag_data_received
 *
 * Create a list of valid uri's from a uri-list drop.
 *
 * Returns: (transfer full): a string array which will hold the uris or
 *   %NULL if there were no valid uris. g_strfreev should be used when
 *   the string array is no longer used
 *
 * Deprecated: Use gtk_selection_data_get_uris() instead; it is exactly the same.
 */
gchar **
dzl_dnd_get_uri_list (GtkSelectionData *selection_data)
{
  return gtk_selection_data_get_uris (selection_data);
}
