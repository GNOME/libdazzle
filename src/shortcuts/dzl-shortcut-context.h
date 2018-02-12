/* dzl-shortcut-context.h
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

#ifndef DZL_SHORTCUT_CONTEXT_H
#define DZL_SHORTCUT_CONTEXT_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

#include "dzl-shortcut-chord.h"

G_BEGIN_DECLS

#define DZL_TYPE_SHORTCUT_CONTEXT (dzl_shortcut_context_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlShortcutContext, dzl_shortcut_context, DZL, SHORTCUT_CONTEXT, GObject)

DZL_AVAILABLE_IN_ALL
DzlShortcutContext *dzl_shortcut_context_new                (const gchar             *name);
DZL_AVAILABLE_IN_ALL
const gchar        *dzl_shortcut_context_get_name           (DzlShortcutContext      *self);
DZL_AVAILABLE_IN_ALL
DzlShortcutMatch    dzl_shortcut_context_activate           (DzlShortcutContext      *self,
                                                             GtkWidget               *widget,
                                                             const DzlShortcutChord  *chord);
DZL_AVAILABLE_IN_ALL
void                dzl_shortcut_context_add_action         (DzlShortcutContext      *self,
                                                             const gchar             *accel,
                                                             const gchar             *detailed_action_name);
DZL_AVAILABLE_IN_ALL
void                dzl_shortcut_context_add_command        (DzlShortcutContext      *self,
                                                             const gchar             *accel,
                                                             const gchar             *command);
DZL_AVAILABLE_IN_ALL
void                dzl_shortcut_context_add_signal         (DzlShortcutContext      *self,
                                                             const gchar             *accel,
                                                             const gchar             *signal_name,
                                                             guint                    n_args,
                                                             ...);
DZL_AVAILABLE_IN_ALL
void                dzl_shortcut_context_add_signal_va_list (DzlShortcutContext      *self,
                                                             const gchar             *accel,
                                                             const gchar             *signal_name,
                                                             guint                    n_args,
                                                             va_list                  args);
DZL_AVAILABLE_IN_ALL
void                dzl_shortcut_context_add_signalv        (DzlShortcutContext      *self,
                                                             const gchar             *accel,
                                                             const gchar             *signal_name,
                                                             GArray                  *values);
DZL_AVAILABLE_IN_ALL
gboolean            dzl_shortcut_context_remove             (DzlShortcutContext      *self,
                                                             const gchar             *accel);
DZL_AVAILABLE_IN_ALL
gboolean            dzl_shortcut_context_load_from_data     (DzlShortcutContext      *self,
                                                             const gchar             *data,
                                                             gssize                   len,
                                                             GError                 **error);
DZL_AVAILABLE_IN_ALL
gboolean            dzl_shortcut_context_load_from_resource (DzlShortcutContext      *self,
                                                             const gchar             *resource_path,
                                                             GError                 **error);

G_END_DECLS

#endif /* DZL_SHORTCUT_CONTEXT_H */
