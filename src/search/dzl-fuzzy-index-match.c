/* dzl-fuzzy-index-match.c
 *
 * Copyright (C) 2016 Christian Hergert <christian@hergert.me>
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

#define G_LOG_DOMAIN "dzl-fuzzy-index-match"

#include "config.h"

#include "search/dzl-fuzzy-index-match.h"
#include "util/dzl-macros.h"

struct _DzlFuzzyIndexMatch
{
  GObject   object;
  GVariant *document;
  gchar    *key;
  gfloat    score;
  guint     priority;
};

enum {
  PROP_0,
  PROP_DOCUMENT,
  PROP_KEY,
  PROP_SCORE,
  PROP_PRIORITY,
  N_PROPS
};

G_DEFINE_TYPE (DzlFuzzyIndexMatch, dzl_fuzzy_index_match, G_TYPE_OBJECT)

static GParamSpec *properties [N_PROPS];

static void
dzl_fuzzy_index_match_finalize (GObject *object)
{
  DzlFuzzyIndexMatch *self = (DzlFuzzyIndexMatch *)object;

  g_clear_pointer (&self->document, g_variant_unref);
  g_clear_pointer (&self->key, g_free);

  G_OBJECT_CLASS (dzl_fuzzy_index_match_parent_class)->finalize (object);
}

static void
dzl_fuzzy_index_match_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  DzlFuzzyIndexMatch *self = DZL_FUZZY_INDEX_MATCH (object);

  switch (prop_id)
    {
    case PROP_DOCUMENT:
      g_value_set_variant (value, self->document);
      break;

    case PROP_SCORE:
      g_value_set_float (value, self->score);
      break;

    case PROP_KEY:
      g_value_set_string (value, self->key);
      break;

    case PROP_PRIORITY:
      g_value_set_uint (value, self->priority);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_fuzzy_index_match_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  DzlFuzzyIndexMatch *self = DZL_FUZZY_INDEX_MATCH (object);

  switch (prop_id)
    {
    case PROP_DOCUMENT:
      self->document = g_value_dup_variant (value);
      break;

    case PROP_SCORE:
      self->score = g_value_get_float (value);
      break;

    case PROP_KEY:
      self->key = g_value_dup_string (value);
      break;

    case PROP_PRIORITY:
      self->priority = g_value_get_uint (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
dzl_fuzzy_index_match_class_init (DzlFuzzyIndexMatchClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_fuzzy_index_match_finalize;
  object_class->get_property = dzl_fuzzy_index_match_get_property;
  object_class->set_property = dzl_fuzzy_index_match_set_property;

  properties [PROP_DOCUMENT] =
    g_param_spec_variant ("document",
                          "Document",
                          "Document",
                          G_VARIANT_TYPE_ANY,
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_KEY] =
    g_param_spec_string ("key",
                         "Key",
                         "The string key that was inserted for the document",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_PRIORITY] =
    g_param_spec_uint ("priority",
                       "Priority",
                       "The priority used when creating the index",
                       0,
                       255,
                       0,
                       (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_SCORE] =
    g_param_spec_float ("score",
                        "Score",
                        "Score",
                        -G_MINFLOAT,
                        G_MAXFLOAT,
                        0.0f,
                        (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_fuzzy_index_match_init (DzlFuzzyIndexMatch *match)
{
}

/**
 * dzl_fuzzy_index_match_get_document:
 *
 * Returns: (transfer none): A #GVariant.
 */
GVariant *
dzl_fuzzy_index_match_get_document (DzlFuzzyIndexMatch *self)
{
  g_return_val_if_fail (DZL_IS_FUZZY_INDEX_MATCH (self), NULL);

  return self->document;
}

gfloat
dzl_fuzzy_index_match_get_score (DzlFuzzyIndexMatch *self)
{
  g_return_val_if_fail (DZL_IS_FUZZY_INDEX_MATCH (self), 0.0f);

  return self->score;
}

const gchar *
dzl_fuzzy_index_match_get_key (DzlFuzzyIndexMatch *self)
{
  g_return_val_if_fail (DZL_IS_FUZZY_INDEX_MATCH (self), NULL);

  return self->key;
}

guint
dzl_fuzzy_index_match_get_priority (DzlFuzzyIndexMatch *self)
{
  g_return_val_if_fail (DZL_IS_FUZZY_INDEX_MATCH (self), 0);

  return self->priority;
}
