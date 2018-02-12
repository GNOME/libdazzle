/* dzl-properties-group.h
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
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

#ifndef DZL_PROPERTIES_GROUP_H
#define DZL_PROPERTIES_GROUP_H

#include <gio/gio.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_PROPERTIES_GROUP (dzl_properties_group_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlPropertiesGroup, dzl_properties_group, DZL, PROPERTIES_GROUP, GObject)

typedef enum
{
  DZL_PROPERTIES_FLAGS_NONE,
  DZL_PROPERTIES_FLAGS_STATEFUL_BOOLEANS = 1 << 0,
} DzlPropertiesFlags;

DZL_AVAILABLE_IN_ALL
DzlPropertiesGroup *dzl_properties_group_new                (GObject            *object);
DZL_AVAILABLE_IN_ALL
DzlPropertiesGroup *dzl_properties_group_new_for_type       (GType               object_type);
DZL_AVAILABLE_IN_ALL
void                dzl_properties_group_add_property       (DzlPropertiesGroup *self,
                                                             const gchar        *name,
                                                             const gchar        *property_name);
DZL_AVAILABLE_IN_ALL
void                dzl_properties_group_add_property_full  (DzlPropertiesGroup *self,
                                                             const gchar        *name,
                                                             const gchar        *property_name,
                                                             DzlPropertiesFlags  flags);
DZL_AVAILABLE_IN_ALL
void                dzl_properties_group_add_all_properties (DzlPropertiesGroup *self);
DZL_AVAILABLE_IN_ALL
void                dzl_properties_group_remove             (DzlPropertiesGroup *self,
                                                             const gchar        *name);

G_END_DECLS

#endif /* DZL_PROPERTIES_GROUP_H */
