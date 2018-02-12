/* dzl-simple-label.h
 *
 * Copyright (C) 2016 Christian Hergert <christian@hergert.me>
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

#ifndef DZL_SIMPLE_LABEL_H
#define DZL_SIMPLE_LABEL_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

/*
 * This widget has one very simple purpose. Allow updating a simple
 * amount of text without causing resizes to propagate up the widget
 * hierarchy. Therefore, it only supports a very minimal amount of
 * features. The label text, and the width-chars to use for sizing.
 */

#define DZL_TYPE_SIMPLE_LABEL (dzl_simple_label_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlSimpleLabel, dzl_simple_label, DZL, SIMPLE_LABEL, GtkWidget)

DZL_AVAILABLE_IN_ALL
GtkWidget   *dzl_simple_label_new             (const gchar    *label);
DZL_AVAILABLE_IN_ALL
const gchar *dzl_simple_label_get_label       (DzlSimpleLabel *self);
DZL_AVAILABLE_IN_ALL
void         dzl_simple_label_set_label       (DzlSimpleLabel *self,
                                               const gchar    *label);
DZL_AVAILABLE_IN_ALL
gint         dzl_simple_label_get_width_chars (DzlSimpleLabel *self);
DZL_AVAILABLE_IN_ALL
void         dzl_simple_label_set_width_chars (DzlSimpleLabel *self,
                                               gint            width_chars);
DZL_AVAILABLE_IN_ALL
gfloat       dzl_simple_label_get_xalign      (DzlSimpleLabel *self);
DZL_AVAILABLE_IN_ALL
void         dzl_simple_label_set_xalign      (DzlSimpleLabel *self,
                                               gfloat          xalign);

G_END_DECLS

#endif /* DZL_SIMPLE_LABEL_H */
