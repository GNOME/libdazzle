/* dzl-application-window.h
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

#ifndef DZL_APPLICATION_WINDOW_H
#define DZL_APPLICATION_WINDOW_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_APPLICATION_WINDOW (dzl_application_window_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlApplicationWindow, dzl_application_window, DZL, APPLICATION_WINDOW, GtkApplicationWindow)

typedef enum
{
  DZL_TITLEBAR_ANIMATION_HIDDEN  = 0,
  DZL_TITLEBAR_ANIMATION_SHOWING = 1,
  DZL_TITLEBAR_ANIMATION_SHOWN   = 2,
  DZL_TITLEBAR_ANIMATION_HIDING  = 3,
} DzlTitlebarAnimation;

struct _DzlApplicationWindowClass
{
  GtkApplicationWindowClass parent_class;

  gboolean (*get_fullscreen) (DzlApplicationWindow *self);
  void     (*set_fullscreen) (DzlApplicationWindow *self,
                              gboolean              fullscreen);

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
  gpointer _reserved5;
  gpointer _reserved6;
  gpointer _reserved7;
  gpointer _reserved8;
};

DZL_AVAILABLE_IN_ALL
gboolean              dzl_application_window_get_fullscreen         (DzlApplicationWindow *self);
DZL_AVAILABLE_IN_ALL
void                  dzl_application_window_set_fullscreen         (DzlApplicationWindow *self,
                                                                     gboolean              fullscreen);
DZL_AVAILABLE_IN_ALL
GtkWidget            *dzl_application_window_get_titlebar           (DzlApplicationWindow *self);
DZL_AVAILABLE_IN_ALL
void                  dzl_application_window_set_titlebar           (DzlApplicationWindow *self,
                                                                     GtkWidget            *titlebar);
DZL_AVAILABLE_IN_3_38
DzlTitlebarAnimation  dzl_application_window_get_titlebar_animation (DzlApplicationWindow *self);

G_END_DECLS

#endif /* DZL_APPLICATION_WINDOW_H */
