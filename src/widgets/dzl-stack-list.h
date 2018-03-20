/* dzl-stack-list.h
 *
 * Copyright (C) 2015-2017 Christian Hergert <christian@hergert.me>
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

#ifndef DZL_STACK_LIST_H
#define DZL_STACK_LIST_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_STACK_LIST (dzl_stack_list_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlStackList, dzl_stack_list, DZL, STACK_LIST, GtkBin)

struct _DzlStackListClass
{
  GtkBinClass parent_instance;

  void (*row_activated)    (DzlStackList  *self,
                            GtkListBoxRow *row);
  void (*header_activated) (DzlStackList  *self,
                            GtkListBoxRow *row);

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
  gpointer _reserved5;
  gpointer _reserved6;
  gpointer _reserved7;
  gpointer _reserved8;
};

/**
 * DzlStackListCreateWidgetFunc:
 * @item: (type GObject): the item from the model for which to create a widget for
 * @user_data: (closure): user data from dzl_stack_list_push()
 *
 * Called for stack lists that are bound to a #GListModel with
 * dzl_stack_list_push() for each item that gets added to the model.
 *
 * Returns: (transfer full): a #GtkWidget that represents @item
 */
typedef GtkWidget *(*DzlStackListCreateWidgetFunc) (gpointer item,
                                                    gpointer user_data);

DZL_AVAILABLE_IN_ALL
GtkWidget *dzl_stack_list_new        (void);
DZL_AVAILABLE_IN_ALL
void       dzl_stack_list_push       (DzlStackList                  *self,
                                      GtkWidget                     *header,
                                      GListModel                    *model,
                                      DzlStackListCreateWidgetFunc   create_widget_func,
                                      gpointer                       user_data,
                                      GDestroyNotify                 user_data_free_func);
DZL_AVAILABLE_IN_ALL
void        dzl_stack_list_pop       (DzlStackList                  *self);
DZL_AVAILABLE_IN_ALL
GListModel *dzl_stack_list_get_model (DzlStackList                  *self);
DZL_AVAILABLE_IN_ALL
guint       dzl_stack_list_get_depth (DzlStackList                  *self);
DZL_AVAILABLE_IN_ALL
void        dzl_stack_list_clear     (DzlStackList                  *self);

G_END_DECLS

#endif /* DZL_STACK_LIST_H */
