/* dzl-shortcut-context.c
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#define G_LOG_DOMAIN "dzl-shortcut-context"

#include <gobject/gvaluecollector.h>
#include <string.h>

#include "shortcuts/dzl-shortcut-chord.h"
#include "shortcuts/dzl-shortcut-context.h"
#include "shortcuts/dzl-shortcut-controller.h"
#include "shortcuts/dzl-shortcut-private.h"

typedef struct
{
  /* The name of the context, interned */
  const gchar *name;

  /* The table of entries in this context which maps to a shortcut.
   * These need to be copied across when merging down to another
   * context layer.
   */
  DzlShortcutChordTable *table;

  /* If we should use binding sets. By default this is true, but
   * we use a signed 2-bit int for -1 being "unset". That allows
   * us to know when the value was set on a layer and merge that
   * value upwards.
   */
  gint use_binding_sets : 2;
} DzlShortcutContextPrivate;

enum {
  PROP_0,
  PROP_NAME,
  PROP_USE_BINDING_SETS,
  N_PROPS
};

struct _DzlShortcutContext
{
  GObject parent_instance;
};

G_DEFINE_TYPE_WITH_PRIVATE (DzlShortcutContext, dzl_shortcut_context, G_TYPE_OBJECT)

static GParamSpec *properties [N_PROPS];

static void
shortcut_free (gpointer data)
{
  Shortcut *shortcut = data;

  if (shortcut != NULL)
    {
      g_clear_pointer (&shortcut->next, shortcut_free);

      switch (shortcut->type)
        {
        case SHORTCUT_ACTION:
          g_clear_pointer (&shortcut->action.param, g_variant_unref);
          break;

        case SHORTCUT_COMMAND:
          break;

        case SHORTCUT_SIGNAL:
          g_array_unref (shortcut->signal.params);
          break;

        default:
          g_assert_not_reached ();
        }

      g_slice_free (Shortcut, shortcut);
    }
}

static gboolean
widget_action (GtkWidget   *widget,
               const gchar *prefix,
               const gchar *action_name,
               GVariant    *parameter)
{
  GtkWidget *toplevel;
  GApplication *app;
  GActionGroup *group = NULL;

  g_assert (GTK_IS_WIDGET (widget));
  g_assert (prefix != NULL);
  g_assert (action_name != NULL);

  app = g_application_get_default ();
  toplevel = gtk_widget_get_toplevel (widget);

  while ((group == NULL) && (widget != NULL))
    {
      group = gtk_widget_get_action_group (widget, prefix);

      if G_UNLIKELY (GTK_IS_POPOVER (widget))
        {
          GtkWidget *relative_to;

          relative_to = gtk_popover_get_relative_to (GTK_POPOVER (widget));

          if (relative_to != NULL)
            widget = relative_to;
          else
            widget = gtk_widget_get_parent (widget);
        }
      else
        {
          widget = gtk_widget_get_parent (widget);
        }
    }

  if (!group && g_str_equal (prefix, "win") && G_IS_ACTION_GROUP (toplevel))
    group = G_ACTION_GROUP (toplevel);

  if (!group && g_str_equal (prefix, "app") && G_IS_ACTION_GROUP (app))
    group = G_ACTION_GROUP (app);

  if (group && g_action_group_has_action (group, action_name))
    {
      g_action_group_activate_action (group, action_name, parameter);
      return TRUE;
    }

  g_warning ("Failed to locate action %s.%s", prefix, action_name);

  return FALSE;
}

static gboolean
shortcut_action_activate (Shortcut  *shortcut,
                          GtkWidget *widget)
{
  g_assert (shortcut != NULL);
  g_assert (GTK_IS_WIDGET (widget));

  return widget_action (widget,
                        shortcut->action.prefix,
                        shortcut->action.name,
                        shortcut->action.param);
}

static gboolean
shortcut_command_activate (Shortcut  *shortcut,
                           GtkWidget *widget)
{
  DzlShortcutController *controller;

  g_assert (shortcut != NULL);
  g_assert (GTK_IS_WIDGET (widget));

  controller = dzl_shortcut_controller_try_find (widget);

  if (controller != NULL)
    {
      /* If the controller has a command registered for this command, execute
       * it. This seems a bit like an inversion of control here, because it
       * sort of is. Rather than deal with this at the controller level, we
       * deal with it in the context so that the code-flow is simpler to
       * follow and requires less duplication.
       *
       * Since shortcut->command has been interned, we can be certain command
       * will be a valid pointer for the lifetime of this function call.
       */

      return dzl_shortcut_controller_execute_command (controller, shortcut->command);
    }

  return FALSE;
}

static gboolean
find_instance_and_signal (GtkWidget          *widget,
                          const gchar        *signal_name,
                          gpointer           *instance,
                          GSignalQuery       *query)
{
  DzlShortcutController *controller;

  g_assert (GTK_IS_WIDGET (widget));
  g_assert (signal_name != NULL);
  g_assert (instance != NULL);
  g_assert (query != NULL);

  *instance = NULL;

  /*
   * First we want to see if we can resolve the signal on the widgets
   * controller (if there is one). This allows us to change contexts
   * from signals without installing signals on the actual widgets.
   */

  controller = dzl_shortcut_controller_find (widget);

  if (controller != NULL)
    {
      guint signal_id;

      signal_id = g_signal_lookup (signal_name, G_OBJECT_TYPE (controller));

      if (signal_id != 0)
        {
          g_signal_query (signal_id, query);
          *instance = controller;
          return TRUE;
        }
    }

  /*
   * This diverts from Gtk signal keybindings a bit in that we
   * allow you to activate a signal on any widget in the focus
   * hierarchy starting from the provided widget up.
   */

  while (widget != NULL)
    {
      guint signal_id;

      signal_id = g_signal_lookup (signal_name, G_OBJECT_TYPE (widget));

      if (signal_id != 0)
        {
          g_signal_query (signal_id, query);
          *instance = widget;
          return TRUE;
        }

      widget = gtk_widget_get_parent (widget);
    }

  return FALSE;
}

static gboolean
shortcut_signal_activate (Shortcut  *shortcut,
                          GtkWidget *widget)
{
  GValue *params;
  GValue return_value = { 0 };
  GSignalQuery query;
  gpointer instance = NULL;

  g_assert (shortcut != NULL);
  g_assert (GTK_IS_WIDGET (widget));

  if (!find_instance_and_signal (widget, shortcut->signal.name, &instance, &query))
    {
      g_warning ("Failed to locate signal %s in hierarchy of %s",
                 shortcut->signal.name, G_OBJECT_TYPE_NAME (widget));
      return TRUE;
    }

  if (query.n_params != shortcut->signal.params->len)
    goto parameter_mismatch;

  for (guint i = 0; i < query.n_params; i++)
    {
      if (!G_VALUE_HOLDS (&g_array_index (shortcut->signal.params, GValue, i), query.param_types[i]))
        goto parameter_mismatch;
    }

  params = g_new0 (GValue, 1 + query.n_params);
  g_value_init_from_instance (&params[0], instance);
  for (guint i = 0; i < query.n_params; i++)
    {
      GValue *src_value = &g_array_index (shortcut->signal.params, GValue, i);

      g_value_init (&params[1+i], G_VALUE_TYPE (src_value));
      g_value_copy (src_value, &params[1+i]);
    }

  if (query.return_type != G_TYPE_NONE)
    g_value_init (&return_value, query.return_type);

  g_signal_emitv (params, query.signal_id, shortcut->signal.detail, &return_value);

  for (guint i = 0; i < query.n_params + 1; i++)
    g_value_unset (&params[i]);
  g_free (params);

  return GDK_EVENT_STOP;

parameter_mismatch:
  g_warning ("The parameters are not correct for signal %s",
             shortcut->signal.name);

  /*
   * If there was a bug with the signal descriptor, we still want
   * to swallow the event to keep it from propagating further.
   */

  return GDK_EVENT_STOP;
}

static gboolean
shortcut_activate (Shortcut  *shortcut,
                   GtkWidget *widget)
{
  gboolean handled = FALSE;

  g_assert (shortcut != NULL);
  g_assert (GTK_IS_WIDGET (widget));

  for (; shortcut != NULL; shortcut = shortcut->next)
    {
      switch (shortcut->type)
        {
        case SHORTCUT_ACTION:
          handled |= shortcut_action_activate (shortcut, widget);
          break;

        case SHORTCUT_COMMAND:
          handled |= shortcut_command_activate (shortcut, widget);
          break;

        case SHORTCUT_SIGNAL:
          handled |= shortcut_signal_activate (shortcut, widget);
          break;

        default:
          g_assert_not_reached ();
        }
    }

  return handled;
}

static void
dzl_shortcut_context_finalize (GObject *object)
{
  DzlShortcutContext *self = (DzlShortcutContext *)object;
  DzlShortcutContextPrivate *priv = dzl_shortcut_context_get_instance_private (self);

  g_clear_pointer (&priv->table, dzl_shortcut_chord_table_free);

  G_OBJECT_CLASS (dzl_shortcut_context_parent_class)->finalize (object);
}

static void
dzl_shortcut_context_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  DzlShortcutContext *self = (DzlShortcutContext *)object;
  DzlShortcutContextPrivate *priv = dzl_shortcut_context_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_NAME:
      g_value_set_string (value, priv->name);
      break;

    case PROP_USE_BINDING_SETS:
      g_value_set_boolean (value, !!priv->use_binding_sets);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_context_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  DzlShortcutContext *self = (DzlShortcutContext *)object;
  DzlShortcutContextPrivate *priv = dzl_shortcut_context_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_NAME:
      priv->name = g_intern_string (g_value_get_string (value));
      break;

    case PROP_USE_BINDING_SETS:
      priv->use_binding_sets = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_context_class_init (DzlShortcutContextClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_shortcut_context_finalize;
  object_class->get_property = dzl_shortcut_context_get_property;
  object_class->set_property = dzl_shortcut_context_set_property;

  properties [PROP_NAME] =
    g_param_spec_string ("name",
                         "Name",
                         "Name",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_USE_BINDING_SETS] =
    g_param_spec_boolean ("use-binding-sets",
                          "Use Binding Sets",
                          "If the context should allow activation using binding sets",
                          TRUE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_shortcut_context_init (DzlShortcutContext *self)
{
  DzlShortcutContextPrivate *priv = dzl_shortcut_context_get_instance_private (self);

  priv->use_binding_sets = -1;
}

DzlShortcutContext *
dzl_shortcut_context_new (const gchar *name)
{
  return g_object_new (DZL_TYPE_SHORTCUT_CONTEXT,
                       "name", name,
                       NULL);
}

const gchar *
dzl_shortcut_context_get_name (DzlShortcutContext *self)
{
  DzlShortcutContextPrivate *priv = dzl_shortcut_context_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTEXT (self), NULL);

  return priv->name;
}

DzlShortcutMatch
dzl_shortcut_context_activate (DzlShortcutContext     *self,
                               GtkWidget              *widget,
                               const DzlShortcutChord *chord)
{
  DzlShortcutContextPrivate *priv = dzl_shortcut_context_get_instance_private (self);
  DzlShortcutMatch match = DZL_SHORTCUT_MATCH_NONE;
  Shortcut *shortcut = NULL;

  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTEXT (self), DZL_SHORTCUT_MATCH_NONE);
  g_return_val_if_fail (GTK_IS_WIDGET (widget), DZL_SHORTCUT_MATCH_NONE);
  g_return_val_if_fail (chord != NULL, DZL_SHORTCUT_MATCH_NONE);

#if 0
  g_print ("Looking up %s in table %p (of size %u)\n",
           dzl_shortcut_chord_to_string (chord),
           priv->table,
           dzl_shortcut_chord_table_size (priv->table));

  dzl_shortcut_chord_table_printf (priv->table);
#endif

  if (priv->table != NULL)
    match = dzl_shortcut_chord_table_lookup (priv->table, chord, (gpointer *)&shortcut);

  if (match == DZL_SHORTCUT_MATCH_EQUAL)
    {
      /*
       * If we got a full match, but it failed to activate, we could potentially
       * have another partial match. However, that lands squarely in the land of
       * undefined behavior. So instead we just assume there was no match.
       */
      if (!shortcut_activate (shortcut, widget))
        return DZL_SHORTCUT_MATCH_NONE;
    }

  return match;
}

static void
dzl_shortcut_context_add (DzlShortcutContext     *self,
                          const DzlShortcutChord *chord,
                          Shortcut               *shortcut)
{
  DzlShortcutContextPrivate *priv = dzl_shortcut_context_get_instance_private (self);
  DzlShortcutMatch match;
  Shortcut *head = NULL;

  g_assert (DZL_IS_SHORTCUT_CONTEXT (self));
  g_assert (shortcut != NULL);

  if (priv->table == NULL)
    {
      priv->table = dzl_shortcut_chord_table_new ();
      dzl_shortcut_chord_table_set_free_func (priv->table, shortcut_free);
    }

  /*
   * If we find that there is another entry for this shortcut, we chain onto
   * the end of that item. This allows us to call multiple signals, or
   * interleave signals and actions.
   */

  match = dzl_shortcut_chord_table_lookup (priv->table, chord, (gpointer *)&head);

  if (match == DZL_SHORTCUT_MATCH_EQUAL)
    {
      while (head->next != NULL)
        head = head->next;
      head->next = shortcut;
    }
  else
    {
      dzl_shortcut_chord_table_add (priv->table, chord, shortcut);
    }
}

void
dzl_shortcut_context_add_action (DzlShortcutContext *self,
                                 const gchar        *accel,
                                 const gchar        *detailed_action_name)
{
  Shortcut *shortcut;
  g_autofree gchar *action_name = NULL;
  g_autofree gchar *prefix = NULL;
  g_autoptr(GError) error = NULL;
  g_autoptr(GVariant) action_target = NULL;
  g_autoptr(DzlShortcutChord) chord = NULL;
  const gchar *dot;
  const gchar *name;

  g_return_if_fail (DZL_IS_SHORTCUT_CONTEXT (self));
  g_return_if_fail (accel != NULL);
  g_return_if_fail (detailed_action_name != NULL);

  chord = dzl_shortcut_chord_new_from_string (accel);

  if (chord == NULL)
    {
      g_warning ("Failed to parse accelerator “%s”", accel);
      return;
    }

  if (!g_action_parse_detailed_name (detailed_action_name, &action_name, &action_target, &error))
    {
      g_warning ("%s", error->message);
      return;
    }

  if (NULL != (dot = strchr (action_name, '.')))
    {
      name = &dot[1];
      prefix = g_strndup (action_name, dot - action_name);
    }
  else
    {
      name = action_name;
      prefix = NULL;
    }

  shortcut = g_slice_new0 (Shortcut);
  shortcut->type = SHORTCUT_ACTION;
  shortcut->action.prefix = prefix ? g_intern_string (prefix) : NULL;
  shortcut->action.name = g_intern_string (name);
  shortcut->action.param = g_steal_pointer (&action_target);

  dzl_shortcut_context_add (self, chord, shortcut);
}

void
dzl_shortcut_context_add_command (DzlShortcutContext *self,
                                  const gchar        *accel,
                                  const gchar        *command)
{
  g_autoptr(GError) error = NULL;
  g_autoptr(DzlShortcutChord) chord = NULL;
  Shortcut *shortcut;

  g_return_if_fail (DZL_IS_SHORTCUT_CONTEXT (self));
  g_return_if_fail (accel != NULL);
  g_return_if_fail (command != NULL);

  chord = dzl_shortcut_chord_new_from_string (accel);

  if (chord == NULL)
    {
      g_warning ("Failed to parse accelerator “%s” for command “%s”",
                 accel, command);
      return;
    }

  shortcut = g_slice_new0 (Shortcut);
  shortcut->type = SHORTCUT_COMMAND;
  shortcut->command = g_intern_string (command);

  dzl_shortcut_context_add (self, chord, shortcut);
}

void
dzl_shortcut_context_add_signal_va_list (DzlShortcutContext *self,
                                         const gchar        *accel,
                                         const gchar        *signal_name,
                                         guint               n_args,
                                         va_list             args)
{
  g_autoptr(GArray) params = NULL;
  g_autoptr(DzlShortcutChord) chord = NULL;
  g_autofree gchar *truncated_name = NULL;
  const gchar *detail_str;
  Shortcut *shortcut;
  GQuark detail = 0;

  g_return_if_fail (DZL_IS_SHORTCUT_CONTEXT (self));
  g_return_if_fail (accel != NULL);
  g_return_if_fail (signal_name != NULL);

  chord = dzl_shortcut_chord_new_from_string (accel);

  if (chord == NULL)
    {
      g_warning ("Failed to parse accelerator \"%s\"", accel);
      return;
    }

  if (NULL != (detail_str = strstr (signal_name, "::")))
    {
      truncated_name = g_strndup (signal_name, detail_str - signal_name);
      signal_name = truncated_name;
      detail_str = &detail_str[2];
      detail = g_quark_try_string (detail_str);
    }

  params = g_array_new (FALSE, FALSE, sizeof (GValue));
  g_array_set_clear_func (params, (GDestroyNotify)g_value_unset);

  for (; n_args > 0; n_args--)
    {
      g_autofree gchar *errstr = NULL;
      GValue value = { 0 };
      GType type;

      type = va_arg (args, GType);

      G_VALUE_COLLECT_INIT (&value, type, args, 0, &errstr);

      if (errstr != NULL)
        {
          g_warning ("%s", errstr);
          break;
        }

      g_array_append_val (params, value);
    }

  shortcut = g_slice_new0 (Shortcut);
  shortcut->type = SHORTCUT_SIGNAL;
  shortcut->signal.name = g_intern_string (signal_name);
  shortcut->signal.detail = detail;
  shortcut->signal.params = g_steal_pointer (&params);

  dzl_shortcut_context_add (self, chord, shortcut);
}

void
dzl_shortcut_context_add_signal (DzlShortcutContext *self,
                                 const gchar        *accel,
                                 const gchar        *signal_name,
                                 guint               n_args,
                                 ...)
{
  va_list args;

  va_start (args, n_args);
  dzl_shortcut_context_add_signal_va_list (self, accel, signal_name, n_args, args);
  va_end (args);
}

/**
 * dzl_shortcut_context_add_signalv:
 * @self: a #DzlShortcutContext
 * @accel: the accelerator for the shortcut
 * @signal_name: the name of the signal
 * @values: (element-type GObject.Value) (nullable) (transfer container): The
 *   values to use when calling the signal.
 *
 * This is similar to dzl_shortcut_context_add_signal() but is easier to use
 * from language bindings.
 *
 * Note that this transfers ownership of the @values array.
 */
void
dzl_shortcut_context_add_signalv (DzlShortcutContext *self,
                                  const gchar        *accel,
                                  const gchar        *signal_name,
                                  GArray             *values)
{
  g_autofree gchar *truncated_name = NULL;
  g_autoptr(DzlShortcutChord) chord = NULL;
  const gchar *detail_str;
  Shortcut *shortcut;
  GQuark detail = 0;

  g_return_if_fail (DZL_IS_SHORTCUT_CONTEXT (self));
  g_return_if_fail (accel != NULL);
  g_return_if_fail (signal_name != NULL);

  chord = dzl_shortcut_chord_new_from_string (accel);

  if (chord == NULL)
    {
      g_warning ("Failed to parse accelerator \"%s\"", accel);
      return;
    }

  if (values == NULL)
    {
      values = g_array_new (FALSE, FALSE, sizeof (GValue));
      g_array_set_clear_func (values, (GDestroyNotify)g_value_unset);
    }

  if (NULL != (detail_str = strstr (signal_name, "::")))
    {
      truncated_name = g_strndup (signal_name, detail_str - signal_name);
      signal_name = truncated_name;
      detail_str = &detail_str[2];
      detail = g_quark_try_string (detail_str);
    }

  shortcut = g_slice_new0 (Shortcut);
  shortcut->type = SHORTCUT_SIGNAL;
  shortcut->signal.name = g_intern_string (signal_name);
  shortcut->signal.detail = detail;
  shortcut->signal.params = values;

  dzl_shortcut_context_add (self, chord, shortcut);
}

gboolean
dzl_shortcut_context_remove (DzlShortcutContext *self,
                             const gchar        *accel)
{
  DzlShortcutContextPrivate *priv = dzl_shortcut_context_get_instance_private (self);
  g_autoptr(DzlShortcutChord) chord = NULL;

  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTEXT (self), FALSE);
  g_return_val_if_fail (accel != NULL, FALSE);

  chord = dzl_shortcut_chord_new_from_string (accel);

  if (chord != NULL && priv->table != NULL)
    return dzl_shortcut_chord_table_remove (priv->table, chord);

  return FALSE;
}

gboolean
dzl_shortcut_context_load_from_data (DzlShortcutContext  *self,
                                     const gchar         *data,
                                     gssize               len,
                                     GError             **error)
{
  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTEXT (self), FALSE);
  g_return_val_if_fail (data != NULL, FALSE);

  if (len < 0)
    len = strlen (data);

  g_set_error (error,
               G_IO_ERROR,
               G_IO_ERROR_INVALID_DATA,
               "Failed to parse shortcut data");

  return FALSE;
}

gboolean
dzl_shortcut_context_load_from_resource (DzlShortcutContext  *self,
                                         const gchar         *resource_path,
                                         GError             **error)
{
  g_autoptr(GBytes) bytes = NULL;
  const gchar *endptr = NULL;
  const gchar *data;
  gsize len;

  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTEXT (self), FALSE);

  if (NULL == (bytes = g_resources_lookup_data (resource_path, 0, error)))
    return FALSE;

  data = g_bytes_get_data (bytes, &len);

  if (!g_utf8_validate (data, len, &endptr))
    {
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_INVALID_DATA,
                   "Invalid UTF-8 at offset %u",
                   (guint)(endptr - data));
      return FALSE;
    }

  return dzl_shortcut_context_load_from_data (self, data, len, error);
}

DzlShortcutChordTable *
_dzl_shortcut_context_get_table (DzlShortcutContext *self)
{
  DzlShortcutContextPrivate *priv = dzl_shortcut_context_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTEXT (self), NULL);

  return priv->table;
}

void
_dzl_shortcut_context_merge (DzlShortcutContext *self,
                             DzlShortcutContext *layer)
{
  DzlShortcutContextPrivate *priv = dzl_shortcut_context_get_instance_private (self);
  DzlShortcutContextPrivate *layer_priv = dzl_shortcut_context_get_instance_private (layer);
  DzlShortcutChordTableIter iter;
  const DzlShortcutChord *chord;
  gpointer value;

  g_return_if_fail (DZL_IS_SHORTCUT_CONTEXT (self));
  g_return_if_fail (DZL_IS_SHORTCUT_CONTEXT (layer));
  g_return_if_fail (layer != self);

  if (layer_priv->use_binding_sets != -1)
    priv->use_binding_sets = layer_priv->use_binding_sets;

  _dzl_shortcut_chord_table_iter_init (&iter, layer_priv->table);
  while (_dzl_shortcut_chord_table_iter_next (&iter, &chord, &value))
    {
      Shortcut *sc = value;

      /* Make sure this doesn't exist in the base layer anymore */
      dzl_shortcut_chord_table_remove (priv->table, chord);

      /* Now add it to our table of chords */
      dzl_shortcut_context_add (self, chord, sc);

      /* Now we can safely steal this from the upper layer */
      _dzl_shortcut_chord_table_iter_steal (&iter);
    }
}
