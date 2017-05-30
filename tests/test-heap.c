/* test-dzl-heap.c
 *
 * Copyright (C) 2014 Christian Hergert <christian@hergert.me>
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

#include <dazzle.h>

typedef struct
{
   gint64 size;
   gpointer pointer;
} Tuple;

static int
cmpint_rev (gconstpointer a,
            gconstpointer b)
{
   return *(const gint *)b - *(const gint *)a;
}

static int
cmpptr_rev (gconstpointer a,
            gconstpointer b)
{
   return GPOINTER_TO_SIZE(*(gpointer *)b) - GPOINTER_TO_SIZE (*(gpointer *)a);
}

static int
cmptuple_rev (gconstpointer a,
              gconstpointer b)
{
   Tuple *at = (Tuple *)a;
   Tuple *bt = (Tuple *)b;

   return bt->size - at->size;
}

static void
test_DzlHeap_insert_val_int (void)
{
   DzlHeap *heap;
   gint i;

   heap = dzl_heap_new (sizeof (gint), cmpint_rev);

   for (i = 0; i < 100000; i++) {
      dzl_heap_insert_val (heap, i);
      g_assert_cmpint (heap->len, ==, i + 1);
   }

   for (i = 0; i < 100000; i++) {
      g_assert_cmpint (heap->len, ==, 100000 - i);
      g_assert_cmpint (dzl_heap_peek (heap, gint), ==, i);
      dzl_heap_extract (heap, NULL);
   }

   dzl_heap_unref (heap);
}

static void
test_DzlHeap_insert_val_ptr (void)
{
   gconstpointer ptr;
   DzlHeap *heap;
   gint i;

   heap = dzl_heap_new (sizeof (gpointer), cmpptr_rev);

   for (i = 0; i < 100000; i++) {
      ptr = GINT_TO_POINTER (i);
      dzl_heap_insert_val (heap, ptr);
      g_assert_cmpint (heap->len, ==, i + 1);
   }

   for (i = 0; i < 100000; i++) {
      g_assert_cmpint (heap->len, ==, 100000 - i);
      g_assert (dzl_heap_peek (heap, gpointer) == GINT_TO_POINTER (i));
      dzl_heap_extract (heap, NULL);
   }

   dzl_heap_unref (heap);
}

static void
test_DzlHeap_insert_val_tuple (void)
{
   Tuple t;
   DzlHeap *heap;
   gint i;

   heap = dzl_heap_new (sizeof (Tuple), cmptuple_rev);

   for (i = 0; i < 100000; i++) {
      t.pointer = GINT_TO_POINTER (i);
      t.size = i;
      dzl_heap_insert_val (heap, t);
      g_assert_cmpint (heap->len, ==, i + 1);
   }

   for (i = 0; i < 100000; i++) {
      g_assert_cmpint (heap->len, ==, 100000 - i);
      g_assert (dzl_heap_peek (heap, Tuple).size == i);
      g_assert (dzl_heap_peek (heap, Tuple).pointer == GINT_TO_POINTER (i));
      dzl_heap_extract (heap, NULL);
   }

   dzl_heap_unref (heap);
}

static void
test_DzlHeap_extract_int (void)
{
   DzlHeap *heap;
   gint removed[5];
   gint i;
   gint v;

   heap = dzl_heap_new (sizeof (gint), cmpint_rev);

   for (i = 0; i < 100000; i++) {
      dzl_heap_insert_val (heap, i);
   }

   removed [0] = dzl_heap_index (heap, gint, 1578); dzl_heap_extract_index (heap, 1578, NULL);
   removed [1] = dzl_heap_index (heap, gint, 2289); dzl_heap_extract_index (heap, 2289, NULL);
   removed [2] = dzl_heap_index (heap, gint, 3312); dzl_heap_extract_index (heap, 3312, NULL);
   removed [3] = dzl_heap_index (heap, gint, 78901); dzl_heap_extract_index (heap, 78901, NULL);
   removed [4] = dzl_heap_index (heap, gint, 99000); dzl_heap_extract_index (heap, 99000, NULL);

   for (i = 0; i < 100000; i++) {
      if (dzl_heap_peek (heap, gint) != i) {
         g_assert ((i == removed[0]) ||
                   (i == removed[1]) ||
                   (i == removed[2]) ||
                   (i == removed[3]) ||
                   (i == removed[4]));
      } else {
         dzl_heap_extract (heap, &v);
         g_assert_cmpint (v, ==, i);
      }
   }

   g_assert_cmpint (heap->len, ==, 0);

   dzl_heap_unref (heap);
}

int
main (gint   argc,
      gchar *argv[])
{
   g_test_init (&argc, &argv, NULL);

   g_test_add_func ("/Dazzle/Heap/insert_and_extract<gint>", test_DzlHeap_insert_val_int);
   g_test_add_func ("/Dazzle/Heap/insert_and_extract<gpointer>", test_DzlHeap_insert_val_ptr);
   g_test_add_func ("/Dazzle/Heap/insert_and_extract<Tuple>", test_DzlHeap_insert_val_tuple);
   g_test_add_func ("/Dazzle/Heap/extract_index<int>", test_DzlHeap_extract_int);

   return g_test_run ();
}
