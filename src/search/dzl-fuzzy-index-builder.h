/* dzl-fuzzy-index-builder.h
 *
 * Copyright (C) 2016 Christian Hergert <christian@hergert.me>
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

#ifndef DZL_FUZZY_INDEX_BUILDER_H
#define DZL_FUZZY_INDEX_BUILDER_H

#include <gio/gio.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_FUZZY_INDEX_BUILDER (dzl_fuzzy_index_builder_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlFuzzyIndexBuilder, dzl_fuzzy_index_builder, DZL, FUZZY_INDEX_BUILDER, GObject)

DZL_AVAILABLE_IN_ALL
DzlFuzzyIndexBuilder *dzl_fuzzy_index_builder_new                 (void);
DZL_AVAILABLE_IN_ALL
gboolean              dzl_fuzzy_index_builder_get_case_sensitive  (DzlFuzzyIndexBuilder  *self);
DZL_AVAILABLE_IN_ALL
void                  dzl_fuzzy_index_builder_set_case_sensitive  (DzlFuzzyIndexBuilder  *self,
                                                                   gboolean               case_sensitive);
DZL_AVAILABLE_IN_ALL
guint64               dzl_fuzzy_index_builder_insert              (DzlFuzzyIndexBuilder  *self,
                                                                   const gchar           *key,
                                                                   GVariant              *document,
                                                                   guint                  priority);
DZL_AVAILABLE_IN_ALL
gboolean              dzl_fuzzy_index_builder_write               (DzlFuzzyIndexBuilder  *self,
                                                                   GFile                 *file,
                                                                   gint                   io_priority,
                                                                   GCancellable          *cancellable,
                                                                   GError               **error);
DZL_AVAILABLE_IN_ALL
void                  dzl_fuzzy_index_builder_write_async         (DzlFuzzyIndexBuilder  *self,
                                                                   GFile                 *file,
                                                                   gint                   io_priority,
                                                                   GCancellable          *cancellable,
                                                                   GAsyncReadyCallback    callback,
                                                                   gpointer               user_data);
DZL_AVAILABLE_IN_ALL
gboolean              dzl_fuzzy_index_builder_write_finish        (DzlFuzzyIndexBuilder  *self,
                                                                   GAsyncResult          *result,
                                                                   GError               **error);
DZL_AVAILABLE_IN_ALL
const GVariant       *dzl_fuzzy_index_builder_get_document        (DzlFuzzyIndexBuilder  *self,
                                                                   guint64                document_id);
DZL_AVAILABLE_IN_ALL
void                  dzl_fuzzy_index_builder_set_metadata        (DzlFuzzyIndexBuilder  *self,
                                                                   const gchar           *key,
                                                                   GVariant              *value);
DZL_AVAILABLE_IN_ALL
void                  dzl_fuzzy_index_builder_set_metadata_string (DzlFuzzyIndexBuilder  *self,
                                                                   const gchar           *key,
                                                                   const gchar           *value);
DZL_AVAILABLE_IN_ALL
void                  dzl_fuzzy_index_builder_set_metadata_uint32 (DzlFuzzyIndexBuilder  *self,
                                                                   const gchar           *key,
                                                                   guint32                value);
DZL_AVAILABLE_IN_ALL
void                  dzl_fuzzy_index_builder_set_metadata_uint64 (DzlFuzzyIndexBuilder  *self,
                                                                   const gchar           *key,
                                                                   guint64                value);

G_END_DECLS

#endif /* DZL_FUZZY_INDEX_BUILDER_H */
