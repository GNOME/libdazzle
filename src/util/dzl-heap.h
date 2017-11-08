/* dzl-heap.h
 *
 * Copyright (C) 2014-2015 Christian Hergert <christian@hergert.me>
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

#ifndef DZL_HEAP_H
#define DZL_HEAP_H

#include <glib-object.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_HEAP            (dzl_heap_get_type())
#define dzl_heap_insert_val(h,v) dzl_heap_insert_vals(h,&(v),1)
#define dzl_heap_index(h,t,i)    (((t*)(void*)(h)->data)[i])
#define dzl_heap_peek(h,t)       dzl_heap_index(h,t,0)

typedef struct _DzlHeap DzlHeap;

struct _DzlHeap
{
  gchar *data;
  gsize  len;
};

DZL_AVAILABLE_IN_ALL
GType      dzl_heap_get_type      (void);
DZL_AVAILABLE_IN_ALL
DzlHeap   *dzl_heap_new           (guint           element_size,
                                   GCompareFunc    compare_func);
DZL_AVAILABLE_IN_ALL
DzlHeap   *dzl_heap_ref           (DzlHeap        *heap);
DZL_AVAILABLE_IN_ALL
void       dzl_heap_unref         (DzlHeap        *heap);
DZL_AVAILABLE_IN_ALL
void       dzl_heap_insert_vals   (DzlHeap        *heap,
                                   gconstpointer   data,
                                   guint           len);
DZL_AVAILABLE_IN_ALL
gboolean   dzl_heap_extract       (DzlHeap        *heap,
                                   gpointer        result);
DZL_AVAILABLE_IN_ALL
gboolean   dzl_heap_extract_index (DzlHeap        *heap,
                                   gsize           index_,
                                   gpointer        result);

G_END_DECLS

#endif /* DZL_HEAP_H */
