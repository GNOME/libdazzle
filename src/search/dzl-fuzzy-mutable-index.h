/* fuzzy.h
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#ifndef DZL_FUZZY_MUTABLE_INDEX_H
#define DZL_FUZZY_MUTABLE_INDEX_H

#include <glib-object.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_FUZZY_MUTABLE_INDEX (dzl_fuzzy_mutable_index_get_type())

typedef struct _DzlFuzzyMutableIndex DzlFuzzyMutableIndex;

typedef struct
{
   const gchar *key;
   gpointer     value;
   gfloat       score;
   guint        id;
} DzlFuzzyMutableIndexMatch;

DZL_AVAILABLE_IN_ALL
GType                     dzl_fuzzy_mutable_index_get_type           (void);
DZL_AVAILABLE_IN_ALL
DzlFuzzyMutableIndex     *dzl_fuzzy_mutable_index_new                (gboolean              case_sensitive);
DZL_AVAILABLE_IN_ALL
DzlFuzzyMutableIndex     *dzl_fuzzy_mutable_index_new_with_free_func (gboolean              case_sensitive,
                                                                      GDestroyNotify        free_func);
DZL_AVAILABLE_IN_ALL
void                      dzl_fuzzy_mutable_index_set_free_func      (DzlFuzzyMutableIndex *fuzzy,
                                                                      GDestroyNotify        free_func);
DZL_AVAILABLE_IN_ALL
void                      dzl_fuzzy_mutable_index_begin_bulk_insert  (DzlFuzzyMutableIndex *fuzzy);
DZL_AVAILABLE_IN_ALL
void                      dzl_fuzzy_mutable_index_end_bulk_insert    (DzlFuzzyMutableIndex *fuzzy);
DZL_AVAILABLE_IN_ALL
gboolean                  dzl_fuzzy_mutable_index_contains           (DzlFuzzyMutableIndex *fuzzy,
                                                                      const gchar          *key);
DZL_AVAILABLE_IN_ALL
void                      dzl_fuzzy_mutable_index_insert             (DzlFuzzyMutableIndex *fuzzy,
                                                                      const gchar          *key,
                                                                      gpointer              value);
DZL_AVAILABLE_IN_ALL
GArray                   *dzl_fuzzy_mutable_index_match              (DzlFuzzyMutableIndex *fuzzy,
                                                                      const gchar          *needle,
                                                                      gsize                 max_matches);
DZL_AVAILABLE_IN_ALL
void                      dzl_fuzzy_mutable_index_remove             (DzlFuzzyMutableIndex *fuzzy,
                                                                      const gchar          *key);
DZL_AVAILABLE_IN_ALL
DzlFuzzyMutableIndex     *dzl_fuzzy_mutable_index_ref                (DzlFuzzyMutableIndex *fuzzy);
DZL_AVAILABLE_IN_ALL
void                      dzl_fuzzy_mutable_index_unref              (DzlFuzzyMutableIndex *fuzzy);
DZL_AVAILABLE_IN_ALL
gchar                    *dzl_fuzzy_highlight                        (const gchar          *str,
                                                                      const gchar          *query,
                                                                      gboolean              case_sensitive);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (DzlFuzzyMutableIndex, dzl_fuzzy_mutable_index_unref)

G_END_DECLS

#endif /* DZL_FUZZY_MUTABLE_INDEX_H */
