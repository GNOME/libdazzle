/* dzl-shortcut-simple-label.h
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

#ifndef DZL_SHORTCUT_SIMPLE_LABEL_H
#define DZL_SHORTCUT_SIMPLE_LABEL_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_SHORTCUT_SIMPLE_LABEL (dzl_shortcut_simple_label_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlShortcutSimpleLabel, dzl_shortcut_simple_label, DZL, SHORTCUT_SIMPLE_LABEL, GtkBox)

DZL_AVAILABLE_IN_ALL
GtkWidget   *dzl_shortcut_simple_label_new         (void);
DZL_AVAILABLE_IN_ALL
const gchar *dzl_shortcut_simple_label_get_accel   (DzlShortcutSimpleLabel *self);
DZL_AVAILABLE_IN_ALL
void         dzl_shortcut_simple_label_set_accel   (DzlShortcutSimpleLabel *self,
                                                    const gchar            *accel);
DZL_AVAILABLE_IN_ALL
const gchar *dzl_shortcut_simple_label_get_action  (DzlShortcutSimpleLabel *self);
DZL_AVAILABLE_IN_ALL
void         dzl_shortcut_simple_label_set_action  (DzlShortcutSimpleLabel *self,
                                                    const gchar            *action);
DZL_AVAILABLE_IN_ALL
const gchar *dzl_shortcut_simple_label_get_command (DzlShortcutSimpleLabel *self);
DZL_AVAILABLE_IN_ALL
void         dzl_shortcut_simple_label_set_command (DzlShortcutSimpleLabel *self,
                                                    const gchar            *command);
DZL_AVAILABLE_IN_ALL
const gchar *dzl_shortcut_simple_label_get_title   (DzlShortcutSimpleLabel *self);
DZL_AVAILABLE_IN_ALL
void         dzl_shortcut_simple_label_set_title   (DzlShortcutSimpleLabel *self,
                                                    const gchar            *title);

G_END_DECLS

#endif /* DZL_SHORTCUT_SIMPLE_LABEL_H */
