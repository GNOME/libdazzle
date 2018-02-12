/* dzl-path-element.h
 *
 * Copyright (C) 2016-2017 Christian Hergert <chergert@redhat.com>
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

#ifndef DZL_PATH_ELEMENT_H
#define DZL_PATH_ELEMENT_H

#include <glib-object.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_PATH_ELEMENT (dzl_path_element_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlPathElement, dzl_path_element, DZL, PATH_ELEMENT, GObject)

DZL_AVAILABLE_IN_ALL
DzlPathElement *dzl_path_element_new           (const gchar    *id,
                                                const gchar    *icon_name,
                                                const gchar    *title);
DZL_AVAILABLE_IN_ALL
const gchar    *dzl_path_element_get_title     (DzlPathElement *self);
DZL_AVAILABLE_IN_ALL
const gchar    *dzl_path_element_get_id        (DzlPathElement *self);
DZL_AVAILABLE_IN_ALL
const gchar    *dzl_path_element_get_icon_name (DzlPathElement *self);

G_END_DECLS

#endif /* DZL_PATH_ELEMENT_H */

