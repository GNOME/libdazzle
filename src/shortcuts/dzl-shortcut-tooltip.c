/* dzl-shortcut-tooltip.c
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

#include "config.h"

#define G_LOG_DOMAIN "dzl-shortcut-tooltip"

#include "shortcuts/dzl-shortcut-controller.h"
#include "shortcuts/dzl-shortcut-private.h"
#include "shortcuts/dzl-shortcut-simple-label.h"
#include "shortcuts/dzl-shortcut-theme.h"
#include "shortcuts/dzl-shortcut-tooltip.h"
#include "util/dzl-macros.h"

/**
 * SECTION:dzl-shortcut-tooltip
 * @title: DzlShortcutTooltip
 * @short_description: display fancy tooltips containing shortcuts
 *
 * This class is used to display a fancy shortcut on a tooltip along with
 * information about the shortcut.
 *
 * The shortcut must be registered with Dazzle using the DzlShortcutManager
 * and have a registered command-id.
 *
 * The display text for the shortcut will match that shown in the shortcuts
 * window help.
 *
 * Since: 3.32
 */

struct _DzlShortcutTooltip
{
  GObject parent_instance;

  /* Interned string with the command-id to update the shortcut
   * based on the current key-theme.
   */
  const gchar *command_id;

  /* If the title is set manually, we do not need to discover it
   * from the registered shortcuts.
   */
  gchar *title;

  /* If no command-id is set, the accelerator can be specified
   * directly to avoid looking up by command-id.
   */
  gchar *accel;

  /* The widget that we are watching the ::query-tooltip for. */
  GtkWidget *widget;

  /* The signal connection id for ::query() on @widget */
  gulong query_handler;

  /* Our signal connection id for ::destroy() on @widget */
  gulong destroy_handler;
};

G_DEFINE_TYPE (DzlShortcutTooltip, dzl_shortcut_tooltip, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_ACCEL,
  PROP_COMMAND_ID,
  PROP_TITLE,
  PROP_WIDGET,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static DzlShortcutManager *
find_manager (GtkWidget *widget)
{
  DzlShortcutController *controller;
  DzlShortcutManager *manager = NULL;

  if (widget == NULL)
    return NULL;

  if (!(controller = dzl_shortcut_controller_try_find (widget)))
    {
      widget = gtk_widget_get_ancestor (widget, GTK_TYPE_WINDOW);
      controller = dzl_shortcut_controller_try_find (widget);
    }

  if (controller != NULL)
    manager = dzl_shortcut_controller_get_manager (controller);

  if (manager == NULL)
    manager = dzl_shortcut_manager_get_default ();

  return manager;
}

static gboolean
dzl_shortcut_tooltip_query_cb (DzlShortcutTooltip *self,
                               gint                x,
                               gint                y,
                               gboolean            keyboard_mode,
                               GtkTooltip         *tooltip,
                               GtkWidget          *widget)
{
  const DzlShortcutChord *chord;
  DzlShortcutManager *manager = NULL;
  DzlShortcutTheme *theme = NULL;
  DzlShortcutSimpleLabel *label;
  g_autofree gchar *accel = NULL;
  const gchar *title = NULL;
  const gchar *subtitle = NULL;

  g_assert (DZL_IS_SHORTCUT_TOOLTIP (self));
  g_assert (GTK_IS_TOOLTIP (tooltip));
  g_assert (GTK_IS_WIDGET (widget));
  g_assert (widget == self->widget);

  manager = find_manager (widget);
  theme = dzl_shortcut_manager_get_theme (manager);

  title = self->title;

  if (title == NULL && self->command_id != NULL)
    _dzl_shortcut_manager_get_command_info (manager, self->command_id, &title, &subtitle);

  if (title == NULL)
    return FALSE;

  if (!(accel = g_strdup (self->accel)))
    {
      if (self->command_id != NULL && theme != NULL &&
          (chord = dzl_shortcut_theme_get_chord_for_command (theme, self->command_id)))
        accel = dzl_shortcut_chord_to_string (chord);
      if (accel == NULL)
        return FALSE;
    }

  g_assert (accel != NULL);
  g_assert (title != NULL);

  label = DZL_SHORTCUT_SIMPLE_LABEL (dzl_shortcut_simple_label_new ());
  if (self->command_id != NULL)
    dzl_shortcut_simple_label_set_command (label, self->command_id);
  dzl_shortcut_simple_label_set_accel (label, accel);
  dzl_shortcut_simple_label_set_title (label, title);
  gtk_widget_show (GTK_WIDGET (label));

  gtk_tooltip_set_custom (tooltip, GTK_WIDGET (label));

  return TRUE;
}

/**
 * dzl_shortcut_tooltip_new:
 *
 * Create a new #DzlShortcutTooltip.
 *
 * Returns: (transfer full): a newly created #DzlShortcutTooltip
 *
 * Since: 3.32
 */
DzlShortcutTooltip *
dzl_shortcut_tooltip_new (void)
{
  return g_object_new (DZL_TYPE_SHORTCUT_TOOLTIP, NULL);
}

static void
dzl_shortcut_tooltip_finalize (GObject *object)
{
  DzlShortcutTooltip *self = (DzlShortcutTooltip *)object;

  if (self->widget != NULL && self->query_handler != 0)
    {
      dzl_clear_signal_handler (self->widget, &self->query_handler);
      dzl_clear_signal_handler (self->widget, &self->destroy_handler);
    }

  self->widget = NULL;

  g_clear_pointer (&self->title, g_free);
  g_clear_pointer (&self->accel, g_free);

  G_OBJECT_CLASS (dzl_shortcut_tooltip_parent_class)->finalize (object);
}

static void
dzl_shortcut_tooltip_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  DzlShortcutTooltip *self = DZL_SHORTCUT_TOOLTIP (object);

  switch (prop_id)
    {
    case PROP_ACCEL:
      g_value_set_string (value, dzl_shortcut_tooltip_get_accel (self));
      break;

    case PROP_WIDGET:
      g_value_set_object (value, dzl_shortcut_tooltip_get_widget (self));
      break;

    case PROP_COMMAND_ID:
      g_value_set_static_string (value, self->command_id);
      break;

    case PROP_TITLE:
      g_value_set_string (value, dzl_shortcut_tooltip_get_title (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_tooltip_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  DzlShortcutTooltip *self = DZL_SHORTCUT_TOOLTIP (object);

  switch (prop_id)
    {
    case PROP_ACCEL:
      dzl_shortcut_tooltip_set_accel (self, g_value_get_object (value));
      break;

    case PROP_WIDGET:
      dzl_shortcut_tooltip_set_widget (self, g_value_get_object (value));
      break;

    case PROP_COMMAND_ID:
      dzl_shortcut_tooltip_set_command_id (self, g_value_get_string (value));
      break;

    case PROP_TITLE:
      dzl_shortcut_tooltip_set_title (self, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_tooltip_class_init (DzlShortcutTooltipClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_shortcut_tooltip_finalize;
  object_class->get_property = dzl_shortcut_tooltip_get_property;
  object_class->set_property = dzl_shortcut_tooltip_set_property;

  properties [PROP_ACCEL] =
    g_param_spec_string ("accel",
                         "Accel",
                         "The accel for the label, if overriding the discovered accel",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_COMMAND_ID] =
    g_param_spec_string ("command-id",
                         "Command Id",
                         "The shortcut command-id to track for shortcut changes",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  /**
   * DzlShortcutTooltip:title:
   *
   * The "title" property contains an alternate title for the tooltip
   * instead of discovering the title from the shortcut manager.
   *
   * Since: 3.32
   */
  properties [PROP_TITLE] =
    g_param_spec_string ("title",
                         "title",
                         "Title for the tooltip",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_WIDGET] =
    g_param_spec_object ("widget",
                         "Widget",
                         "The widget to monitor for query-tooltip",
                         GTK_TYPE_WIDGET,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_shortcut_tooltip_init (DzlShortcutTooltip *self)
{
}

/**
 * dzl_shortcut_tooltip_get_title:
 *
 * Gets the #DzlShortcutTooltip:title property, if set.
 *
 * Returns: (nullable): a string containing the title, or %NULL
 *
 * Since: 3.32
 */
const gchar *
dzl_shortcut_tooltip_get_title (DzlShortcutTooltip *self)
{
  g_return_val_if_fail (DZL_IS_SHORTCUT_TOOLTIP (self), NULL);

  return self->title;
}

/**
 * dzl_shortcut_tooltip_set_title:
 * @self: a #DzlShortcutTooltip
 * @title: (nullable): a title for the tooltip, or %NULL
 *
 * Sets the #DzlShortcutTooltip:title property, which can be used to
 * override the default title for the tooltip as discovered from the
 * shortcut manager.
 *
 * Since: 3.32
 */
void
dzl_shortcut_tooltip_set_title (DzlShortcutTooltip *self,
                                const gchar        *title)
{
  g_return_if_fail (DZL_IS_SHORTCUT_TOOLTIP (self));

  if (!dzl_str_equal0 (title, self->title))
    {
      g_free (self->title);
      self->title = g_strdup (title);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_TITLE]);
    }
}

/**
 * dzl_shortcut_tooltip_get_command_id:
 * @self: a #DzlShortcutTooltip
 *
 * Gets the #DzlShortcutTooltip:command-id property.
 *
 * Returns: (nullable): a string containing the command id
 *
 * Since: 3.32
 */
const gchar *
dzl_shortcut_tooltip_get_command_id (DzlShortcutTooltip *self)
{
  g_return_val_if_fail (DZL_IS_SHORTCUT_TOOLTIP (self), NULL);

  return self->command_id;
}

/**
 * dzl_shortcut_tooltip_set_command_id:
 * @self: a #DzlShortcutTooltip
 * @command_id: the command-id of the shortcut registered
 *
 * This sets the #DzlShortcutTooltip:command-id property which denotes which
 * shortcut registered with libdazzle to display when a tooltip request is
 * received.
 *
 * Since: 3.32
 */
void
dzl_shortcut_tooltip_set_command_id (DzlShortcutTooltip *self,
                                     const gchar        *command_id)
{
  g_return_if_fail (DZL_IS_SHORTCUT_TOOLTIP (self));

  command_id = g_intern_string (command_id);

  if (command_id != self->command_id)
    {
      self->command_id = command_id;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_COMMAND_ID]);
    }
}

/**
 * dzl_shortcut_tooltip_get_widget:
 *
 * Gets the #GtkWidget that the shortcut-tooltip is wrapping.
 *
 * Returns: (nullable) (transfer none): a #GtkWidget or %NULL if unset
 *
 * Since: 3.32
 */
GtkWidget *
dzl_shortcut_tooltip_get_widget (DzlShortcutTooltip *self)
{
  g_return_val_if_fail (DZL_IS_SHORTCUT_TOOLTIP (self), NULL);

  return self->widget;
}

/**
 * dzl_shortcut_tooltip_set_widget:
 * @self: a #DzlShortcutTooltip
 * @widget: (nullable): a #GtkWidget or %NULL
 *
 * Sets the widget to connect to the #GtkWidget::query-tooltip signal.
 *
 * If configured, the widget will be displayed with an appropriate tooltip
 * message matching the shortcut from #DzlShortcutTooltip:command-id.
 *
 * Since: 3.32
 */
void
dzl_shortcut_tooltip_set_widget (DzlShortcutTooltip *self,
                                 GtkWidget          *widget)
{
  g_return_if_fail (DZL_IS_SHORTCUT_TOOLTIP (self));

  if (widget != self->widget)
    {
      if (self->widget != NULL)
        {
          gtk_widget_set_has_tooltip (self->widget, FALSE);
          dzl_clear_signal_handler (self->widget, &self->query_handler);
          dzl_clear_signal_handler (self->widget, &self->destroy_handler);
          self->widget = NULL;
        }

      if (widget != NULL)
        {
          self->widget = widget;
          gtk_widget_set_has_tooltip (self->widget, TRUE);
          self->query_handler =
            g_signal_connect_object (self->widget,
                                     "query-tooltip",
                                     G_CALLBACK (dzl_shortcut_tooltip_query_cb),
                                     self,
                                     G_CONNECT_SWAPPED | G_CONNECT_AFTER);
          self->destroy_handler =
            g_signal_connect (self->widget,
                              "destroy",
                              G_CALLBACK (gtk_widget_destroyed),
                              &self->widget);
        }

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_WIDGET]);
    }
}

/**
 * dzl_shortcut_tooltip_get_accel:
 * @self: a #DzlShortcutTooltip
 *
 * Gets the #DzlShortcutTooltip:accel property, which can be used to override
 * the commands accel.
 *
 * Returns: (nullable): an override accel, or %NULL
 *
 * Since: 3.32
 */
const gchar *
dzl_shortcut_tooltip_get_accel (DzlShortcutTooltip *self)
{
  g_return_val_if_fail (DZL_IS_SHORTCUT_TOOLTIP (self), NULL);

  return self->accel;
}

/**
 * dzl_shortcut_tooltip_set_accel:
 * @self: #DzlShortcutTooltip
 * @accel: (nullable): Sets the accelerator to use, or %NULL to unset
 *   and use the default
 *
 * Allows overriding the accel that is used.
 *
 * Since: 3.32
 */
void
dzl_shortcut_tooltip_set_accel (DzlShortcutTooltip *self,
                                const gchar        *accel)
{
  g_return_if_fail (DZL_IS_SHORTCUT_TOOLTIP (self));

  if (!dzl_str_equal0 (self->accel, accel))
    {
      g_free (self->accel);
      self->accel = g_strdup (accel);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_ACCEL]);
    }
}
