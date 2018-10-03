/* test-shortcut-tooltip.c
 *
 * Copyright 2018 Christian Hergert <chergert@redhat.com>
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
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <dazzle.h>

static const DzlShortcutEntry entries[] = {
  {
    "org.gnome.dazzle.test.fullscreen",
    DZL_SHORTCUT_PHASE_CAPTURE,
    "F11",
    "Window",
    "Management",
    "Fullscreen window",
    NULL
  }
};

static void
button_clicked_cb (GtkButton          *button,
                   DzlShortcutTooltip *tooltip)
{
  static guint count;

  count++;

  dzl_shortcut_tooltip_set_accel (tooltip, count % 2 ? "<Shift>F11" : "F11");
  dzl_shortcut_tooltip_set_title (tooltip, count % 2 ? "Unfullscreen window" : "Fullscreen window");
}

gint
main (gint argc,
      gchar *argv[])
{
  GtkWindow *window;
  GtkWidget *button;
  g_autoptr(DzlShortcutTooltip) tooltip = NULL;
  DzlShortcutController *controller;

  gtk_init (&argc, &argv);

  dzl_shortcut_manager_add_shortcut_entries (NULL, entries, G_N_ELEMENTS (entries), NULL);

  window = g_object_new (GTK_TYPE_WINDOW,
                         "title", "Shorcut tooltip test",
                         NULL);
  button = g_object_new (GTK_TYPE_BUTTON,
                         "visible", TRUE,
                         "label", "Test Button",
                         NULL);
  controller = dzl_shortcut_controller_find (button);
  dzl_shortcut_controller_add_command_action (controller,
                                              "org.gnome.dazzle.test.fullscreen",
                                              "F11",
                                              DZL_SHORTCUT_PHASE_CAPTURE,
                                              "win.fullscren");

  gtk_container_add (GTK_CONTAINER (window), button);

  tooltip = dzl_shortcut_tooltip_new ();

  dzl_shortcut_tooltip_set_widget (tooltip, button);
  dzl_shortcut_tooltip_set_command_id (tooltip, "org.gnome.dazzle.test.fullscreen");

  g_signal_connect (button, "clicked", G_CALLBACK (button_clicked_cb), tooltip);

  g_signal_connect (window, "delete-event", gtk_main_quit, NULL);
  gtk_window_present (GTK_WINDOW (window));

  gtk_main ();

  return 0;
}
