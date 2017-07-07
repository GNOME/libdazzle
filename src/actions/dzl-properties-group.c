/* dzl-properties-group.c
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

#define G_LOG_DOMAIN "dzl-properties-group"

#include "dzl-properties-group.h"

/**
 * SECTION:dzl-properties-group
 * @title: DzlPropertiesGroup
 * @short_description: A #GActionGroup of properties on an object
 *
 * This class is a #GActionGroup which provides stateful access to
 * properties in a #GObject. This can be useful when you want to
 * expose properties from a GObject as a #GAction, espectially with
 * use in GtkApplications.
 *
 * Call dzl_properties_group_add_property() to setup the mappings
 * for action-name to property-name for the actions you'd like to
 * add.
 *
 * Not all property types can be supported. What is current supported
 * are properties of type:
 *
 *  %G_TYPE_INT
 *  %G_TYPE_UINT
 *  %G_TYPE_BOOLEAN
 *  %G_TYPE_STRING
 *  %G_TYPE_DOUBLE
 *
 * Since: 3.26
 */

struct _DzlPropertiesGroup
{
  GObject parent_instance;

  /*
   * Weak ref to the object we are monitoring for property changes.
   * We hold both a GWeakRef and a g_object_weak_ref() on the object
   * so that we can get notified of destruction *AND* know when we
   * can safely weak_unref() without invalid pointer user.
   */
  GWeakRef object_ref;

  /*
   * Since the list of mappings are fairly small, we just choose to
   * use an array of all mappings rather than two-hashtables to map
   * from action-name -> property-name and vice versa. Element type
   * is of struct Mapping.
   *
   * The strings in the mapping are intern'd to allow for direct
   * pointer comparison with GParamSpec information.
   */
  GArray *mappings;
};

typedef struct
{
  const gchar        *action_name;
  const GVariantType *param_type;
  const GVariantType *state_type;
  const gchar        *property_name;
  GType               property_type;
} Mapping;

enum {
  PROP_0,
  PROP_OBJECT,
  N_PROPS
};

static GVariant *
get_action_state (GObject       *object,
                  const Mapping *mapping)
{
  g_auto(GValue) value = G_VALUE_INIT;
  GVariant *ret = NULL;

  g_assert (G_IS_OBJECT (object));
  g_assert (mapping != NULL);

  g_value_init (&value, mapping->property_type);
  g_object_get_property (object, mapping->property_name, &value);

  switch (mapping->property_type)
    {
    case G_TYPE_INT:
      ret = g_variant_new_boolean (g_value_get_int (&value));
      break;

    case G_TYPE_UINT:
      ret = g_variant_new_uint32 (g_value_get_uint (&value));
      break;

    case G_TYPE_DOUBLE:
      ret = g_variant_new_double (g_value_get_double (&value));
      break;

    case G_TYPE_STRING:
      ret = g_variant_new_string (g_value_get_string (&value));
      break;

    case G_TYPE_BOOLEAN:
      ret = g_variant_new_boolean (g_value_get_boolean (&value));

    default:
      g_assert_not_reached ();
    }

  return g_variant_take_ref (ret);
}

static gboolean
dzl_properties_group_query_action (GActionGroup        *group,
                                   const gchar         *action_name,
                                   gboolean            *enabled,
                                   const GVariantType **param_type,
                                   const GVariantType **state_type,
                                   GVariant           **state_hint,
                                   GVariant           **state)
{
  DzlPropertiesGroup *self = (DzlPropertiesGroup *)group;

  g_assert (DZL_IS_PROPERTIES_GROUP (self));
  g_assert (action_name != NULL);

  for (guint i = 0; i < self->mappings->len; i++)
    {
      const Mapping *mapping = &g_array_index (self->mappings, Mapping, i);

      if (g_strcmp0 (mapping->action_name, action_name) == 0)
        {
          g_autoptr(GObject) object = g_weak_ref_get (&self->object_ref);

          if (enabled)
            *enabled = (object != NULL);

          if (param_type)
            *param_type = mapping->param_type;

          if (state_type)
            *state_type = mapping->state_type;

          if (state_hint)
            *state_hint = NULL;

          if (state)
            *state = get_action_state (object, mapping);

          return TRUE;
        }
    }

  return FALSE;
}

static gchar **
dzl_properties_group_list_actions (GActionGroup *group)
{
  DzlPropertiesGroup *self = (DzlPropertiesGroup *)group;
  GPtrArray *ar;

  g_assert (DZL_IS_PROPERTIES_GROUP (self));

  ar = g_ptr_array_new ();

  for (guint i = 0; i < self->mappings->len; i++)
    {
      const Mapping *mapping = &g_array_index (self->mappings, Mapping, i);

      g_ptr_array_add (ar, g_strdup (mapping->action_name));
    }

  g_ptr_array_add (ar, NULL);

  return (gchar **)g_ptr_array_free (ar, FALSE);
}

static gboolean
dzl_properties_group_has_action (GActionGroup *group,
                                 const gchar  *name)
{
  DzlPropertiesGroup *self = (DzlPropertiesGroup *)group;

  g_assert (DZL_IS_PROPERTIES_GROUP (self));
  g_assert (name != NULL);

  for (guint i = 0; i < self->mappings->len; i++)
    {
      const Mapping *mapping = &g_array_index (self->mappings, Mapping, i);

      if (g_strcmp0 (name, mapping->action_name) == 0)
        return TRUE;
    }

  return FALSE;
}

static gboolean
dzl_properties_group_get_action_enabled (GActionGroup *group,
                                         const gchar  *name)
{
  DzlPropertiesGroup *self = (DzlPropertiesGroup *)group;
  g_autoptr(GObject) object = NULL;

  g_assert (DZL_IS_PROPERTIES_GROUP (self));
  g_assert (name != NULL);

  object = g_weak_ref_get (&self->object_ref);

  return (object != NULL);
}

static const GVariantType *
dzl_properties_group_get_action_parameter_type (GActionGroup *group,
                                                const gchar  *name)
{
  DzlPropertiesGroup *self = (DzlPropertiesGroup *)group;

  g_assert (DZL_IS_PROPERTIES_GROUP (self));
  g_assert (name != NULL);

  for (guint i = 0; i < self->mappings->len; i++)
    {
      const Mapping *mapping = &g_array_index (self->mappings, Mapping, i);

      if (g_strcmp0 (name, mapping->action_name) == 0)
        return mapping->param_type;
    }

  return NULL;
}

static const GVariantType *
dzl_properties_group_get_action_state_type (GActionGroup *group,
                                            const gchar  *name)
{
  DzlPropertiesGroup *self = (DzlPropertiesGroup *)group;

  g_assert (DZL_IS_PROPERTIES_GROUP (self));
  g_assert (name != NULL);

  for (guint i = 0; i < self->mappings->len; i++)
    {
      const Mapping *mapping = &g_array_index (self->mappings, Mapping, i);

      if (g_strcmp0 (name, mapping->action_name) == 0)
        return mapping->state_type;
    }

  return NULL;
}

static GVariant *
dzl_properties_group_get_action_state_hint (GActionGroup *group,
                                            const gchar  *name)
{
  g_assert (DZL_IS_PROPERTIES_GROUP (group));
  g_assert (name != NULL);

  return NULL;
}

static void
dzl_properties_group_change_action_state (GActionGroup *group,
                                          const gchar  *name,
                                          GVariant     *variant)
{
  DzlPropertiesGroup *self = (DzlPropertiesGroup *)group;
  g_autoptr(GObject) object = NULL;

  g_assert (DZL_IS_PROPERTIES_GROUP (self));
  g_assert (name != NULL);
  g_assert (variant != NULL);

  object = g_weak_ref_get (&self->object_ref);

  if (object == NULL)
    {
      g_warning ("Attempt to change state of %s after action was disabled",
                 name);
      return;
    }

  for (guint i = 0; i < self->mappings->len; i++)
    {
      const Mapping *mapping = &g_array_index (self->mappings, Mapping, i);

      if (g_strcmp0 (name, mapping->action_name) == 0)
        {
          g_auto(GValue) value = G_VALUE_INIT;

          switch (mapping->property_type)
            {
            case G_TYPE_INT:
              g_value_init (&value, G_TYPE_INT);
              g_value_set_int (&value, g_variant_get_int32 (variant));
              break;

            case G_TYPE_UINT:
              g_value_init (&value, G_TYPE_UINT);
              g_value_set_uint (&value, g_variant_get_uint32 (variant));
              break;

            case G_TYPE_BOOLEAN:
              g_value_init (&value, G_TYPE_BOOLEAN);
              g_value_set_boolean (&value, g_variant_get_boolean (variant));
              break;

            case G_TYPE_STRING:
              g_value_init (&value, G_TYPE_STRING);
              /* No need to dup the string, its lifetime is longer */
              g_value_set_static_string (&value, g_variant_get_string (variant, NULL));
              break;

            case G_TYPE_DOUBLE:
              g_value_init (&value, G_TYPE_DOUBLE);
              g_value_set_double (&value, g_variant_get_double (variant));

            default:
              g_assert_not_reached ();
            }

          g_object_set_property (object, mapping->property_name, &value);

          break;
        }
    }
}

static void
dzl_properties_group_activate_action (GActionGroup *group,
                                      const gchar  *name,
                                      GVariant     *variant)
{
  DzlPropertiesGroup *self = (DzlPropertiesGroup *)group;
  g_autoptr(GObject) object = NULL;

  g_assert (DZL_IS_PROPERTIES_GROUP (self));
  g_assert (name != NULL);

  object = g_weak_ref_get (&self->object_ref);

  if (object == NULL)
    {
      g_warning ("Attempt to activate %s after action was disabled", name);
      return;
    }

  for (guint i = 0; i < self->mappings->len; i++)
    {
      const Mapping *mapping = &g_array_index (self->mappings, Mapping, i);

      if (g_strcmp0 (name, mapping->action_name) == 0)
        {
          if (mapping->property_type == G_TYPE_BOOLEAN)
            {
              gboolean value = FALSE;

              g_object_get (object, mapping->property_name, &value, NULL);
              value = !value;
              g_object_set (object, mapping->property_name, value, NULL);
            }
          else
            {
              dzl_properties_group_change_action_state (group, name, variant);
            }

          break;
        }
    }
}

static void
action_group_iface_init (GActionGroupInterface *iface)
{
  iface->has_action = dzl_properties_group_has_action;
  iface->list_actions = dzl_properties_group_list_actions;
  iface->get_action_enabled = dzl_properties_group_get_action_enabled;
  iface->get_action_parameter_type = dzl_properties_group_get_action_parameter_type;
  iface->get_action_state_type = dzl_properties_group_get_action_state_type;
  iface->get_action_state_hint = dzl_properties_group_get_action_state_hint;
  iface->change_action_state = dzl_properties_group_change_action_state;
  iface->activate_action = dzl_properties_group_activate_action;
  iface->query_action = dzl_properties_group_query_action;
}

G_DEFINE_TYPE_WITH_CODE (DzlPropertiesGroup, dzl_properties_group, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_ACTION_GROUP,
                                                action_group_iface_init))

static GParamSpec *properties [N_PROPS];

static void
dzl_properties_group_notify (DzlPropertiesGroup *self,
                             GParamSpec         *pspec,
                             GObject            *object)
{
  g_assert (DZL_IS_PROPERTIES_GROUP (self));
  g_assert (pspec != NULL);
  g_assert (G_IS_OBJECT (object));

  /* mappings is generally quite small, so iterating the array
   * is going to have similar performance to a hashtable lookup
   * plus pointer chaseing.
   */

  for (guint i = 0; i < self->mappings->len; i++)
    {
      const Mapping *mapping = &g_array_index (self->mappings, Mapping, i);

      if (mapping->property_name == pspec->name)
        {
          g_autoptr(GVariant) state = get_action_state (object, mapping);
          g_action_group_action_state_changed (G_ACTION_GROUP (self),
                                               mapping->action_name,
                                               state);
          break;
        }
    }
}

static const GVariantType *
get_param_type_for_type (GType type)
{
  switch (type)
    {
    case G_TYPE_INT:     return G_VARIANT_TYPE_INT32;
    case G_TYPE_UINT:    return G_VARIANT_TYPE_UINT32;
    case G_TYPE_BOOLEAN: return NULL;
    case G_TYPE_STRING:  return G_VARIANT_TYPE_STRING;
    case G_TYPE_DOUBLE:  return G_VARIANT_TYPE_DOUBLE;
    default:
      g_warning ("%s is not a supported type", g_type_name (type));
      return NULL;
    }
}

static const GVariantType *
get_state_type_for_type (GType type)
{
  switch (type)
    {
    case G_TYPE_INT:     return G_VARIANT_TYPE_INT32;
    case G_TYPE_UINT:    return G_VARIANT_TYPE_UINT32;
    case G_TYPE_BOOLEAN: return G_VARIANT_TYPE_BOOLEAN;
    case G_TYPE_STRING:  return G_VARIANT_TYPE_STRING;
    case G_TYPE_DOUBLE:  return G_VARIANT_TYPE_DOUBLE;
    default:
      g_warning ("%s is not a supported type", g_type_name (type));
      return NULL;
    }
}

static void
dzl_properties_group_weak_notify (gpointer  data,
                                  GObject  *where_object_was)
{
  DzlPropertiesGroup *self = data;

  g_assert (DZL_IS_PROPERTIES_GROUP (self));

  /*
   * Mark all of the actions as disabled due to losing our reference
   * on the source object.
   */

  for (guint i = 0; i < self->mappings->len; i++)
    {
      const Mapping *mapping = &g_array_index (self->mappings, Mapping, i);

      g_action_group_action_enabled_changed (G_ACTION_GROUP (self),
                                             mapping->action_name,
                                             FALSE);
    }
}

static void
dzl_properties_group_set_object (DzlPropertiesGroup *self,
                                 GObject            *object)
{
  g_assert (DZL_IS_PROPERTIES_GROUP (self));
  g_assert (!object || G_IS_OBJECT (object));

  if (object == NULL)
    {
      g_critical ("DzlPropertiesGroup: object must be non-NULL.");
      return;
    }

  g_signal_connect_object (object,
                           "notify",
                           G_CALLBACK (dzl_properties_group_notify),
                           self,
                           G_CONNECT_SWAPPED);

  /* WeakRef so we can detect 3-rd degree disposal */
  g_weak_ref_set (&self->object_ref, object);

  /* Weak notify so we can get notified of the case */
  g_object_weak_ref (G_OBJECT (object),
                     dzl_properties_group_weak_notify,
                     self);
}

static void
dzl_properties_group_finalize (GObject *object)
{
  DzlPropertiesGroup *self = (DzlPropertiesGroup *)object;
  g_autoptr(GObject) weak_obj = NULL;

  weak_obj = g_weak_ref_get (&self->object_ref);
  if (weak_obj != NULL)
    g_object_weak_unref (weak_obj,
                         dzl_properties_group_weak_notify,
                         self);

  g_weak_ref_clear (&self->object_ref);

  g_clear_pointer (&self->mappings, g_array_unref);

  G_OBJECT_CLASS (dzl_properties_group_parent_class)->finalize (object);
}

static void
dzl_properties_group_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  DzlPropertiesGroup *self = DZL_PROPERTIES_GROUP (object);

  switch (prop_id)
    {
    case PROP_OBJECT:
      g_value_take_object (value, g_weak_ref_get (&self->object_ref));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_properties_group_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  DzlPropertiesGroup *self = DZL_PROPERTIES_GROUP (object);

  switch (prop_id)
    {
    case PROP_OBJECT:
      dzl_properties_group_set_object (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_properties_group_class_init (DzlPropertiesGroupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_properties_group_finalize;
  object_class->get_property = dzl_properties_group_get_property;
  object_class->set_property = dzl_properties_group_set_property;

  properties [PROP_OBJECT] =
    g_param_spec_object ("object",
                         "Object",
                         "The source object for the properties",
                         G_TYPE_OBJECT,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_properties_group_init (DzlPropertiesGroup *self)
{
  g_weak_ref_init (&self->object_ref, NULL);

  self->mappings = g_array_new (FALSE, FALSE, sizeof (Mapping));
}

/**
 * dzl_properties_group_new:
 * @object: The object containing the properties
 *
 * This creates a new #DzlPropertiesGroup to create stateful actions
 * around properties in @object.
 *
 * Call dzl_properties_group_add_property() to add a property to
 * action name mapping for this group. Until you've called this,
 * no actions are mapped.
 *
 * Returns: (transfer full): A #DzlPropertiesGroup
 *
 * Since: 3.26
 */
DzlPropertiesGroup *
dzl_properties_group_new (GObject *object)
{
  return g_object_new (DZL_TYPE_PROPERTIES_GROUP,
                       "object", object,
                       NULL);
}

/**
 * dzl_properties_group_add_property:
 * @self: a #DzlPropertiesGroup
 * @name: the name of the action
 * @property_name: the name of the property
 *
 * Adds a new stateful action named @name which maps to the underlying
 * property @property_name of #DzlPropertiesGroup:object.
 *
 * Since: 3.26
 */
void
dzl_properties_group_add_property (DzlPropertiesGroup *self,
                                   const gchar        *name,
                                   const gchar        *property_name)
{
  g_autoptr(GObject) object = NULL;
  GObjectClass *object_class;
  GParamSpec *pspec;
  Mapping mapping = { 0 };

  g_return_if_fail (DZL_IS_PROPERTIES_GROUP (self));
  g_return_if_fail (name != NULL);
  g_return_if_fail (property_name != NULL);

  object = g_weak_ref_get (&self->object_ref);

  if (object == NULL)
    {
      g_warning ("Cannot add property, object was already disposed.");
      return;
    }

  object_class = G_OBJECT_GET_CLASS (object);
  pspec = g_object_class_find_property (object_class, property_name);

  if (pspec == NULL)
    {
      g_warning ("No such property \"%s\" on type %s",
                 property_name, G_OBJECT_TYPE_NAME (object));
      return;
    }

  mapping.action_name = g_intern_string (name);
  mapping.param_type = get_param_type_for_type (pspec->value_type);
  mapping.state_type = get_state_type_for_type (pspec->value_type);
  mapping.property_name = pspec->name;
  mapping.property_type = pspec->value_type;

  /* we already warned, ignore this */
  if (mapping.state_type == NULL)
    return;

  g_array_append_val (self->mappings, mapping);

  g_action_group_action_added (G_ACTION_GROUP (self), mapping.action_name);
}

/**
 * dzl_properties_group_remove:
 * @self: a #DzlPropertiesGroup
 * @name: the name of the action
 *
 * Removes an action from @self that was previously added with
 * dzl_properties_group_add_property(). @name should match the
 * name parameter to that function.
 *
 * Since: 3.26
 */
void
dzl_properties_group_remove (DzlPropertiesGroup *self,
                             const gchar        *name)
{
  g_return_if_fail (DZL_IS_PROPERTIES_GROUP (self));
  g_return_if_fail (name != NULL);
}
