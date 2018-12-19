/* dzl-date-time.c
 *
 * Copyright (C) 2015 Christian Hergert <chergert@redhat.com>
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

#include "config.h"

#include <glib/gi18n.h>

#include "dzl-date-time.h"

/**
 * SECTION:dzl-date-time
 * @title: Date and Time Functions
 * @short_description: Date/time helper functions
 *
 * This section provides a few functions to help with displaying dates and times in an
 * easily readable way.
 */

/**
 * dzl_g_date_time_format_for_display:
 * @self: A #GDateTime
 *
 * Helper function to create a human-friendly string describing approximately
 * how long ago a #GDateTime is.
 *
 * Returns: (transfer full): A newly allocated string describing the
 *   date and time imprecisely such as "Yesterday".
 */
gchar *
dzl_g_date_time_format_for_display (GDateTime *self)
{
  g_autoptr(GDateTime) now = NULL;
  GTimeSpan diff;
  gint years;

  /*
   * TODO:
   *
   * There is probably a lot more we can do here to be friendly for
   * various locales, but this will get us started.
   */

  g_return_val_if_fail (self != NULL, NULL);

  now = g_date_time_new_now_utc ();
  diff = g_date_time_difference (now, self) / G_USEC_PER_SEC;

  if (diff < 0)
    return g_strdup ("");
  else if (diff < (60 * 45))
    return g_strdup (_("Just now"));
  else if (diff < (60 * 90))
    return g_strdup (_("An hour ago"));
  else if (diff < (60 * 60 * 24 * 2))
    return g_strdup (_("Yesterday"));
  else if (diff < (60 * 60 * 24 * 7))
    return g_date_time_format (self, "%A");
  else if (diff < (60 * 60 * 24 * 365))
    return g_date_time_format (self, "%OB");
  else if (diff < (60 * 60 * 24 * 365 * 1.5))
    return g_strdup (_("About a year ago"));

  years = MAX (2, diff / (60 * 60 * 24 * 365));

  return g_strdup_printf (ngettext ("About %u year ago", "About %u years ago", years), years);
}

/**
 * dzl_g_time_span_to_label:
 * @span: the span of time
 *
 * Creates a string describing the time span in hours, minutes, and seconds.
 * For example, a time span of three and a half minutes would be "3:30".
 * 2 days, 3 hours, 6 minutes, and 20 seconds would be "51:06:20".
 *
 * Returns: (transfer full): A newly allocated string describing the time span.
 */
gchar *
dzl_g_time_span_to_label (GTimeSpan span)
{
  gint64 hours;
  gint64 minutes;
  gint64 seconds;

  span = ABS (span);

  hours = span / G_TIME_SPAN_HOUR;
  minutes = (span % G_TIME_SPAN_HOUR) / G_TIME_SPAN_MINUTE;
  seconds = (span % G_TIME_SPAN_MINUTE) / G_TIME_SPAN_SECOND;

  g_assert (minutes < 60);
  g_assert (seconds < 60);

  if (hours == 0)
    return g_strdup_printf ("%02"G_GINT64_FORMAT":%02"G_GINT64_FORMAT,
                            minutes, seconds);
  else
    return g_strdup_printf ("%02"G_GINT64_FORMAT":%02"G_GINT64_FORMAT":%02"G_GINT64_FORMAT,
                            hours, minutes, seconds);
}

/**
 * dzl_g_time_span_to_label_mapping:
 *
 * A #GBindingTransformFunc to transform a time span into a string label using
 * dzl_g_time_span_to_label().
 */
gboolean
dzl_g_time_span_to_label_mapping (GBinding     *binding,
                                  const GValue *from_value,
                                  GValue       *to_value,
                                  gpointer      user_data)
{
  GTimeSpan span;

  g_assert (G_IS_BINDING (binding));
  g_assert (from_value != NULL);
  g_assert (G_VALUE_HOLDS (from_value, G_TYPE_INT64));
  g_assert (to_value != NULL);
  g_assert (G_VALUE_HOLDS (to_value, G_TYPE_STRING));

  span = g_value_get_int64 (from_value);
  g_value_take_string (to_value, dzl_g_time_span_to_label (span));

  return TRUE;
}
