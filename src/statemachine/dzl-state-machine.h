/* dzl-state-machine.h
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DZL_STATE_MACHINE_H
#define DZL_STATE_MACHINE_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_STATE_MACHINE  (dzl_state_machine_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlStateMachine, dzl_state_machine, DZL, STATE_MACHINE, GObject)

struct _DzlStateMachineClass
{
  GObjectClass parent;
};

DZL_AVAILABLE_IN_ALL
DzlStateMachine *dzl_state_machine_new               (void);
DZL_AVAILABLE_IN_ALL
const gchar     *dzl_state_machine_get_state         (DzlStateMachine *self);
DZL_AVAILABLE_IN_ALL
void             dzl_state_machine_set_state         (DzlStateMachine *self,
                                                      const gchar     *state);
DZL_AVAILABLE_IN_3_28
gboolean         dzl_state_machine_is_state          (DzlStateMachine *self,
                                                      const gchar     *state);
DZL_AVAILABLE_IN_ALL
GAction         *dzl_state_machine_create_action     (DzlStateMachine *self,
                                                      const gchar     *name);
DZL_AVAILABLE_IN_ALL
void             dzl_state_machine_add_property      (DzlStateMachine *self,
                                                      const gchar     *state,
                                                      gpointer         object,
                                                      const gchar     *property,
                                                      ...);
DZL_AVAILABLE_IN_ALL
void             dzl_state_machine_add_property_valist
                                                     (DzlStateMachine *self,
                                                      const gchar     *state,
                                                      gpointer         object,
                                                      const gchar     *property,
                                                      va_list          var_args);
DZL_AVAILABLE_IN_ALL
void             dzl_state_machine_add_propertyv     (DzlStateMachine *self,
                                                      const gchar     *state,
                                                      gpointer         object,
                                                      const gchar     *property,
                                                      const GValue    *value);
DZL_AVAILABLE_IN_ALL
void             dzl_state_machine_add_binding       (DzlStateMachine *self,
                                                      const gchar     *state,
                                                      gpointer         source_object,
                                                      const gchar     *source_property,
                                                      gpointer         target_object,
                                                      const gchar     *target_property,
                                                      GBindingFlags    flags);
DZL_AVAILABLE_IN_ALL
void             dzl_state_machine_add_style         (DzlStateMachine *self,
                                                      const gchar     *state,
                                                      GtkWidget       *widget,
                                                      const gchar     *style);
DZL_AVAILABLE_IN_ALL
void             dzl_state_machine_connect_object    (DzlStateMachine *self,
                                                      const gchar     *state,
                                                      gpointer         source,
                                                      const gchar     *detailed_signal,
                                                      GCallback        callback,
                                                      gpointer         user_data,
                                                      GConnectFlags    flags);

G_END_DECLS

#endif /* DZL_STATE_MACHINE_H */
