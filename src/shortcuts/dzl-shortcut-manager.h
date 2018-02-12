/* dzl-shortcut-manager.c
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

#ifndef DZL_SHORTCUT_MANAGER_H
#define DZL_SHORTCUT_MANAGER_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

#include "dzl-shortcut-phase.h"
#include "dzl-shortcut-theme.h"
#include "dzl-shortcuts-window.h"

G_BEGIN_DECLS

#define DZL_TYPE_SHORTCUT_MANAGER (dzl_shortcut_manager_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlShortcutManager, dzl_shortcut_manager, DZL, SHORTCUT_MANAGER, GObject)

/**
 * DzlShortcutEntry:
 * @command: the command identifier
 * @phase: the phase for activation, or 0 for the default
 * @default_accel: the default accelerator for the command, if any
 * @section: the section for the shortcuts window
 * @group: the group for the shortcuts window
 * @title: the title for the shortcuts window
 * @subtitle: the subtitle for the shortcuts window, if any
 *
 * The #DzlShortcutEntry structure can be used to bulk register shortcuts
 * for a particular widget. It can also do the necessary hooks of registering
 * commands that can be changed using the keytheme components.
 */
typedef struct
{
  const gchar      *command;
  DzlShortcutPhase  phase;
  const gchar      *default_accel;
  const gchar      *section;
  const gchar      *group;
  const gchar      *title;
  const gchar      *subtitle;
} DzlShortcutEntry;

struct _DzlShortcutManagerClass
{
  GObjectClass parent_instance;

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
  gpointer _reserved5;
  gpointer _reserved6;
  gpointer _reserved7;
  gpointer _reserved8;
};

DZL_AVAILABLE_IN_ALL
DzlShortcutManager *dzl_shortcut_manager_get_default             (void);
DZL_AVAILABLE_IN_ALL
void                dzl_shortcut_manager_queue_reload            (DzlShortcutManager     *self);
DZL_AVAILABLE_IN_ALL
void                dzl_shortcut_manager_reload                  (DzlShortcutManager     *self,
                                                                  GCancellable           *cancellable);
DZL_AVAILABLE_IN_ALL
void                dzl_shortcut_manager_append_search_path      (DzlShortcutManager     *self,
                                                                  const gchar            *directory);
DZL_AVAILABLE_IN_ALL
void                dzl_shortcut_manager_prepend_search_path     (DzlShortcutManager     *self,
                                                                  const gchar            *directory);
DZL_AVAILABLE_IN_ALL
void                dzl_shortcut_manager_remove_search_path      (DzlShortcutManager     *self,
                                                                  const gchar            *directory);
DZL_AVAILABLE_IN_ALL
DzlShortcutTheme   *dzl_shortcut_manager_get_theme               (DzlShortcutManager     *self);
DZL_AVAILABLE_IN_ALL
void                dzl_shortcut_manager_set_theme               (DzlShortcutManager     *self,
                                                                  DzlShortcutTheme       *theme);
DZL_AVAILABLE_IN_ALL
const gchar        *dzl_shortcut_manager_get_theme_name          (DzlShortcutManager     *self);
DZL_AVAILABLE_IN_ALL
void                dzl_shortcut_manager_set_theme_name          (DzlShortcutManager     *self,
                                                                  const gchar            *theme_name);
DZL_AVAILABLE_IN_ALL
DzlShortcutTheme   *dzl_shortcut_manager_get_theme_by_name       (DzlShortcutManager     *self,
                                                                  const gchar            *theme_name);
DZL_AVAILABLE_IN_ALL
gboolean            dzl_shortcut_manager_handle_event            (DzlShortcutManager     *self,
                                                                  const GdkEventKey      *event,
                                                                  GtkWidget              *toplevel);
DZL_AVAILABLE_IN_ALL
const gchar        *dzl_shortcut_manager_get_user_dir            (DzlShortcutManager     *self);
DZL_AVAILABLE_IN_ALL
void                dzl_shortcut_manager_set_user_dir            (DzlShortcutManager     *self,
                                                                  const gchar            *user_dir);
DZL_AVAILABLE_IN_ALL
void                dzl_shortcut_manager_add_action              (DzlShortcutManager     *self,
                                                                  const gchar            *detailed_action_name,
                                                                  const gchar            *section,
                                                                  const gchar            *group,
                                                                  const gchar            *title,
                                                                  const gchar            *subtitle);
DZL_AVAILABLE_IN_ALL
void                dzl_shortcut_manager_add_command             (DzlShortcutManager     *self,
                                                                  const gchar            *command,
                                                                  const gchar            *section,
                                                                  const gchar            *group,
                                                                  const gchar            *title,
                                                                  const gchar            *subtitle);
DZL_AVAILABLE_IN_ALL
void                dzl_shortcut_manager_add_shortcut_entries    (DzlShortcutManager     *self,
                                                                  const DzlShortcutEntry *shortcuts,
                                                                  guint                   n_shortcuts,
                                                                  const gchar            *translation_domain);
DZL_AVAILABLE_IN_ALL
void                dzl_shortcut_manager_add_shortcuts_to_window (DzlShortcutManager     *self,
                                                                  DzlShortcutsWindow     *window);

G_END_DECLS

#endif /* DZL_SHORTCUT_MANAGER_H */
