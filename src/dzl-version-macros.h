/* dzl-version-macros.h
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

#ifndef DZL_VERSION_MACROS_H
#define DZL_VERSION_MACROS_H

#if !defined(DAZZLE_INSIDE) && !defined(DAZZLE_COMPILATION)
# error "Only <dazzle.h> can be included directly."
#endif

#include <glib.h>

#include "dzl-version.h"

#ifndef _DZL_EXTERN
#define _DZL_EXTERN extern
#endif

#ifdef DZL_DISABLE_DEPRECATION_WARNINGS
#define DZL_DEPRECATED _DZL_EXTERN
#define DZL_DEPRECATED_FOR(f) _DZL_EXTERN
#define DZL_UNAVAILABLE(maj,min) _DZL_EXTERN
#else
#define DZL_DEPRECATED G_DEPRECATED _DZL_EXTERN
#define DZL_DEPRECATED_FOR(f) G_DEPRECATED_FOR(f) _DZL_EXTERN
#define DZL_UNAVAILABLE(maj,min) G_UNAVAILABLE(maj,min) _DZL_EXTERN
#endif

#define DZL_VERSION_3_28 (G_ENCODE_VERSION (3, 28))
#define DZL_VERSION_3_30 (G_ENCODE_VERSION (3, 30))
#define DZL_VERSION_3_32 (G_ENCODE_VERSION (3, 32))
#define DZL_VERSION_3_34 (G_ENCODE_VERSION (3, 34))
#define DZL_VERSION_3_36 (G_ENCODE_VERSION (3, 36))
#define DZL_VERSION_3_38 (G_ENCODE_VERSION (3, 38))

#if (DZL_MINOR_VERSION == 99)
# define DZL_VERSION_CUR_STABLE (G_ENCODE_VERSION (DZL_MAJOR_VERSION + 1, 0))
#elif (DZL_MINOR_VERSION % 2)
# define DZL_VERSION_CUR_STABLE (G_ENCODE_VERSION (DZL_MAJOR_VERSION, DZL_MINOR_VERSION + 1))
#else
# define DZL_VERSION_CUR_STABLE (G_ENCODE_VERSION (DZL_MAJOR_VERSION, DZL_MINOR_VERSION))
#endif

#if (DZL_MINOR_VERSION == 99)
# define DZL_VERSION_PREV_STABLE (G_ENCODE_VERSION (DZL_MAJOR_VERSION + 1, 0))
#elif (DZL_MINOR_VERSION % 2)
# define DZL_VERSION_PREV_STABLE (G_ENCODE_VERSION (DZL_MAJOR_VERSION, DZL_MINOR_VERSION - 1))
#else
# define DZL_VERSION_PREV_STABLE (G_ENCODE_VERSION (DZL_MAJOR_VERSION, DZL_MINOR_VERSION - 2))
#endif

/**
 * DZL_VERSION_MIN_REQUIRED:
 *
 * A macro that should be defined by the user prior to including
 * the dazzle.h header.
 *
 * The definition should be one of the predefined DZL version
 * macros: %DZL_VERSION_3_28, ...
 *
 * This macro defines the lower bound for the Dazzle API to use.
 *
 * If a function has been deprecated in a newer version of Dazzle,
 * it is possible to use this symbol to avoid the compiler warnings
 * without disabling warning for every deprecated function.
 *
 * Since: 3.28
 */
#ifndef DZL_VERSION_MIN_REQUIRED
# define DZL_VERSION_MIN_REQUIRED (DZL_VERSION_CUR_STABLE)
#endif

/**
 * DZL_VERSION_MAX_ALLOWED:
 *
 * A macro that should be defined by the user prior to including
 * the dazzle.h header.

 * The definition should be one of the predefined Dazzle version
 * macros: %DZL_VERSION_1_0, %DZL_VERSION_1_2,...
 *
 * This macro defines the upper bound for the DZL API to use.
 *
 * If a function has been introduced in a newer version of Dazzle,
 * it is possible to use this symbol to get compiler warnings when
 * trying to use that function.
 *
 * Since: 3.28
 */
#ifndef DZL_VERSION_MAX_ALLOWED
# if DZL_VERSION_MIN_REQUIRED > DZL_VERSION_PREV_STABLE
#  define DZL_VERSION_MAX_ALLOWED (DZL_VERSION_MIN_REQUIRED)
# else
#  define DZL_VERSION_MAX_ALLOWED (DZL_VERSION_CUR_STABLE)
# endif
#endif

#if DZL_VERSION_MAX_ALLOWED < DZL_VERSION_MIN_REQUIRED
#error "DZL_VERSION_MAX_ALLOWED must be >= DZL_VERSION_MIN_REQUIRED"
#endif
#if DZL_VERSION_MIN_REQUIRED < DZL_VERSION_3_28
#error "DZL_VERSION_MIN_REQUIRED must be >= DZL_VERSION_3_28"
#endif

#define DZL_AVAILABLE_IN_ALL                   _DZL_EXTERN

#if DZL_VERSION_MIN_REQUIRED >= DZL_VERSION_3_28
# define DZL_DEPRECATED_IN_3_28                DZL_DEPRECATED
# define DZL_DEPRECATED_IN_3_28_FOR(f)         DZL_DEPRECATED_FOR(f)
#else
# define DZL_DEPRECATED_IN_3_28                _DZL_EXTERN
# define DZL_DEPRECATED_IN_3_28_FOR(f)         _DZL_EXTERN
#endif

#if DZL_VERSION_MAX_ALLOWED < DZL_VERSION_3_28
# define DZL_AVAILABLE_IN_3_28                 DZL_UNAVAILABLE(3, 28)
#else
# define DZL_AVAILABLE_IN_3_28                 _DZL_EXTERN
#endif

#if DZL_VERSION_MIN_REQUIRED >= DZL_VERSION_3_30
# define DZL_DEPRECATED_IN_3_30                DZL_DEPRECATED
# define DZL_DEPRECATED_IN_3_30_FOR(f)         DZL_DEPRECATED_FOR(f)
#else
# define DZL_DEPRECATED_IN_3_30                _DZL_EXTERN
# define DZL_DEPRECATED_IN_3_30_FOR(f)         _DZL_EXTERN
#endif

#if DZL_VERSION_MAX_ALLOWED < DZL_VERSION_3_30
# define DZL_AVAILABLE_IN_3_30                 DZL_UNAVAILABLE(3, 30)
#else
# define DZL_AVAILABLE_IN_3_30                 _DZL_EXTERN
#endif

#if DZL_VERSION_MIN_REQUIRED >= DZL_VERSION_3_32
# define DZL_DEPRECATED_IN_3_32                DZL_DEPRECATED
# define DZL_DEPRECATED_IN_3_32_FOR(f)         DZL_DEPRECATED_FOR(f)
#else
# define DZL_DEPRECATED_IN_3_32                _DZL_EXTERN
# define DZL_DEPRECATED_IN_3_32_FOR(f)         _DZL_EXTERN
#endif

#if DZL_VERSION_MAX_ALLOWED < DZL_VERSION_3_32
# define DZL_AVAILABLE_IN_3_32                 DZL_UNAVAILABLE(3, 32)
#else
# define DZL_AVAILABLE_IN_3_32                 _DZL_EXTERN
#endif

#if DZL_VERSION_MIN_REQUIRED >= DZL_VERSION_3_34
# define DZL_DEPRECATED_IN_3_34                DZL_DEPRECATED
# define DZL_DEPRECATED_IN_3_34_FOR(f)         DZL_DEPRECATED_FOR(f)
#else
# define DZL_DEPRECATED_IN_3_34                _DZL_EXTERN
# define DZL_DEPRECATED_IN_3_34_FOR(f)         _DZL_EXTERN
#endif

#if DZL_VERSION_MAX_ALLOWED < DZL_VERSION_3_34
# define DZL_AVAILABLE_IN_3_34                 DZL_UNAVAILABLE(3, 34)
#else
# define DZL_AVAILABLE_IN_3_34                 _DZL_EXTERN
#endif

#if DZL_VERSION_MAX_ALLOWED < DZL_VERSION_3_36
# define DZL_AVAILABLE_IN_3_36                 DZL_UNAVAILABLE(3, 36)
#else
# define DZL_AVAILABLE_IN_3_36                 _DZL_EXTERN
#endif

#if DZL_VERSION_MAX_ALLOWED < DZL_VERSION_3_38
# define DZL_AVAILABLE_IN_3_38                 DZL_UNAVAILABLE(3, 38)
#else
# define DZL_AVAILABLE_IN_3_38                 _DZL_EXTERN
#endif

#endif /* DZL_VERSION_MACROS_H */
