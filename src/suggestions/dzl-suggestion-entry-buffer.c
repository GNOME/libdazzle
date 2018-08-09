/* dzl-suggestion-entry-buffer.c
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
 *
 * This file is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define G_LOG_DOMAIN "dzl-suggestion-entry-buffer"

#include "config.h"

#include <string.h>

#include "suggestions/dzl-suggestion-entry-buffer.h"
#include "util/dzl-macros.h"

typedef struct
{
  DzlSuggestion *suggestion;
  gchar         *text;
  gchar         *suffix;
  guint          in_insert : 1;
  guint          in_delete : 1;
} DzlSuggestionEntryBufferPrivate;

enum {
  PROP_0,
  PROP_SUGGESTION,
  N_PROPS
};

G_DEFINE_TYPE_WITH_PRIVATE (DzlSuggestionEntryBuffer, dzl_suggestion_entry_buffer, GTK_TYPE_ENTRY_BUFFER)

static GParamSpec *properties [N_PROPS];

static void
dzl_suggestion_entry_buffer_drop_suggestion (DzlSuggestionEntryBuffer *self)
{
  DzlSuggestionEntryBufferPrivate *priv = dzl_suggestion_entry_buffer_get_instance_private (self);

  g_assert (DZL_IS_SUGGESTION_ENTRY_BUFFER (self));

  if (priv->suffix != NULL)
    {
      guint length = GTK_ENTRY_BUFFER_CLASS (dzl_suggestion_entry_buffer_parent_class)->get_length (GTK_ENTRY_BUFFER (self));
      guint suffix_len = strlen (priv->suffix);

      g_clear_pointer (&priv->suffix, g_free);
      gtk_entry_buffer_emit_deleted_text (GTK_ENTRY_BUFFER (self), length, suffix_len);
    }
}

static void
dzl_suggestion_entry_buffer_insert_suggestion (DzlSuggestionEntryBuffer *self)
{
  DzlSuggestionEntryBufferPrivate *priv = dzl_suggestion_entry_buffer_get_instance_private (self);

  g_assert (DZL_IS_SUGGESTION_ENTRY_BUFFER (self));

  if (priv->suggestion != NULL)
    {
      g_autofree gchar *suffix = NULL;
      const gchar *text;
      guint length;

      length = GTK_ENTRY_BUFFER_CLASS (dzl_suggestion_entry_buffer_parent_class)->get_length (GTK_ENTRY_BUFFER (self));
      text = GTK_ENTRY_BUFFER_CLASS (dzl_suggestion_entry_buffer_parent_class)->get_text (GTK_ENTRY_BUFFER (self), NULL);
      suffix = dzl_suggestion_suggest_suffix (priv->suggestion, text);

      if (suffix != NULL)
        {
          priv->suffix = g_steal_pointer (&suffix);
          gtk_entry_buffer_emit_inserted_text (GTK_ENTRY_BUFFER (self),
                                               length,
                                               priv->suffix,
                                               g_utf8_strlen (priv->suffix, -1));
        }
    }
}

const gchar *
dzl_suggestion_entry_buffer_get_typed_text (DzlSuggestionEntryBuffer *self)
{
  g_return_val_if_fail (DZL_IS_SUGGESTION_ENTRY_BUFFER (self), NULL);

  return GTK_ENTRY_BUFFER_CLASS (dzl_suggestion_entry_buffer_parent_class)->get_text (GTK_ENTRY_BUFFER (self), NULL);
}

static const gchar *
dzl_suggestion_entry_buffer_get_text (GtkEntryBuffer *buffer,
                                      gsize          *n_bytes)
{
  DzlSuggestionEntryBuffer *self = (DzlSuggestionEntryBuffer *)buffer;
  DzlSuggestionEntryBufferPrivate *priv = dzl_suggestion_entry_buffer_get_instance_private (self);

  g_assert (DZL_IS_SUGGESTION_ENTRY_BUFFER (self));

  if (priv->text == NULL)
    {
      const gchar *text;
      GString *str = NULL;

      text = GTK_ENTRY_BUFFER_CLASS (dzl_suggestion_entry_buffer_parent_class)->get_text (buffer, n_bytes);

      str = g_string_new (text);
      if (priv->suffix != NULL)
        g_string_append (str, priv->suffix);
      priv->text = g_string_free (str, FALSE);
    }

  if (n_bytes != NULL)
    *n_bytes = strlen (priv->text);

  return priv->text;
}

static guint
dzl_suggestion_entry_buffer_get_length (GtkEntryBuffer *buffer)
{
  DzlSuggestionEntryBuffer *self = (DzlSuggestionEntryBuffer *)buffer;
  DzlSuggestionEntryBufferPrivate *priv = dzl_suggestion_entry_buffer_get_instance_private (self);
  guint ret;

  g_assert (GTK_IS_ENTRY_BUFFER (buffer));

  ret = GTK_ENTRY_BUFFER_CLASS (dzl_suggestion_entry_buffer_parent_class)->get_length (buffer);

  if (priv->suffix != NULL)
    ret += strlen (priv->suffix);

  return ret;
}

static void
dzl_suggestion_entry_buffer_inserted_text (GtkEntryBuffer *buffer,
                                           guint           position,
                                           const gchar    *chars,
                                           guint           n_chars)
{
  DzlSuggestionEntryBuffer *self = (DzlSuggestionEntryBuffer *)buffer;
  DzlSuggestionEntryBufferPrivate *priv = dzl_suggestion_entry_buffer_get_instance_private (self);

  g_assert (GTK_IS_ENTRY_BUFFER (buffer));

  g_clear_pointer (&priv->text, g_free);

  GTK_ENTRY_BUFFER_CLASS (dzl_suggestion_entry_buffer_parent_class)->inserted_text (buffer, position, chars, n_chars);
}

static void
dzl_suggestion_entry_buffer_deleted_text (GtkEntryBuffer *buffer,
                                          guint           position,
                                          guint           n_chars)
{
  DzlSuggestionEntryBuffer *self = (DzlSuggestionEntryBuffer *)buffer;
  DzlSuggestionEntryBufferPrivate *priv = dzl_suggestion_entry_buffer_get_instance_private (self);

  g_assert (GTK_IS_ENTRY_BUFFER (buffer));

  g_clear_pointer (&priv->text, g_free);

  GTK_ENTRY_BUFFER_CLASS (dzl_suggestion_entry_buffer_parent_class)->deleted_text (buffer, position, n_chars);
}

static guint
dzl_suggestion_entry_buffer_insert_text (GtkEntryBuffer *buffer,
                                         guint           position,
                                         const gchar    *chars,
                                         guint           n_chars)
{
  DzlSuggestionEntryBuffer *self = (DzlSuggestionEntryBuffer *)buffer;
  DzlSuggestionEntryBufferPrivate *priv = dzl_suggestion_entry_buffer_get_instance_private (self);
  guint ret = 0;

  g_assert (GTK_IS_ENTRY_BUFFER (buffer));
  g_assert (chars != NULL || n_chars == 0);
  g_assert (priv->in_insert == FALSE);

  priv->in_insert = TRUE;

  if (n_chars == 0)
    goto failure;

  dzl_suggestion_entry_buffer_drop_suggestion (self);

  ret = GTK_ENTRY_BUFFER_CLASS (dzl_suggestion_entry_buffer_parent_class)->insert_text (buffer, position, chars, n_chars);
  if (ret < n_chars)
    goto failure;

  dzl_suggestion_entry_buffer_insert_suggestion (self);

failure:
  priv->in_insert = FALSE;

  return ret;
}

static guint
dzl_suggestion_entry_buffer_delete_text (GtkEntryBuffer *buffer,
                                         guint           position,
                                         guint           n_chars)
{
  DzlSuggestionEntryBuffer *self = (DzlSuggestionEntryBuffer *)buffer;
  DzlSuggestionEntryBufferPrivate *priv = dzl_suggestion_entry_buffer_get_instance_private (self);
  guint length;
  guint ret = 0;

  g_assert (GTK_IS_ENTRY_BUFFER (buffer));

  priv->in_delete = TRUE;

  length = GTK_ENTRY_BUFFER_CLASS (dzl_suggestion_entry_buffer_parent_class)->get_length (buffer);

  if (position >= length)
    goto failure;

  if (position + n_chars > length)
    n_chars = length - position;

  dzl_suggestion_entry_buffer_drop_suggestion (self);

  ret = GTK_ENTRY_BUFFER_CLASS (dzl_suggestion_entry_buffer_parent_class)->delete_text (buffer, position, n_chars);

  if (ret != 0 && priv->suggestion != NULL)
    dzl_suggestion_entry_buffer_insert_suggestion (self);

failure:
  priv->in_delete = FALSE;

  return ret;
}

static void
dzl_suggestion_entry_buffer_finalize (GObject *object)
{
  DzlSuggestionEntryBuffer *self = (DzlSuggestionEntryBuffer *)object;
  DzlSuggestionEntryBufferPrivate *priv = dzl_suggestion_entry_buffer_get_instance_private (self);

  g_clear_object (&priv->suggestion);
  g_clear_pointer (&priv->text, g_free);
  g_clear_pointer (&priv->suffix, g_free);

  G_OBJECT_CLASS (dzl_suggestion_entry_buffer_parent_class)->finalize (object);
}

static void
dzl_suggestion_entry_buffer_get_property (GObject    *object,
                                          guint       prop_id,
                                          GValue     *value,
                                          GParamSpec *pspec)
{
  DzlSuggestionEntryBuffer *self = DZL_SUGGESTION_ENTRY_BUFFER (object);

  switch (prop_id)
    {
    case PROP_SUGGESTION:
      g_value_set_object (value, dzl_suggestion_entry_buffer_get_suggestion (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_suggestion_entry_buffer_set_property (GObject      *object,
                                          guint         prop_id,
                                          const GValue *value,
                                          GParamSpec   *pspec)
{
  DzlSuggestionEntryBuffer *self = DZL_SUGGESTION_ENTRY_BUFFER (object);

  switch (prop_id)
    {
    case PROP_SUGGESTION:
      dzl_suggestion_entry_buffer_set_suggestion (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_suggestion_entry_buffer_class_init (DzlSuggestionEntryBufferClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkEntryBufferClass *entry_buffer_class = GTK_ENTRY_BUFFER_CLASS (klass);

  object_class->finalize = dzl_suggestion_entry_buffer_finalize;
  object_class->get_property = dzl_suggestion_entry_buffer_get_property;
  object_class->set_property = dzl_suggestion_entry_buffer_set_property;

  entry_buffer_class->inserted_text = dzl_suggestion_entry_buffer_inserted_text;
  entry_buffer_class->deleted_text = dzl_suggestion_entry_buffer_deleted_text;
  entry_buffer_class->get_text = dzl_suggestion_entry_buffer_get_text;
  entry_buffer_class->get_length = dzl_suggestion_entry_buffer_get_length;
  entry_buffer_class->insert_text = dzl_suggestion_entry_buffer_insert_text;
  entry_buffer_class->delete_text = dzl_suggestion_entry_buffer_delete_text;

  properties [PROP_SUGGESTION] =
    g_param_spec_object ("suggestion",
                         "Suggestion",
                         "The suggestion currently selected",
                         DZL_TYPE_SUGGESTION,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_suggestion_entry_buffer_init (DzlSuggestionEntryBuffer *self)
{
}

DzlSuggestionEntryBuffer *
dzl_suggestion_entry_buffer_new (void)
{
  return g_object_new (DZL_TYPE_SUGGESTION_ENTRY_BUFFER, NULL);
}

/**
 * dzl_suggestion_entry_buffer_get_suggestion:
 * @self: a #DzlSuggestionEntryBuffer
 *
 * Gets the #DzlSuggestion that is the current "preview suffix" of the
 * text in the entry.
 *
 * Returns: (transfer none) (nullable): An #DzlSuggestion or %NULL.
 */
DzlSuggestion *
dzl_suggestion_entry_buffer_get_suggestion (DzlSuggestionEntryBuffer *self)
{
  DzlSuggestionEntryBufferPrivate *priv = dzl_suggestion_entry_buffer_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SUGGESTION_ENTRY_BUFFER (self), NULL);

  return priv->suggestion;
}

/**
 * dzl_suggestion_entry_buffer_set_suggestion:
 * @self: a #DzlSuggestionEntryBuffer
 * @suggestion: (nullable): An #DzlSuggestion or %NULL
 *
 * Sets the current suggestion for the entry buffer.
 *
 * The suggestion is used to get a potential suffix for the current entry
 * text. This allows the entry to show "preview text" after the entered
 * text for what might be inserted should they activate the current item.
 */
void
dzl_suggestion_entry_buffer_set_suggestion (DzlSuggestionEntryBuffer *self,
                                            DzlSuggestion            *suggestion)
{
  DzlSuggestionEntryBufferPrivate *priv = dzl_suggestion_entry_buffer_get_instance_private (self);

  g_return_if_fail (DZL_IS_SUGGESTION_ENTRY_BUFFER (self));
  g_return_if_fail (!suggestion || DZL_IS_SUGGESTION (suggestion));

  if (priv->suggestion != suggestion)
    {
      dzl_suggestion_entry_buffer_drop_suggestion (self);
      g_set_object (&priv->suggestion, suggestion);
      if (!priv->in_delete && !priv->in_insert)
        dzl_suggestion_entry_buffer_insert_suggestion (self);

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_SUGGESTION]);
    }
}

guint
dzl_suggestion_entry_buffer_get_typed_length (DzlSuggestionEntryBuffer *self)
{
  const gchar *text;

  g_return_val_if_fail (DZL_IS_SUGGESTION_ENTRY_BUFFER (self), 0);

  text = dzl_suggestion_entry_buffer_get_typed_text (self);

  return text ? g_utf8_strlen (text, -1) : 0;
}

void
dzl_suggestion_entry_buffer_commit (DzlSuggestionEntryBuffer *self)
{
  DzlSuggestionEntryBufferPrivate *priv = dzl_suggestion_entry_buffer_get_instance_private (self);

  g_return_if_fail (DZL_IS_SUGGESTION_ENTRY_BUFFER (self));

  if (priv->suffix != NULL)
    {
      g_autofree gchar *suffix = g_steal_pointer (&priv->suffix);
      guint position;

      g_clear_object (&priv->suggestion);
      position = GTK_ENTRY_BUFFER_CLASS (dzl_suggestion_entry_buffer_parent_class)->get_length (GTK_ENTRY_BUFFER (self));
      GTK_ENTRY_BUFFER_CLASS (dzl_suggestion_entry_buffer_parent_class)->insert_text (GTK_ENTRY_BUFFER (self),
                                                                                      position,
                                                                                      suffix,
                                                                                      g_utf8_strlen (suffix, -1));
    }
}

void
dzl_suggestion_entry_buffer_clear (DzlSuggestionEntryBuffer *self)
{
  DzlSuggestionEntryBufferPrivate *priv = dzl_suggestion_entry_buffer_get_instance_private (self);

  g_return_if_fail (DZL_IS_SUGGESTION_ENTRY_BUFFER (self));

  g_clear_pointer (&priv->text, g_free);
  g_clear_pointer (&priv->suffix, g_free);
  g_clear_object (&priv->suggestion);
  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_SUGGESTION]);
}
