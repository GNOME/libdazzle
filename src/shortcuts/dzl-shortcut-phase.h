/* dzl-shortcut-phase.h
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

#ifndef DZL_SHORTCUT_PHASE_H
#define DZL_SHORTCUT_PHASE_H

#include <glib-object.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_SHORTCUT_PHASE (dzl_shortcut_phase_get_type())

/**
 * DzlShortcutPhase:
 * @DZL_SHORTCUT_PHASE_CAPTURE: Indicates the capture phase of the shortcut
 *   activation. This allows parent widgets to intercept the keybinding before
 *   it is dispatched to the target #GdkWindow.
 * @DZL_SHORTCUT_DISPATCH: Indicates the typical dispatch phase of the shortcut
 *   to the widget of the target #GdkWindow.
 * @DZL_SHORTCUT_BUBBLE: The final phase of event delivery. The event is
 *   delivered to each widget as it progresses from the target window widget
 *   up to the toplevel.
 * @DZL_SHORTCUT_GLOBAL: The shortcut can be activated from anywhere in the
 *   widget hierarchy, even outside the direct chain of focus.
 */
typedef enum
{
  DZL_SHORTCUT_PHASE_DISPATCH = 0,
  DZL_SHORTCUT_PHASE_CAPTURE  = 1 << 0,
  DZL_SHORTCUT_PHASE_BUBBLE   = 1 << 1,
  DZL_SHORTCUT_PHASE_GLOBAL   = 1 << 2,
} DzlShortcutPhase;

DZL_AVAILABLE_IN_ALL
GType dzl_shortcut_phase_get_type (void);

G_END_DECLS

#endif /* DZL_SHORTCUT_PHASE_H */
