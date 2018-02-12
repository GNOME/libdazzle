/* dzl-preferences-bin.h
 *
 * Copyright (C) 2015-2017 Christian Hergert <chergert@redhat.com>
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

#ifndef DZL_PREFERENCES_BIN_H
#define DZL_PREFERENCES_BIN_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

#include "search/dzl-pattern-spec.h"

G_BEGIN_DECLS

#define DZL_TYPE_PREFERENCES_BIN (dzl_preferences_bin_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlPreferencesBin, dzl_preferences_bin, DZL, PREFERENCES_BIN, GtkBin)

struct _DzlPreferencesBinClass
{
  GtkBinClass parent_class;

  void     (*connect)    (DzlPreferencesBin *self,
                          GSettings         *settings);
  void     (*disconnect) (DzlPreferencesBin *self,
                          GSettings         *settings);
  gboolean (*matches)    (DzlPreferencesBin *self,
                          DzlPatternSpec    *spec);

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
  gpointer _reserved5;
  gpointer _reserved6;
  gpointer _reserved7;
  gpointer _reserved8;
};

G_END_DECLS

#endif /* DZL_PREFERENCES_BIN_H */
