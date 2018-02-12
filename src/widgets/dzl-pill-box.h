/* dzl-pill-box.h
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
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

#ifndef DZL_PILL_BOX_H
#define DZL_PILL_BOX_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_PILL_BOX (dzl_pill_box_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlPillBox, dzl_pill_box, DZL, PILL_BOX, GtkEventBox)

DZL_AVAILABLE_IN_ALL
GtkWidget   *dzl_pill_box_new       (const gchar *label);
DZL_AVAILABLE_IN_ALL
const gchar *dzl_pill_box_get_label (DzlPillBox  *self);
DZL_AVAILABLE_IN_ALL
void         dzl_pill_box_set_label (DzlPillBox  *self,
                                     const gchar *label);

G_END_DECLS

#endif /* DZL_PILL_BOX_H */
