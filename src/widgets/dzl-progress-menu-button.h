/* dzl-progress-menu-button.h
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

#ifndef DZL_PROGRESS_MENU_BUTTON_H
#define DZL_PROGRESS_MENU_BUTTON_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_PROGRESS_MENU_BUTTON (dzl_progress_menu_button_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlProgressMenuButton, dzl_progress_menu_button, DZL, PROGRESS_MENU_BUTTON, GtkMenuButton)

struct _DzlProgressMenuButtonClass
{
  GtkMenuButtonClass parent_class;

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
};

DZL_AVAILABLE_IN_ALL
GtkWidget *dzl_progress_menu_button_new               (void);
DZL_AVAILABLE_IN_ALL
gdouble    dzl_progress_menu_button_get_progress      (DzlProgressMenuButton *button);
DZL_AVAILABLE_IN_ALL
void       dzl_progress_menu_button_set_progress      (DzlProgressMenuButton *button,
                                                       gdouble                progress);
DZL_AVAILABLE_IN_ALL
gboolean   dzl_progress_menu_button_get_show_theatric (DzlProgressMenuButton *self);
DZL_AVAILABLE_IN_ALL
void       dzl_progress_menu_button_set_show_theatric (DzlProgressMenuButton *self,
                                                       gboolean               show_theatic);
DZL_AVAILABLE_IN_ALL
void       dzl_progress_menu_button_reset_theatrics   (DzlProgressMenuButton *self);
DZL_AVAILABLE_IN_3_32
gboolean   dzl_progress_menu_button_get_show_progress (DzlProgressMenuButton *self);
DZL_AVAILABLE_IN_3_32
void       dzl_progress_menu_button_set_show_progress (DzlProgressMenuButton *self,
                                                       gboolean               show_progress);

G_END_DECLS

#endif /* DZL_PROGRESS_MENU_BUTTON_H */

