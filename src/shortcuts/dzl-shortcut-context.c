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

#include "config.h"

#include <gobject/gvaluecollector.h>
#include <string.h>

#include "dzl-debug.h"

#include "shortcuts/dzl-shortcut-chord.h"
#include "shortcuts/dzl-shortcut-closure-chain.h"
#include "shortcuts/dzl-shortcut-context.h"
#include "shortcuts/dzl-shortcut-controller.h"
#include "shortcuts/dzl-shortcut-private.h"
#include "util/dzl-macros.h"

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

gboolean
_dzl_shortcut_context_contains (DzlShortcutContext     *self,
                                const DzlShortcutChord *chord)
{
  DzlShortcutContextPrivate *priv = dzl_shortcut_context_get_instance_private (self);
  gpointer data;

  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTEXT (self), FALSE);
  g_return_val_if_fail (chord != NULL, FALSE);

  return priv->table != NULL &&
         dzl_shortcut_chord_table_lookup (priv->table, chord, &data) == DZL_SHORTCUT_MATCH_EQUAL;
}

DzlShortcutMatch
dzl_shortcut_context_activate (DzlShortcutContext     *self,
                               GtkWidget              *widget,
                               const DzlShortcutChord *chord)
{
  DzlShortcutContextPrivate *priv = dzl_shortcut_context_get_instance_private (self);
  DzlShortcutMatch match = DZL_SHORTCUT_MATCH_NONE;
  DzlShortcutClosureChain *chain = NULL;

  DZL_ENTRY;

  g_return_val_if_fail (DZL_IS_SHORTCUT_CONTEXT (self), DZL_SHORTCUT_MATCH_NONE);
  g_return_val_if_fail (GTK_IS_WIDGET (widget), DZL_SHORTCUT_MATCH_NONE);
  g_return_val_if_fail (chord != NULL, DZL_SHORTCUT_MATCH_NONE);

  if (priv->table == NULL)
    DZL_RETURN (DZL_SHORTCUT_MATCH_NONE);

#if 0
  g_print ("Looking up %s in table %p (of size %u)\n",
           dzl_shortcut_chord_to_string (chord),
           priv->table,
           dzl_shortcut_chord_table_size (priv->table));

  dzl_shortcut_chord_table_printf (priv->table);
#endif

  match = dzl_shortcut_chord_table_lookup (priv->table, chord, (gpointer *)&chain);

  if (match == DZL_SHORTCUT_MATCH_EQUAL)
    {
      g_assert (chain != NULL);

      /*
       * If we got a full match, but it failed to activate, we could potentially
       * have another partial match. However, that lands squarely in the land of
       * undefined behavior. So instead we just assume there was no match.
       */
      if (!dzl_shortcut_closure_chain_execute (chain, widget))
        match = DZL_SHORTCUT_MATCH_NONE;
    }

  DZL_TRACE_MSG ("%s: match = %d", priv->name, match);

  DZL_RETURN (match);
}

static void
dzl_shortcut_context_add (DzlShortcutContext      *self,
                          const DzlShortcutChord  *chord,
                          DzlShortcutClosureChain *chain)
{
  DzlShortcutContextPrivate *priv = dzl_shortcut_context_get_instance_private (self);
  DzlShortcutClosureChain *head = NULL;
  DzlShortcutMatch match;

  g_assert (DZL_IS_SHORTCUT_CONTEXT (self));
  g_assert (chord != NULL);
  g_assert (chain != NULL);

  if (priv->table == NULL)
    {
      priv->table = dzl_shortcut_chord_table_new ();
      dzl_shortcut_chord_table_set_free_func (priv->table,
                                              (GDestroyNotify)dzl_shortcut_closure_chain_free);
    }

  /*
   * If we find that there is another entry for this shortcut, we chain onto
   * the end of that item. This allows us to call multiple signals, or
   * interleave signals and actions.
   */

  match = dzl_shortcut_chord_table_lookup (priv->table, chord, (gpointer *)&head);

  if (match == DZL_SHORTCUT_MATCH_EQUAL)
    dzl_shortcut_closure_chain_append (head, chain);
  else
    dzl_shortcut_chord_table_add (priv->table, chord, chain);
}

void
dzl_shortcut_context_add_action (DzlShortcutContext *self,
                                 const gchar        *accel,
                                 const gchar        *detailed_action_name)
{
  g_autoptr(DzlShortcutChord) chord = NULL;
  DzlShortcutClosureChain *chain;

  g_return_if_fail (DZL_IS_SHORTCUT_CONTEXT (self));
  g_return_if_fail (accel != NULL);
  g_return_if_fail (detailed_action_name != NULL);

  chord = dzl_shortcut_chord_new_from_string (accel);

  if (chord == NULL)
    {
      g_warning ("Failed to parse accelerator “%s”", accel);
      return;
    }

  chain = dzl_shortcut_closure_chain_append_action_string (NULL, detailed_action_name);

  dzl_shortcut_context_add (self, chord, chain);
}

void
dzl_shortcut_context_add_command (DzlShortcutContext *self,
                                  const gchar        *accel,
                                  const gchar        *command)
{
  g_autoptr(DzlShortcutChord) chord = NULL;
  DzlShortcutClosureChain *chain;

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

  chain = dzl_shortcut_closure_chain_append_command (NULL, command);

  dzl_shortcut_context_add (self, chord, chain);
}

void
dzl_shortcut_context_add_signal_va_list (DzlShortcutContext *self,
                                         const gchar        *accel,
                                         const gchar        *signal_name,
                                         guint               n_args,
                                         va_list             args)
{
  g_autoptr(DzlShortcutChord) chord = NULL;
  DzlShortcutClosureChain *chain;

  g_return_if_fail (DZL_IS_SHORTCUT_CONTEXT (self));
  g_return_if_fail (accel != NULL);
  g_return_if_fail (signal_name != NULL);

  chord = dzl_shortcut_chord_new_from_string (accel);

  if (chord == NULL)
    {
      g_warning ("Failed to parse accelerator \"%s\"", accel);
      return;
    }

  chain = dzl_shortcut_closure_chain_append_signal (NULL, signal_name, n_args, args);

  dzl_shortcut_context_add (self, chord, chain);
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
 * @values: (element-type GObject.Value) (nullable) (transfer none): The
 *   values to use when calling the signal.
 *
 * This is similar to dzl_shortcut_context_add_signal() but is easier to use
 * from language bindings.
 */
void
dzl_shortcut_context_add_signalv (DzlShortcutContext *self,
                                  const gchar        *accel,
                                  const gchar        *signal_name,
                                  GArray             *values)
{
  g_autoptr(DzlShortcutChord) chord = NULL;
  DzlShortcutClosureChain *chain;

  g_return_if_fail (DZL_IS_SHORTCUT_CONTEXT (self));
  g_return_if_fail (accel != NULL);
  g_return_if_fail (signal_name != NULL);

  chord = dzl_shortcut_chord_new_from_string (accel);

  if (chord == NULL)
    {
      g_warning ("Failed to parse accelerator \"%s\"", accel);
      return;
    }

  chain = dzl_shortcut_closure_chain_append_signalv (NULL, signal_name, values);

  dzl_shortcut_context_add (self, chord, chain);
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
      DzlShortcutClosureChain *chain = value;

      /* Make sure this doesn't exist in the base layer anymore */
      dzl_shortcut_chord_table_remove (priv->table, chord);

      /* Now add it to our table of chords */
      dzl_shortcut_context_add (self, chord, chain);

      /* Now we can safely steal this from the upper layer */
      _dzl_shortcut_chord_table_iter_steal (&iter);
    }
}
