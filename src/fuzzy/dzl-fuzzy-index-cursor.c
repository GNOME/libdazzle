/* dzl-fuzzy-index-cursor.c
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

#define G_LOG_DOMAIN "dzl-fuzzy-index-cursor"

#include <string.h>

#include "fuzzy/dzl-fuzzy-index-cursor.h"
#include "fuzzy/dzl-fuzzy-index-match.h"
#include "fuzzy/dzl-fuzzy-index-private.h"

struct _DzlFuzzyIndexCursor
{
  GObject          object;

  DzlFuzzyIndex   *index;
  gchar           *query;
  GVariantDict    *tables;
  GArray          *matches;
  guint            max_matches;
  guint            case_sensitive : 1;
};

typedef struct
{
  guint position;
  guint lookaside_id;
} DzlFuzzyIndexItem;

typedef struct
{
  const gchar *key;
  guint        document_id;
  gfloat       score;
  guint        priority;
} DzlFuzzyMatch;

typedef struct
{
  DzlFuzzyIndex                   *index;
  const DzlFuzzyIndexItem * const *tables;
  const gsize                     *tables_n_elements;
  gint                            *tables_state;
  guint                            n_tables;
  guint                            max_matches;
  const gchar                     *needle;
  GHashTable                      *matches;
} DzlFuzzyLookup;

enum {
  PROP_0,
  PROP_CASE_SENSITIVE,
  PROP_INDEX,
  PROP_TABLES,
  PROP_MAX_MATCHES,
  PROP_QUERY,
  N_PROPS
};

static void async_initable_iface_init (GAsyncInitableIface *iface);
static void list_model_iface_init     (GListModelInterface *iface);

static GParamSpec *properties [N_PROPS];

G_DEFINE_TYPE_EXTENDED (DzlFuzzyIndexCursor, dzl_fuzzy_index_cursor, G_TYPE_OBJECT, 0,
                        G_IMPLEMENT_INTERFACE (G_TYPE_ASYNC_INITABLE, async_initable_iface_init)
                        G_IMPLEMENT_INTERFACE (G_TYPE_LIST_MODEL, list_model_iface_init))

static inline gfloat
pointer_to_float (gpointer ptr)
{
  union {
    gpointer ptr;
#if __WORDSIZE == 64
    gdouble fval;
#else
    gfloat fval;
#endif
  } convert;
  convert.ptr = ptr;
  return (gfloat)convert.fval;
}

static inline gpointer
float_to_pointer (gfloat fval)
{
  union {
    gpointer ptr;
#if __WORDSIZE == 64
    gdouble fval;
#else
    gfloat fval;
#endif
  } convert;
  convert.fval = fval;
  return convert.ptr;
}

static void
dzl_fuzzy_index_cursor_finalize (GObject *object)
{
  DzlFuzzyIndexCursor *self = (DzlFuzzyIndexCursor *)object;

  g_clear_object (&self->index);
  g_clear_pointer (&self->query, g_free);
  g_clear_pointer (&self->matches, g_array_unref);
  g_clear_pointer (&self->tables, g_variant_dict_unref);

  G_OBJECT_CLASS (dzl_fuzzy_index_cursor_parent_class)->finalize (object);
}

static void
dzl_fuzzy_index_cursor_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  DzlFuzzyIndexCursor *self = DZL_FUZZY_INDEX_CURSOR(object);

  switch (prop_id)
    {
    case PROP_CASE_SENSITIVE:
      g_value_set_boolean (value, self->case_sensitive);
      break;

    case PROP_INDEX:
      g_value_set_object (value, self->index);
      break;

    case PROP_MAX_MATCHES:
      g_value_set_uint (value, self->max_matches);
      break;

    case PROP_QUERY:
      g_value_set_string (value, self->query);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_fuzzy_index_cursor_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  DzlFuzzyIndexCursor *self = DZL_FUZZY_INDEX_CURSOR(object);

  switch (prop_id)
    {
    case PROP_CASE_SENSITIVE:
      self->case_sensitive = g_value_get_boolean (value);
      break;

    case PROP_INDEX:
      self->index = g_value_dup_object (value);
      break;

    case PROP_TABLES:
      self->tables = g_value_dup_boxed (value);
      break;

    case PROP_MAX_MATCHES:
      self->max_matches = g_value_get_uint (value);
      break;

    case PROP_QUERY:
      self->query = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_fuzzy_index_cursor_class_init (DzlFuzzyIndexCursorClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_fuzzy_index_cursor_finalize;
  object_class->get_property = dzl_fuzzy_index_cursor_get_property;
  object_class->set_property = dzl_fuzzy_index_cursor_set_property;

  properties [PROP_CASE_SENSITIVE] =
    g_param_spec_boolean ("case-sensitive",
                          "Case Sensitive",
                          "Case Sensitive",
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_INDEX] =
    g_param_spec_object ("index",
                         "Index",
                         "The index this cursor is iterating",
                         DZL_TYPE_FUZZY_INDEX,
                         (G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_TABLES] =
    g_param_spec_boxed ("tables",
                        "Tables",
                        "The dictionary of character indexes",
                        G_TYPE_VARIANT_DICT,
                        (G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_QUERY] =
    g_param_spec_string ("query",
                         "Query",
                         "The query for the index",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_MAX_MATCHES] =
    g_param_spec_uint ("max-matches",
                       "Max Matches",
                       "The max number of matches to display",
                       0,
                       G_MAXUINT,
                       0,
                       (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_fuzzy_index_cursor_init (DzlFuzzyIndexCursor *self)
{
  self->matches = g_array_new (FALSE, FALSE, sizeof (DzlFuzzyMatch));
}

static gint
fuzzy_match_compare (gconstpointer a,
                     gconstpointer b)
{
  const DzlFuzzyMatch *ma = a;
  const DzlFuzzyMatch *mb = b;

  if (ma->score < mb->score)
    return 1;
  else if (ma->score > mb->score)
    return -1;

  return strcmp (ma->key, mb->key);
}

static gboolean
fuzzy_do_match (const DzlFuzzyLookup    *lookup,
                const DzlFuzzyIndexItem *item,
                guint                    table_index,
                gint                     score)
{
  const DzlFuzzyIndexItem *table;
  const DzlFuzzyIndexItem *iter;
  gssize n_elements;
  gint *state;
  gint iter_score;

  g_assert (lookup != NULL);
  g_assert (item != NULL);
  g_assert (score >= 0);

  table = lookup->tables [table_index];
  n_elements = (gssize)lookup->tables_n_elements [table_index];
  state = &lookup->tables_state [table_index];

  for (; state [0] < n_elements; state [0]++)
    {
      gpointer lookup_score;
      gboolean contains_document;

      iter = &table [state [0]];

      if ((iter->lookaside_id < item->lookaside_id) ||
          ((iter->lookaside_id == item->lookaside_id) &&
           (iter->position <= item->position)))
        continue;
      else if (iter->lookaside_id > item->lookaside_id)
        break;

      iter_score = score + (iter->position - item->position);

      if (table_index + 1 < lookup->n_tables)
        {
          if (fuzzy_do_match (lookup, iter, table_index + 1, iter_score))
            return TRUE;
          continue;
        }

      contains_document = g_hash_table_lookup_extended (lookup->matches,
                                                        GUINT_TO_POINTER (item->lookaside_id),
                                                        NULL,
                                                        (gpointer *)&lookup_score);

      if (!contains_document || iter_score < GPOINTER_TO_INT (lookup_score))
        g_hash_table_insert (lookup->matches,
                             GUINT_TO_POINTER (item->lookaside_id),
                             GINT_TO_POINTER (iter_score));

      return TRUE;
    }

  return FALSE;
}

static void
dzl_fuzzy_index_cursor_worker (GTask        *task,
                               gpointer      source_object,
                               gpointer      task_data,
                               GCancellable *cancellable)
{
  DzlFuzzyIndexCursor *self = source_object;
  g_autoptr(GHashTable) matches = NULL;
  g_autoptr(GHashTable) by_document = NULL;
  g_autoptr(GPtrArray) tables = NULL;
  g_autoptr(GArray) tables_n_elements = NULL;
  g_autofree gint *tables_state = NULL;
  g_autofree gchar *freeme = NULL;
  const gchar *query;
  DzlFuzzyLookup lookup = { 0 };
  GHashTableIter iter;
  const gchar *str;
  gpointer key, value;
  guint i;

  g_assert (DZL_IS_FUZZY_INDEX_CURSOR (self));
  g_assert (G_IS_TASK (task));

  if (g_task_return_error_if_cancelled (task))
    return;

  /* No matches with empty query */
  if (self->query == NULL || *self->query == '\0')
    goto cleanup;

  /* If we are not case-sensitive, we need to downcase the query string */
  query = self->query;
  if (!self->case_sensitive)
    query = freeme = g_utf8_casefold (query, -1);

  tables = g_ptr_array_new ();
  tables_n_elements = g_array_new (FALSE, FALSE, sizeof (gsize));
  matches = g_hash_table_new (NULL, NULL);

  for (str = query; *str; str = g_utf8_next_char (str))
    {
      gunichar ch = g_utf8_get_char (str);
      g_autoptr(GVariant) table = NULL;
      gconstpointer fixed;
      gsize n_elements;
      gchar char_key[8];

      if (g_unichar_isspace (ch))
        continue;

      char_key [g_unichar_to_utf8 (ch, char_key)] = '\0';
      table = g_variant_dict_lookup_value (self->tables,
                                           char_key,
                                           (const GVariantType *)"a(uu)");

      /* No possible matches, missing table for character */
      if (table == NULL)
        goto cleanup;

      fixed = g_variant_get_fixed_array (table, &n_elements, sizeof (DzlFuzzyIndexItem));
      g_array_append_val (tables_n_elements, n_elements);
      g_ptr_array_add (tables, (gpointer)fixed);
    }

  if (tables->len == 0)
    goto cleanup;

  g_assert (tables->len > 0);
  g_assert (tables->len == tables_n_elements->len);

  tables_state = g_new0 (gint, tables->len);

  lookup.index = self->index;
  lookup.matches = matches;
  lookup.tables = (const DzlFuzzyIndexItem * const *)tables->pdata;
  lookup.tables_n_elements = (const gsize *)tables_n_elements->data;
  lookup.tables_state = tables_state;
  lookup.n_tables = tables->len;
  lookup.needle = query;
  lookup.max_matches = self->max_matches;

  if G_LIKELY (lookup.n_tables > 1)
    {
      for (i = 0; i < lookup.tables_n_elements[0]; i++)
        {
          const DzlFuzzyIndexItem *item = &lookup.tables[0][i];

          fuzzy_do_match (&lookup, item, 1, MIN (16, item->position * 4));
        }
    }
  else
    {
      guint last_id = G_MAXUINT;

      for (i = 0; i < lookup.tables_n_elements[0]; i++)
        {
          const DzlFuzzyIndexItem *item = &lookup.tables[0][i];
          DzlFuzzyMatch match;
          gfloat penalty;

          if (item->lookaside_id != last_id)
            {
              last_id = item->lookaside_id;

              if G_UNLIKELY (!_dzl_fuzzy_index_resolve (self->index,
                                                        item->lookaside_id,
                                                        &match.document_id,
                                                        &match.key,
                                                        &penalty,
                                                        &match.priority))
                continue;

              match.score = penalty + ((0.9 / 255.0) * (1.0 / (strlen (match.key) + item->position)));

              g_array_append_val (self->matches, match);
            }
        }

      goto cleanup;
    }

  if (g_task_return_error_if_cancelled (task))
    return;

  by_document = g_hash_table_new (NULL, NULL);

  g_hash_table_iter_init (&iter, matches);

  while (g_hash_table_iter_next (&iter, &key, &value))
    {
      guint lookaside_id = GPOINTER_TO_UINT (key);
      guint score = GPOINTER_TO_UINT (value);
      gpointer other_score;
      DzlFuzzyMatch match;
      gfloat penalty;

      if G_UNLIKELY (!_dzl_fuzzy_index_resolve (self->index,
                                                lookaside_id,
                                                &match.document_id,
                                                &match.key,
                                                &penalty,
                                                &match.priority))
        continue;

      match.score = penalty + ((0.9 / 255.0) * (1.0 / (strlen (match.key) + score)));

      if (g_hash_table_lookup_extended (by_document,
                                        GUINT_TO_POINTER (match.document_id),
                                        NULL,
                                        &other_score) &&
          match.score <= pointer_to_float (other_score))
        continue;

      g_hash_table_insert (by_document,
                           GUINT_TO_POINTER (match.document_id),
                           float_to_pointer (match.score));

      g_array_append_val (self->matches, match);
    }

  /*
   * Because we have to do the deduplication of documents after
   * searching all the potential matches, we could have duplicates
   * in the array. So we need to filter them out. Not that big
   * of a deal, since we can do remove_fast on the index and
   * we have to sort them afterwards anyway. Potentially, we could
   * do both at once if we felt like doing our own sort.
   */
  for (i = 0; i < self->matches->len; i++)
    {
      DzlFuzzyMatch *match;
      gpointer other_score;

    next:
      match = &g_array_index (self->matches, DzlFuzzyMatch, i);
      other_score = g_hash_table_lookup (by_document, GUINT_TO_POINTER (match->document_id));

      /*
       * This item should have been discarded, but we didn't have enough
       * information at the time we built the array.
       */
      if (pointer_to_float (other_score) < match->score)
        {
          g_array_remove_index_fast (self->matches, i);
          if (i < self->matches->len - 1)
            goto next;
        }
    }

  if (g_task_return_error_if_cancelled (task))
    return;

cleanup:
  if (self->matches != NULL)
    {
      g_array_sort (self->matches, fuzzy_match_compare);
      if (lookup.max_matches > 0 && lookup.max_matches < self->matches->len)
        g_array_set_size (self->matches, lookup.max_matches);
    }

  g_task_return_boolean (task, TRUE);
}

static void
dzl_fuzzy_index_cursor_init_async (GAsyncInitable      *initable,
                               gint                 io_priority,
                               GCancellable        *cancellable,
                               GAsyncReadyCallback  callback,
                               gpointer             user_data)
{
  DzlFuzzyIndexCursor *self = (DzlFuzzyIndexCursor *)initable;
  g_autoptr(GTask) task = NULL;

  g_assert (DZL_IS_FUZZY_INDEX_CURSOR (self));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  task = g_task_new (self, cancellable, callback, user_data);
  g_task_set_source_tag (task, dzl_fuzzy_index_cursor_init_async);
  g_task_set_priority (task, io_priority);
  g_task_set_check_cancellable (task, FALSE);
  g_task_run_in_thread (task, dzl_fuzzy_index_cursor_worker);
}

static gboolean
dzl_fuzzy_index_cursor_init_finish (GAsyncInitable  *initiable,
                                GAsyncResult    *result,
                                GError         **error)
{
  g_assert (DZL_IS_FUZZY_INDEX_CURSOR (initiable));
  g_assert (G_IS_TASK (result));

  return g_task_propagate_boolean (G_TASK (result), error);
}

static void
async_initable_iface_init (GAsyncInitableIface *iface)
{
  iface->init_async = dzl_fuzzy_index_cursor_init_async;
  iface->init_finish = dzl_fuzzy_index_cursor_init_finish;
}

static GType
dzl_fuzzy_index_cursor_get_item_type (GListModel *model)
{
  return DZL_TYPE_FUZZY_INDEX_MATCH;
}

static guint
dzl_fuzzy_index_cursor_get_n_items (GListModel *model)
{
  DzlFuzzyIndexCursor *self = (DzlFuzzyIndexCursor *)model;

  g_assert (DZL_IS_FUZZY_INDEX_CURSOR (self));

  return self->matches->len;
}

static gpointer
dzl_fuzzy_index_cursor_get_item (GListModel *model,
                                 guint       position)
{
  DzlFuzzyIndexCursor *self = (DzlFuzzyIndexCursor *)model;
  g_autoptr(GVariant) document = NULL;
  DzlFuzzyMatch *match;

  g_assert (DZL_IS_FUZZY_INDEX_CURSOR (self));
  g_assert (position < self->matches->len);

  match = &g_array_index (self->matches, DzlFuzzyMatch, position);

  document = _dzl_fuzzy_index_lookup_document (self->index, match->document_id);

  return g_object_new (DZL_TYPE_FUZZY_INDEX_MATCH,
                       "document", document,
                       "key", match->key,
                       "score", match->score,
                       "priority", match->priority,
                       NULL);
}

static void
list_model_iface_init (GListModelInterface *iface)
{
  iface->get_item_type = dzl_fuzzy_index_cursor_get_item_type;
  iface->get_n_items = dzl_fuzzy_index_cursor_get_n_items;
  iface->get_item = dzl_fuzzy_index_cursor_get_item;
}

/**
 * dzl_fuzzy_index_cursor_get_index:
 * @self: A #DzlFuzzyIndexCursor
 *
 * Gets the index the cursor is iterating.
 *
 * Returns: (transfer none): A #DzlFuzzyIndex.
 */
DzlFuzzyIndex *
dzl_fuzzy_index_cursor_get_index (DzlFuzzyIndexCursor *self)
{
  g_return_val_if_fail (DZL_IS_FUZZY_INDEX_CURSOR (self), NULL);

  return self->index;
}
