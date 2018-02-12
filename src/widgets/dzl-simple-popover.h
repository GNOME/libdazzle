/* dzl-simple-popover.h
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
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

#ifndef DZL_SIMPLE_POPOVER_H
#define DZL_SIMPLE_POPOVER_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_SIMPLE_POPOVER (dzl_simple_popover_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlSimplePopover, dzl_simple_popover, DZL, SIMPLE_POPOVER, GtkPopover)

struct _DzlSimplePopoverClass
{
  GtkPopoverClass parent;

  /**
   * DzlSimplePopover::activate:
   * @self: The #DzlSimplePopover instance.
   * @text: The text at the time of activation.
   *
   * This signal is emitted when the popover's forward button is activated.
   * Connect to this signal to perform your forward progress.
   */
  void (*activate) (DzlSimplePopover *self,
                    const gchar      *text);

  /**
   * DzlSimplePopover::insert-text:
   * @self: A #DzlSimplePopover.
   * @position: the position in UTF-8 characters.
   * @chars: the NULL terminated UTF-8 text to insert.
   * @n_chars: the number of UTF-8 characters in chars.
   *
   * Use this signal to determine if text should be allowed to be inserted
   * into the text buffer. Return GDK_EVENT_STOP to prevent the text from
   * being inserted.
   */
  gboolean (*insert_text) (DzlSimplePopover *self,
                           guint             position,
                           const gchar      *chars,
                           guint             n_chars);


  /**
   * DzlSimplePopover::changed:
   * @self: A #DzlSimplePopover.
   *
   * This signal is emitted when the entry text changes.
   */
  void (*changed) (DzlSimplePopover *self);
};

DZL_AVAILABLE_IN_ALL
GtkWidget   *dzl_simple_popover_new             (void);
DZL_AVAILABLE_IN_ALL
const gchar *dzl_simple_popover_get_text        (DzlSimplePopover *self);
DZL_AVAILABLE_IN_ALL
void         dzl_simple_popover_set_text        (DzlSimplePopover *self,
                                                 const gchar     *text);
DZL_AVAILABLE_IN_ALL
const gchar *dzl_simple_popover_get_message     (DzlSimplePopover *self);
DZL_AVAILABLE_IN_ALL
void         dzl_simple_popover_set_message     (DzlSimplePopover *self,
                                                 const gchar     *message);
DZL_AVAILABLE_IN_ALL
const gchar *dzl_simple_popover_get_title       (DzlSimplePopover *self);
DZL_AVAILABLE_IN_ALL
void         dzl_simple_popover_set_title       (DzlSimplePopover *self,
                                                 const gchar     *title);
DZL_AVAILABLE_IN_ALL
const gchar *dzl_simple_popover_get_button_text (DzlSimplePopover *self);
DZL_AVAILABLE_IN_ALL
void         dzl_simple_popover_set_button_text (DzlSimplePopover *self,
                                                 const gchar     *button_text);
DZL_AVAILABLE_IN_ALL
gboolean     dzl_simple_popover_get_ready       (DzlSimplePopover *self);
DZL_AVAILABLE_IN_ALL
void         dzl_simple_popover_set_ready       (DzlSimplePopover *self,
                                                 gboolean         ready);

G_END_DECLS

#endif /* DZL_SIMPLE_POPOVER_H */
