/* dzl-column-layout.h
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

#ifndef DZL_COLUMN_LAYOUT_H
#define DZL_COLUMN_LAYOUT_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_COLUMN_LAYOUT (dzl_column_layout_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlColumnLayout, dzl_column_layout, DZL, COLUMN_LAYOUT, GtkContainer)

struct _DzlColumnLayoutClass
{
  GtkContainerClass parent;
};

DZL_AVAILABLE_IN_ALL
GtkWidget *dzl_column_layout_new                (void);
DZL_AVAILABLE_IN_ALL
guint      dzl_column_layout_get_max_columns    (DzlColumnLayout *self);
DZL_AVAILABLE_IN_ALL
void       dzl_column_layout_set_max_columns    (DzlColumnLayout *self,
                                                 guint            max_columns);
DZL_AVAILABLE_IN_ALL
gint       dzl_column_layout_get_column_width   (DzlColumnLayout *self);
DZL_AVAILABLE_IN_ALL
void       dzl_column_layout_set_column_width   (DzlColumnLayout *self,
                                                 gint             column_width);
DZL_AVAILABLE_IN_ALL
gint       dzl_column_layout_get_column_spacing (DzlColumnLayout *self);
DZL_AVAILABLE_IN_ALL
void       dzl_column_layout_set_column_spacing (DzlColumnLayout *self,
                                                 gint             column_spacing);
DZL_AVAILABLE_IN_ALL
gint       dzl_column_layout_get_row_spacing    (DzlColumnLayout *self);
DZL_AVAILABLE_IN_ALL
void       dzl_column_layout_set_row_spacing    (DzlColumnLayout *self,
                                                 gint             row_spacing);

G_END_DECLS

#endif /* DZL_COLUMN_LAYOUT_H */
