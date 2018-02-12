/* dzl-dock-revealer.h
 *
 * Copyright (C) 2016 Christian Hergert <christian@hergert.me>
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

#ifndef DZL_DOCK_REVEALER_H
#define DZL_DOCK_REVEALER_H

#include "dzl-version-macros.h"

#include "widgets/dzl-bin.h"

G_BEGIN_DECLS

#define DZL_TYPE_DOCK_REVEALER_TRANSITION_TYPE (dzl_dock_revealer_transition_type_get_type())
#define DZL_TYPE_DOCK_REVEALER                 (dzl_dock_revealer_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlDockRevealer, dzl_dock_revealer, DZL, DOCK_REVEALER, DzlBin)

typedef enum
{
  DZL_DOCK_REVEALER_TRANSITION_TYPE_NONE,
  DZL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT,
  DZL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_LEFT,
  DZL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_UP,
  DZL_DOCK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN,
} DzlDockRevealerTransitionType;

struct _DzlDockRevealerClass
{
  DzlBinClass parent;

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
GType                          dzl_dock_revealer_transition_type_get_type (void);
DZL_AVAILABLE_IN_ALL
GtkWidget                     *dzl_dock_revealer_new                      (void);
DZL_AVAILABLE_IN_ALL
void                           dzl_dock_revealer_animate_to_position      (DzlDockRevealer               *self,
                                                                           gint                           position,
                                                                           guint                          transition_duration);
DZL_AVAILABLE_IN_ALL
DzlDockRevealerTransitionType  dzl_dock_revealer_get_transition_type      (DzlDockRevealer *self);
DZL_AVAILABLE_IN_ALL
void                           dzl_dock_revealer_set_transition_type      (DzlDockRevealer               *self,
                                                                           DzlDockRevealerTransitionType  transition_type);
DZL_AVAILABLE_IN_ALL
gboolean                       dzl_dock_revealer_get_child_revealed       (DzlDockRevealer               *self);
DZL_AVAILABLE_IN_ALL
void                           dzl_dock_revealer_set_reveal_child         (DzlDockRevealer               *self,
                                                                           gboolean                       reveal_child);
DZL_AVAILABLE_IN_ALL
gboolean                       dzl_dock_revealer_get_reveal_child         (DzlDockRevealer               *self);
DZL_AVAILABLE_IN_ALL
gint                           dzl_dock_revealer_get_position             (DzlDockRevealer               *self);
DZL_AVAILABLE_IN_ALL
void                           dzl_dock_revealer_set_position             (DzlDockRevealer               *self,
                                                                           gint                           position);
DZL_AVAILABLE_IN_ALL
gboolean                       dzl_dock_revealer_get_position_set         (DzlDockRevealer               *self);
DZL_AVAILABLE_IN_ALL
void                           dzl_dock_revealer_set_position_set         (DzlDockRevealer               *self,
                                                                           gboolean                       position_set);
DZL_AVAILABLE_IN_ALL
guint                          dzl_dock_revealer_get_transition_duration  (DzlDockRevealer               *self);
DZL_AVAILABLE_IN_ALL
void                           dzl_dock_revealer_set_transition_duration  (DzlDockRevealer               *self,
                                                                           guint                          transition_duration);
DZL_AVAILABLE_IN_ALL
gboolean                       dzl_dock_revealer_is_animating             (DzlDockRevealer               *self);

G_END_DECLS

#endif /* DZL_DOCK_REVEALER_H */
