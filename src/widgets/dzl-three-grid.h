/* dzl-three-grid.h
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

#ifndef DZL_THREE_GRID_H
#define DZL_THREE_GRID_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_THREE_GRID        (dzl_three_grid_get_type())
#define DZL_TYPE_THREE_GRID_COLUMN (dzl_three_grid_column_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlThreeGrid, dzl_three_grid, DZL, THREE_GRID, GtkContainer)

struct _DzlThreeGridClass
{
  GtkContainerClass parent_class;

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
  gpointer _reserved5;
  gpointer _reserved6;
  gpointer _reserved7;
  gpointer _reserved8;
};

typedef enum
{
  DZL_THREE_GRID_COLUMN_LEFT,
  DZL_THREE_GRID_COLUMN_CENTER,
  DZL_THREE_GRID_COLUMN_RIGHT
} DzlThreeGridColumn;

DZL_AVAILABLE_IN_ALL
GType      dzl_three_grid_column_get_type (void);
DZL_AVAILABLE_IN_ALL
GtkWidget *dzl_three_grid_new             (void);

G_END_DECLS

#endif /* DZL_THREE_GRID_H */
