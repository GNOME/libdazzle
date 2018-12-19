/* dzl-cairo.h
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

#ifndef DZL_CAIRO_H
#define DZL_CAIRO_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

DZL_AVAILABLE_IN_ALL
cairo_region_t *dzl_cairo_region_create_from_clip_extents (cairo_t            *cr);
DZL_AVAILABLE_IN_ALL
void            dzl_cairo_rounded_rectangle               (cairo_t            *cr,
                                                           const GdkRectangle *rect,
                                                           gint                x_radius,
                                                           gint                y_radius);

/**
 * dzl_cairo_rectangle_x2:
 * @rect: the cairo rectangle to find the right side of
 *
 * Finds the X coordinate of the right side of a rectangle.
 *
 * Returns: The X coordinate as an integer
 */
static inline gint
dzl_cairo_rectangle_x2 (const cairo_rectangle_int_t *rect)
{
  return rect->x + rect->width;
}

/**
 * dzl_cairo_rectangle_y2:
 * @rect: the cairo rectangle to find the bottom of
 *
 * Finds the Y coordinate of the bottom of a rectangle.
 *
 * Returns: The Y coordinate as an integer
 */
static inline gint
dzl_cairo_rectangle_y2 (const cairo_rectangle_int_t *rect)
{
  return rect->y + rect->height;
}

/**
 * dzl_cairo_rectangle_center:
 * @rect: the cairo rectangle to find the center of
 *
 * Finds the X coordinate of the center of a rectangle.
 *
 * Returns: The X coordinate as an integer
 */
static inline gint
dzl_cairo_rectangle_center (const cairo_rectangle_int_t *rect)
{
  return rect->x + (rect->width/2);
}

/**
 * dzl_cairo_rectangle_middle:
 * @rect: the cairo rectangle to find the center of
 *
 * Finds the Y coordinate of the center of a rectangle.
 *
 * Returns: The Y coordinate as an integer
 */
static inline gint
dzl_cairo_rectangle_middle (const cairo_rectangle_int_t *rect)
{
  return rect->y + (rect->height/2);
}

/**
 * dzl_cairo_rectangle_contains_rectangle:
 * @a: the outer rectangle
 * @b: the inner rectangle
 *
 * Determines whether rectangle @a completely contains rectangle @b.
 * @b may share edges with @a and still be considered contained.
 *
 * Returns: Whether @a contains @b
 */
static inline cairo_bool_t
dzl_cairo_rectangle_contains_rectangle (const cairo_rectangle_int_t *a,
                                        const cairo_rectangle_int_t *b)
{
    return (a->x <= b->x &&
            a->x + (int) a->width >= b->x + (int) b->width &&
            a->y <= b->y &&
            a->y + (int) a->height >= b->y + (int) b->height);
}

G_END_DECLS

#endif /* DZL_CAIRO_H */
