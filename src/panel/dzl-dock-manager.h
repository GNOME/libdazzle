/* dzl-dock-manager.h
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

#ifndef DZL_DOCK_MANAGER_H
#define DZL_DOCK_MANAGER_H

#include "dzl-version-macros.h"

#include "dzl-dock-types.h"

G_BEGIN_DECLS

struct _DzlDockManagerClass
{
  GObjectClass parent;

  void (*register_dock)   (DzlDockManager *self,
                           DzlDock        *dock);
  void (*unregister_dock) (DzlDockManager *self,
                           DzlDock        *dock);

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
DzlDockManager *dzl_dock_manager_new                    (void);
DZL_AVAILABLE_IN_ALL
void            dzl_dock_manager_register_dock          (DzlDockManager *self,
                                                         DzlDock        *dock);
DZL_AVAILABLE_IN_ALL
void            dzl_dock_manager_unregister_dock        (DzlDockManager *self,
                                                         DzlDock        *dock);
DZL_AVAILABLE_IN_ALL
void            dzl_dock_manager_pause_grabs            (DzlDockManager *self);
DZL_AVAILABLE_IN_ALL
void            dzl_dock_manager_unpause_grabs          (DzlDockManager *self);
DZL_AVAILABLE_IN_3_30
void            dzl_dock_manager_release_transient_grab (DzlDockManager *self);

G_END_DECLS

#endif /* DZL_DOCK_MANAGER_H */
