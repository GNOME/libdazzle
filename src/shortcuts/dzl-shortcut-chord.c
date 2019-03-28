/* dzl-shortcut-chord.c
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

#define G_LOG_DOMAIN "dzl-shortcut-chord"

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "shortcuts/dzl-shortcut-chord.h"
#include "shortcuts/dzl-shortcut-private.h"
#include "util/dzl-macros.h"

#define MAX_CHORD_SIZE 4
#define CHORD_MAGIC    0x83316672

G_DEFINE_BOXED_TYPE (DzlShortcutChord, dzl_shortcut_chord,
                     dzl_shortcut_chord_copy, dzl_shortcut_chord_free)
G_DEFINE_POINTER_TYPE (DzlShortcutChordTable, dzl_shortcut_chord_table)

typedef struct
{
  guint           keyval;
  GdkModifierType modifier;
} DzlShortcutKey;

struct _DzlShortcutChord
{
  DzlShortcutKey keys[MAX_CHORD_SIZE];
  guint magic;
};

typedef struct
{
  DzlShortcutChord chord;
  gpointer data;
} DzlShortcutChordTableEntry;

struct _DzlShortcutChordTable
{
  DzlShortcutChordTableEntry *entries;
  GDestroyNotify              destroy;
  guint                       len;
  guint                       size;
};

static inline gboolean
IS_SHORTCUT_CHORD (const DzlShortcutChord *chord)
{
  return chord != NULL && chord->magic == CHORD_MAGIC;
}

static GdkModifierType
sanitize_modifier_mask (GdkModifierType mods)
{
  mods &= gtk_accelerator_get_default_mod_mask ();
  mods &= ~GDK_LOCK_MASK;

  return mods;
}

static gint
dzl_shortcut_chord_compare (const DzlShortcutChord *a,
                            const DzlShortcutChord *b)
{
  g_assert (IS_SHORTCUT_CHORD (a));
  g_assert (IS_SHORTCUT_CHORD (b));

  return memcmp (a, b, sizeof *a);
}

static gboolean
dzl_shortcut_chord_is_valid (const DzlShortcutChord *self)
{
  g_assert (IS_SHORTCUT_CHORD (self));

  /* Ensure we got a valid first key at least */
  if (self->keys[0].keyval == 0 && self->keys[0].modifier == 0)
    return FALSE;

  return TRUE;
}

DzlShortcutChord *
dzl_shortcut_chord_new_from_event (const GdkEventKey *key)
{
  DzlShortcutChord *self;

  g_return_val_if_fail (key != NULL, NULL);

  self = g_slice_new0 (DzlShortcutChord);
  self->magic = CHORD_MAGIC;

  self->keys[0].keyval = gdk_keyval_to_lower (key->keyval);
  self->keys[0].modifier = sanitize_modifier_mask (key->state);

  if ((key->state & GDK_SHIFT_MASK) != 0 &&
      self->keys[0].keyval == key->keyval)
    self->keys[0].modifier &= ~GDK_SHIFT_MASK;

  if ((key->state & GDK_LOCK_MASK) == 0 &&
      self->keys[0].keyval != key->keyval)
    self->keys[0].modifier |= GDK_SHIFT_MASK;

  if (!dzl_shortcut_chord_is_valid (self))
    g_clear_pointer (&self, dzl_shortcut_chord_free);

  return self;
}

DzlShortcutChord *
dzl_shortcut_chord_new_from_string (const gchar *accelerator)
{
  DzlShortcutChord *self;
  g_auto(GStrv) parts = NULL;

  g_return_val_if_fail (accelerator != NULL, NULL);

  /* We might have a single key, or chord defined */
  parts = g_strsplit (accelerator, "|", 0);

  /* Make sure we won't overflow the keys array */
  if (g_strv_length (parts) > G_N_ELEMENTS (self->keys))
    return NULL;

  self = g_slice_new0 (DzlShortcutChord);
  self->magic = CHORD_MAGIC;

  /* Parse each section from the accelerator */
  for (guint i = 0; parts[i]; i++)
    gtk_accelerator_parse (parts[i], &self->keys[i].keyval, &self->keys[i].modifier);

  /* Ensure we got a valid first key at least */
  if (!dzl_shortcut_chord_is_valid (self))
    g_clear_pointer (&self, dzl_shortcut_chord_free);

  return self;
}

gboolean
dzl_shortcut_chord_append_event (DzlShortcutChord  *self,
                                 const GdkEventKey *key)
{
  guint i;

  g_return_val_if_fail (IS_SHORTCUT_CHORD (self), FALSE);
  g_return_val_if_fail (key != NULL, FALSE);

  for (i = 0; i < G_N_ELEMENTS (self->keys); i++)
    {
      /* We might have just a state (control, etc), and we want
       * to consume that if we've gotten another key here. So we
       * only check against key.keyval.
       */
      if (self->keys[i].keyval == 0)
        {
          self->keys[i].keyval = gdk_keyval_to_lower (key->keyval);
          self->keys[i].modifier = sanitize_modifier_mask (key->state);

          if ((key->state & GDK_LOCK_MASK) == 0 &&
              self->keys[i].keyval != key->keyval)
            self->keys[i].modifier |= GDK_SHIFT_MASK;

          return TRUE;
        }
    }

  return FALSE;
}

static inline guint
dzl_shortcut_chord_count_keys (const DzlShortcutChord *self)
{
  guint count = 0;

  g_assert (IS_SHORTCUT_CHORD (self));

  for (guint i = 0; i < G_N_ELEMENTS (self->keys); i++)
    {
      if (self->keys[i].keyval != 0)
        count++;
      else
        break;
    }

  return count;
}

DzlShortcutMatch
dzl_shortcut_chord_match (const DzlShortcutChord *self,
                          const DzlShortcutChord *other)
{
  guint self_count = 0;
  guint other_count = 0;

  g_return_val_if_fail (IS_SHORTCUT_CHORD (self), DZL_SHORTCUT_MATCH_NONE);
  g_return_val_if_fail (other != NULL, DZL_SHORTCUT_MATCH_NONE);

  self_count = dzl_shortcut_chord_count_keys (self);
  other_count = dzl_shortcut_chord_count_keys (other);

  if (self_count > other_count)
    return DZL_SHORTCUT_MATCH_NONE;

  if (0 == memcmp (self->keys, other->keys, sizeof (DzlShortcutKey) * self_count))
    return self_count == other_count ? DZL_SHORTCUT_MATCH_EQUAL : DZL_SHORTCUT_MATCH_PARTIAL;

  return DZL_SHORTCUT_MATCH_NONE;
}

gchar *
dzl_shortcut_chord_to_string (const DzlShortcutChord *self)
{
  GString *str;

  g_assert (IS_SHORTCUT_CHORD (self));

  if (self == NULL || self->keys[0].keyval == 0)
    return NULL;

  str = g_string_new (NULL);

  for (guint i = 0; i < G_N_ELEMENTS (self->keys); i++)
    {
      const DzlShortcutKey *key = &self->keys[i];
      g_autofree gchar *name = NULL;

      if (key->keyval == 0 && key->modifier == 0)
        break;

      name = gtk_accelerator_name (key->keyval, key->modifier);

      if (i != 0)
        g_string_append_c (str, '|');

      g_string_append (str, name);
    }

  return g_string_free (str, FALSE);
}

gchar *
dzl_shortcut_chord_get_label (const DzlShortcutChord *self)
{
  GString *str;

  if (self == NULL || self->keys[0].keyval == 0)
    return NULL;

  g_return_val_if_fail (IS_SHORTCUT_CHORD (self), NULL);

  str = g_string_new (NULL);

  for (guint i = 0; i < G_N_ELEMENTS (self->keys); i++)
    {
      const DzlShortcutKey *key = &self->keys[i];
      g_autofree gchar *name = NULL;

      if (key->keyval == 0 && key->modifier == 0)
        break;

      name = gtk_accelerator_get_label (key->keyval, key->modifier);

      if (i != 0)
        g_string_append_c (str, ' ');

      g_string_append (str, name);
    }

  return g_string_free (str, FALSE);
}

DzlShortcutChord *
dzl_shortcut_chord_copy (const DzlShortcutChord *self)
{
  if (self == NULL)
    return NULL;

  return g_slice_dup (DzlShortcutChord, self);
}

guint
dzl_shortcut_chord_hash (gconstpointer data)
{
  const DzlShortcutChord *self = data;
  guint hash = 0;

  g_assert (IS_SHORTCUT_CHORD (self));

  for (guint i = 0; i < G_N_ELEMENTS (self->keys); i++)
    {
      const DzlShortcutKey *key = &self->keys[i];

      hash ^= key->keyval;
      hash ^= key->modifier;
    }

  return hash;
}

gboolean
dzl_shortcut_chord_equal (gconstpointer data1,
                          gconstpointer data2)
{
  if (data1 == data2)
    return TRUE;
  else if (data1 == NULL || data2 == NULL)
    return FALSE;

  return 0 == memcmp (((const DzlShortcutChord *)data1)->keys,
                      ((const DzlShortcutChord *)data2)->keys,
                      sizeof (DzlShortcutChord));
}

void
dzl_shortcut_chord_free (DzlShortcutChord *self)
{
  g_assert (!self || IS_SHORTCUT_CHORD (self));

  if (self != NULL)
    {
      self->magic = 0xAAAAAAAA;
      g_slice_free (DzlShortcutChord, self);
    }
}

GType
dzl_shortcut_match_get_type (void)
{
  static GType type_id;

  if (g_once_init_enter (&type_id))
    {
      static GEnumValue values[] = {
        { DZL_SHORTCUT_MATCH_NONE, "DZL_SHORTCUT_MATCH_NONE", "none" },
        { DZL_SHORTCUT_MATCH_EQUAL, "DZL_SHORTCUT_MATCH_EQUAL", "equal" },
        { DZL_SHORTCUT_MATCH_PARTIAL, "DZL_SHORTCUT_MATCH_PARTIAL", "partial" },
        { 0 }
      };
      GType _type_id = g_enum_register_static ("DzlShortcutMatch", values);
      g_once_init_leave (&type_id, _type_id);
    }

  return type_id;
}

static gint
dzl_shortcut_chord_table_sort (gconstpointer a,
                               gconstpointer b)
{
  const DzlShortcutChordTableEntry *keya = a;
  const DzlShortcutChordTableEntry *keyb = b;

  g_assert (IS_SHORTCUT_CHORD (a));
  g_assert (IS_SHORTCUT_CHORD (b));

  return dzl_shortcut_chord_compare (&keya->chord, &keyb->chord);
}

/**
 * dzl_shortcut_chord_table_new: (skip)
 */
DzlShortcutChordTable *
dzl_shortcut_chord_table_new (void)
{
  DzlShortcutChordTable *table;

  table = g_slice_new0 (DzlShortcutChordTable);
  table->len = 0;
  table->size = 4;
  table->destroy = NULL;
  table->entries = g_new0 (DzlShortcutChordTableEntry, table->size);

  return table;
}

void
dzl_shortcut_chord_table_free (DzlShortcutChordTable *self)
{
  if (self != NULL)
    {
      if (self->destroy != NULL)
        {
          for (guint i = 0; i < self->len; i++)
            self->destroy (self->entries[i].data);
        }
      g_free (self->entries);
      g_slice_free (DzlShortcutChordTable, self);
    }
}

void
dzl_shortcut_chord_table_add (DzlShortcutChordTable  *self,
                              const DzlShortcutChord *chord,
                              gpointer                data)
{
  g_return_if_fail (self != NULL);
  g_return_if_fail (chord != NULL);

  if (self->len == self->size)
    {
      self->size *= 2;
      self->entries = g_renew (DzlShortcutChordTableEntry, self->entries, self->size);
    }

  self->entries[self->len].chord = *chord;
  self->entries[self->len].data = data;

  self->len++;

  qsort (self->entries,
         self->len,
         sizeof (DzlShortcutChordTableEntry),
         dzl_shortcut_chord_table_sort);
}

static void
dzl_shortcut_chord_table_remove_index (DzlShortcutChordTable *self,
                                       guint                  position)
{
  DzlShortcutChordTableEntry *entry;
  gpointer data;

  g_assert (self != NULL);
  g_assert (position < self->len);

  entry = &self->entries[position];
  data = g_steal_pointer (&entry->data);

  if (position + 1 < self->len)
    memmove ((gpointer)entry,
             entry + 1,
             sizeof *entry * (self->len - position - 1));

  self->len--;

  if (self->destroy != NULL)
    self->destroy (data);
}

gboolean
dzl_shortcut_chord_table_remove (DzlShortcutChordTable  *self,
                                 const DzlShortcutChord *chord)
{
  g_return_val_if_fail (self != NULL, FALSE);

  if (chord == NULL)
    return FALSE;

  for (guint i = 0; i < self->len; i++)
    {
      const DzlShortcutChordTableEntry *ele = &self->entries[i];

      if (dzl_shortcut_chord_equal (&ele->chord, chord))
        {
          dzl_shortcut_chord_table_remove_index (self, i);
          return TRUE;
        }
    }

  return FALSE;
}

gboolean
dzl_shortcut_chord_table_remove_data (DzlShortcutChordTable *self,
                                      gpointer               data)
{
  g_return_val_if_fail (self != NULL, FALSE);

  for (guint i = 0; i < self->len; i++)
    {
      const DzlShortcutChordTableEntry *ele = &self->entries[i];

      if (ele->data == data)
        {
          dzl_shortcut_chord_table_remove_index (self, i);
          return TRUE;
        }
    }

  return FALSE;
}

const DzlShortcutChord *
dzl_shortcut_chord_table_lookup_data (DzlShortcutChordTable *self,
                                      gpointer               data)
{
  if (self == NULL)
    return NULL;

  for (guint i = 0; i < self->len; i++)
    {
      const DzlShortcutChordTableEntry *ele = &self->entries[i];

      if (ele->data == data)
        return &ele->chord;
    }

  return NULL;
}

static gint
dzl_shortcut_chord_find_partial (gconstpointer a,
                                 gconstpointer b)
{
  const DzlShortcutChord *key = a;
  const DzlShortcutChordTableEntry *element = b;

  /*
   * We are only looking for a partial match here so that we can walk backwards
   * after the bsearch to the first partial match.
   */
  if (dzl_shortcut_chord_match (key, &element->chord) != DZL_SHORTCUT_MATCH_NONE)
    return 0;

  return dzl_shortcut_chord_compare (key, &element->chord);
}

DzlShortcutMatch
dzl_shortcut_chord_table_lookup (DzlShortcutChordTable  *self,
                                 const DzlShortcutChord *chord,
                                 gpointer               *data)
{
  const DzlShortcutChordTableEntry *match;

  if (data != NULL)
    *data = NULL;

  if (self == NULL)
    return DZL_SHORTCUT_MATCH_NONE;

  if (chord == NULL)
    return DZL_SHORTCUT_MATCH_NONE;

  if (self->len == 0)
    return DZL_SHORTCUT_MATCH_NONE;

  /*
   * This function works by performing a binary search to locate ourself
   * somewhere within a match zone of the array. Once we are there, we walk
   * back to the first item that is a partial match.  After that, we walk
   * through every potential match looking for an exact match until we reach a
   * non-partial-match or the end of the array.
   *
   * Based on our findings, we return the appropriate DzlShortcutMatch.
   */

  match = bsearch (chord, self->entries, self->len, sizeof (DzlShortcutChordTableEntry),
                   dzl_shortcut_chord_find_partial);

  if (match != NULL)
    {
      const DzlShortcutChordTableEntry *begin = self->entries;
      const DzlShortcutChordTableEntry *end = self->entries + self->len;
      DzlShortcutMatch ret = DZL_SHORTCUT_MATCH_PARTIAL;

      /* Find the first patial match */
      while ((match - 1) >= begin &&
             dzl_shortcut_chord_match (chord, &(match - 1)->chord) != DZL_SHORTCUT_MATCH_NONE)
        match--;

      g_assert (match >= begin);

      /* Now walk forward to see if we have an exact match */
      while (DZL_SHORTCUT_MATCH_NONE != (ret = dzl_shortcut_chord_match (chord, &match->chord)))
        {
          if (ret == DZL_SHORTCUT_MATCH_EQUAL)
            {
              if (data != NULL)
                *data = match->data;
              return DZL_SHORTCUT_MATCH_EQUAL;
            }

          match++;

          g_assert (match <= end);

          if (ret == 0 || match == end)
            break;
        }

      return DZL_SHORTCUT_MATCH_PARTIAL;
    }

  return DZL_SHORTCUT_MATCH_NONE;
}

void
dzl_shortcut_chord_table_set_free_func (DzlShortcutChordTable *self,
                                        GDestroyNotify         destroy)
{
  g_return_if_fail (self != NULL);

  self->destroy = destroy;
}

guint
dzl_shortcut_chord_table_size (const DzlShortcutChordTable *self)
{
  /* I know this is confusing, but len is the number of items in the
   * table (which consumers think of as size), and @size is the allocated
   * size of the ^2 growing array.
   */
  return self ? self->len : 0;
}

void
dzl_shortcut_chord_table_printf (const DzlShortcutChordTable *self)
{
  if (self == NULL)
    return;

  for (guint i = 0; i < self->len; i++)
    {
      const DzlShortcutChordTableEntry *entry = &self->entries[i];
      g_autofree gchar *str = dzl_shortcut_chord_to_string (&entry->chord);

      g_print ("%s\n", str);
    }
}

void
_dzl_shortcut_chord_table_iter_init (DzlShortcutChordTableIter *iter,
                                     DzlShortcutChordTable     *table)
{
  g_return_if_fail (iter != NULL);

  iter->table = table;
  iter->position = 0;
}

gboolean
_dzl_shortcut_chord_table_iter_next (DzlShortcutChordTableIter  *iter,
                                     const DzlShortcutChord    **chord,
                                     gpointer                   *value)
{
  g_return_val_if_fail (iter != NULL, FALSE);

  /*
   * Be safe against NULL tables which we allow in
   * _dzl_shortcut_chord_table_iter_init() for convenience.
   */
  if (iter->table == NULL)
    return FALSE;

  if (iter->position < iter->table->len)
    {
      *chord = &iter->table->entries[iter->position].chord;
      *value = iter->table->entries[iter->position].data;
      iter->position++;
      return TRUE;
    }

  return FALSE;
}

void
_dzl_shortcut_chord_table_iter_steal (DzlShortcutChordTableIter  *iter)
{
  g_return_if_fail (iter != NULL);
  g_return_if_fail (iter->table != NULL);

  if (iter->position > 0 && iter->position < iter->table->len)
    {
      dzl_shortcut_chord_table_remove_index (iter->table, --iter->position);
      return;
    }

  g_warning ("Attempt to steal item from table that does not exist");
}

gboolean
dzl_shortcut_chord_has_modifier (const DzlShortcutChord *self)
{
  g_return_val_if_fail (self != NULL, FALSE);

  return self->keys[0].modifier != 0;
}

guint
dzl_shortcut_chord_get_length (const DzlShortcutChord *self)
{
  if (self != NULL)
    {
      for (guint i = 0; i < G_N_ELEMENTS (self->keys); i++)
        {
          if (self->keys[i].keyval == 0)
            return i;
        }

      return G_N_ELEMENTS (self->keys);
    }

  return 0;
}

void
dzl_shortcut_chord_get_nth_key (const DzlShortcutChord *self,
                                guint                   nth,
                                guint                  *keyval,
                                GdkModifierType        *modifier)
{
  if (nth < G_N_ELEMENTS (self->keys))
    {
      if (keyval)
        *keyval = self->keys[nth].keyval;
      if (modifier)
        *modifier = self->keys[nth].modifier;
    }
  else
    {
      if (keyval)
        *keyval = 0;
      if (modifier)
        *modifier = 0;
    }
}

/**
 * dzl_shortcut_chord_table_foreach:
 * @self: a #DzlShortcutChordTable
 * @foreach_func: (scope call) (closure foreach_data): A callback for each chord
 * @foreach_data: user data for @foreach_func
 *
 * This function will call @foreach_func for each chord in the table.
 */
void
dzl_shortcut_chord_table_foreach (const DzlShortcutChordTable  *self,
                                  DzlShortcutChordTableForeach  foreach_func,
                                  gpointer                      foreach_data)
{
  g_return_if_fail (foreach_func != NULL);

  if (self == NULL)
    return;

  /*
   * Walk backwards just in case the caller somehow thinks it is okay to
   * remove items while iterating the list. We don't officially support that
   * (which is why self is const), but this is just defensive.
   */

  for (guint i = self->len; i > 0; i--)
    {
      const DzlShortcutChordTableEntry *entry = &self->entries[i-1];

      foreach_func (&entry->chord, entry->data, foreach_data);
    }
}
