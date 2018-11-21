/* dzl-file-chooser-entry.h
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
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

#ifndef DZL_FILE_CHOOSER_ENTRY_H
#define DZL_FILE_CHOOSER_ENTRY_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_FILE_CHOOSER_ENTRY (dzl_file_chooser_entry_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlFileChooserEntry, dzl_file_chooser_entry, DZL, FILE_CHOOSER_ENTRY, GtkBin)

struct _DzlFileChooserEntryClass
{
  GtkBinClass parent_class;

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
};

DZL_AVAILABLE_IN_3_30
GtkWidget           *dzl_file_chooser_entry_new       (const gchar          *title,
                                                       GtkFileChooserAction  action);
DZL_AVAILABLE_IN_ALL
GFile               *dzl_file_chooser_entry_get_file  (DzlFileChooserEntry  *self);
DZL_AVAILABLE_IN_ALL
void                 dzl_file_chooser_entry_set_file  (DzlFileChooserEntry  *self,
                                                       GFile                *file);
DZL_AVAILABLE_IN_3_32
GtkEntry            *dzl_file_chooser_entry_get_entry (DzlFileChooserEntry  *self);

G_END_DECLS

#endif /* DZL_FILE_CHOOSER_ENTRY_H */
