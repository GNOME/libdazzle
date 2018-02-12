/* dzl-counters-window.h
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

#ifndef DZL_COUNTERS_WINDOW_H
#define DZL_COUNTERS_WINDOW_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

#include "util/dzl-counter.h"

G_BEGIN_DECLS

#define DZL_TYPE_COUNTERS_WINDOW (dzl_counters_window_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlCountersWindow, dzl_counters_window, DZL, COUNTERS_WINDOW, GtkWindow)

struct _DzlCountersWindowClass
{
  GtkWindowClass parent_class;

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
};

DZL_AVAILABLE_IN_ALL
GtkWidget       *dzl_counters_window_new       (void);
DZL_AVAILABLE_IN_ALL
DzlCounterArena *dzl_counters_window_get_arena (DzlCountersWindow *self);
DZL_AVAILABLE_IN_ALL
void             dzl_counters_window_set_arena (DzlCountersWindow *self,
                                                DzlCounterArena   *arena);

G_END_DECLS

#endif /* DZL_COUNTERS_WINDOW_H */
