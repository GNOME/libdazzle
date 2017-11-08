/* dzl-tab-strip.h
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

#ifndef DZL_TAB_STRIP_H
#define DZL_TAB_STRIP_H

#include "dzl-version-macros.h"

#include "dzl-dock-types.h"

G_BEGIN_DECLS

struct _DzlTabStripClass
{
  GtkBoxClass parent;

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
GtkWidget       *dzl_tab_strip_new             (void);
DZL_AVAILABLE_IN_ALL
GtkStack        *dzl_tab_strip_get_stack       (DzlTabStrip     *self);
DZL_AVAILABLE_IN_ALL
void             dzl_tab_strip_set_stack       (DzlTabStrip     *self,
                                                GtkStack        *stack);
DZL_AVAILABLE_IN_ALL
GtkPositionType  dzl_tab_strip_get_edge        (DzlTabStrip     *self);
DZL_AVAILABLE_IN_ALL
void             dzl_tab_strip_set_edge        (DzlTabStrip     *self,
                                                GtkPositionType  edge);
DZL_AVAILABLE_IN_ALL
DzlTabStyle      dzl_tab_strip_get_style       (DzlTabStrip     *self);
DZL_AVAILABLE_IN_ALL
void             dzl_tab_strip_set_style       (DzlTabStrip     *self,
                                                DzlTabStyle      style);
DZL_AVAILABLE_IN_ALL
void             dzl_tab_strip_add_control     (DzlTabStrip     *self,
                                                GtkWidget       *widget);

G_END_DECLS

#endif /* DZL_TAB_STRIP_H */
