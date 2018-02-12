/* dzl-signal-group.h
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
 * Copyright (C) 2015 Garrett Regier <garrettregier@gmail.com>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DZL_SIGNAL_GROUP_H
#define DZL_SIGNAL_GROUP_H

#include <glib-object.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_SIGNAL_GROUP (dzl_signal_group_get_type())

DZL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DzlSignalGroup, dzl_signal_group, DZL, SIGNAL_GROUP, GObject)

DZL_AVAILABLE_IN_ALL
DzlSignalGroup *dzl_signal_group_new             (GType           target_type);

DZL_AVAILABLE_IN_ALL
void            dzl_signal_group_set_target      (DzlSignalGroup *self,
                                                  gpointer        target);
DZL_AVAILABLE_IN_ALL
gpointer        dzl_signal_group_get_target      (DzlSignalGroup *self);

DZL_AVAILABLE_IN_ALL
void            dzl_signal_group_block           (DzlSignalGroup *self);
DZL_AVAILABLE_IN_ALL
void            dzl_signal_group_unblock         (DzlSignalGroup *self);

DZL_AVAILABLE_IN_ALL
void            dzl_signal_group_connect_object  (DzlSignalGroup *self,
                                                  const gchar    *detailed_signal,
                                                  GCallback       c_handler,
                                                  gpointer        object,
                                                  GConnectFlags   flags);
DZL_AVAILABLE_IN_ALL
void            dzl_signal_group_connect_data    (DzlSignalGroup *self,
                                                  const gchar    *detailed_signal,
                                                  GCallback       c_handler,
                                                  gpointer        data,
                                                  GClosureNotify  notify,
                                                  GConnectFlags   flags);
DZL_AVAILABLE_IN_ALL
void            dzl_signal_group_connect         (DzlSignalGroup *self,
                                                  const gchar    *detailed_signal,
                                                  GCallback       c_handler,
                                                  gpointer        data);
DZL_AVAILABLE_IN_ALL
void            dzl_signal_group_connect_after   (DzlSignalGroup *self,
                                                  const gchar    *detailed_signal,
                                                  GCallback       c_handler,
                                                  gpointer        data);
DZL_AVAILABLE_IN_ALL
void            dzl_signal_group_connect_swapped (DzlSignalGroup *self,
                                                  const gchar    *detailed_signal,
                                                  GCallback       c_handler,
                                                  gpointer        data);

G_END_DECLS

#endif /* DZL_SIGNAL_GROUP_H */
