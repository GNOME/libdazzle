/* dzl-suggestion-entry-buffer.h
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

#ifndef DZL_SUGGESTION_ENTRY_BUFFER_H
#define DZL_SUGGESTION_ENTRY_BUFFER_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

#include "dzl-suggestion.h"

G_BEGIN_DECLS

#define DZL_TYPE_SUGGESTION_ENTRY_BUFFER (dzl_suggestion_entry_buffer_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlSuggestionEntryBuffer, dzl_suggestion_entry_buffer, DZL, SUGGESTION_ENTRY_BUFFER, GtkEntryBuffer)

struct _DzlSuggestionEntryBufferClass
{
  GtkEntryBufferClass parent_class;

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
};

DZL_AVAILABLE_IN_ALL
DzlSuggestionEntryBuffer *dzl_suggestion_entry_buffer_new              (void);
DZL_AVAILABLE_IN_ALL
DzlSuggestion            *dzl_suggestion_entry_buffer_get_suggestion   (DzlSuggestionEntryBuffer *self);
DZL_AVAILABLE_IN_ALL
void                      dzl_suggestion_entry_buffer_set_suggestion   (DzlSuggestionEntryBuffer *self,
                                                                        DzlSuggestion            *suggestion);
DZL_AVAILABLE_IN_ALL
const gchar              *dzl_suggestion_entry_buffer_get_typed_text   (DzlSuggestionEntryBuffer *self);
DZL_AVAILABLE_IN_ALL
guint                     dzl_suggestion_entry_buffer_get_typed_length (DzlSuggestionEntryBuffer *self);
DZL_AVAILABLE_IN_ALL
void                      dzl_suggestion_entry_buffer_commit           (DzlSuggestionEntryBuffer *self);
DZL_AVAILABLE_IN_ALL
void                      dzl_suggestion_entry_buffer_clear            (DzlSuggestionEntryBuffer *self);

G_END_DECLS

#endif /* DZL_SUGGESTION_ENTRY_BUFFER_H */
