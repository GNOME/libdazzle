/* dzl-dock-overlay.h
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

#ifndef DZL_DOCK_OVERLAY_H
#define DZL_DOCK_OVERLAY_H

#include "dzl-version-macros.h"

#include "dzl-dock.h"
#include "dzl-dock-overlay-edge.h"

G_BEGIN_DECLS

struct _DzlDockOverlayClass
{
  GtkEventBoxClass parent;

  void (*hide_edges) (DzlDockOverlay *self);

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
  gpointer _reserved5;
  gpointer _reserved6;
  gpointer _reserved7;
  gpointer _reserved8;
};

DZL_AVAILABLE_IN_ALL
GtkWidget              *dzl_dock_overlay_new                   (void);
DZL_AVAILABLE_IN_ALL
void                    dzl_overlay_add_child                  (DzlDockOverlay  *self,
                                                                GtkWidget       *child,
                                                                const gchar     *type);
DZL_AVAILABLE_IN_ALL
DzlDockOverlayEdge     *dzl_dock_overlay_get_edge              (DzlDockOverlay  *self,
                                                                GtkPositionType  position);
DZL_AVAILABLE_IN_ALL
GtkAdjustment          *dzl_dock_overlay_get_edge_adjustment   (DzlDockOverlay  *self,
                                                                GtkPositionType  position);
G_END_DECLS

#endif /* DZL_DOCK_OVERLAY_H */
