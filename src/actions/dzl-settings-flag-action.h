/* dzl-settings-flag-action.h
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

#ifndef DZL_SETTINGS_FLAG_ACTION_H
#define DZL_SETTINGS_FLAG_ACTION_H

#include <gio/gio.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_SETTINGS_FLAG_ACTION (dzl_settings_flag_action_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlSettingsFlagAction, dzl_settings_flag_action, DZL, SETTINGS_FLAG_ACTION, GObject)

DZL_AVAILABLE_IN_ALL
GAction *dzl_settings_flag_action_new (const gchar *schema_id,
                                       const gchar *schema_key,
                                       const gchar *flag_nick);

G_END_DECLS

#endif /* DZL_SETTINGS_FLAG_ACTION_H */
