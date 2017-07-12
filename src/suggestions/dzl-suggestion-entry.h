/* dzl-suggestion-entry.h
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

#ifndef DZL_SUGGESTION_ENTRY_H
#define DZL_SUGGESTION_ENTRY_H

#include <gtk/gtk.h>

#include "dzl-suggestion.h"

G_BEGIN_DECLS

#define DZL_TYPE_SUGGESTION_ENTRY (dzl_suggestion_entry_get_type())

G_DECLARE_DERIVABLE_TYPE (DzlSuggestionEntry, dzl_suggestion_entry, DZL, SUGGESTION_ENTRY, GtkEntry)

/**
 * DzlSuggestionPositionFunc:
 * @entry: a #DzlSuggestionEntry
 * @area: (inout): location to place the popover
 * @is_absolute: (inout): If the area is in absolute coordinates
 * @user_data: closure data
 *
 * Positions the popover in the coordinates defined by @area.
 *
 * If @is_absolute is set to %TRUE, then absolute coordinates are used.
 * Otherwise, the position is expected to be relative to @entry.
 *
 * Since: 3.26
 */
typedef void (*DzlSuggestionPositionFunc) (DzlSuggestionEntry *entry,
                                           GdkRectangle       *area,
                                           gboolean           *is_absolute,
                                           gpointer            user_data);

struct _DzlSuggestionEntryClass
{
  GtkEntryClass parent_class;

  void (*hide_suggestions)     (DzlSuggestionEntry *self);
  void (*show_suggestions)     (DzlSuggestionEntry *self);
  void (*move_suggestion )     (DzlSuggestionEntry *self,
                                gint                amount);
  void (*suggestion_activated) (DzlSuggestionEntry *self,
                                DzlSuggestion      *suggestion);

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
  gpointer _reserved5;
  gpointer _reserved6;
  gpointer _reserved7;
  gpointer _reserved8;
};

GtkWidget     *dzl_suggestion_entry_new               (void);
void           dzl_suggestion_entry_set_model         (DzlSuggestionEntry        *self,
                                                       GListModel                *model);
GListModel    *dzl_suggestion_entry_get_model         (DzlSuggestionEntry        *self);
const gchar   *dzl_suggestion_entry_get_typed_text    (DzlSuggestionEntry        *self);
DzlSuggestion *dzl_suggestion_entry_get_suggestion    (DzlSuggestionEntry        *self);
void           dzl_suggestion_entry_set_suggestion    (DzlSuggestionEntry        *self,
                                                       DzlSuggestion             *suggestion);
void           dzl_suggestion_entry_set_position_func (DzlSuggestionEntry        *self,
                                                       DzlSuggestionPositionFunc  func,
                                                       gpointer                   func_data,
                                                       GDestroyNotify             func_data_destroy);

void dzl_suggestion_entry_default_position_func (DzlSuggestionEntry *self,
                                                 GdkRectangle       *area,
                                                 gboolean           *is_absolute,
                                                 gpointer            user_data);
void dzl_suggestion_entry_window_position_func  (DzlSuggestionEntry *self,
                                                 GdkRectangle       *area,
                                                 gboolean           *is_absolute,
                                                 gpointer            user_data);

G_END_DECLS

#endif /* DZL_SUGGESTION_ENTRY_H */
