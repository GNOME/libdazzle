/* dzl-list-box-private.h
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software: you *privatean redistribute it and/or modify
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

#ifndef DZL_LIST_BOX_PRIVATE_H
#define DZL_LIST_BOX_PRIVATE_H

#include "dzl-list-box.h"
#include "dzl-list-box-row.h"

G_BEGIN_DECLS

typedef void (*DzlListBoxAttachFunc) (DzlListBox    *list_box,
                                      DzlListBoxRow *row,
                                      gpointer       user_data);

gboolean _dzl_list_box_cache           (DzlListBox           *self,
                                        DzlListBoxRow        *row);
void     _dzl_list_box_forall          (DzlListBox           *self,
                                        GtkCallback           callback,
                                        gpointer              user_data);
void     _dzl_list_box_set_attach_func (DzlListBox           *self,
                                        DzlListBoxAttachFunc  func,
                                        gpointer              user_data);

G_END_DECLS

#endif /* DZL_LIST_BOX_PRIVATE_H */
