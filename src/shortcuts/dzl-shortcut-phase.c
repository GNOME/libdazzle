/* dzl-shortcut-phase.c
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

#define G_LOG_DOMAIN "dzl-shortcut-phase"

#include "config.h"

#include "shortcuts/dzl-shortcut-phase.h"

GType
dzl_shortcut_phase_get_type (void)
{
  static GType type_id;

  if (g_once_init_enter (&type_id))
    {
      static const GFlagsValue values[] = {
        { DZL_SHORTCUT_PHASE_DISPATCH, "DZL_SHORTCUT_PHASE_DISPATCH", "dispatch" },
        { DZL_SHORTCUT_PHASE_CAPTURE, "DZL_SHORTCUT_PHASE_CAPTURE", "capture" },
        { DZL_SHORTCUT_PHASE_BUBBLE, "DZL_SHORTCUT_PHASE_BUBBLE", "bubble" },
        { DZL_SHORTCUT_PHASE_GLOBAL, "DZL_SHORTCUT_PHASE_GLOBAL", "global" },
        { 0 }
      };
      GType _type_id = g_flags_register_static ("DzlShortcutPhase", values);
      g_once_init_leave (&type_id, _type_id);
    }

  return type_id;
}
