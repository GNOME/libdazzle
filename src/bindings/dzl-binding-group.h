/* dzl-binding-group.h
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
 * Copyright (C) 2015 Garrett Regier <garrettregier@gmail.com>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DZL_BINDING_GROUP_H
#define DZL_BINDING_GROUP_H

#include <gio/gio.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_BINDING_GROUP (dzl_binding_group_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlBindingGroup, dzl_binding_group, DZL, BINDING_GROUP, GObject)

DZL_AVAILABLE_IN_ALL
DzlBindingGroup *dzl_binding_group_new                (void);
DZL_AVAILABLE_IN_ALL
GObject         *dzl_binding_group_get_source         (DzlBindingGroup       *self);
DZL_AVAILABLE_IN_ALL
void             dzl_binding_group_set_source         (DzlBindingGroup       *self,
                                                       gpointer               source);
DZL_AVAILABLE_IN_ALL
void             dzl_binding_group_bind               (DzlBindingGroup       *self,
                                                       const gchar           *source_property,
                                                       gpointer               target,
                                                       const gchar           *target_property,
                                                       GBindingFlags          flags);
DZL_AVAILABLE_IN_ALL
void             dzl_binding_group_bind_full          (DzlBindingGroup       *self,
                                                       const gchar           *source_property,
                                                       gpointer               target,
                                                       const gchar           *target_property,
                                                       GBindingFlags          flags,
                                                       GBindingTransformFunc  transform_to,
                                                       GBindingTransformFunc  transform_from,
                                                       gpointer               user_data,
                                                       GDestroyNotify         user_data_destroy);
DZL_AVAILABLE_IN_ALL
void             dzl_binding_group_bind_with_closures (DzlBindingGroup       *self,
                                                       const gchar           *source_property,
                                                       gpointer               target,
                                                       const gchar           *target_property,
                                                       GBindingFlags          flags,
                                                       GClosure              *transform_to,
                                                       GClosure              *transform_from);

G_END_DECLS

#endif /* DZL_BINDING_GROUP_H */
