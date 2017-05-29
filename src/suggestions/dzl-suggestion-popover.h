/* dzl-suggestion-popover.h
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
 *
 * This file is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef DZL_SUGGESTION_POPOVER_H
#define DZL_SUGGESTION_POPOVER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define DZL_TYPE_SUGGESTION_POPOVER (dzl_suggestion_popover_get_type())

G_DECLARE_FINAL_TYPE (DzlSuggestionPopover, dzl_suggestion_popover, DZL, SUGGESTION_POPOVER, GtkWindow)

GtkWidget     *dzl_suggestion_popover_new               (void);
GtkWidget     *dzl_suggestion_popover_get_relative_to   (DzlSuggestionPopover *self);
void           dzl_suggestion_popover_set_relative_to   (DzlSuggestionPopover *self,
                                                         GtkWidget            *widget);
void           dzl_suggestion_popover_popup             (DzlSuggestionPopover *self);
void           dzl_suggestion_popover_popdown           (DzlSuggestionPopover *self);
GListModel    *dzl_suggestion_popover_get_model         (DzlSuggestionPopover *self);
void           dzl_suggestion_popover_set_model         (DzlSuggestionPopover *self,
                                                         GListModel           *model);
void           dzl_suggestion_popover_move_by           (DzlSuggestionPopover *self,
                                                         gint                  amount);
DzlSuggestion *dzl_suggestion_popover_get_selected      (DzlSuggestionPopover *self);
void           dzl_suggestion_popover_set_selected      (DzlSuggestionPopover *self,
                                                         DzlSuggestion        *suggestion);
void           dzl_suggestion_popover_activate_selected (DzlSuggestionPopover *self);

G_END_DECLS

#endif /* DZL_SUGGESTION_POPOVER_H */
