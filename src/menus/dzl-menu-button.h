/* dzl-menu-button.h
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
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

#ifndef DZL_MENU_BUTTON_H
#define DZL_MENU_BUTTON_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define DZL_TYPE_MENU_BUTTON (dzl_menu_button_get_type())

G_DECLARE_DERIVABLE_TYPE (DzlMenuButton, dzl_menu_button, DZL, MENU_BUTTON, GtkMenuButton)

struct _DzlMenuButtonClass
{
  GtkMenuButtonClass parent_class;
};

GtkWidget     *dzl_menu_button_new_with_model  (const gchar   *icon_name,
                                                GMenuModel    *model);
GMenuModel    *dzl_menu_button_get_model       (DzlMenuButton *self);
void           dzl_menu_button_set_model       (DzlMenuButton *self,
                                                GMenuModel    *model);
gboolean       dzl_menu_button_get_show_arrow  (DzlMenuButton *self);
void           dzl_menu_button_set_show_arrow  (DzlMenuButton *self,
                                                gboolean       show_arrow);
gboolean       dzl_menu_button_get_show_icons  (DzlMenuButton *self);
void           dzl_menu_button_set_show_icons  (DzlMenuButton *self,
                                                gboolean       show_icons);
gboolean       dzl_menu_button_get_show_accels (DzlMenuButton *self);
void           dzl_menu_button_set_show_accels (DzlMenuButton *self,
                                                gboolean       show_accels);

G_END_DECLS

#endif /* DZL_MENU_BUTTON_H */
