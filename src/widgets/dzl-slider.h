/* dzl-slider.h
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

#ifndef DZL_SLIDER_H
#define DZL_SLIDER_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_SLIDER          (dzl_slider_get_type())
#define DZL_TYPE_SLIDER_POSITION (dzl_slider_position_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DzlSlider, dzl_slider, DZL, SLIDER, GtkContainer)

typedef enum
{
  DZL_SLIDER_NONE,
  DZL_SLIDER_TOP,
  DZL_SLIDER_RIGHT,
  DZL_SLIDER_BOTTOM,
  DZL_SLIDER_LEFT,
} DzlSliderPosition;

struct _DzlSliderClass
{
  GtkContainerClass parent_instance;
};

DZL_AVAILABLE_IN_ALL
GType              dzl_slider_position_get_type (void);
DZL_AVAILABLE_IN_ALL
GtkWidget         *dzl_slider_new               (void);
DZL_AVAILABLE_IN_ALL
void               dzl_slider_add_slider        (DzlSlider         *self,
                                                 GtkWidget         *widget,
                                                 DzlSliderPosition  position);
DZL_AVAILABLE_IN_ALL
DzlSliderPosition  dzl_slider_get_position      (DzlSlider         *self);
DZL_AVAILABLE_IN_ALL
void               dzl_slider_set_position      (DzlSlider         *self,
                                                 DzlSliderPosition  position);

G_END_DECLS

#endif /* DZL_SLIDER_H */
