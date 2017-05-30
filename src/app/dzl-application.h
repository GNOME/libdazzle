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

#include "menus/dzl-menu-manager.h"
#include "theming/dzl-theme-manager.h"

G_BEGIN_DECLS

#define DZL_APPLICATION_DEFAULT (DZL_APPLICATION (g_application_get_default ()))
#define DZL_TYPE_APPLICATION    (dzl_application_get_type())

G_DECLARE_DERIVABLE_TYPE (DzlApplication, dzl_application, DZL, APPLICATION, GtkApplication)

struct _DzlApplicationClass
{
  GtkApplicationClass parent_class;

  void (*add_resource_path)    (DzlApplication *self,
                                const gchar    *resource_path);
  void (*remove_resource_path) (DzlApplication *self,
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

DzlMenuManager  *dzl_application_get_menu_manager     (DzlApplication *self);
DzlThemeManager *dzl_application_get_theme_manager    (DzlApplication *self);
GMenu           *dzl_application_get_menu_by_id       (DzlApplication *self,
                                                       const gchar    *menu_id);
void             dzl_application_add_resource_path    (DzlApplication *self,
                                                       const gchar    *resource_path);
void             dzl_application_remove_resource_path (DzlApplication *self,
                                                       const gchar    *resource_path);

G_END_DECLS

#endif /* DZL_APPLICATION_H */
