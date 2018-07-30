/* dzl-preferences-switch.c
 *
 * Copyright (C) 2015-2017 Christian Hergert <chergert@redhat.com>
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

#include "config.h"

#include "util/dzl-util-private.h"
#include "prefs/dzl-preferences-switch.h"

struct _DzlPreferencesSwitch
{
  DzlPreferencesBin parent_instance;

  guint     is_radio : 1;
  guint     updating : 1;

  gulong    handler;

  gchar     *key;
  GVariant  *target;
  GSettings *settings;

  GtkLabel  *subtitle;
  GtkLabel  *title;
  GtkSwitch *widget;
  GtkImage  *image;
};

G_DEFINE_TYPE (DzlPreferencesSwitch, dzl_preferences_switch, DZL_TYPE_PREFERENCES_BIN)

enum {
  PROP_0,
  PROP_IS_RADIO,
  PROP_KEY,
  PROP_SUBTITLE,
  PROP_TARGET,
  PROP_TITLE,
  LAST_PROP
};

enum {
  ACTIVATED,
  LAST_SIGNAL
};

static GParamSpec *properties [LAST_PROP];
static guint signals [LAST_SIGNAL];

static void
dzl_preferences_switch_changed (DzlPreferencesSwitch *self,
                                const gchar          *key,
                                GSettings            *settings)
{
  GVariant *value;
  gboolean active = FALSE;

  g_assert (DZL_IS_PREFERENCES_SWITCH (self));
  g_assert (key != NULL);
  g_assert (G_IS_SETTINGS (settings));

  if (self->updating == TRUE)
    return;

  value = g_settings_get_value (settings, key);

  if (g_variant_is_of_type (value, G_VARIANT_TYPE_BOOLEAN))
    active = g_variant_get_boolean (value);
  else if ((self->target != NULL) &&
           g_variant_is_of_type (value, g_variant_get_type (self->target)))
    active = g_variant_equal (value, self->target);
  else if ((self->target != NULL) &&
           g_variant_is_of_type (self->target, G_VARIANT_TYPE_STRING) &&
           g_variant_is_of_type (value, G_VARIANT_TYPE_STRING_ARRAY))
    {
      g_autofree const gchar **strv = g_variant_get_strv (value, NULL);
      const gchar *flag = g_variant_get_string (self->target, NULL);
      active = g_strv_contains (strv, flag);
    }

  self->updating = TRUE;

  if (self->is_radio)
    {
      gtk_widget_set_visible (GTK_WIDGET (self->image), active);
    }
  else
    {
      gtk_switch_set_active (self->widget, active);
      gtk_switch_set_state (self->widget, active);
    }

  self->updating = FALSE;

  g_variant_unref (value);
}

static void
dzl_preferences_switch_connect (DzlPreferencesBin *bin,
                                GSettings         *settings)
{
  DzlPreferencesSwitch *self = (DzlPreferencesSwitch *)bin;
  g_autofree gchar *signal_detail = NULL;

  g_assert (DZL_IS_PREFERENCES_SWITCH (self));

  signal_detail = g_strdup_printf ("changed::%s", self->key);

  self->settings = g_object_ref (settings);

  self->handler =
    g_signal_connect_object (settings,
                             signal_detail,
                             G_CALLBACK (dzl_preferences_switch_changed),
                             self,
                             G_CONNECT_SWAPPED);

  dzl_preferences_switch_changed (self, self->key, settings);
}

static void
dzl_preferences_switch_disconnect (DzlPreferencesBin *bin,
                                   GSettings         *settings)
{
  DzlPreferencesSwitch *self = (DzlPreferencesSwitch *)bin;

  g_assert (DZL_IS_PREFERENCES_SWITCH (self));

  g_signal_handler_disconnect (settings, self->handler);
  self->handler = 0;
}

static void
dzl_preferences_switch_toggle (DzlPreferencesSwitch *self,
                               gboolean              state)
{
  GVariant *value;

  g_assert (DZL_IS_PREFERENCES_SWITCH (self));

  if (self->updating)
    return;

  self->updating = TRUE;

  value = g_settings_get_value (self->settings, self->key);

  if (g_variant_is_of_type (value, G_VARIANT_TYPE_BOOLEAN))
    {
      g_settings_set_boolean (self->settings, self->key, state);
    }
  else if ((self->target != NULL) &&
           g_variant_is_of_type (self->target, G_VARIANT_TYPE_STRING) &&
           g_variant_is_of_type (value, G_VARIANT_TYPE_STRING_ARRAY))
    {
      g_autofree const gchar **strv = g_variant_get_strv (value, NULL);
      g_autoptr(GPtrArray) ar = g_ptr_array_new ();
      const gchar *flag = g_variant_get_string (self->target, NULL);
      gboolean found = FALSE;
      gint i;

      for (i = 0; strv [i]; i++)
        {
          if (!state && dzl_str_equal0 (strv [i], flag))
            continue;
          if (dzl_str_equal0 (strv [i], flag))
            found = TRUE;
          g_ptr_array_add (ar, (gchar *)strv [i]);
        }

      if (state && !found)
        g_ptr_array_add (ar, (gchar *)flag);

      g_ptr_array_add (ar, NULL);

      g_settings_set_strv (self->settings, self->key, (const gchar * const *)ar->pdata);
    }
  else if ((self->target != NULL) &&
           g_variant_is_of_type (value, g_variant_get_type (self->target)))
    {
      g_settings_set_value (self->settings, self->key, self->target);
    }
  else
    {
      g_warning ("I don't know how to set a variant of type %s to %s",
                 (const gchar *)g_variant_get_type (value),
                 self->target ? (const gchar *)g_variant_get_type (self->target) : "(nil)");
    }


  g_variant_unref (value);

  if (self->is_radio)
    gtk_widget_set_visible (GTK_WIDGET (self->image), state);
  else
    gtk_switch_set_state (self->widget, state);

  self->updating = FALSE;

  /* For good measure, so that we cleanup in the boolean deselection case */
  dzl_preferences_switch_changed (self, self->key, self->settings);
}

static gboolean
dzl_preferences_switch_state_set (DzlPreferencesSwitch *self,
                                  gboolean              state,
                                  GtkSwitch            *widget)
{
  g_assert (DZL_IS_PREFERENCES_SWITCH (self));
  g_assert (GTK_IS_SWITCH (widget));

  dzl_preferences_switch_toggle (self, state);

  return TRUE;
}

static void
dzl_preferences_switch_activate (DzlPreferencesSwitch *self)
{
  g_assert (DZL_IS_PREFERENCES_SWITCH (self));

  if (!gtk_widget_get_sensitive (GTK_WIDGET (self)) || (self->settings == NULL))
    return;

  if (self->is_radio)
    {
      gboolean state;

      state = !gtk_widget_get_visible (GTK_WIDGET (self->image));
      dzl_preferences_switch_toggle (self, state);
    }
  else
    gtk_widget_activate (GTK_WIDGET (self->widget));

}

static gboolean
dzl_preferences_switch_matches (DzlPreferencesBin *bin,
                                DzlPatternSpec    *spec)
{
  DzlPreferencesSwitch *self = (DzlPreferencesSwitch *)bin;
  const gchar *tmp;

  g_assert (DZL_IS_PREFERENCES_SWITCH (self));
  g_assert (spec != NULL);

  tmp = gtk_label_get_label (self->title);
  if (tmp && dzl_pattern_spec_match (spec, tmp))
    return TRUE;

  tmp = gtk_label_get_label (self->subtitle);
  if (tmp && dzl_pattern_spec_match (spec, tmp))
    return TRUE;

  if (self->key && dzl_pattern_spec_match (spec, self->key))
    return TRUE;

  return FALSE;
}

static void
dzl_preferences_switch_finalize (GObject *object)
{
  DzlPreferencesSwitch *self = (DzlPreferencesSwitch *)object;

  g_clear_pointer (&self->key, g_free);
  g_clear_pointer (&self->target, g_variant_unref);
  g_clear_object (&self->settings);

  G_OBJECT_CLASS (dzl_preferences_switch_parent_class)->finalize (object);
}

static void
dzl_preferences_switch_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  DzlPreferencesSwitch *self = DZL_PREFERENCES_SWITCH (object);

  switch (prop_id)
    {
    case PROP_IS_RADIO:
      g_value_set_boolean (value, self->is_radio);
      break;

    case PROP_KEY:
      g_value_set_string (value, self->key);
      break;

    case PROP_TARGET:
      g_value_set_variant (value, self->target);
      break;

    case PROP_TITLE:
      g_value_set_string (value, gtk_label_get_label (self->title));
      break;

    case PROP_SUBTITLE:
      g_value_set_string (value, gtk_label_get_label (self->subtitle));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_switch_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  DzlPreferencesSwitch *self = DZL_PREFERENCES_SWITCH (object);

  switch (prop_id)
    {
    case PROP_IS_RADIO:
      self->is_radio = g_value_get_boolean (value);
      gtk_widget_set_visible (GTK_WIDGET (self->widget), !self->is_radio);
      gtk_widget_set_visible (GTK_WIDGET (self->image), self->is_radio);
      break;

    case PROP_KEY:
      self->key = g_value_dup_string (value);
      break;

    case PROP_TARGET:
      self->target = g_value_dup_variant (value);
      break;

    case PROP_TITLE:
      gtk_label_set_label (self->title, g_value_get_string (value));
      break;

    case PROP_SUBTITLE:
      g_object_set (self->subtitle,
                    "label", g_value_get_string (value),
                    "visible", !!g_value_get_string (value),
                    NULL);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_switch_class_init (DzlPreferencesSwitchClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  DzlPreferencesBinClass *bin_class = DZL_PREFERENCES_BIN_CLASS (klass);

  object_class->finalize = dzl_preferences_switch_finalize;
  object_class->get_property = dzl_preferences_switch_get_property;
  object_class->set_property = dzl_preferences_switch_set_property;

  bin_class->connect = dzl_preferences_switch_connect;
  bin_class->disconnect = dzl_preferences_switch_disconnect;
  bin_class->matches = dzl_preferences_switch_matches;

  signals [ACTIVATED] =
    g_signal_new_class_handler ("activated",
                                G_TYPE_FROM_CLASS (klass),
                                G_SIGNAL_RUN_LAST,
                                G_CALLBACK (dzl_preferences_switch_activate),
                                NULL, NULL, NULL, G_TYPE_NONE, 0);

  widget_class->activate_signal = signals [ACTIVATED];

  properties [PROP_IS_RADIO] =
    g_param_spec_boolean ("is-radio",
                         "Is Radio",
                         "If a radio style should be used instead of a switch.",
                         FALSE,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));


  properties [PROP_TARGET] =
    g_param_spec_variant ("target",
                          "Target",
                          "Target",
                          G_VARIANT_TYPE_ANY,
                          NULL,
                          (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_KEY] =
    g_param_spec_string ("key",
                         "Key",
                         "Key",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "Title",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_SUBTITLE] =
    g_param_spec_string ("subtitle",
                         "Subtitle",
                         "Subtitle",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/dazzle/ui/dzl-preferences-switch.ui");
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesSwitch, image);
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesSwitch, subtitle);
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesSwitch, title);
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesSwitch, widget);
}

static void
dzl_preferences_switch_init (DzlPreferencesSwitch *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect_object (self->widget,
                           "state-set",
                           G_CALLBACK (dzl_preferences_switch_state_set),
                           self,
                           G_CONNECT_SWAPPED);
}
