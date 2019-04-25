/* dzl-suggestion-private.h
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


#ifndef DZL_SUGGESTION_PRIVATE_H
#define DZL_SUGGESTION_PRIVATE_H

#include "suggestions/dzl-suggestion-entry.h"
#include "suggestions/dzl-suggestion-popover.h"
#include "suggestions/dzl-suggestion-row.h"

void _dzl_suggestion_entry_reposition       (DzlSuggestionEntry   *entry,
                                             DzlSuggestionPopover *popover);
void _dzl_suggestion_popover_set_compact    (DzlSuggestionPopover *self,
                                             gboolean              compact);
void _dzl_suggestion_popover_set_device     (DzlSuggestionPopover *self,
                                             GdkDevice            *device);
void _dzl_suggestion_popover_set_focused    (DzlSuggestionPopover *self,
                                             gboolean              entry_focused);
void _dzl_suggestion_popover_set_max_height (DzlSuggestionPopover *popover,
                                             gint                  max_height);
void _dzl_suggestion_popover_adjust_margin  (DzlSuggestionPopover *popover,
                                             GdkRectangle         *area);
void _dzl_suggestion_popover_set_click_mode (DzlSuggestionPopover *popover,
                                             gboolean              single_click);
void _dzl_suggestion_row_set_ellipsize      (DzlSuggestionRow     *self,
                                             PangoEllipsizeMode    title,
                                             PangoEllipsizeMode    subtitle);

#endif /* DZL_SUGGESTION_PRIVATE_H */
