/* dzl-shortcut-accel-dialog.h
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#ifndef DZL_SHORTCUT_ACCEL_DIALOG_H
#define DZL_SHORTCUT_ACCEL_DIALOG_H

#include <gtk/gtk.h>

#include "dzl-shortcut-chord.h"

G_BEGIN_DECLS

#define DZL_TYPE_SHORTCUT_ACCEL_DIALOG (dzl_shortcut_accel_dialog_get_type())

G_DECLARE_FINAL_TYPE (DzlShortcutAccelDialog, dzl_shortcut_accel_dialog, DZL, SHORTCUT_ACCEL_DIALOG, GtkDialog)

GtkWidget              *dzl_shortcut_accel_dialog_new                (void);
gchar                  *dzl_shortcut_accel_dialog_get_accelerator    (DzlShortcutAccelDialog *self);
void                    dzl_shortcut_accel_dialog_set_accelerator    (DzlShortcutAccelDialog *self,
                                                                      const gchar            *accelerator);
const DzlShortcutChord *dzl_shortcut_accel_dialog_get_chord          (DzlShortcutAccelDialog *self);
const gchar            *dzl_shortcut_accel_dialog_get_shortcut_title (DzlShortcutAccelDialog *self);
void                    dzl_shortcut_accel_dialog_set_shortcut_title (DzlShortcutAccelDialog *self,
                                                                      const gchar            *title);

G_END_DECLS

#endif /* DZL_SHORTCUT_ACCEL_DIALOG_H */
