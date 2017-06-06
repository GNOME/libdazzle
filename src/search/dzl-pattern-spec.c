/* dzl-pattern-spec.c
 *
 * Copyright (C) 2015-2017 Christian Hergert <christian@hergert.me>
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

#define G_LOG_DOMAIN "dzl-pattern-spec"

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <string.h>

#include "dzl-pattern-spec.h"

G_DEFINE_BOXED_TYPE (DzlPatternSpec, dzl_pattern_spec, dzl_pattern_spec_ref, dzl_pattern_spec_unref)

/**
 * SECTION:dzl-pattern-spec
 * @title: DzlPatternSpec
 * @short_description: Simple glob-like searching
 *
 * This works similar to #GPatternSpec except the query syntax is different.
 * It tries to mtach word boundaries, but with matching partial words up
 * to those boundaries. For example, "gtk widg" would match "gtk_widget_show".
 * Word boundaries include '_' and ' '. If any character is uppercase, then
 * case sensitivity is used.
 */

struct _DzlPatternSpec
{
  volatile gint   ref_count;
  gchar          *needle;
  gchar         **parts;
  guint           case_sensitive : 1;
};

DzlPatternSpec *
dzl_pattern_spec_new (const gchar *needle)
{
  DzlPatternSpec *self;
  const gchar *tmp;

  g_return_val_if_fail (needle, NULL);

  self = g_new0 (DzlPatternSpec, 1);
  self->ref_count = 1;
  self->needle = g_strdup (needle);
  self->parts = g_strsplit (needle, " ", 0);
  self->case_sensitive = FALSE;

  for (tmp = needle; *tmp; tmp = g_utf8_next_char (tmp))
    {
      if (g_unichar_isupper (g_utf8_get_char (tmp)))
        {
          self->case_sensitive = TRUE;
          break;
        }
    }

  return self;
}

const gchar *
dzl_pattern_spec_get_text (DzlPatternSpec *self)
{
  g_return_val_if_fail (self != NULL, NULL);

  return self->needle;
}

static void
dzl_pattern_spec_free (DzlPatternSpec *self)
{
  g_strfreev (self->parts);
  g_free (self->needle);
  g_free (self);
}

static inline gboolean
is_word_break (gunichar ch)
{
  return (ch == ' ' || ch == '_' || ch == '-');
}

static const gchar *
next_word_start (const gchar *haystack)
{
  for (; *haystack; haystack = g_utf8_next_char (haystack))
    {
      gunichar ch = g_utf8_get_char (haystack);

      if (is_word_break (ch))
        break;
    }

  for (; *haystack; haystack = g_utf8_next_char (haystack))
    {
      gunichar ch = g_utf8_get_char (haystack);

      if (is_word_break (ch))
        continue;

      break;
    }

  g_return_val_if_fail (*haystack == '\0' || !is_word_break (*haystack), NULL);

  return haystack;
}

gboolean
dzl_pattern_spec_match (DzlPatternSpec *self,
                        const gchar    *haystack)
{
  gsize i;

  g_return_val_if_fail (self, FALSE);
  g_return_val_if_fail (haystack, FALSE);

  for (i = 0; (haystack != NULL) && self->parts [i]; i++)
    {
      if (self->parts [i][0] == '\0')
        continue;

      if (self->case_sensitive)
        haystack = strstr (haystack, self->parts [i]);
      else
        haystack = strcasestr (haystack, self->parts [i]);

      if (haystack == NULL)
        return FALSE;

      if (self->parts [i + 1] != NULL)
        haystack = next_word_start (haystack + strlen (self->parts [i]));
    }

  return TRUE;
}

DzlPatternSpec *
dzl_pattern_spec_ref (DzlPatternSpec *self)
{
  g_return_val_if_fail (self, NULL);
  g_return_val_if_fail (self->ref_count > 0, NULL);

  g_atomic_int_inc (&self->ref_count);

  return self;
}

void
dzl_pattern_spec_unref (DzlPatternSpec *self)
{
  g_return_if_fail (self);
  g_return_if_fail (self->ref_count > 0);

  if (g_atomic_int_dec_and_test (&self->ref_count))
    dzl_pattern_spec_free (self);
}
