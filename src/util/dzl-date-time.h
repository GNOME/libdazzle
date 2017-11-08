/* dzl-date-time.h
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

#ifndef DZL_DATE_TIME_H
#define DZL_DATE_TIME_H

#include <gio/gio.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

DZL_AVAILABLE_IN_ALL
gchar       *dzl_g_date_time_format_for_display  (GDateTime      *self);
DZL_AVAILABLE_IN_ALL
gchar       *dzl_g_time_span_to_label            (GTimeSpan       span);
DZL_AVAILABLE_IN_ALL
gboolean     dzl_g_time_span_to_label_mapping    (GBinding       *binding,
                                                  const GValue   *from_value,
                                                  GValue         *to_value,
                                                  gpointer        user_data);

G_END_DECLS

#endif /* DZL_DATE_TIME_H */
