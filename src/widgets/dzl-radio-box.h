/* dzl-radio-box.h
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
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

#ifndef DZL_RADIO_BOX_H
#define DZL_RADIO_BOX_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_RADIO_BOX (dzl_radio_box_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlRadioBox, dzl_radio_box, DZL, RADIO_BOX, GtkBin)

struct _DzlRadioBoxClass
{
  GtkBinClass parent_class;

  gpointer _padding1;
  gpointer _padding2;
  gpointer _padding3;
  gpointer _padding4;
};

DZL_AVAILABLE_IN_ALL
GtkWidget   *dzl_radio_box_new           (void);
DZL_AVAILABLE_IN_ALL
void         dzl_radio_box_add_item      (DzlRadioBox *self,
                                          const gchar *id,
                                          const gchar *text);
DZL_AVAILABLE_IN_3_32
void         dzl_radio_box_remove_item   (DzlRadioBox *self,
                                          const gchar *id);
DZL_AVAILABLE_IN_ALL
const gchar *dzl_radio_box_get_active_id (DzlRadioBox *self);
DZL_AVAILABLE_IN_ALL
void         dzl_radio_box_set_active_id (DzlRadioBox *self,
                                          const gchar *id);

G_END_DECLS

#endif /* DZL_RADIO_BOX_H */
