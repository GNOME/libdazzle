/* dzl-variant.c
 *
 * Copyright (C) 2016-2017 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "dzl-variant"

#include "config.h"

#include "util/dzl-variant.h"

guint
dzl_g_variant_hash (gconstpointer data)
{
  GVariant *variant = (GVariant *)data;
  GBytes *bytes;
  guint ret;

  if (!g_variant_is_container (variant))
    return g_variant_hash (variant);

  /* Generally we wouldn't want to create a bytes to hash
   * during a hash call, since that'd be fairly expensive.
   * But since GHashTable caches hash values, its not that
   * big of a deal.
   */
  bytes = g_variant_get_data_as_bytes (variant);
  ret = g_bytes_hash (bytes);
  g_bytes_unref (bytes);

  return ret;
}
