/* dzl-dock-overlay-edge.h
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
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

#if !defined(DAZZLE_INSIDE) && !defined(DAZZLE_COMPILATION)
# error "Only <dzl.h> can be included directly."
#endif

#ifndef DZL_DOCK_OVERLAY_EDGE_H
#define DZL_DOCK_OVERLAY_EDGE_H

#include "dzl-version-macros.h"

#include "dzl-dock-types.h"

G_BEGIN_DECLS

DZL_AVAILABLE_IN_ALL
GtkPositionType dzl_dock_overlay_edge_get_edge     (DzlDockOverlayEdge *self);
DZL_AVAILABLE_IN_ALL
void            dzl_dock_overlay_edge_set_edge     (DzlDockOverlayEdge *self,
                                                    GtkPositionType     edge);
DZL_AVAILABLE_IN_ALL
gint            dzl_dock_overlay_edge_get_position (DzlDockOverlayEdge *self);
DZL_AVAILABLE_IN_ALL
void            dzl_dock_overlay_edge_set_position (DzlDockOverlayEdge *self,
                                                    gint                position);

G_END_DECLS

#endif /* DZL_DOCK_OVERLAY_EDGE_H */
