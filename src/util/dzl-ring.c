/* dzl-ring.c
 *
 * Copyright (C) 2010 Christian Hergert <chris@dronelabs.com>
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

#define G_LOG_DOMAIN "dzl-ring"

#include "config.h"

#include <string.h>

#include "dzl-ring.h"

#define get_element(r,i) ((r)->data + ((r)->elt_size * i))

typedef struct
{
  /*< public >*/
  guint8          *data;      /* Pointer to real data. */
  guint            len;       /* Length of data allocation. */
  guint            pos;       /* Position in ring. */

  /*< private >*/
  guint            elt_size;  /* Size of each element. */
  gboolean         looped;    /* Have we wrapped around at least once. */
  GDestroyNotify   destroy;   /* Destroy element callback. */
  volatile gint    ref_count; /* Hidden Reference count. */
} DzlRingImpl;

G_DEFINE_BOXED_TYPE (DzlRing, dzl_ring, dzl_ring_ref, dzl_ring_unref)

/**
 * dzl_ring_sized_new:
 * @element_size: (in): The size per element.
 * @reserved_size: (in): The number of elements to allocate.
 * @element_destroy: (in): Notification called when removing an element.
 *
 * Creates a new instance of #DzlRing with the given number of elements.
 *
 * Returns: A new #DzlRing.
 */
DzlRing*
dzl_ring_sized_new (guint          element_size,
                    guint          reserved_size,
                    GDestroyNotify element_destroy)
{
  DzlRingImpl *ring_impl;

  ring_impl = g_slice_new0 (DzlRingImpl);
  ring_impl->elt_size = element_size;
  ring_impl->len = reserved_size;
  ring_impl->data = g_malloc0_n (reserved_size, element_size);
  ring_impl->destroy = element_destroy;
  ring_impl->ref_count = 1;

  return (DzlRing *)ring_impl;
}

/**
 * dzl_ring_append_vals:
 * @ring: (in): A #DzlRing.
 * @data: (in): A pointer to the array of values.
 * @len: (in): The number of values.
 *
 * Appends @len values located at @data.
 *
 * Returns: the index of the first item.
 */
guint
dzl_ring_append_vals (DzlRing        *ring,
                      gconstpointer  data,
                      guint          len)
{
  DzlRingImpl *ring_impl = (DzlRingImpl *)ring;
  gpointer idx;
  gint ret = -1;
  gint x;

  g_return_val_if_fail (ring_impl != NULL, 0);
  g_return_val_if_fail (len <= ring->len, 0);
  g_return_val_if_fail (len > 0, 0);
  g_return_val_if_fail (len <= G_MAXINT, 0);

  for (gint i = 0; i < (gint)len; i++)
    {
      x = ring->pos - i;
      x = (x >= 0) ? x : (gint)ring->len + x;
      idx = ring->data + (ring_impl->elt_size * x);
      if (ring_impl->destroy && (ring_impl->looped == TRUE))
        ring_impl->destroy (idx);
      if (ret == -1)
        ret = x;
      memcpy (idx, data, ring_impl->elt_size);
      ring->pos++;
      if (ring->pos >= ring->len)
        ring_impl->looped = TRUE;
      ring->pos %= ring->len;
      data = ((guint8 *)data) + ring_impl->elt_size;
    }

  return (guint)ret;
}

/**
 * dzl_ring_foreach:
 * @ring: (in): A #DzlRing.
 * @func: (in) (scope call): A #GFunc to call for each element.
 * @user_data: (in): user data for @func.
 *
 * Calls @func for every item in the #DzlRing starting from the most recently
 * inserted element to the least recently inserted.
 */
void
dzl_ring_foreach (DzlRing  *ring,
                  GFunc     func,
                  gpointer  user_data)
{
  DzlRingImpl *ring_impl = (DzlRingImpl *)ring;
  gint i;

  g_return_if_fail (ring_impl != NULL);
  g_return_if_fail (func != NULL);

  if (!ring_impl->looped)
    {
      for (i = 0; i < (gint)ring_impl->pos; i++)
        func (get_element (ring_impl, i), user_data);
      return;
    }

  for (i = ring_impl->pos; i < (gint)ring_impl->len; i++)
    func (get_element (ring_impl, i), user_data);

  for (i = 0; i < (gint)ring_impl->pos; i++)
    func (get_element (ring_impl, i), user_data);
}

/**
 * dzl_ring_destroy:
 * @ring: (in): A #DzlRing.
 *
 * Cleans up after a #DzlRing that is no longer in use.
 */
void
dzl_ring_destroy (DzlRing *ring)
{
  DzlRingImpl *ring_impl = (DzlRingImpl *)ring;

  g_return_if_fail (ring != NULL);
  g_return_if_fail (ring_impl->ref_count == 0);

  if (ring_impl->destroy != NULL)
    dzl_ring_foreach (ring, (GFunc)ring_impl->destroy, NULL);

  g_free (ring_impl->data);

  g_slice_free (DzlRingImpl, ring_impl);
}

/**
 * dzl_ring_ref:
 * @ring: (in): A #DzlRing.
 *
 * Atomically increments the reference count of @ring by one.
 *
 * Returns: The @ring pointer.
 */
DzlRing *
dzl_ring_ref (DzlRing *ring)
{
  DzlRingImpl *ring_impl = (DzlRingImpl *)ring;

  g_return_val_if_fail (ring != NULL, NULL);
  g_return_val_if_fail (ring_impl->ref_count > 0, NULL);

  g_atomic_int_inc (&ring_impl->ref_count);

  return ring;
}

/**
 * dzl_ring_unref:
 * @ring: (in): A #DzlRing.
 *
 * Atomically decrements the reference count of @ring by one.  When the
 * reference count reaches zero, the structure is freed.
 */
void
dzl_ring_unref (DzlRing *ring)
{
  DzlRingImpl *ring_impl = (DzlRingImpl *)ring;

  g_return_if_fail (ring != NULL);
  g_return_if_fail (ring_impl->ref_count > 0);

  if (g_atomic_int_dec_and_test (&ring_impl->ref_count))
    dzl_ring_destroy (ring);
}
