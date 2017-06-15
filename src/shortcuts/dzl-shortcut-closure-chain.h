/* dzl-shortcut-closure-chain.h
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

#ifndef DZL_SHORTCUT_CLOSURE_CHAIN_H
#define DZL_SHORTCUT_CLOSURE_CHAIN_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _DzlShortcutClosureChain DzlShortcutClosureChain;

DzlShortcutClosureChain *dzl_shortcut_closure_chain_append               (DzlShortcutClosureChain *chain,
                                                                          DzlShortcutClosureChain *link);
DzlShortcutClosureChain *dzl_shortcut_closure_chain_append_signal        (DzlShortcutClosureChain *chain,
                                                                          const gchar             *signal_name,
                                                                          guint                    n_args,
                                                                          va_list                  args);
DzlShortcutClosureChain *dzl_shortcut_closure_chain_append_signalv       (DzlShortcutClosureChain *chain,
                                                                          const gchar             *signal_name,
                                                                          GArray                  *params);
DzlShortcutClosureChain *dzl_shortcut_closure_chain_append_action        (DzlShortcutClosureChain *chain,
                                                                          const gchar             *group_name,
                                                                          const gchar             *action_name,
                                                                          GVariant                *params);
DzlShortcutClosureChain *dzl_shortcut_closure_chain_append_action_string (DzlShortcutClosureChain *chain,
                                                                          const gchar             *detailed_action_name);
DzlShortcutClosureChain *dzl_shortcut_closure_chain_append_command       (DzlShortcutClosureChain *chain,
                                                                          const gchar             *command);
DzlShortcutClosureChain *dzl_shortcut_closure_chain_append_callback      (DzlShortcutClosureChain *chain,
                                                                          GtkCallback              callback,
                                                                          gpointer                 user_data,
                                                                          GDestroyNotify           notify);
gboolean                 dzl_shortcut_closure_chain_execute              (DzlShortcutClosureChain *chain,
                                                                          GtkWidget               *widget);
void                     dzl_shortcut_closure_chain_free                 (DzlShortcutClosureChain *chain);

G_END_DECLS

#endif /* DZL_SHORTCUT_CLOSURE_CHAIN_H */
