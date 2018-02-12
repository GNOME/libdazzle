/* dzl-css-provider.h
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

#ifndef DZL_CSS_PROVIDER_H
#define DZL_CSS_PROVIDER_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_CSS_PROVIDER (dzl_css_provider_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlCssProvider, dzl_css_provider, DZL, CSS_PROVIDER, GtkCssProvider)

DZL_AVAILABLE_IN_ALL
GtkCssProvider *dzl_css_provider_new (const gchar *base_path);

G_END_DECLS

#endif /* DZL_CSS_PROVIDER_H */
