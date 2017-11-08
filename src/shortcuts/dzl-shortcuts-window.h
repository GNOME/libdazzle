/* dzl-shortcuts-window.h
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __DZL_SHORTCUTS_WINDOW_H__
#define __DZL_SHORTCUTS_WINDOW_H__

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_SHORTCUTS_WINDOW            (dzl_shortcuts_window_get_type ())
#define DZL_SHORTCUTS_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DZL_TYPE_SHORTCUTS_WINDOW, DzlShortcutsWindow))
#define DZL_SHORTCUTS_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DZL_TYPE_SHORTCUTS_WINDOW, DzlShortcutsWindowClass))
#define DZL_IS_SHORTCUTS_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DZL_TYPE_SHORTCUTS_WINDOW))
#define DZL_IS_SHORTCUTS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DZL_TYPE_SHORTCUTS_WINDOW))
#define DZL_SHORTCUTS_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DZL_TYPE_SHORTCUTS_WINDOW, DzlShortcutsWindowClass))


typedef struct _DzlShortcutsWindow         DzlShortcutsWindow;
typedef struct _DzlShortcutsWindowClass    DzlShortcutsWindowClass;


struct _DzlShortcutsWindow
{
  GtkWindow window;
};

struct _DzlShortcutsWindowClass
{
  GtkWindowClass parent_class;

  void (*close)  (DzlShortcutsWindow *self);
  void (*search) (DzlShortcutsWindow *self);
};

DZL_AVAILABLE_IN_ALL
GType dzl_shortcuts_window_get_type (void) G_GNUC_CONST;

G_DEFINE_AUTOPTR_CLEANUP_FUNC(DzlShortcutsWindow, g_object_unref)

G_END_DECLS

#endif /* DZL_SHORTCUTS_WINDOW _H */
