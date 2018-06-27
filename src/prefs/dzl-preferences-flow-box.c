/* dzl-preferences-flow-box.c
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

#define G_LOG_DOMAIN "dzl-preferences-flow-box"

#include "config.h"

#include "prefs/dzl-preferences-flow-box.h"

struct _DzlPreferencesFlowBox
{
  DzlColumnLayout parent;
};

G_DEFINE_TYPE (DzlPreferencesFlowBox, dzl_preferences_flow_box, DZL_TYPE_COLUMN_LAYOUT)

static void
dzl_preferences_flow_box_class_init (DzlPreferencesFlowBoxClass *klass)
{
}

static void
dzl_preferences_flow_box_init (DzlPreferencesFlowBox *self)
{
}

GtkWidget *
dzl_preferences_flow_box_new (void)
{
  return g_object_new (DZL_TYPE_PREFERENCES_FLOW_BOX, NULL);
}
