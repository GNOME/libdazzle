/* dzl-shortcut-label.h
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

#ifndef DZL_SHORTCUT_LABEL_H
#define DZL_SHORTCUT_LABEL_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

#include "dzl-shortcut-chord.h"

G_BEGIN_DECLS

#define DZL_TYPE_SHORTCUT_LABEL (dzl_shortcut_label_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlShortcutLabel, dzl_shortcut_label, DZL, SHORTCUT_LABEL, GtkBox)

DZL_AVAILABLE_IN_ALL
GtkWidget              *dzl_shortcut_label_new             (void);
DZL_AVAILABLE_IN_ALL
gchar                  *dzl_shortcut_label_get_accelerator (DzlShortcutLabel       *self);
DZL_AVAILABLE_IN_ALL
void                    dzl_shortcut_label_set_accelerator (DzlShortcutLabel       *self,
                                                            const gchar            *accelerator);
DZL_AVAILABLE_IN_ALL
void                    dzl_shortcut_label_set_chord       (DzlShortcutLabel       *self,
                                                            const DzlShortcutChord *chord);
DZL_AVAILABLE_IN_ALL
const DzlShortcutChord *dzl_shortcut_label_get_chord       (DzlShortcutLabel       *self);

G_END_DECLS

#endif /* DZL_SHORTCUT_LABEL_H */
