/* dzl-shortcut-closure-chain.c
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

#define G_LOG_DOMAIN "dzl-shortcut-closure-chain"

#include "config.h"

#include <gobject/gvaluecollector.h>
#include <string.h>

#include "dzl-debug.h"

#include "shortcuts/dzl-shortcut-closure-chain.h"
#include "shortcuts/dzl-shortcut-controller.h"
#include "shortcuts/dzl-shortcut-private.h"
#include "util/dzl-gtk.h"
#include "util/dzl-util-private.h"

static DzlShortcutClosureChain *
dzl_shortcut_closure_chain_new (DzlShortcutClosureType type)
{
  DzlShortcutClosureChain *ret;

  g_assert (type > 0);
  g_assert (type < DZL_SHORTCUT_CLOSURE_LAST);

  ret = g_slice_new0 (DzlShortcutClosureChain);
  ret->magic = DZL_SHORTCUT_CLOSURE_CHAIN_MAGIC;
  ret->node.data = ret;
  ret->type = type;

  return ret;
}

DzlShortcutClosureChain *
dzl_shortcut_closure_chain_append (DzlShortcutClosureChain *chain,
                                   DzlShortcutClosureChain *element)
{
  DzlShortcutClosureChain *ret;

  g_return_val_if_fail (!chain || DZL_IS_SHORTCUT_CLOSURE_CHAIN (chain), NULL);
  g_return_val_if_fail (!element || DZL_IS_SHORTCUT_CLOSURE_CHAIN (element), NULL);
  g_return_val_if_fail (chain || element, NULL);

  if (chain == NULL)
    return element;

  if (element == NULL)
    return chain;

  ret = g_slist_concat (&chain->node, &element->node)->data;

  g_return_val_if_fail (DZL_IS_SHORTCUT_CLOSURE_CHAIN (ret), NULL);

  return ret;
}

void
dzl_shortcut_closure_chain_free (DzlShortcutClosureChain *chain)
{
  if (chain == NULL)
    return;

  g_assert (DZL_IS_SHORTCUT_CLOSURE_CHAIN (chain));
  g_assert (chain->node.data == (gpointer)chain);

  if (chain->executing)
    {
      g_warning ("Attempt to dispose a closure chain while executing, leaking");
      return;
    }

  chain->magic = 0xAAAAAAAA;

  if (chain->node.next)
    dzl_shortcut_closure_chain_free (chain->node.next->data);

  chain->node.next = NULL;
  chain->node.data = NULL;

  if (chain->type == DZL_SHORTCUT_CLOSURE_ACTION)
    g_clear_pointer (&chain->action.params, g_variant_unref);
  else if (chain->type == DZL_SHORTCUT_CLOSURE_CALLBACK)
    {
      if (chain->callback.notify)
        g_clear_pointer (&chain->callback.user_data, chain->callback.notify);
    }
  else if (chain->type == DZL_SHORTCUT_CLOSURE_SIGNAL)
    g_clear_pointer (&chain->signal.params, g_array_unref);

  g_slice_free (DzlShortcutClosureChain, chain);
}

DzlShortcutClosureChain *
dzl_shortcut_closure_chain_append_callback (DzlShortcutClosureChain *chain,
                                            GtkCallback              callback,
                                            gpointer                 user_data,
                                            GDestroyNotify           notify)
{
  DzlShortcutClosureChain *tail;

  g_return_val_if_fail (!chain || DZL_IS_SHORTCUT_CLOSURE_CHAIN (chain), NULL);
  g_return_val_if_fail (callback != NULL, NULL);

  tail = dzl_shortcut_closure_chain_new (DZL_SHORTCUT_CLOSURE_CALLBACK);
  tail->callback.callback = callback;
  tail->callback.user_data = user_data;
  tail->callback.notify = notify;

  return dzl_shortcut_closure_chain_append (chain, tail);
}

DzlShortcutClosureChain *
dzl_shortcut_closure_chain_append_command (DzlShortcutClosureChain *chain,
                                           const gchar             *command)
{
  DzlShortcutClosureChain *tail;

  g_return_val_if_fail (!chain || DZL_IS_SHORTCUT_CLOSURE_CHAIN (chain), NULL);
  g_return_val_if_fail (command != NULL, NULL);

  tail = dzl_shortcut_closure_chain_new (DZL_SHORTCUT_CLOSURE_COMMAND);
  tail->command.name = g_intern_string (command);

  return dzl_shortcut_closure_chain_append (chain, tail);
}

DzlShortcutClosureChain *
dzl_shortcut_closure_chain_append_action (DzlShortcutClosureChain *chain,
                                          const gchar             *group_name,
                                          const gchar             *action_name,
                                          GVariant                *params)
{
  DzlShortcutClosureChain *tail;

  g_return_val_if_fail (!chain || DZL_IS_SHORTCUT_CLOSURE_CHAIN (chain), NULL);
  g_return_val_if_fail (group_name != NULL, NULL);
  g_return_val_if_fail (action_name != NULL, NULL);

  tail = dzl_shortcut_closure_chain_new (DZL_SHORTCUT_CLOSURE_ACTION);
  tail->action.group = g_intern_string (group_name);
  tail->action.name = g_intern_string (action_name);
  tail->action.params = params ? g_variant_ref_sink (params) : NULL;

  return dzl_shortcut_closure_chain_append (chain, tail);
}

DzlShortcutClosureChain *
dzl_shortcut_closure_chain_append_action_string (DzlShortcutClosureChain *chain,
                                                 const gchar             *detailed_action_name)
{
  DzlShortcutClosureChain *tail;
  g_autoptr(GVariant) target_value = NULL;
  g_autofree gchar *prefix = NULL;
  g_autofree gchar *name = NULL;

  g_return_val_if_fail (!chain || DZL_IS_SHORTCUT_CLOSURE_CHAIN (chain), NULL);
  g_return_val_if_fail (detailed_action_name != NULL, NULL);

  if (!dzl_g_action_name_parse_full (detailed_action_name, &prefix, &name, &target_value))
    {
      g_warning ("Failed to parse action: %s", detailed_action_name);
      return NULL;
    }

  tail = dzl_shortcut_closure_chain_new (DZL_SHORTCUT_CLOSURE_ACTION);
  tail->action.group = g_intern_string (prefix);
  tail->action.name = g_intern_string (name);
  tail->action.params = g_steal_pointer (&target_value);

  return dzl_shortcut_closure_chain_append (chain, tail);
}

DzlShortcutClosureChain *
dzl_shortcut_closure_chain_append_signalv (DzlShortcutClosureChain *chain,
                                           const gchar             *signal_name,
                                           GArray                  *params)
{
  g_autofree gchar *truncated_name = NULL;
  DzlShortcutClosureChain *tail;
  g_autoptr(GArray) copy = NULL;
  const gchar *detail_str;
  GQuark detail = 0;

  g_return_val_if_fail (!chain || DZL_IS_SHORTCUT_CLOSURE_CHAIN (chain), NULL);
  g_return_val_if_fail (signal_name != NULL, NULL);

  if (params != NULL)
    {
      copy = g_array_sized_new (FALSE, TRUE, sizeof (GValue), params->len);
      g_array_set_clear_func (copy, (GDestroyNotify)g_value_unset);
      g_array_set_size (copy, params->len);

      for (guint i = 0; i < params->len; i++)
        {
          GValue *src = &g_array_index (params, GValue, i);
          GValue *dst = &g_array_index (copy, GValue, i);

          g_value_init (dst, G_VALUE_TYPE (src));
          g_value_copy (src, dst);
        }
    }

  if (NULL != (detail_str = strstr (signal_name, "::")))
    {
      truncated_name = g_strndup (signal_name, detail_str - signal_name);
      signal_name = truncated_name;
      detail_str = &detail_str[2];
      detail = g_quark_try_string (detail_str);
    }

  tail = dzl_shortcut_closure_chain_new (DZL_SHORTCUT_CLOSURE_SIGNAL);
  tail->signal.name = g_intern_string (signal_name);
  tail->signal.params = g_steal_pointer (&copy);
  tail->signal.detail = detail;

  return dzl_shortcut_closure_chain_append (chain, tail);
}

DzlShortcutClosureChain *
dzl_shortcut_closure_chain_append_signal (DzlShortcutClosureChain *chain,
                                          const gchar             *signal_name,
                                          guint                    n_args,
                                          va_list                  args)
{
  g_autoptr(GArray) params = NULL;

  g_return_val_if_fail (!chain || DZL_IS_SHORTCUT_CLOSURE_CHAIN (chain), NULL);
  g_return_val_if_fail (signal_name != NULL, NULL);

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

  return dzl_shortcut_closure_chain_append_signalv (chain, signal_name, params);
}

static gboolean
find_instance_and_signal (GtkWidget    *widget,
                          const gchar  *signal_name,
                          gpointer     *instance,
                          GSignalQuery *query)
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

  controller = dzl_shortcut_controller_try_find (widget);

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
signal_activate (DzlShortcutClosureChain *chain,
                 GtkWidget               *widget)
{
  GValue *params;
  GValue return_value = { 0 };
  GSignalQuery query;
  gpointer instance = NULL;

  g_assert (DZL_IS_SHORTCUT_CLOSURE_CHAIN (chain));
  g_assert (chain->type == DZL_SHORTCUT_CLOSURE_SIGNAL);
  g_assert (GTK_IS_WIDGET (widget));

  if (!find_instance_and_signal (widget, chain->signal.name, &instance, &query))
    {
      g_warning ("Failed to locate signal %s in hierarchy of %s",
                 chain->signal.name, G_OBJECT_TYPE_NAME (widget));
      return TRUE;
    }

  if (query.n_params != chain->signal.params->len)
    goto parameter_mismatch;

  for (guint i = 0; i < query.n_params; i++)
    {
      if (!G_VALUE_HOLDS (&g_array_index (chain->signal.params, GValue, i), query.param_types[i]))
        goto parameter_mismatch;
    }

  params = g_new0 (GValue, 1 + query.n_params);
  g_value_init_from_instance (&params[0], instance);
  for (guint i = 0; i < query.n_params; i++)
    {
      GValue *src_value = &g_array_index (chain->signal.params, GValue, i);

      g_value_init (&params[1+i], G_VALUE_TYPE (src_value));
      g_value_copy (src_value, &params[1+i]);
    }

  if (query.return_type != G_TYPE_NONE)
    g_value_init (&return_value, query.return_type);

  g_signal_emitv (params, query.signal_id, chain->signal.detail, &return_value);

  for (guint i = 0; i < query.n_params + 1; i++)
    g_value_unset (&params[i]);
  g_free (params);

  if (query.return_type != G_TYPE_NONE)
    g_value_unset (&return_value);

  return GDK_EVENT_STOP;

parameter_mismatch:
  g_warning ("The parameters are not correct for signal %s",
             chain->signal.name);

  /*
   * If there was a bug with the signal descriptor, we still want
   * to swallow the event to keep it from propagating further.
   */

  return GDK_EVENT_STOP;
}

static gboolean
command_activate (DzlShortcutClosureChain *chain,
                  GtkWidget               *widget)
{
  g_assert (DZL_IS_SHORTCUT_CLOSURE_CHAIN (chain));
  g_assert (GTK_IS_WIDGET (widget));

  for (; widget != NULL; widget = gtk_widget_get_parent (widget))
    {
      DzlShortcutController *controller = dzl_shortcut_controller_try_find (widget);

      if (controller != NULL)
        {
          if (dzl_shortcut_controller_execute_command (controller, chain->command.name))
            return TRUE;
        }
    }

  g_warning ("Failed to locate controller command: %s", chain->command.name);

  return FALSE;
}

gboolean
dzl_shortcut_closure_chain_execute (DzlShortcutClosureChain *chain,
                                    GtkWidget               *widget)
{
  g_autoptr(GtkWidget) hold = NULL;
  gboolean ret = FALSE;

  DZL_ENTRY;

  g_return_val_if_fail (DZL_IS_SHORTCUT_CLOSURE_CHAIN (chain), FALSE);
  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);

  if (chain->executing)
    {
      g_warning ("Attempt for re-entrancy in closure chain activation blocked");
      DZL_RETURN (FALSE);
    }

  if (gtk_widget_in_destruction (widget))
    {
      g_warning ("Attempt to activate shortcut while in destruction");
      DZL_RETURN (FALSE);
    }

  hold = g_object_ref (widget);

  chain->executing = TRUE;

  switch (chain->type)
    {
    case DZL_SHORTCUT_CLOSURE_ACTION:
      DZL_TRACE_MSG ("executing closure action %s.%s",
                     chain->action.group, chain->action.name);
      ret |= dzl_gtk_widget_action (hold,
                                    chain->action.group,
                                    chain->action.name,
                                    chain->action.params);
      break;

    case DZL_SHORTCUT_CLOSURE_CALLBACK:
      DZL_TRACE_MSG ("executing closure callback");
      chain->callback.callback (hold, chain->callback.user_data);
      ret = TRUE;
      break;

    case DZL_SHORTCUT_CLOSURE_SIGNAL:
      DZL_TRACE_MSG ("executing closure signal");
      ret |= signal_activate (chain, hold);
      break;

    case DZL_SHORTCUT_CLOSURE_COMMAND:
      DZL_TRACE_MSG ("executing closure command \"%s\"", chain->command.name);
      ret |= command_activate (chain, hold);
      break;

    case DZL_SHORTCUT_CLOSURE_LAST:
    default:
      g_warning ("Unknown closure type");
      break;
    }

  if (chain->node.next != NULL)
    ret |= dzl_shortcut_closure_chain_execute (chain->node.next->data, hold);

  chain->executing = FALSE;

  DZL_TRACE_MSG ("ret = %d", ret);

  DZL_RETURN (ret);
}
