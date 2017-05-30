/* dzl-fuzzy-index.h
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
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

#ifndef DZL_FUZZY_INDEX_H
#define DZL_FUZZY_INDEX_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define DZL_TYPE_FUZZY_INDEX (dzl_fuzzy_index_get_type())

G_DECLARE_FINAL_TYPE (DzlFuzzyIndex, dzl_fuzzy_index, DZL, FUZZY_INDEX, GObject)

DzlFuzzyIndex  *dzl_fuzzy_index_new                 (void);
gboolean        dzl_fuzzy_index_load_file           (DzlFuzzyIndex        *self,
                                                     GFile                *file,
                                                     GCancellable         *cancellable,
                                                     GError              **error);
void            dzl_fuzzy_index_load_file_async     (DzlFuzzyIndex        *self,
                                                     GFile                *file,
                                                     GCancellable         *cancellable,
                                                     GAsyncReadyCallback   callback,
                                                     gpointer              user_data);
gboolean        dzl_fuzzy_index_load_file_finish    (DzlFuzzyIndex        *self,
                                                     GAsyncResult         *result,
                                                     GError              **error);
void            dzl_fuzzy_index_query_async         (DzlFuzzyIndex        *self,
                                                     const gchar          *query,
                                                     guint                 max_matches,
                                                     GCancellable         *cancellable,
                                                     GAsyncReadyCallback   callback,
                                                     gpointer              user_data);
GListModel     *dzl_fuzzy_index_query_finish        (DzlFuzzyIndex        *self,
                                                     GAsyncResult         *result,
                                                     GError              **error);
GVariant       *dzl_fuzzy_index_get_metadata        (DzlFuzzyIndex        *self,
                                                     const gchar          *key);
guint32         dzl_fuzzy_index_get_metadata_uint32 (DzlFuzzyIndex        *self,
                                                     const gchar          *key);
guint64         dzl_fuzzy_index_get_metadata_uint64 (DzlFuzzyIndex        *self,
                                                     const gchar          *key);
const gchar    *dzl_fuzzy_index_get_metadata_string (DzlFuzzyIndex        *self,
                                                     const gchar          *key);

G_END_DECLS

#endif /* DZL_FUZZY_INDEX_H */
