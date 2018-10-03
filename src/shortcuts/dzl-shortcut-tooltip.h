/* dzl-shortcut-tooltip.h
 *
 * Copyright 2018 Christian Hergert <chergert@redhat.com>
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
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_SHORTCUT_TOOLTIP (dzl_shortcut_tooltip_get_type())

DZL_AVAILABLE_IN_3_32
G_DECLARE_FINAL_TYPE (DzlShortcutTooltip, dzl_shortcut_tooltip, DZL, SHORTCUT_TOOLTIP, GObject)

DZL_AVAILABLE_IN_3_32
DzlShortcutTooltip *dzl_shortcut_tooltip_new            (void);
DZL_AVAILABLE_IN_3_32
const gchar        *dzl_shortcut_tooltip_get_accel      (DzlShortcutTooltip *self);
DZL_AVAILABLE_IN_3_32
void                dzl_shortcut_tooltip_set_accel      (DzlShortcutTooltip *self,
                                                         const gchar        *accel);
DZL_AVAILABLE_IN_3_32
GtkWidget          *dzl_shortcut_tooltip_get_widget     (DzlShortcutTooltip *self);
DZL_AVAILABLE_IN_3_32
void                dzl_shortcut_tooltip_set_widget     (DzlShortcutTooltip *self,
                                                         GtkWidget          *widget);
DZL_AVAILABLE_IN_3_32
const gchar        *dzl_shortcut_tooltip_get_command_id (DzlShortcutTooltip *self);
DZL_AVAILABLE_IN_3_32
void                dzl_shortcut_tooltip_set_command_id (DzlShortcutTooltip *self,
                                                         const gchar        *command_id);
DZL_AVAILABLE_IN_3_32
const gchar        *dzl_shortcut_tooltip_get_title      (DzlShortcutTooltip *self);
DZL_AVAILABLE_IN_3_32
void                dzl_shortcut_tooltip_set_title      (DzlShortcutTooltip *self,
                                                         const gchar        *title);


G_END_DECLS
