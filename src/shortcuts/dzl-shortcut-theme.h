/* dzl-shortcut-theme.h
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

#ifndef DZL_SHORTCUT_THEME_H
#define DZL_SHORTCUT_THEME_H

#include <gtk/gtk.h>

#include "dzl-shortcut-chord.h"
#include "dzl-shortcut-context.h"

G_BEGIN_DECLS

#define DZL_TYPE_SHORTCUT_THEME (dzl_shortcut_theme_get_type())

G_DECLARE_DERIVABLE_TYPE (DzlShortcutTheme, dzl_shortcut_theme, DZL, SHORTCUT_THEME, GObject)

struct _DzlShortcutThemeClass
{
  GObjectClass parent_class;

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
  gpointer _reserved5;
  gpointer _reserved6;
  gpointer _reserved7;
  gpointer _reserved8;
};

DzlShortcutTheme       *dzl_shortcut_theme_new                   (const gchar             *name);
const gchar            *dzl_shortcut_theme_get_name              (DzlShortcutTheme        *self);
const gchar            *dzl_shortcut_theme_get_title             (DzlShortcutTheme        *self);
const gchar            *dzl_shortcut_theme_get_subtitle          (DzlShortcutTheme        *self);
DzlShortcutTheme       *dzl_shortcut_theme_get_parent            (DzlShortcutTheme        *self);
const gchar            *dzl_shortcut_theme_get_parent_name       (DzlShortcutTheme        *self);
void                    dzl_shortcut_theme_set_parent_name       (DzlShortcutTheme        *self,
                                                                  const gchar             *parent_name);
DzlShortcutContext     *dzl_shortcut_theme_find_default_context  (DzlShortcutTheme        *self,
                                                                  GtkWidget               *widget);
DzlShortcutContext     *dzl_shortcut_theme_find_context_by_name  (DzlShortcutTheme        *self,
                                                                  const gchar             *name);
void                    dzl_shortcut_theme_add_command           (DzlShortcutTheme        *self,
                                                                  const gchar             *accelerator,
                                                                  const gchar             *command);
void                    dzl_shortcut_theme_add_context           (DzlShortcutTheme        *self,
                                                                  DzlShortcutContext      *context);
void                    dzl_shortcut_theme_set_chord_for_action  (DzlShortcutTheme        *self,
                                                                  const gchar             *detailed_action_name,
                                                                  const DzlShortcutChord  *chord);
const DzlShortcutChord *dzl_shortcut_theme_get_chord_for_action  (DzlShortcutTheme        *self,
                                                                  const gchar             *detailed_action_name);
void                    dzl_shortcut_theme_set_accel_for_action  (DzlShortcutTheme        *self,
                                                                  const gchar             *detailed_action_name,
                                                                  const gchar             *accel);
void                    dzl_shortcut_theme_set_chord_for_command (DzlShortcutTheme        *self,
                                                                  const gchar             *command,
                                                                  const DzlShortcutChord  *chord);
const DzlShortcutChord *dzl_shortcut_theme_get_chord_for_command (DzlShortcutTheme        *self,
                                                                  const gchar             *command);
void                    dzl_shortcut_theme_set_accel_for_command (DzlShortcutTheme        *self,
                                                                  const gchar             *command,
                                                                  const gchar             *accel);
gboolean                dzl_shortcut_theme_load_from_data        (DzlShortcutTheme        *self,
                                                                  const gchar             *data,
                                                                  gssize                   len,
                                                                  GError                 **error);
gboolean                dzl_shortcut_theme_load_from_file        (DzlShortcutTheme        *self,
                                                                  GFile                   *file,
                                                                  GCancellable            *cancellable,
                                                                  GError                 **error);
gboolean                dzl_shortcut_theme_load_from_path        (DzlShortcutTheme        *self,
                                                                  const gchar             *path,
                                                                  GCancellable            *cancellable,
                                                                  GError                 **error);
gboolean                dzl_shortcut_theme_save_to_file          (DzlShortcutTheme        *self,
                                                                  GFile                   *file,
                                                                  GCancellable            *cancellable,
                                                                  GError                 **error);
gboolean                dzl_shortcut_theme_save_to_stream        (DzlShortcutTheme        *self,
                                                                  GOutputStream           *stream,
                                                                  GCancellable            *cancellable,
                                                                  GError                 **error);
gboolean                dzl_shortcut_theme_save_to_path          (DzlShortcutTheme        *self,
                                                                  const gchar             *path,
                                                                  GCancellable            *cancellable,
                                                                  GError                 **error);
void                    dzl_shortcut_theme_add_css_resource      (DzlShortcutTheme        *self,
                                                                  const gchar             *path);
void                    dzl_shortcut_theme_remove_css_resource   (DzlShortcutTheme        *self,
                                                                  const gchar             *path);

G_END_DECLS

#endif /* DZL_SHORTCUT_THEME_H */
