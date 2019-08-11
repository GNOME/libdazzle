/* dzl-shortcut-controller.h
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
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

#ifndef DZL_SHORTCUT_CONTROLLER_H
#define DZL_SHORTCUT_CONTROLLER_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

#include "shortcuts/dzl-shortcut-context.h"
#include "shortcuts/dzl-shortcut-manager.h"
#include "shortcuts/dzl-shortcut-phase.h"

G_BEGIN_DECLS

#define DZL_TYPE_SHORTCUT_CONTROLLER (dzl_shortcut_controller_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlShortcutController, dzl_shortcut_controller, DZL, SHORTCUT_CONTROLLER, GObject)

DZL_AVAILABLE_IN_ALL
DzlShortcutController  *dzl_shortcut_controller_new                   (GtkWidget             *widget);
DZL_AVAILABLE_IN_3_34
GtkWidget              *dzl_shortcut_controller_get_widget            (DzlShortcutController *self);
DZL_AVAILABLE_IN_ALL
DzlShortcutManager     *dzl_shortcut_controller_get_manager           (DzlShortcutController *self);
DZL_AVAILABLE_IN_ALL
void                    dzl_shortcut_controller_set_manager           (DzlShortcutController *self,
                                                                       DzlShortcutManager    *manager);
DZL_AVAILABLE_IN_ALL
DzlShortcutController  *dzl_shortcut_controller_find                  (GtkWidget             *widget);
DZL_AVAILABLE_IN_ALL
DzlShortcutController  *dzl_shortcut_controller_try_find              (GtkWidget             *widget);
DZL_AVAILABLE_IN_ALL
DzlShortcutContext     *dzl_shortcut_controller_get_context           (DzlShortcutController *self);
DZL_AVAILABLE_IN_ALL
void                    dzl_shortcut_controller_set_context_by_name   (DzlShortcutController *self,
                                                                       const gchar           *name);
DZL_AVAILABLE_IN_ALL
DzlShortcutContext     *dzl_shortcut_controller_get_context_for_phase (DzlShortcutController *self,
                                                                       DzlShortcutPhase       phase);
DZL_AVAILABLE_IN_ALL
gboolean                dzl_shortcut_controller_execute_command       (DzlShortcutController *self,
                                                                       const gchar           *command);
DZL_AVAILABLE_IN_ALL
const DzlShortcutChord *dzl_shortcut_controller_get_current_chord     (DzlShortcutController *self);
DZL_AVAILABLE_IN_3_34
void                    dzl_shortcut_controller_remove_accel          (DzlShortcutController *self,
                                                                       const gchar           *accel,
                                                                       DzlShortcutPhase       phase);
DZL_AVAILABLE_IN_ALL
void                    dzl_shortcut_controller_add_command_action    (DzlShortcutController *self,
                                                                       const gchar           *command_id,
                                                                       const gchar           *default_accel,
                                                                       DzlShortcutPhase       phase,
                                                                       const gchar           *action);
DZL_AVAILABLE_IN_ALL
void                    dzl_shortcut_controller_add_command_callback  (DzlShortcutController *self,
                                                                       const gchar           *command_id,
                                                                       const gchar           *default_accel,
                                                                       DzlShortcutPhase       phase,
                                                                       GtkCallback            callback,
                                                                       gpointer               callback_data,
                                                                       GDestroyNotify         callback_data_destroy);
DZL_AVAILABLE_IN_ALL
void                    dzl_shortcut_controller_add_command_signal    (DzlShortcutController *self,
                                                                       const gchar           *command_id,
                                                                       const gchar           *default_accel,
                                                                       DzlShortcutPhase       phase,
                                                                       const gchar           *signal_name,
                                                                       guint                  n_args,
                                                                       ...);

G_END_DECLS

#endif /* DZL_SHORTCUT_CONTROLLER_H */
