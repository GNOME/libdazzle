/* dzl-path.c
 *
 * Copyright (C) 2016-2017 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "dzl-path"

#include "config.h"

#include "dzl-path.h"
#include "dzl-path-element.h"

struct _DzlPath
{
  GObject  parent_instance;
  GQueue  *elements;
};

G_DEFINE_TYPE (DzlPath, dzl_path, G_TYPE_OBJECT)

static void
dzl_path_finalize (GObject *object)
{
  DzlPath *self = (DzlPath *)object;

  g_queue_free_full (self->elements, g_object_unref);
  self->elements = NULL;

  G_OBJECT_CLASS (dzl_path_parent_class)->finalize (object);
}

static void
dzl_path_class_init (DzlPathClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_path_finalize;
}

static void
dzl_path_init (DzlPath *self)
{
  self->elements = g_queue_new ();
}

/**
 * dzl_path_get_elements:
 *
 * Returns: (transfer none) (element-type Dazzle.PathElement): The elements of the path.
 */
GList *
dzl_path_get_elements (DzlPath *self)
{
  g_return_val_if_fail (DZL_IS_PATH (self), NULL);

  return self->elements->head;
}

DzlPath *
dzl_path_new (void)
{
  return g_object_new (DZL_TYPE_PATH, NULL);
}

void
dzl_path_append (DzlPath        *self,
                 DzlPathElement *element)
{
  g_return_if_fail (DZL_IS_PATH (self));
  g_return_if_fail (DZL_IS_PATH_ELEMENT (element));

  g_queue_push_tail (self->elements, g_object_ref (element));
}

void
dzl_path_prepend (DzlPath        *self,
                  DzlPathElement *element)
{
  g_return_if_fail (DZL_IS_PATH (self));
  g_return_if_fail (DZL_IS_PATH_ELEMENT (element));

  g_queue_push_head (self->elements, g_object_ref (element));
}

gboolean
dzl_path_has_prefix (DzlPath *self,
                     DzlPath *prefix)
{
  const GList *iter;
  const GList *spec;

  g_return_val_if_fail (DZL_IS_PATH (self), FALSE);
  g_return_val_if_fail (DZL_IS_PATH (prefix), FALSE);

  if (self->elements->length < prefix->elements->length)
    return FALSE;

  for (iter = self->elements->head, spec = prefix->elements->head;
       iter != NULL && spec != NULL;
       iter = iter->next, spec = spec->next)
    {
      DzlPathElement *spec_element = spec->data;
      DzlPathElement *iter_element = iter->data;
      const gchar *spec_id = dzl_path_element_get_id (spec_element);
      const gchar *iter_id = dzl_path_element_get_id (iter_element);

      if (g_strcmp0 (spec_id, iter_id) == 0)
        continue;

      return FALSE;
    }

  return TRUE;
}

guint
dzl_path_get_length (DzlPath *self)
{
  g_return_val_if_fail (DZL_IS_PATH (self), 0);

  return self->elements->length;
}

gchar *
dzl_path_printf (DzlPath *self)
{
  const GList *iter;
  GString *str;

  g_return_val_if_fail (DZL_IS_PATH (self), NULL);

  str = g_string_new (NULL);

  for (iter = self->elements->head; iter != NULL; iter = iter->next)
    {
      DzlPathElement *element = iter->data;
      const gchar *id = dzl_path_element_get_id (element);

      g_string_append (str, id);
      if (iter->next != NULL)
        g_string_append_c (str, ',');
    }

  return g_string_free (str, FALSE);
}

gboolean
dzl_path_is_empty (DzlPath *self)
{
  g_return_val_if_fail (DZL_IS_PATH (self), FALSE);

  return self->elements->length == 0;
}

/**
 * dzl_path_get_element:
 *
 * Gets the path element found at @index.
 *
 * Indexes start from zero.
 *
 * Returns: (nullable) (transfer none): An #DzlPathElement.
 */
DzlPathElement *
dzl_path_get_element (DzlPath *self,
                      guint     index)
{
  g_return_val_if_fail (DZL_IS_PATH (self), NULL);
  g_return_val_if_fail (index < self->elements->length, NULL);

  return g_queue_peek_nth (self->elements, index);
}
