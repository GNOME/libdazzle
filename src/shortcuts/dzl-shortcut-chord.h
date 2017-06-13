/* dzl-shortcut-chord.h
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

#ifndef DZL_SHORTCUT_CHORD_H
#define DZL_SHORTCUT_CHORD_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef enum
{
  DZL_SHORTCUT_MATCH_NONE,
  DZL_SHORTCUT_MATCH_EQUAL,
  DZL_SHORTCUT_MATCH_PARTIAL
} DzlShortcutMatch;

#define DZL_TYPE_SHORTCUT_CHORD       (dzl_shortcut_chord_get_type())
#define DZL_TYPE_SHORTCUT_CHORD_TABLE (dzl_shortcut_chord_table_get_type())
#define DZL_TYPE_SHORTCUT_MATCH       (dzl_shortcut_match_get_type())

typedef struct _DzlShortcutChord      DzlShortcutChord;
typedef struct _DzlShortcutChordTable DzlShortcutChordTable;

typedef void (*DzlShortcutChordTableForeach) (const DzlShortcutChord *chord,
                                              gpointer                chord_data,
                                              gpointer                user_data);

GType                   dzl_shortcut_chord_get_type            (void);
DzlShortcutChord       *dzl_shortcut_chord_new_from_event      (const GdkEventKey           *event);
DzlShortcutChord       *dzl_shortcut_chord_new_from_string     (const gchar                 *accelerator);
gchar                  *dzl_shortcut_chord_to_string           (const DzlShortcutChord      *self);
gchar                  *dzl_shortcut_chord_get_label           (const DzlShortcutChord      *self);
guint                   dzl_shortcut_chord_get_length          (const DzlShortcutChord      *self);
void                    dzl_shortcut_chord_get_nth_key         (const DzlShortcutChord      *self,
                                                                guint                        nth,
                                                                guint                       *keyval,
                                                                GdkModifierType             *modifier);
gboolean                dzl_shortcut_chord_has_modifier        (const DzlShortcutChord      *self);
gboolean                dzl_shortcut_chord_append_event        (DzlShortcutChord            *self,
                                                                const GdkEventKey           *event);
DzlShortcutMatch        dzl_shortcut_chord_match               (const DzlShortcutChord      *self,
                                                                const DzlShortcutChord      *other);
guint                   dzl_shortcut_chord_hash                (gconstpointer                data);
gboolean                dzl_shortcut_chord_equal               (gconstpointer                data1,
                                                                gconstpointer                data2);
DzlShortcutChord       *dzl_shortcut_chord_copy                (const DzlShortcutChord      *self);
void                    dzl_shortcut_chord_free                (DzlShortcutChord            *self);
GType                   dzl_shortcut_chord_table_get_type      (void);
DzlShortcutChordTable  *dzl_shortcut_chord_table_new           (void);
void                    dzl_shortcut_chord_table_set_free_func (DzlShortcutChordTable       *self,
                                                                GDestroyNotify               notify);
void                    dzl_shortcut_chord_table_free          (DzlShortcutChordTable       *self);
void                    dzl_shortcut_chord_table_add           (DzlShortcutChordTable       *self,
                                                                const DzlShortcutChord      *chord,
                                                                gpointer                     data);
gboolean                dzl_shortcut_chord_table_remove        (DzlShortcutChordTable       *self,
                                                                const DzlShortcutChord      *chord);
gboolean                dzl_shortcut_chord_table_remove_data   (DzlShortcutChordTable       *self,
                                                                gpointer                     data);
DzlShortcutMatch        dzl_shortcut_chord_table_lookup        (DzlShortcutChordTable       *self,
                                                                const DzlShortcutChord      *chord,
                                                                gpointer                    *data);
const DzlShortcutChord *dzl_shortcut_chord_table_lookup_data   (DzlShortcutChordTable       *self,
                                                                gpointer                     data);
guint                   dzl_shortcut_chord_table_size          (const DzlShortcutChordTable *self);
void                    dzl_shortcut_chord_table_printf        (const DzlShortcutChordTable *self);
void                    dzl_shortcut_chord_table_foreach       (const DzlShortcutChordTable  *self,
                                                                DzlShortcutChordTableForeach  foreach_func,
                                                                gpointer                      foreach_data);
GType                   dzl_shortcut_match_get_type            (void);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (DzlShortcutChord, dzl_shortcut_chord_free)

G_END_DECLS

#endif /* DZL_SHORTCUT_CHORD_H */
