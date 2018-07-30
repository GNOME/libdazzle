/* dzl-path-element.c
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

#define G_LOG_DOMAIN "dzl-path-element"

#include "config.h"

#include "pathbar/dzl-path-element.h"
#include "util/dzl-macros.h"

struct _DzlPathElement
{
  GObject parent_instance;
  gchar *icon_name;
  gchar *id;
  gchar *title;
};

G_DEFINE_TYPE (DzlPathElement, dzl_path_element, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_ICON_NAME,
  PROP_ID,
  PROP_TITLE,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

/**
 * dzl_path_element_new:
 * @id: (nullable): An id for the path element.
 * @icon_name: (nullable): An optional icon name for the element
 * @title: The title of the element.
 *
 * Creates a new path element for an #DzlPath.
 *
 * Returns: (transfer full): A #DzlPathElement
 *
 * Since: 3.26
 */
DzlPathElement *
dzl_path_element_new (const gchar *id,
                      const gchar *icon_name,
                      const gchar *title)
{
  return g_object_new (DZL_TYPE_PATH_ELEMENT,
                       "icon-name", icon_name,
                       "id", id,
                       "title", title,
                       NULL);
}

static void
dzl_path_element_finalize (GObject *object)
{
  DzlPathElement *self = (DzlPathElement *)object;

  g_clear_pointer (&self->icon_name, g_free);
  g_clear_pointer (&self->id, g_free);
  g_clear_pointer (&self->title, g_free);

  G_OBJECT_CLASS (dzl_path_element_parent_class)->finalize (object);
}

static void
dzl_path_element_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  DzlPathElement *self = DZL_PATH_ELEMENT (object);

  switch (prop_id)
    {
    case PROP_ICON_NAME:
      g_value_set_string (value, self->icon_name);
      break;

    case PROP_ID:
      g_value_set_string (value, self->id);
      break;

    case PROP_TITLE:
      g_value_set_string (value, self->title);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_path_element_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  DzlPathElement *self = DZL_PATH_ELEMENT (object);

  switch (prop_id)
    {
    case PROP_ICON_NAME:
      self->icon_name = g_value_dup_string (value);
      break;

    case PROP_ID:
      self->id = g_value_dup_string (value);
      break;

    case PROP_TITLE:
      self->title = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_path_element_class_init (DzlPathElementClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_path_element_finalize;
  object_class->get_property = dzl_path_element_get_property;
  object_class->set_property = dzl_path_element_set_property;

  /**
   * DzlPathElement:icon-name:
   *
   * The icon-name of the icon to display next to the path element
   * in the path bar. Set to %NULL for no icon.
   *
   * Since: 3.26
   */
  properties [PROP_ICON_NAME] =
    g_param_spec_string ("icon-name",
                         "Icon Name",
                         "The icon name for the path element",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY| G_PARAM_STATIC_STRINGS));

  /**
   * DzlPathElement:id:
   *
   * The id property is an application specific identifier for the
   * element within the path.
   *
   * Since: 3.26
   */
  properties [PROP_ID] =
    g_param_spec_string ("id",
                         "Identifier",
                         "Identifier",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY| G_PARAM_STATIC_STRINGS));

  /**
   * DzlPathElement:title:
   *
   * The title property should contain the display text that should
   * be shown to represent the element in the #DzlPathBar.
   *
   * Since: 3.26
   */
  properties [PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "Title",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_path_element_init (DzlPathElement *self)
{
}

/**
 * dzl_path_element_get_id:
 * @self: A #DzlPathElement
 *
 * Gets the id for the element. Generally, a path is built of
 * multiple elements and each element should have an id that
 * is useful to the application that it using it. You might store
 * the name of a directory, or some other key as the id.
 *
 * Returns: (transfer none): The id for the #DzlPathElement.
 *
 * Since: 3.26
 */
const gchar *
dzl_path_element_get_id (DzlPathElement *self)
{
  g_return_val_if_fail (DZL_IS_PATH_ELEMENT (self), NULL);

  return self->id;
}

/**
 * dzl_path_element_get_icon_name:
 * @self: A #DzlPathElement
 *
 * Gets the #DzlPathElement:icon-name property. This is used by the
 * path bar to display an icon next to the element of the path.
 *
 * Returns: (transfer none) (nullable): The icon-name for the #DzlPathElement.
 *
 * Since: 3.26
 */
const gchar *
dzl_path_element_get_icon_name (DzlPathElement *self)
{
  g_return_val_if_fail (DZL_IS_PATH_ELEMENT (self), NULL);

  return self->icon_name;
}

/**
 * dzl_path_element_get_title:
 * @self: A #DzlPathElement
 *
 * Gets the #DzlPathElement:title property. This is used by the
 * path bar to display text representing the element of the path.
 *
 * Returns: (transfer none) (nullable): The title for the #DzlPathElement.
 *
 * Since: 3.26
 */
const gchar *
dzl_path_element_get_title (DzlPathElement *self)
{
  g_return_val_if_fail (DZL_IS_PATH_ELEMENT (self), NULL);

  return self->title;
}
