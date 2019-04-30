/* dzl-tab.h
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#if !defined(DAZZLE_INSIDE) && !defined(DAZZLE_COMPILATION)
# error "Only <dzl.h> can be included directly."
#endif

#ifndef DZL_TAB_H
#define DZL_TAB_H

#include "dzl-version-macros.h"

#include "dzl-dock-types.h"

G_BEGIN_DECLS

DZL_AVAILABLE_IN_3_34
void             dzl_tab_set_gicon      (DzlTab          *self,
                                         GIcon           *gicon);
DZL_AVAILABLE_IN_ALL
const gchar     *dzl_tab_get_icon_name  (DzlTab          *self);
DZL_AVAILABLE_IN_ALL
void             dzl_tab_set_icon_name  (DzlTab          *self,
                                         const gchar     *icon_name);
DZL_AVAILABLE_IN_ALL
const gchar     *dzl_tab_get_title      (DzlTab          *self);
DZL_AVAILABLE_IN_ALL
void             dzl_tab_set_title      (DzlTab          *self,
                                         const gchar     *title);
DZL_AVAILABLE_IN_ALL
GtkPositionType  dzl_tab_get_edge       (DzlTab          *self);
DZL_AVAILABLE_IN_ALL
void             dzl_tab_set_edge       (DzlTab          *self,
                                         GtkPositionType  edge);
DZL_AVAILABLE_IN_ALL
GtkWidget       *dzl_tab_get_widget     (DzlTab          *self);
DZL_AVAILABLE_IN_ALL
void             dzl_tab_set_widget     (DzlTab          *self,
                                         GtkWidget       *widget);
DZL_AVAILABLE_IN_ALL
gboolean         dzl_tab_get_active     (DzlTab          *self);
DZL_AVAILABLE_IN_ALL
void             dzl_tab_set_active     (DzlTab          *self,
                                         gboolean         active);
DZL_AVAILABLE_IN_ALL
gboolean         dzl_tab_get_can_close  (DzlTab          *self);
DZL_AVAILABLE_IN_ALL
void             dzl_tab_set_can_close  (DzlTab          *self,
                                         gboolean         can_close);
DZL_AVAILABLE_IN_ALL
DzlTabStyle      dzl_tab_get_style      (DzlTab          *self);
DZL_AVAILABLE_IN_ALL
void             dzl_tab_set_style      (DzlTab          *self,
                                         DzlTabStyle      style);

G_END_DECLS

#endif /* DZL_TAB_H */
