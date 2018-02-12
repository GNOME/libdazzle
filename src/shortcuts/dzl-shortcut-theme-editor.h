/* dzl-shortcut-theme-editor.h
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
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

#ifndef DZL_SHORTCUT_THEME_EDITOR_H
#define DZL_SHORTCUT_THEME_EDITOR_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

#include "dzl-shortcut-theme.h"

G_BEGIN_DECLS

#define DZL_TYPE_SHORTCUT_THEME_EDITOR (dzl_shortcut_theme_editor_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlShortcutThemeEditor, dzl_shortcut_theme_editor, DZL, SHORTCUT_THEME_EDITOR, GtkBin)

struct _DzlShortcutThemeEditorClass
{
  GtkBinClass parent_class;

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
GtkWidget        *dzl_shortcut_theme_editor_new       (void);
DZL_AVAILABLE_IN_ALL
DzlShortcutTheme *dzl_shortcut_theme_editor_get_theme (DzlShortcutThemeEditor *self);
DZL_AVAILABLE_IN_ALL
void              dzl_shortcut_theme_editor_set_theme (DzlShortcutThemeEditor *self,
                                                       DzlShortcutTheme       *theme);

G_END_DECLS

#endif /* DZL_SHORTCUT_THEME_EDITOR_H */
