/* dzl-levenshtein.c
 *
 * Copyright (C) 2013-2017 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <glib.h>
#include <string.h>

#include "dzl-levenshtein.h"

gint
dzl_levenshtein (const gchar *needle,
                 const gchar *haystack)
{
   const gchar *s;
   const gchar *t;
   gunichar sc;
   gunichar tc;
   gint *v0;
   gint *v1;
   gint haystack_char_len;
   gint cost;
   gint i;
   gint j;
   gint ret;

   g_return_val_if_fail (needle, G_MAXINT);
   g_return_val_if_fail (haystack, G_MAXINT);

   /*
    * Handle degenerate cases.
    */
   if (!g_strcmp0(needle, haystack)) {
      return 0;
   } else if (!*needle) {
      return g_utf8_strlen(haystack, -1);
   } else if (!*haystack) {
      return g_utf8_strlen(needle, -1);
   }

   haystack_char_len = g_utf8_strlen (haystack, -1);

   /*
    * Create two vectors to hold our states.
    */
   v0 = g_new0(int, haystack_char_len + 1);
   v1 = g_new0(int, haystack_char_len + 1);

   /*
    * initialize v0 (the previous row of distances).
    * this row is A[0][i]: edit distance for an empty needle.
    * the distance is just the number of characters to delete from haystack.
    */
   for (i = 0; i < haystack_char_len + 1; i++) {
      v0[i] = i;
   }

   for (i = 0, s = needle; s && *s; i++, s = g_utf8_next_char(s)) {
      /*
       * Calculate v1 (current row distances) from the previous row v0.
       */

      sc = g_utf8_get_char(s);

      /*
       * first element of v1 is A[i+1][0]
       *
       * edit distance is delete (i+1) chars from needle to match empty
       * haystack.
       */
      v1[0] = i + 1;

      /*
       * use formula to fill in the rest of the row.
       */
      for (j = 0, t = haystack; t && *t; j++, t = g_utf8_next_char(t)) {
         tc = g_utf8_get_char(t);
         cost = (sc == tc) ? 0 : 1;
         v1[j+1] = MIN(v1[j] + 1, MIN(v0[j+1] + 1, v0[j] + cost));
      }

      /*
       * copy v1 (current row) to v0 (previous row) for next iteration.
       */
      memcpy(v0, v1, sizeof(gint) * haystack_char_len);
   }

   ret = v1[haystack_char_len];

   g_free(v0);
   g_free(v1);

   return ret;
}
