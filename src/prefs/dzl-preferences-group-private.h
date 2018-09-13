/* dzl-preferences-group-private.h
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

#ifndef DZL_PREFERENCES_GROUP_PRIVATE_H
#define DZL_PREFERENCES_GROUP_PRIVATE_H

#include "prefs/dzl-preferences-group.h"
#include "search/dzl-pattern-spec.h"

G_BEGIN_DECLS

struct _DzlPreferencesGroup
{
  GtkBin      parent_instance;

  gint        priority;
  guint       is_list : 1;

  GtkLabel   *title;
  GtkBox     *box;
  GtkListBox *list_box;
  GtkFrame   *list_box_frame;

  GPtrArray  *widgets;

  GtkListBoxRow *last_focused;
  guint          last_focused_tab_backward : 1;

  GHashTable    *size_groups;
};

G_END_DECLS

#endif /* DZL_PREFERENCES_GROUP_PRIVATE_H */
