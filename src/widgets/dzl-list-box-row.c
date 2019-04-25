/* dzl-list-box-row.c
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "dzl-list-box-row"

#include "config.h"

#include "dzl-list-box.h"
#include "dzl-list-box-private.h"
#include "dzl-list-box-row.h"

G_DEFINE_ABSTRACT_TYPE (DzlListBoxRow, dzl_list_box_row, GTK_TYPE_LIST_BOX_ROW)

static void
dzl_list_box_row_dispose (GObject *object)
{
  DzlListBoxRow *self = (DzlListBoxRow *)object;
  GtkWidget *parent;

  /*
   * Chaining up will cause a lot of things to be deleted such as
   * our widget tree starting from this instance. We don't want that
   * to happen so that we can reuse the widgets. This should only
   * need to be done if the caller has not notified us they will be
   * caching the widget first.
   */

  parent = gtk_widget_get_parent (GTK_WIDGET (self));
  if (DZL_IS_LIST_BOX (parent) &&
      _dzl_list_box_cache (DZL_LIST_BOX (parent), self))
    return;

  G_OBJECT_CLASS (dzl_list_box_row_parent_class)->dispose (object);
}

static void
dzl_list_box_row_class_init (DzlListBoxRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = dzl_list_box_row_dispose;
}

static void
dzl_list_box_row_init (DzlListBoxRow *self)
{
}
