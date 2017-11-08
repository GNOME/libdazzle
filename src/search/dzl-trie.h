/* dzl-trie.h
 *
 * Copyright (C) 2012 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DZL_TRIE_H
#define DZL_TRIE_H

#include <glib-object.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_TRIE (dzl_trie_get_type())

typedef struct _DzlTrie DzlTrie;

typedef gboolean (*DzlTrieTraverseFunc) (DzlTrie     *dzl_trie,
                                         const gchar *key,
                                         gpointer     value,
                                         gpointer     user_data);

DZL_AVAILABLE_IN_ALL
GType     dzl_trie_get_type (void);
DZL_AVAILABLE_IN_ALL
void      dzl_trie_destroy  (DzlTrie             *trie);
DZL_AVAILABLE_IN_ALL
void      dzl_trie_unref    (DzlTrie             *trie);
DZL_AVAILABLE_IN_ALL
DzlTrie  *dzl_trie_ref      (DzlTrie             *trie);
DZL_AVAILABLE_IN_ALL
void      dzl_trie_insert   (DzlTrie             *trie,
                             const gchar         *key,
                             gpointer             value);
DZL_AVAILABLE_IN_ALL
gpointer  dzl_trie_lookup   (DzlTrie             *trie,
                             const gchar         *key);
DZL_AVAILABLE_IN_ALL
DzlTrie  *dzl_trie_new      (GDestroyNotify       value_destroy);
DZL_AVAILABLE_IN_ALL
gboolean  dzl_trie_remove   (DzlTrie             *trie,
                             const gchar         *key);
DZL_AVAILABLE_IN_ALL
void      dzl_trie_traverse (DzlTrie             *trie,
                             const gchar         *key,
                             GTraverseType        order,
                             GTraverseFlags       flags,
                             gint                 max_depth,
                             DzlTrieTraverseFunc  func,
                             gpointer             user_data);

G_END_DECLS

#endif /* DZL_TRIE_H */
