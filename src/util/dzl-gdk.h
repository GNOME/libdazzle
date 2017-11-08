/* dzl-gdk.h
 *
 * Copyright (C) 2015 Christian Hergert <chergert@redhat.com>
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

#ifndef DZL_GDK_H
#define DZL_GDK_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

DZL_AVAILABLE_IN_ALL
GdkEventKey *dzl_gdk_synthesize_event_key    (GdkWindow *window,
                                              gunichar   ch);
DZL_AVAILABLE_IN_ALL
GdkEventKey *dzl_gdk_synthesize_event_keyval (GdkWindow *window,
                                              guint      keyval);

G_END_DECLS

#endif /* DZL_GDK_H */
