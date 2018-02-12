/* dzl-shortcut-theme-save.c
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

#define G_LOG_DOMAIN "dzl-shortcut-theme-save"

#include "config.h"

#include "shortcuts/dzl-shortcut-theme.h"
#include "shortcuts/dzl-shortcut-private.h"

gboolean
dzl_shortcut_theme_save_to_stream (DzlShortcutTheme  *self,
                                   GOutputStream     *stream,
                                   GCancellable      *cancellable,
                                   GError           **error)
{
  g_autoptr(GString) str = NULL;
  DzlShortcutContext *context;
  GHashTable *contexts;
  GHashTableIter iter;
  const gchar *name;
  const gchar *parent;
  const gchar *title;
  const gchar *subtitle;

  g_return_val_if_fail (DZL_IS_SHORTCUT_THEME (self), FALSE);
  g_return_val_if_fail (G_IS_OUTPUT_STREAM (stream), FALSE);
  g_return_val_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable), FALSE);

  contexts = _dzl_shortcut_theme_get_contexts (self);

  str = g_string_new ("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");

  name = dzl_shortcut_theme_get_name (self);
  parent = dzl_shortcut_theme_get_parent_name (self);
  title = dzl_shortcut_theme_get_title (self);
  subtitle = dzl_shortcut_theme_get_subtitle (self);

  if (parent != NULL && !g_str_equal (parent, "internal"))
    g_string_append_printf (str, "<keytheme name=\"%s\" parent=\"%s\">\n", name, parent);
  else
    g_string_append_printf (str, "<keytheme name=\"%s\">\n", name);

  g_string_append_printf (str, "  <property name=\"title\" translatable=\"yes\">%s</property>\n", title ? title : "");
  g_string_append_printf (str, "  <property name=\"subtitle\" translatable=\"yes\">%s</property>\n", subtitle ? subtitle : "");

  g_hash_table_iter_init (&iter, contexts);

  while (g_hash_table_iter_next (&iter, NULL, (gpointer *)&context))
    {
      DzlShortcutChordTable *table;
      DzlShortcutChordTableIter citer;
      gboolean use_binding_sets = FALSE;
      const DzlShortcutChord *chord = NULL;
      DzlShortcutClosureChain *chain = NULL;

      table = _dzl_shortcut_context_get_table (context);
      name = dzl_shortcut_context_get_name (context);
      g_object_get (context, "use-binding-sets", &use_binding_sets, NULL);

      g_string_append_printf (str, "  <context name=\"%s\">\n", name);

      if (!use_binding_sets)
        g_string_append (str, "    <property name=\"use-binding-sets\">false</property>\n");

      _dzl_shortcut_chord_table_iter_init (&citer, table);

      while (_dzl_shortcut_chord_table_iter_next (&citer, &chord, (gpointer *)&chain))
        {
          g_autofree gchar *accel = dzl_shortcut_chord_to_string (chord);

          if (chain == NULL || accel == NULL)
            continue;

          g_string_append_printf (str, "    <shortcut accelerator=\"%s\">\n", accel);

          for (const GSList *node = &chain->node; node != NULL; node = node->next)
            {
              chain = node->data;

              if (chain->type == DZL_SHORTCUT_CLOSURE_ACTION)
                {
                  if (chain->action.params == NULL)
                    {
                      g_string_append_printf (str, "      <action name=\"%s.%s\"/>\n",
                                              chain->action.group, chain->action.name);
                    }
                  else
                    {
                      g_autofree gchar *fmt = g_variant_print (chain->action.params, FALSE);
                      g_string_append_printf (str, "      <action name=\"%s.%s::%s\"/>\n",
                                              chain->action.group, chain->action.name, fmt);
                    }
                }
              else if (chain->type == DZL_SHORTCUT_CLOSURE_SIGNAL)
                {
                  if (chain->signal.detail)
                    g_string_append_printf (str, "      <signal name=\"%s::%s\"",
                                            chain->signal.name,
                                            g_quark_to_string (chain->signal.detail));
                  else
                    g_string_append_printf (str, "      <signal name=\"%s\"",
                                            chain->signal.name);

                  if (chain->signal.params == NULL || chain->signal.params->len == 0)
                    {
                      g_string_append (str, "/>\n");
                      continue;
                    }

                  g_string_append (str, ">\n");

                  for (guint j = 0; j < chain->signal.params->len; j++)
                    {
                      GValue *value = &g_array_index (chain->signal.params, GValue, j);

                      if (G_VALUE_HOLDS_STRING (value))
                        {
                          g_autofree gchar *escape = g_markup_escape_text (g_value_get_string (value), -1);

                          g_string_append_printf (str, "        <param>\"%s\"</param>\n", escape);
                        }
                      else
                        {
                          g_auto(GValue) translated = G_VALUE_INIT;

                          g_value_init (&translated, G_TYPE_STRING);
                          g_value_transform (value, &translated);
                          g_string_append_printf (str, "        <param>%s</param>\n", g_value_get_string (&translated));
                        }
                    }

                  g_string_append (str, "      </signal>\n");

                }
            }

          g_string_append (str, "    </shortcut>\n");
        }

      g_string_append (str, "  </context>\n");
    }

  g_string_append (str, "</keytheme>\n");

  return g_output_stream_write_all (stream, str->str, str->len, NULL, cancellable, error);
}

gboolean
dzl_shortcut_theme_save_to_file (DzlShortcutTheme  *self,
                                 GFile             *file,
                                 GCancellable      *cancellable,
                                 GError           **error)
{
  g_autoptr(GFileOutputStream) stream = NULL;

  g_return_val_if_fail (DZL_IS_SHORTCUT_THEME (self), FALSE);
  g_return_val_if_fail (G_IS_FILE (file), FALSE);
  g_return_val_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable), FALSE);

  stream = g_file_replace (file,
                           NULL,
                           FALSE,
                           G_FILE_CREATE_REPLACE_DESTINATION,
                           cancellable,
                           error);

  if (stream == NULL)
    return FALSE;

  return dzl_shortcut_theme_save_to_stream (self, G_OUTPUT_STREAM (stream), cancellable, error);
}

gboolean
dzl_shortcut_theme_save_to_path (DzlShortcutTheme  *self,
                                 const gchar       *path,
                                 GCancellable      *cancellable,
                                 GError           **error)
{
  g_autoptr(GFile) file = NULL;

  g_return_val_if_fail (DZL_IS_SHORTCUT_THEME (self), FALSE);
  g_return_val_if_fail (path != NULL, FALSE);
  g_return_val_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable), FALSE);

  file = g_file_new_for_path (path);

  return dzl_shortcut_theme_save_to_file (self, file, cancellable, error);
}
