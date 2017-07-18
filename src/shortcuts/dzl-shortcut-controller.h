/* dzl-shortcut-controller.h
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
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

#include "shortcuts/dzl-shortcut-context.h"
#include "shortcuts/dzl-shortcut-manager.h"
#include "shortcuts/dzl-shortcut-phase.h"

G_BEGIN_DECLS

#define DZL_TYPE_SHORTCUT_CONTROLLER (dzl_shortcut_controller_get_type())

G_DECLARE_FINAL_TYPE (DzlShortcutController, dzl_shortcut_controller, DZL, SHORTCUT_CONTROLLER, GObject)

DzlShortcutController  *dzl_shortcut_controller_new                   (GtkWidget             *widget);
DzlShortcutManager     *dzl_shortcut_controller_get_manager           (DzlShortcutController *self);
void                    dzl_shortcut_controller_set_manager           (DzlShortcutController *self,
                                                                       DzlShortcutManager    *manager);
DzlShortcutController  *dzl_shortcut_controller_find                  (GtkWidget             *widget);
DzlShortcutController  *dzl_shortcut_controller_try_find              (GtkWidget             *widget);
DzlShortcutContext     *dzl_shortcut_controller_get_context           (DzlShortcutController *self);
void                    dzl_shortcut_controller_set_context_by_name   (DzlShortcutController *self,
                                                                       const gchar           *name);
DzlShortcutContext     *dzl_shortcut_controller_get_context_for_phase (DzlShortcutController *self,
                                                                       DzlShortcutPhase       phase);
gboolean                dzl_shortcut_controller_execute_command       (DzlShortcutController *self,
                                                                       const gchar           *command);
const DzlShortcutChord *dzl_shortcut_controller_get_current_chord     (DzlShortcutController *self);
void                    dzl_shortcut_controller_add_command_action    (DzlShortcutController *self,
                                                                       const gchar           *command_id,
                                                                       const gchar           *default_accel,
                                                                       const gchar           *action);
void                    dzl_shortcut_controller_add_command_callback  (DzlShortcutController *self,
                                                                       const gchar           *command_id,
                                                                       const gchar           *default_accel,
                                                                       GtkCallback            callback,
                                                                       gpointer               callback_data,
                                                                       GDestroyNotify         callback_data_destroy);
void                    dzl_shortcut_controller_add_command_signal    (DzlShortcutController *self,
                                                                       const gchar           *command_id,
                                                                       const gchar           *default_accel,
                                                                       const gchar           *signal_name,
                                                                       guint                  n_args,
                                                                       ...);

G_END_DECLS

#endif /* DZL_SHORTCUT_CONTROLLER_H */
