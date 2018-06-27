/* dzl-list-box.h
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

#ifndef DZL_LIST_BOX_H
#define DZL_LIST_BOX_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_LIST_BOX (dzl_list_box_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlListBox, dzl_list_box, DZL, LIST_BOX, GtkListBox)

struct _DzlListBoxClass
{
  GtkListBoxClass parent_class;

  /*< private >*/
  gpointer _reserved[4];
};

DZL_AVAILABLE_IN_ALL
GtkWidget   *dzl_list_box_new               (GType        row_type,
                                             const gchar *property_name);
DZL_AVAILABLE_IN_ALL
GType        dzl_list_box_get_row_type      (DzlListBox  *self);
DZL_AVAILABLE_IN_ALL
const gchar *dzl_list_box_get_property_name (DzlListBox  *self);
DZL_AVAILABLE_IN_ALL
GListModel  *dzl_list_box_get_model         (DzlListBox  *self);
DZL_AVAILABLE_IN_ALL
void         dzl_list_box_set_model         (DzlListBox  *self,
                                             GListModel  *model);
DZL_AVAILABLE_IN_3_28
void         dzl_list_box_set_recycle_max   (DzlListBox  *self,
                                             guint        recycle_max);

G_END_DECLS

#endif /* DZL_LIST_BOX_H */
