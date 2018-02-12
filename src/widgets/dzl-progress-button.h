/*
 * Copyright (C) 2013-2014 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef DZL_PROGRESS_BUTTON_H
#define DZL_PROGRESS_BUTTON_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_PROGRESS_BUTTON (dzl_progress_button_get_type ())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlProgressButton, dzl_progress_button, DZL, PROGRESS_BUTTON, GtkButton)

struct _DzlProgressButtonClass
{
  GtkButtonClass parent_class;

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
};

DZL_AVAILABLE_IN_ALL
GtkWidget	*dzl_progress_button_new               (void);
DZL_AVAILABLE_IN_ALL
guint      dzl_progress_button_get_progress      (DzlProgressButton *self);
DZL_AVAILABLE_IN_ALL
void       dzl_progress_button_set_progress	     (DzlProgressButton	*button,
                                                  guint              percentage);
DZL_AVAILABLE_IN_ALL
gboolean   dzl_progress_button_get_show_progress (DzlProgressButton *self);
DZL_AVAILABLE_IN_ALL
void       dzl_progress_button_set_show_progress (DzlProgressButton *button,
					      																 	gboolean           show_progress);

G_END_DECLS

#endif /* DZL_PROGRESS_BUTTON_H */
