/* dzl-application.h
 *
 * Copyright (C) 2014-2017 Christian Hergert <christian@hergert.me>
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

#if !defined (DAZZLE_INSIDE) && !defined (DAZZLE_COMPILATION)
# error "Only <dazzle.h> can be included directly."
#endif

#ifndef DZL_APPLICATION_H
#define DZL_APPLICATION_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

#include "menus/dzl-menu-manager.h"
#include "shortcuts/dzl-shortcut-manager.h"
#include "theming/dzl-theme-manager.h"

G_BEGIN_DECLS

#define DZL_APPLICATION_DEFAULT (DZL_APPLICATION (g_application_get_default ()))
#define DZL_TYPE_APPLICATION    (dzl_application_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlApplication, dzl_application, DZL, APPLICATION, GtkApplication)

struct _DzlApplicationClass
{
  GtkApplicationClass parent_class;

  void (*add_resources)    (DzlApplication *self,
                            const gchar    *resource_path);
  void (*remove_resources) (DzlApplication *self,
                            const gchar    *resource_path);

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
  gpointer _reserved5;
  gpointer _reserved6;
  gpointer _reserved7;
  gpointer _reserved8;
};

DZL_AVAILABLE_IN_3_34
DzlApplication     *dzl_application_new                  (const gchar       *application_id,
                                                          GApplicationFlags  flags);
DZL_AVAILABLE_IN_ALL
DzlMenuManager     *dzl_application_get_menu_manager     (DzlApplication    *self);
DZL_AVAILABLE_IN_ALL
DzlShortcutManager *dzl_application_get_shortcut_manager (DzlApplication    *self);
DZL_AVAILABLE_IN_ALL
DzlThemeManager    *dzl_application_get_theme_manager    (DzlApplication    *self);
DZL_AVAILABLE_IN_ALL
GMenu              *dzl_application_get_menu_by_id       (DzlApplication    *self,
                                                          const gchar       *menu_id);
DZL_AVAILABLE_IN_ALL
void                dzl_application_add_resources        (DzlApplication    *self,
                                                          const gchar       *resource_path);
DZL_AVAILABLE_IN_ALL
void                dzl_application_remove_resources     (DzlApplication    *self,
                                                          const gchar       *resource_path);

G_END_DECLS

#endif /* DZL_APPLICATION_H */
