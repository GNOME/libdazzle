/* dzl-suggestion-button.h
 *
 * Copyright 2019 Christian Hergert <chergert@redhat.com>
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
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <gtk/gtk.h>

#include "dzl-suggestion-entry.h"
#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_SUGGESTION_BUTTON (dzl_suggestion_button_get_type())

DZL_AVAILABLE_IN_3_34
G_DECLARE_DERIVABLE_TYPE (DzlSuggestionButton, dzl_suggestion_button, DZL, SUGGESTION_BUTTON, GtkStack)

struct _DzlSuggestionButtonClass
{
  GtkStackClass parent_class;

  /*< private >*/
  gpointer _reserved[8];
};

DZL_AVAILABLE_IN_3_34
GtkWidget          *dzl_suggestion_button_new        (void);
DZL_AVAILABLE_IN_3_34
DzlSuggestionEntry *dzl_suggestion_button_get_entry  (DzlSuggestionButton *self);
DZL_AVAILABLE_IN_3_34
GtkButton          *dzl_suggestion_button_get_button (DzlSuggestionButton *self);

G_END_DECLS
