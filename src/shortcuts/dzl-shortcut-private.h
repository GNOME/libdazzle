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

#pragma once

#include "shortcuts/dzl-shortcut-chord.h"
#include "shortcuts/dzl-shortcut-closure-chain.h"
#include "shortcuts/dzl-shortcut-controller.h"
#include "shortcuts/dzl-shortcut-simple-label.h"
#include "shortcuts/dzl-shortcut-manager.h"
#include "shortcuts/dzl-shortcut-theme.h"

G_BEGIN_DECLS

#define DZL_SHORTCUT_CLOSURE_CHAIN_MAGIC 0x81236261
#define DZL_SHORTCUT_NODE_DATA_MAGIC     0x81746332

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
  guint                magic;
  const gchar         *name;
  const gchar         *title;
  const gchar         *subtitle;
} DzlShortcutNodeData;

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

typedef enum
{
  DZL_SHORTCUT_CLOSURE_ACTION = 1,
  DZL_SHORTCUT_CLOSURE_CALLBACK,
  DZL_SHORTCUT_CLOSURE_COMMAND,
  DZL_SHORTCUT_CLOSURE_SIGNAL,
  DZL_SHORTCUT_CLOSURE_LAST
} DzlShortcutClosureType;

struct _DzlShortcutClosureChain
{
  GSList node;

  guint magic;

  DzlShortcutClosureType type : 3;
  DzlShortcutPhase phase : 3;
  guint executing : 1;

  union {
    struct {
      const gchar *group;
      const gchar *name;
      GVariant    *params;
    } action;
    struct {
      const gchar *name;
    } command;
    struct {
      GQuark       detail;
      const gchar *name;
      GArray      *params;
    } signal;
    struct {
      GtkCallback    callback;
      gpointer       user_data;
      GDestroyNotify notify;
    } callback;
  };
};

DzlShortcutMatch       _dzl_shortcut_controller_handle              (DzlShortcutController      *self,
                                                                     const GdkEventKey          *event,
                                                                     const DzlShortcutChord     *chord,
                                                                     DzlShortcutPhase            phase,
                                                                     GtkWidget                  *widget);
DzlShortcutChord      *_dzl_shortcut_controller_push                (DzlShortcutController      *self,
                                                                     const GdkEventKey          *event);
void                   _dzl_shortcut_controller_clear               (DzlShortcutController      *self);
GNode                 *_dzl_shortcut_manager_get_root               (DzlShortcutManager         *self);
DzlShortcutTheme      *_dzl_shortcut_manager_get_internal_theme     (DzlShortcutManager         *self);
void                   _dzl_shortcut_simple_label_set_size_group    (DzlShortcutSimpleLabel     *self,
                                                                     GtkSizeGroup               *size_group);
void                   _dzl_shortcut_theme_attach                   (DzlShortcutTheme           *self);
void                   _dzl_shortcut_theme_detach                   (DzlShortcutTheme           *self);
GtkTreeModel          *_dzl_shortcut_theme_create_model             (DzlShortcutTheme           *self);
GHashTable            *_dzl_shortcut_theme_get_contexts             (DzlShortcutTheme           *self);
DzlShortcutContext    *_dzl_shortcut_theme_try_find_context_by_name (DzlShortcutTheme           *self,
                                                                     const gchar                *name);
DzlShortcutContext     *_dzl_shortcut_theme_find_default_context_with_phase
                                                                    (DzlShortcutTheme           *self,
                                                                     GtkWidget                  *widget,
                                                                     DzlShortcutPhase            phase);
void                   _dzl_shortcut_theme_set_manager              (DzlShortcutTheme           *self,
                                                                     DzlShortcutManager         *manager);
void                   _dzl_shortcut_theme_set_name                 (DzlShortcutTheme           *self,
                                                                     const gchar                *name);
const gchar           *_dzl_shortcut_theme_lookup_action            (DzlShortcutTheme           *self,
                                                                     const DzlShortcutChord     *chord);
void                   _dzl_shortcut_theme_merge                    (DzlShortcutTheme           *self,
                                                                     DzlShortcutTheme           *layer);
DzlShortcutMatch       _dzl_shortcut_theme_match                    (DzlShortcutTheme           *self,
                                                                     DzlShortcutPhase            phase,
                                                                     const DzlShortcutChord     *chord,
                                                                     DzlShortcutClosureChain   **chain);
gboolean               _dzl_shortcut_context_contains               (DzlShortcutContext         *self,
                                                                     const DzlShortcutChord     *chord);
DzlShortcutChordTable *_dzl_shortcut_context_get_table              (DzlShortcutContext         *self);
void                   _dzl_shortcut_context_merge                  (DzlShortcutContext         *self,
                                                                     DzlShortcutContext         *layer);
void                   _dzl_shortcut_chord_table_iter_init          (DzlShortcutChordTableIter  *iter,
                                                                     DzlShortcutChordTable      *table);
gboolean               _dzl_shortcut_chord_table_iter_next          (DzlShortcutChordTableIter  *iter,
                                                                     const DzlShortcutChord    **chord,
                                                                     gpointer                   *value);
void                   _dzl_shortcut_chord_table_iter_steal         (DzlShortcutChordTableIter  *iter);
gboolean               _dzl_shortcut_manager_get_command_info       (DzlShortcutManager         *self,
                                                                     const gchar                *command_id,
                                                                     const gchar               **title,
                                                                     const gchar               **subtitle);

static inline gboolean
DZL_IS_SHORTCUT_CLOSURE_CHAIN (DzlShortcutClosureChain *self)
{
  return self != NULL && self->magic == DZL_SHORTCUT_CLOSURE_CHAIN_MAGIC;
}

static inline gboolean
DZL_IS_SHORTCUT_NODE_DATA (DzlShortcutNodeData *data)
{
  return data != NULL && data->magic == DZL_SHORTCUT_NODE_DATA_MAGIC;
}

G_END_DECLS
