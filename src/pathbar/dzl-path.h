/* dzl-path.h
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

#ifndef DZL_PATH_H
#define DZL_PATH_H

#include <glib-object.h>

#include "dzl-path-element.h"

G_BEGIN_DECLS

#define DZL_TYPE_PATH (dzl_path_get_type())

G_DECLARE_FINAL_TYPE (DzlPath, dzl_path, DZL, PATH, GObject)

DzlPath        *dzl_path_new          (void);
void            dzl_path_prepend      (DzlPath        *self,
                                       DzlPathElement *element);
void            dzl_path_append       (DzlPath        *self,
                                       DzlPathElement *element);
GList          *dzl_path_get_elements (DzlPath        *self);
gboolean        dzl_path_has_prefix   (DzlPath        *self,
                                       DzlPath        *prefix);
guint           dzl_path_get_length   (DzlPath        *self);
DzlPathElement *dzl_path_get_element  (DzlPath        *self,
                                       guint           index);
gchar          *dzl_path_printf       (DzlPath        *self);
gboolean        dzl_path_is_empty     (DzlPath        *self);

G_END_DECLS

#endif /* DZL_PATH_H */
