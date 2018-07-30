/* dzl-counters-window.c
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

#define G_LOG_DOMAIN "dzl-counters-window"

#include "config.h"

#include "util/dzl-macros.h"
#include "widgets/dzl-counters-window.h"

typedef struct
{
  GtkTreeView         *tree_view;
  GtkTreeStore        *tree_store;
  GtkTreeViewColumn   *value_column;
  GtkCellRendererText *value_cell;

  DzlCounterArena     *arena;

  guint                update_source;
} DzlCountersWindowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (DzlCountersWindow, dzl_counters_window, GTK_TYPE_WINDOW)

static void
get_value_cell_data_func (GtkCellLayout   *cell_layout,
                          GtkCellRenderer *cell,
                          GtkTreeModel    *model,
                          GtkTreeIter     *iter,
                          gpointer         user_data)
{
  static PangoAttrList *attrs;
  DzlCounter *counter = NULL;
  g_autofree gchar *str = NULL;
  gint64 value = 0;

  if G_UNLIKELY (attrs == NULL)
    {
      attrs = pango_attr_list_new ();
      pango_attr_list_insert (attrs, pango_attr_foreground_alpha_new (0.35 * G_MAXUSHORT));
    }

  gtk_tree_model_get (model, iter, 0, &counter, -1);

  if (counter != NULL)
    value = dzl_counter_get (counter);

  str = g_strdup_printf ("%"G_GINT64_FORMAT, value);

  g_object_set (cell,
                "attributes", value == 0 ? attrs : NULL,
                "text", str,
                NULL);
}

static gboolean
update_display (gpointer data)
{
  DzlCountersWindow *self = data;
  DzlCountersWindowPrivate *priv = dzl_counters_window_get_instance_private (self);
  GdkWindow *window;

  g_assert (DZL_IS_COUNTERS_WINDOW (self));

  window = gtk_tree_view_get_bin_window (priv->tree_view);
  gdk_window_invalidate_rect (window, NULL, FALSE);

  return G_SOURCE_CONTINUE;
}

static void
dzl_counters_window_realize (GtkWidget *widget)
{
  DzlCountersWindow *self = (DzlCountersWindow *)widget;
  DzlCountersWindowPrivate *priv = dzl_counters_window_get_instance_private (self);

  g_assert (DZL_IS_COUNTERS_WINDOW (self));

  priv->update_source =
    g_timeout_add_seconds_full (G_PRIORITY_LOW, 1, update_display, self, NULL);

  GTK_WIDGET_CLASS (dzl_counters_window_parent_class)->realize (widget);
}

static void
dzl_counters_window_unrealize (GtkWidget *widget)
{
  DzlCountersWindow *self = (DzlCountersWindow *)widget;
  DzlCountersWindowPrivate *priv = dzl_counters_window_get_instance_private (self);

  g_assert (DZL_IS_COUNTERS_WINDOW (self));

  if (priv->update_source != 0)
    {
      g_source_remove (priv->update_source);
      priv->update_source = 0;
    }

  GTK_WIDGET_CLASS (dzl_counters_window_parent_class)->unrealize (widget);
}

static void
dzl_counters_window_finalize (GObject *object)
{
  DzlCountersWindow *self = (DzlCountersWindow *)object;
  DzlCountersWindowPrivate *priv = dzl_counters_window_get_instance_private (self);

  g_clear_pointer (&priv->arena, dzl_counter_arena_unref);
  g_clear_object (&priv->tree_store);

  G_OBJECT_CLASS (dzl_counters_window_parent_class)->finalize (object);
}

static void
dzl_counters_window_class_init (DzlCountersWindowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = dzl_counters_window_finalize;

  widget_class->realize = dzl_counters_window_realize;
  widget_class->unrealize = dzl_counters_window_unrealize;

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/dazzle/ui/dzl-counters-window.ui");
  gtk_widget_class_bind_template_child_private (widget_class, DzlCountersWindow, tree_view);
  gtk_widget_class_bind_template_child_private (widget_class, DzlCountersWindow, value_cell);
  gtk_widget_class_bind_template_child_private (widget_class, DzlCountersWindow, value_column);
}

static void
dzl_counters_window_init (DzlCountersWindow *self)
{
  DzlCountersWindowPrivate *priv = dzl_counters_window_get_instance_private (self);

  gtk_widget_init_template (GTK_WIDGET (self));

  priv->tree_store = gtk_tree_store_new (4, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
  gtk_tree_view_set_model (priv->tree_view, GTK_TREE_MODEL (priv->tree_store));

  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (priv->value_column),
                                      GTK_CELL_RENDERER (priv->value_cell),
                                      get_value_cell_data_func,
                                      NULL, NULL);
}

static void
foreach_counter_cb (DzlCounter *counter,
                    gpointer    user_data)
{
  DzlCountersWindow *self = user_data;
  DzlCountersWindowPrivate *priv = dzl_counters_window_get_instance_private (self);
  GtkTreeIter iter;

  g_assert (DZL_IS_COUNTERS_WINDOW (self));

  gtk_tree_store_append (priv->tree_store, &iter, NULL);
  gtk_tree_store_set (priv->tree_store, &iter,
                      0, counter,
                      1, counter->category,
                      2, counter->name,
                      3, counter->description,
                      -1);
}

static void
dzl_counters_window_reload (DzlCountersWindow *self)
{
  DzlCountersWindowPrivate *priv = dzl_counters_window_get_instance_private (self);

  g_assert (DZL_IS_COUNTERS_WINDOW (self));

  gtk_tree_store_clear (priv->tree_store);
  if (priv->arena == NULL)
    return;

  dzl_counter_arena_foreach (priv->arena, foreach_counter_cb, self);
}

GtkWidget *
dzl_counters_window_new (void)
{
  return g_object_new (DZL_TYPE_COUNTERS_WINDOW, NULL);
}

/**
 * dzl_counters_window_get_arena:
 * @self: a #DzlCountersWindow
 *
 * Gets the currently viewed arena, if any.
 *
 * Returns: (transfer none) (nullable): A #DzlCounterArena or %NULL.
 */
DzlCounterArena *
dzl_counters_window_get_arena (DzlCountersWindow *self)
{
  DzlCountersWindowPrivate *priv = dzl_counters_window_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_COUNTERS_WINDOW (self), NULL);

  return priv->arena;
}

void
dzl_counters_window_set_arena (DzlCountersWindow *self,
                               DzlCounterArena   *arena)
{
  DzlCountersWindowPrivate *priv = dzl_counters_window_get_instance_private (self);

  g_return_if_fail (DZL_IS_COUNTERS_WINDOW (self));

  if (arena != priv->arena)
    {
      g_clear_pointer (&priv->arena, dzl_counter_arena_unref);
      if (arena != NULL)
        priv->arena = dzl_counter_arena_ref (arena);
      dzl_counters_window_reload (self);
    }
}
