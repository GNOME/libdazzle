/* dzl-joined-menu.h
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

#ifndef DZL_JOINED_MENU_H
#define DZL_JOINED_MENU_H

#include <gio/gio.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_JOINED_MENU (dzl_joined_menu_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlJoinedMenu, dzl_joined_menu, DZL, JOINED_MENU, GMenuModel)

DZL_AVAILABLE_IN_ALL
DzlJoinedMenu *dzl_joined_menu_new          (void);
DZL_AVAILABLE_IN_ALL
guint          dzl_joined_menu_get_n_joined (DzlJoinedMenu *self);
DZL_AVAILABLE_IN_ALL
void           dzl_joined_menu_append_menu  (DzlJoinedMenu *self,
                                             GMenuModel    *model);
DZL_AVAILABLE_IN_ALL
void           dzl_joined_menu_prepend_menu (DzlJoinedMenu *self,
                                             GMenuModel    *model);
DZL_AVAILABLE_IN_ALL
void           dzl_joined_menu_remove_menu  (DzlJoinedMenu *self,
                                             GMenuModel    *model);
DZL_AVAILABLE_IN_ALL
void           dzl_joined_menu_remove_index (DzlJoinedMenu *self,
                                             guint          index);

G_END_DECLS

#endif /* DZL_JOINED_MENU_H */
