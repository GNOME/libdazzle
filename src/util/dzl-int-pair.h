/* dzl-int-pair.h
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
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

#ifndef DZL_INT_PAIR_H
#define DZL_INT_PAIR_H

#ifndef __GI_SCANNER__

#include <glib.h>
#include <stdlib.h>

G_BEGIN_DECLS

#if GLIB_SIZEOF_VOID_P == 8
# define DZL_INT_PAIR_64
#endif

#ifdef DZL_INT_PAIR_64

typedef union
{
  /*< private >*/
  struct {
    gint first;
    gint second;
  };
  gpointer ptr;
} DzlIntPair;

typedef union
{
  /*< private >*/
  struct {
    guint first;
    guint second;
  };
  gpointer ptr;
} DzlUIntPair;

#else

typedef struct
{
  /*< private >*/
  gint first;
  gint second;
} DzlIntPair;

typedef struct
{
  /*< private >*/
  guint first;
  guint second;
} DzlUIntPair;

#endif

/**
 * dzl_int_pair_new: (skip)
 */
static inline DzlIntPair *
dzl_int_pair_new (gint first, gint second)
{
  DzlIntPair pair;

  /* Avoid tripping g-ir-scanner by putting this
   * inside the inline function.
   */
  G_STATIC_ASSERT (sizeof (DzlIntPair) == 8);

  pair.first = first;
  pair.second = second;

#ifdef DZL_INT_PAIR_64
  return (DzlIntPair *)pair.ptr;
#else
  return g_slice_copy (sizeof (DzlIntPair), &pair);
#endif
}

/**
 * dzl_uint_pair_new: (skip)
 */
static inline DzlUIntPair *
dzl_uint_pair_new (guint first, guint second)
{
  DzlUIntPair pair;

  /* Avoid tripping g-ir-scanner by putting this
   * inside the inline function.
   */
  G_STATIC_ASSERT (sizeof (DzlUIntPair) == 8);

  pair.first = first;
  pair.second = second;

#ifdef DZL_INT_PAIR_64
  return (DzlUIntPair *)pair.ptr;
#else
  return g_slice_copy (sizeof (DzlUIntPair), &pair);
#endif
}

/**
 * dzl_int_pair_first: (skip)
 */
static inline gint
dzl_int_pair_first (DzlIntPair *pair)
{
  DzlIntPair p;
#ifdef DZL_INT_PAIR_64
  p.ptr = pair;
#else
  p = *pair;
#endif
  return p.first;
}

/**
 * dzl_int_pair_second: (skip)
 */
static inline gint
dzl_int_pair_second (DzlIntPair *pair)
{
  DzlIntPair p;
#ifdef DZL_INT_PAIR_64
  p.ptr = pair;
#else
  p = *pair;
#endif
  return p.second;
}

/**
 * dzl_uint_pair_first: (skip)
 */
static inline guint
dzl_uint_pair_first (DzlUIntPair *pair)
{
  DzlUIntPair p;
#ifdef DZL_INT_PAIR_64
  p.ptr = pair;
#else
  p = *pair;
#endif
  return p.first;
}

/**
 * dzl_uint_pair_second: (skip)
 */
static inline guint
dzl_uint_pair_second (DzlUIntPair *pair)
{
  DzlUIntPair p;
#ifdef DZL_INT_PAIR_64
  p.ptr = pair;
#else
  p = *pair;
#endif
  return p.second;
}

/**
 * dzl_int_pair_free: (skip)
 */
static inline void
dzl_int_pair_free (DzlIntPair *pair)
{
#ifdef DZL_INT_PAIR_64
  /* Do Nothing */
#else
  g_slice_free (DzlIntPair, pair);
#endif
}

/**
 * dzl_uint_pair_free: (skip)
 */
static inline void
dzl_uint_pair_free (DzlUIntPair *pair)
{
#ifdef DZL_INT_PAIR_64
  /* Do Nothing */
#else
  g_slice_free (DzlUIntPair, pair);
#endif
}

G_END_DECLS

#endif /* __GI_SCANNER__ */

#endif /* DZL_INT_PAIR_H */
