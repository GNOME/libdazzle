/* dzl-shortcut-private.h
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

#ifndef DZL_SHORTCUT_THEME_PRIVATE_H
#define DZL_SHORTCUT_THEME_PRIVATE_H

#include "shortcuts/dzl-shortcut-chord.h"
#include "shortcuts/dzl-shortcut-closure-chain.h"
#include "shortcuts/dzl-shortcut-manager.h"
#include "shortcuts/dzl-shortcut-theme.h"

G_BEGIN_DECLS

typedef struct
{
  DzlShortcutChordTable *table;
  guint                  position;
} DzlShortcutChordTableIter;

typedef enum
{
  DZL_SHORTCUT_NODE_SECTION = 1,
  DZL_SHORTCUT_NODE_GROUP,
  DZL_SHORTCUT_NODE_ACTION,
  DZL_SHORTCUT_NODE_COMMAND,
} DzlShortcutNodeType;

typedef struct
{
  DzlShortcutNodeType  type;
  const gchar         *name;
  const gchar         *title;
  const gchar         *subtitle;
} DzlShortcutNodeData;

typedef enum
{
  SHORTCUT_ACTION = 1,
  SHORTCUT_SIGNAL,
  SHORTCUT_COMMAND,
} ShortcutType;

typedef struct _Shortcut
{
  ShortcutType      type;
  union {
    struct {
      const gchar  *prefix;
      const gchar  *name;
      GVariant     *param;
    } action;
    struct {
      const gchar  *command;
    };
    struct {
      const gchar  *name;
      GQuark        detail;
      GArray       *params;
    } signal;
  };
  struct _Shortcut *next;
} Shortcut;

typedef enum
{
  DZL_SHORTCUT_MODEL_COLUMN_TYPE,
  DZL_SHORTCUT_MODEL_COLUMN_ID,
  DZL_SHORTCUT_MODEL_COLUMN_TITLE,
  DZL_SHORTCUT_MODEL_COLUMN_ACCEL,
  DZL_SHORTCUT_MODEL_COLUMN_KEYWORDS,
  DZL_SHORTCUT_MODEL_COLUMN_CHORD,
  DZL_SHORTCUT_MODEL_N_COLUMNS
} DzlShortcutModelColumn;

gboolean               _dzl_gtk_widget_activate_action          (GtkWidget                  *widget,
                                                                 const gchar                *prefix,
                                                                 const gchar                *action_name,
                                                                 GVariant                   *parameter);
GNode                 *_dzl_shortcut_manager_get_root           (DzlShortcutManager         *self);
DzlShortcutTheme      *_dzl_shortcut_manager_get_internal_theme (DzlShortcutManager         *self);
GtkTreeModel          *_dzl_shortcut_theme_create_model         (DzlShortcutTheme           *self);
GHashTable            *_dzl_shortcut_theme_get_contexts         (DzlShortcutTheme           *self);
void                   _dzl_shortcut_theme_set_manager          (DzlShortcutTheme           *self,
                                                                 DzlShortcutManager         *manager);
void                   _dzl_shortcut_theme_set_name             (DzlShortcutTheme           *self,
                                                                 const gchar                *name);
void                   _dzl_shortcut_theme_merge                (DzlShortcutTheme           *self,
                                                                 DzlShortcutTheme           *layer);
DzlShortcutMatch       _dzl_shortcut_theme_match                (DzlShortcutTheme           *self,
                                                                 const DzlShortcutChord     *chord,
                                                                 DzlShortcutClosureChain   **chain);
DzlShortcutChordTable *_dzl_shortcut_context_get_table          (DzlShortcutContext         *self);
void                   _dzl_shortcut_context_merge              (DzlShortcutContext         *self,
                                                                 DzlShortcutContext         *layer);
void                   _dzl_shortcut_chord_table_iter_init      (DzlShortcutChordTableIter  *iter,
                                                                 DzlShortcutChordTable      *table);
gboolean               _dzl_shortcut_chord_table_iter_next      (DzlShortcutChordTableIter  *iter,
                                                                 const DzlShortcutChord    **chord,
                                                                 gpointer                   *value);
void                   _dzl_shortcut_chord_table_iter_steal     (DzlShortcutChordTableIter  *iter);

G_END_DECLS

#endif /* DZL_SHORTCUT_THEME_PRIVATE_H */
