/* dzl-search-bar.h
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DZL_SEARCH_BAR_H
#define DZL_SEARCH_BAR_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_SEARCH_BAR (dzl_search_bar_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlSearchBar, dzl_search_bar, DZL, SEARCH_BAR, GtkBin)

struct _DzlSearchBarClass
{
  GtkBinClass parent_class;
};

DZL_AVAILABLE_IN_ALL
GtkWidget *dzl_search_bar_new                     (void);
DZL_AVAILABLE_IN_ALL
gboolean   dzl_search_bar_get_search_mode_enabled (DzlSearchBar *self);
DZL_AVAILABLE_IN_ALL
void       dzl_search_bar_set_search_mode_enabled (DzlSearchBar *self,
                                                   gboolean      search_mode_enabled);
DZL_AVAILABLE_IN_ALL
gboolean   dzl_search_bar_get_show_close_button   (DzlSearchBar *self);
DZL_AVAILABLE_IN_ALL
void       dzl_search_bar_set_show_close_button   (DzlSearchBar *self,
                                                   gboolean      show_close_button);
DZL_AVAILABLE_IN_ALL
GtkWidget *dzl_search_bar_get_entry               (DzlSearchBar *self);

G_END_DECLS

#endif /* DZL_SEARCH_BAR_H */
