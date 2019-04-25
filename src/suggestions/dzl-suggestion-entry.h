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

#include "dzl-version-macros.h"

#include "dzl-suggestion.h"

G_BEGIN_DECLS

#define DZL_TYPE_SUGGESTION_ENTRY (dzl_suggestion_entry_get_type())

DZL_AVAILABLE_IN_ALL
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
  void (*suggestion_selected)  (DzlSuggestionEntry *self,
                                DzlSuggestion      *suggestion);

  /*< private >*/
  gpointer _reserved[7];
};

DZL_AVAILABLE_IN_ALL
GtkWidget     *dzl_suggestion_entry_new                          (void);
DZL_AVAILABLE_IN_ALL
void           dzl_suggestion_entry_set_model                    (DzlSuggestionEntry        *self,
                                                                  GListModel                *model);
DZL_AVAILABLE_IN_ALL
GListModel    *dzl_suggestion_entry_get_model                    (DzlSuggestionEntry        *self);
DZL_AVAILABLE_IN_ALL
const gchar   *dzl_suggestion_entry_get_typed_text               (DzlSuggestionEntry        *self);
DZL_AVAILABLE_IN_ALL
DzlSuggestion *dzl_suggestion_entry_get_suggestion               (DzlSuggestionEntry        *self);
DZL_AVAILABLE_IN_ALL
void           dzl_suggestion_entry_set_suggestion               (DzlSuggestionEntry        *self,
                                                                  DzlSuggestion             *suggestion);
DZL_AVAILABLE_IN_ALL
void           dzl_suggestion_entry_set_position_func            (DzlSuggestionEntry        *self,
                                                                  DzlSuggestionPositionFunc  func,
                                                                  gpointer                   func_data,
                                                                  GDestroyNotify             func_data_destroy);
DZL_AVAILABLE_IN_3_30
gboolean       dzl_suggestion_entry_get_activate_on_single_click (DzlSuggestionEntry        *self);
DZL_AVAILABLE_IN_3_30
void           dzl_suggestion_entry_set_activate_on_single_click (DzlSuggestionEntry        *self,
                                                                  gboolean                   activate_on_single_click);
DZL_AVAILABLE_IN_3_30
void           dzl_suggestion_entry_hide_suggestions             (DzlSuggestionEntry        *self);
DZL_AVAILABLE_IN_3_32
GtkWidget     *dzl_suggestion_entry_get_popover                  (DzlSuggestionEntry        *self);
DZL_AVAILABLE_IN_3_34
gboolean       dzl_suggestion_entry_get_compact                  (DzlSuggestionEntry        *self);
DZL_AVAILABLE_IN_3_34
void           dzl_suggestion_entry_set_compact                  (DzlSuggestionEntry        *self,
                                                                  gboolean                   compact);

DZL_AVAILABLE_IN_ALL
void dzl_suggestion_entry_default_position_func (DzlSuggestionEntry *self,
                                                 GdkRectangle       *area,
                                                 gboolean           *is_absolute,
                                                 gpointer            user_data);
DZL_AVAILABLE_IN_ALL
void dzl_suggestion_entry_window_position_func  (DzlSuggestionEntry *self,
                                                 GdkRectangle       *area,
                                                 gboolean           *is_absolute,
                                                 gpointer            user_data);

G_END_DECLS

#endif /* DZL_SUGGESTION_ENTRY_H */
