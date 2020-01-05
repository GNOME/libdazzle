/* dzl-graph-model.c
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
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

#include "config.h"

#define G_LOG_DOMAIN "dzl-graph-model"

#include <glib/gi18n.h>

#include "graphing/dzl-graph-column-private.h"
#include "graphing/dzl-graph-model.h"
#include "util/dzl-macros.h"

typedef struct
{
  GPtrArray       *columns;
  DzlGraphColumn  *timestamps;

  guint            last_index;

  guint            max_samples;
  GTimeSpan        timespan;
  gdouble          value_max;
  gdouble          value_min;
} DzlGraphModelPrivate;

typedef struct
{
  DzlGraphModel *table;
  gint64         timestamp;
  guint          index;
} DzlGraphModelIterImpl;

enum {
  PROP_0,
  PROP_MAX_SAMPLES,
  PROP_TIMESPAN,
  PROP_VALUE_MAX,
  PROP_VALUE_MIN,
  LAST_PROP
};

enum {
  CHANGED,
  LAST_SIGNAL
};

G_DEFINE_TYPE_WITH_PRIVATE (DzlGraphModel, dzl_graph_view_model, G_TYPE_OBJECT)

static GParamSpec *properties [LAST_PROP];
static guint signals [LAST_SIGNAL];

gint64
dzl_graph_view_model_get_timespan (DzlGraphModel *self)
{
  DzlGraphModelPrivate *priv = dzl_graph_view_model_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_GRAPH_MODEL (self), 0);

  return priv->timespan;
}

void
dzl_graph_view_model_set_timespan (DzlGraphModel *self,
                                   GTimeSpan      timespan)
{
  DzlGraphModelPrivate *priv = dzl_graph_view_model_get_instance_private (self);

  g_return_if_fail (DZL_IS_GRAPH_MODEL (self));

  if (timespan != priv->timespan)
    {
      priv->timespan = timespan;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_TIMESPAN]);
    }
}

static void
dzl_graph_view_model_set_value_max (DzlGraphModel *self,
                                    gdouble        value_max)
{
  DzlGraphModelPrivate *priv = dzl_graph_view_model_get_instance_private (self);

  g_return_if_fail (DZL_IS_GRAPH_MODEL (self));

  if (priv->value_max != value_max)
    {
      priv->value_max = value_max;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_VALUE_MAX]);
    }
}

static void
dzl_graph_view_model_set_value_min (DzlGraphModel *self,
                                    gdouble        value_min)
{
  DzlGraphModelPrivate *priv = dzl_graph_view_model_get_instance_private (self);

  g_return_if_fail (DZL_IS_GRAPH_MODEL (self));

  if (priv->value_min != value_min)
    {
      priv->value_min = value_min;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_VALUE_MIN]);
    }
}

DzlGraphModel *
dzl_graph_view_model_new (void)
{
  return g_object_new (DZL_TYPE_GRAPH_MODEL, NULL);
}

guint
dzl_graph_view_model_add_column (DzlGraphModel  *self,
                                 DzlGraphColumn *column)
{
  DzlGraphModelPrivate *priv = dzl_graph_view_model_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_GRAPH_MODEL (self), 0);
  g_return_val_if_fail (DZL_IS_GRAPH_COLUMN (column), 0);

  _dzl_graph_view_column_set_n_rows (column, priv->max_samples);

  g_ptr_array_add (priv->columns, g_object_ref (column));

  return priv->columns->len - 1;
}

guint
dzl_graph_view_model_get_n_columns (DzlGraphModel  *self)
{
  DzlGraphModelPrivate *priv = dzl_graph_view_model_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_GRAPH_MODEL (self), 0);

  return priv->columns->len;
}

guint
dzl_graph_view_model_get_max_samples (DzlGraphModel *self)
{
  DzlGraphModelPrivate *priv = dzl_graph_view_model_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_GRAPH_MODEL (self), 0);

  return priv->max_samples;
}

void
dzl_graph_view_model_set_max_samples (DzlGraphModel *self,
                                      guint          max_samples)
{
  DzlGraphModelPrivate *priv = dzl_graph_view_model_get_instance_private (self);
  gsize i;

  g_return_if_fail (DZL_IS_GRAPH_MODEL (self));
  g_return_if_fail (max_samples > 0);

  if (max_samples == priv->max_samples)
    return;

  for (i = 0; i < priv->columns->len; i++)
    {
      DzlGraphColumn *column;

      column = g_ptr_array_index (priv->columns, i);
      _dzl_graph_view_column_set_n_rows (column, max_samples);
    }

  _dzl_graph_view_column_set_n_rows (priv->timestamps, max_samples);

  priv->max_samples = max_samples;

  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_MAX_SAMPLES]);
}

/**
 * dzl_graph_view_model_push:
 * @self: Table to push to
 * @iter: (out): Newly created #DzlGraphModelIter
 * @timestamp: Time of new event
 */
void
dzl_graph_view_model_push (DzlGraphModel     *self,
                           DzlGraphModelIter *iter,
                           gint64             timestamp)
{
  DzlGraphModelPrivate *priv = dzl_graph_view_model_get_instance_private (self);
  DzlGraphModelIterImpl *impl = (DzlGraphModelIterImpl *)iter;
  guint pos;
  gsize i;

  g_return_if_fail (DZL_IS_GRAPH_MODEL (self));
  g_return_if_fail (iter != NULL);
  g_return_if_fail (timestamp > 0);

  for (i = 0; i < priv->columns->len; i++)
    {
      DzlGraphColumn *column;

      column = g_ptr_array_index (priv->columns, i);
      _dzl_graph_view_column_push (column);
    }

  pos = _dzl_graph_view_column_push (priv->timestamps);
  _dzl_graph_view_column_set (priv->timestamps, pos, timestamp);

  impl->table = self;
  impl->timestamp = timestamp;
  impl->index = pos;

  priv->last_index = pos;

  g_signal_emit (self, signals [CHANGED], 0);
}

gboolean
dzl_graph_view_model_get_iter_last (DzlGraphModel     *self,
                                    DzlGraphModelIter *iter)
{
  DzlGraphModelPrivate *priv = dzl_graph_view_model_get_instance_private (self);
  DzlGraphModelIterImpl *impl = (DzlGraphModelIterImpl *)iter;

  g_return_val_if_fail (DZL_IS_GRAPH_MODEL (self), FALSE);
  g_return_val_if_fail (iter != NULL, FALSE);
  g_return_val_if_fail (impl != NULL, FALSE);

  impl->table = self;
  impl->index = priv->last_index;
  impl->timestamp = 0;

  _dzl_graph_view_column_get (priv->timestamps, impl->index, &impl->timestamp);

  return (impl->timestamp != 0);
}

gint64
dzl_graph_view_model_get_end_time (DzlGraphModel *self)
{
  DzlGraphModelIter iter;

  g_return_val_if_fail (DZL_IS_GRAPH_MODEL (self), 0);

  if (dzl_graph_view_model_get_iter_last (self, &iter))
    return dzl_graph_view_model_iter_get_timestamp (&iter);

  return g_get_monotonic_time ();
}

gboolean
dzl_graph_view_model_get_iter_first (DzlGraphModel     *self,
                                     DzlGraphModelIter *iter)
{
  DzlGraphModelPrivate *priv = dzl_graph_view_model_get_instance_private (self);
  DzlGraphModelIterImpl *impl = (DzlGraphModelIterImpl *)iter;

  g_return_val_if_fail (DZL_IS_GRAPH_MODEL (self), FALSE);
  g_return_val_if_fail (iter != NULL, FALSE);
  g_return_val_if_fail (impl != NULL, FALSE);

  impl->table = self;
  impl->index = (priv->last_index + 1) % priv->max_samples;
  impl->timestamp = 0;

  _dzl_graph_view_column_get (priv->timestamps, impl->index, &impl->timestamp);

  /*
   * Maybe this is our first time around the ring, and we can just
   * assume the 0 index is the real first entry.
   */
  if (impl->timestamp == 0)
    {
      impl->index = 0;
      _dzl_graph_view_column_get (priv->timestamps, impl->index, &impl->timestamp);
    }

  return (impl->timestamp != 0);
}

gboolean
dzl_graph_view_model_iter_next (DzlGraphModelIter *iter)
{
  DzlGraphModelPrivate *priv;
  DzlGraphModelIterImpl *impl = (DzlGraphModelIterImpl *)iter;

  g_return_val_if_fail (iter != NULL, FALSE);
  g_return_val_if_fail (impl != NULL, FALSE);
  g_return_val_if_fail (DZL_IS_GRAPH_MODEL (impl->table), FALSE);

  priv = dzl_graph_view_model_get_instance_private (impl->table);

  if (impl->index == priv->last_index)
    {
      impl->table = NULL;
      impl->index = 0;
      impl->timestamp = 0;
      return FALSE;
    }

  do
    {
      impl->index = (impl->index + 1) % priv->max_samples;

      impl->timestamp = 0;
      _dzl_graph_view_column_get (priv->timestamps, impl->index, &impl->timestamp);

      if (impl->timestamp > 0)
        break;
    }
  while (impl->index < priv->last_index);

  return (impl->timestamp > 0);
}

gint64
dzl_graph_view_model_iter_get_timestamp (DzlGraphModelIter *iter)
{
  DzlGraphModelIterImpl *impl = (DzlGraphModelIterImpl *)iter;

  g_return_val_if_fail (iter != NULL, 0);

  return impl->timestamp;
}

void
dzl_graph_view_model_iter_set (DzlGraphModelIter *iter,
                               gint               first_column,
                               ...)
{
  DzlGraphModelIterImpl *impl = (DzlGraphModelIterImpl *)iter;
  DzlGraphModelPrivate *priv;
  gint column_id = first_column;
  va_list args;

  g_return_if_fail (iter != NULL);
  g_return_if_fail (impl != NULL);
  g_return_if_fail (DZL_IS_GRAPH_MODEL (impl->table));

  priv = dzl_graph_view_model_get_instance_private (impl->table);

  va_start (args, first_column);

  while (column_id >= 0)
    {
      DzlGraphColumn *column;

      if (column_id >= (gint)priv->columns->len)
        {
          g_critical ("No such column %d", column_id);
          goto cleanup;
        }

      column = g_ptr_array_index (priv->columns, column_id);

      _dzl_graph_view_column_collect (column, impl->index, &args);

      column_id = va_arg (args, gint);
    }

  if (column_id != -1)
    g_critical ("Invalid column sentinel: %d", column_id);

cleanup:
  va_end (args);
}

void
dzl_graph_view_model_iter_get (DzlGraphModelIter *iter,
                               gint               first_column,
                               ...)
{
  DzlGraphModelIterImpl *impl = (DzlGraphModelIterImpl *)iter;
  DzlGraphModelPrivate *priv;
  gint column_id = first_column;
  va_list args;

  g_return_if_fail (iter != NULL);
  g_return_if_fail (impl != NULL);
  g_return_if_fail (DZL_IS_GRAPH_MODEL (impl->table));

  priv = dzl_graph_view_model_get_instance_private (impl->table);

  va_start (args, first_column);

  while (column_id >= 0)
    {
      DzlGraphColumn *column;

      if (column_id >= (gint)priv->columns->len)
        {
          g_critical ("No such column %d", column_id);
          goto cleanup;
        }

      column = g_ptr_array_index (priv->columns, column_id);

      _dzl_graph_view_column_lcopy (column, impl->index, &args);

      column_id = va_arg (args, gint);
    }

  if (column_id != -1)
    g_critical ("Invalid column sentinel: %d", column_id);

cleanup:
  va_end (args);
}

void
dzl_graph_view_model_iter_get_value (DzlGraphModelIter *iter,
                                     guint              column,
                                     GValue            *value)
{
  DzlGraphModelIterImpl *impl = (DzlGraphModelIterImpl *)iter;
  DzlGraphModelPrivate *priv;
  DzlGraphColumn *col;

  g_return_if_fail (iter != NULL);
  g_return_if_fail (impl != NULL);
  g_return_if_fail (DZL_IS_GRAPH_MODEL (impl->table));
  priv = dzl_graph_view_model_get_instance_private (impl->table);
  g_return_if_fail (column < priv->columns->len);

  col = g_ptr_array_index (priv->columns, column);
  _dzl_graph_view_column_get_value (col, impl->index, value);
}

/**
 * dzl_graph_view_model_iter_set_value: (rename-to dzl_graph_view_model_iter_set)
 * @iter: the iter to set
 * @column: the column to set
 * @value: the new value for the column
 *
 * Sets an individual value within a specific column.
 *
 * Since: 3.30
 */
void
dzl_graph_view_model_iter_set_value (DzlGraphModelIter *iter,
                                     guint              column,
                                     const GValue      *value)
{
  DzlGraphModelIterImpl *impl = (DzlGraphModelIterImpl *)iter;
  DzlGraphModelPrivate *priv;
  DzlGraphColumn *col;

  g_return_if_fail (iter != NULL);
  g_return_if_fail (impl != NULL);
  g_return_if_fail (DZL_IS_GRAPH_MODEL (impl->table));
  priv = dzl_graph_view_model_get_instance_private (impl->table);
  g_return_if_fail (column < priv->columns->len);

  col = g_ptr_array_index (priv->columns, column);

  g_assert (col != NULL);

  _dzl_graph_view_column_set_value (col, impl->index, value);
}

static void
dzl_graph_view_model_finalize (GObject *object)
{
  DzlGraphModel *self = (DzlGraphModel *)object;
  DzlGraphModelPrivate *priv = dzl_graph_view_model_get_instance_private (self);

  g_clear_pointer (&priv->columns, g_ptr_array_unref);
  g_clear_object (&priv->timestamps);

  G_OBJECT_CLASS (dzl_graph_view_model_parent_class)->finalize (object);
}

static void
dzl_graph_view_model_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  DzlGraphModel *self = (DzlGraphModel *)object;
  DzlGraphModelPrivate *priv = dzl_graph_view_model_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_TIMESPAN:
      g_value_set_int64 (value, priv->timespan);
      break;

    case PROP_MAX_SAMPLES:
      g_value_set_uint (value, priv->max_samples);
      break;

    case PROP_VALUE_MAX:
      g_value_set_double (value, priv->value_max);
      break;

    case PROP_VALUE_MIN:
      g_value_set_double (value, priv->value_min);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_graph_view_model_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  DzlGraphModel *self = (DzlGraphModel *)object;

  switch (prop_id)
    {
    case PROP_MAX_SAMPLES:
      dzl_graph_view_model_set_max_samples (self, g_value_get_uint (value));
      break;

    case PROP_TIMESPAN:
      dzl_graph_view_model_set_timespan (self, g_value_get_int64 (value));
      break;

    case PROP_VALUE_MAX:
      dzl_graph_view_model_set_value_max (self, g_value_get_double (value));
      break;

    case PROP_VALUE_MIN:
      dzl_graph_view_model_set_value_min (self, g_value_get_double (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_graph_view_model_class_init (DzlGraphModelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_graph_view_model_finalize;
  object_class->get_property = dzl_graph_view_model_get_property;
  object_class->set_property = dzl_graph_view_model_set_property;

  properties [PROP_MAX_SAMPLES] =
    g_param_spec_uint ("max-samples",
                       "Max Samples",
                       "Max Samples",
                       1, G_MAXUINT,
                       120,
                       (G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));

  properties [PROP_TIMESPAN] =
    g_param_spec_int64 ("timespan",
                        "Timespan",
                        "Timespan to visualize, in microseconds.",
                        1, G_MAXINT64,
                        G_USEC_PER_SEC * 60L,
                        (G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS));

  properties [PROP_VALUE_MAX] =
    g_param_spec_double ("value-max",
                         "Value Max",
                         "Value Max",
                         -G_MAXDOUBLE, G_MAXDOUBLE,
                         100.0,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_VALUE_MIN] =
    g_param_spec_double ("value-min",
                         "Value Min",
                         "Value Min",
                         -G_MAXDOUBLE, G_MAXDOUBLE,
                         100.0,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);

  signals [CHANGED] = g_signal_new ("changed",
                                     G_TYPE_FROM_CLASS (klass),
                                     G_SIGNAL_RUN_LAST,
                                     0,
                                     NULL, NULL, NULL,
                                     G_TYPE_NONE,
                                     0);
  g_signal_set_va_marshaller (signals [CHANGED],
                              G_TYPE_FROM_CLASS (klass),
                              g_cclosure_marshal_VOID__VOIDv);
}

static void
dzl_graph_view_model_init (DzlGraphModel *self)
{
  DzlGraphModelPrivate *priv = dzl_graph_view_model_get_instance_private (self);

  priv->max_samples = 60;
  priv->value_min = 0.0;
  priv->value_max = 100.0;

  priv->columns = g_ptr_array_new_with_free_func (g_object_unref);

  priv->timestamps = dzl_graph_view_column_new (NULL, G_TYPE_INT64);
  _dzl_graph_view_column_set_n_rows (priv->timestamps, priv->max_samples);
}
