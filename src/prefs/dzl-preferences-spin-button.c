/* dzl-preferences-spin-button.c
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
#include "prefs/dzl-preferences-spin-button.h"

struct _DzlPreferencesSpinButton
{
  DzlPreferencesBin        parent_instance;

  gulong                   handler;

  guint                    updating : 1;

  gchar                   *key;
  GSettings               *settings;

  const GVariantType      *type;

  GtkSpinButton           *spin_button;
  GtkLabel                *title;
  GtkLabel                *subtitle;
};

G_DEFINE_TYPE (DzlPreferencesSpinButton, dzl_preferences_spin_button, DZL_TYPE_PREFERENCES_BIN)

enum {
  PROP_0,
  PROP_KEY,
  PROP_SUBTITLE,
  PROP_TITLE,
  LAST_PROP
};

enum {
  ACTIVATE,
  LAST_SIGNAL
};

static GParamSpec *properties [LAST_PROP];
static guint signals [LAST_SIGNAL];

static void
dzl_preferences_spin_button_activate (DzlPreferencesSpinButton *self)
{
  g_assert (DZL_IS_PREFERENCES_SPIN_BUTTON (self));

  gtk_widget_grab_focus (GTK_WIDGET (self->spin_button));
}

static void
apply_value (GtkAdjustment *adj,
             GVariant      *value,
             const gchar   *property)
{
  GValue val = { 0 };
  gdouble v = 0.0;

  g_assert (GTK_IS_ADJUSTMENT (adj));
  g_assert (value != NULL);
  g_assert (property != NULL);

  if (g_variant_is_of_type (value, G_VARIANT_TYPE_DOUBLE))
    v = g_variant_get_double (value);

  else if (g_variant_is_of_type (value, G_VARIANT_TYPE_INT16))
    v = g_variant_get_int16 (value);
  else if (g_variant_is_of_type (value, G_VARIANT_TYPE_UINT16))
    v = g_variant_get_uint16 (value);

  else if (g_variant_is_of_type (value, G_VARIANT_TYPE_INT32))
    v = g_variant_get_int32 (value);
  else if (g_variant_is_of_type (value, G_VARIANT_TYPE_UINT32))
    v = g_variant_get_uint32 (value);

  else if (g_variant_is_of_type (value, G_VARIANT_TYPE_INT64))
    v = g_variant_get_int64 (value);
  else if (g_variant_is_of_type (value, G_VARIANT_TYPE_UINT64))
    v = g_variant_get_uint64 (value);

  else
    g_warning ("Unknown variant type: %s\n", (gchar *)g_variant_get_type (value));

  g_value_init (&val, G_TYPE_DOUBLE);
  g_value_set_double (&val, v);
  g_object_set_property (G_OBJECT (adj), property, &val);
  g_value_unset (&val);
}

static void
dzl_preferences_spin_button_value_changed (DzlPreferencesSpinButton *self,
                                           GParamSpec               *pspec,
                                           GtkSpinButton            *spin_button)
{
  GVariant *variant = NULL;
  gdouble value;

  g_assert (DZL_IS_PREFERENCES_SPIN_BUTTON (self));
  g_assert (pspec != NULL);
  g_assert (GTK_IS_SPIN_BUTTON (spin_button));

  value = gtk_spin_button_get_value (spin_button);

  if (g_variant_type_equal (self->type, G_VARIANT_TYPE_DOUBLE))
    variant = g_variant_new_double (value);
  else if (g_variant_type_equal (self->type, G_VARIANT_TYPE_INT16))
    variant = g_variant_new_int16 (value);
  else if (g_variant_type_equal (self->type, G_VARIANT_TYPE_UINT16))
    variant = g_variant_new_uint16 (value);
  else if (g_variant_type_equal (self->type, G_VARIANT_TYPE_INT32))
    variant = g_variant_new_int32 (value);
  else if (g_variant_type_equal (self->type, G_VARIANT_TYPE_UINT32))
    variant = g_variant_new_uint32 (value);
  else if (g_variant_type_equal (self->type, G_VARIANT_TYPE_INT64))
    variant = g_variant_new_int64 (value);
  else if (g_variant_type_equal (self->type, G_VARIANT_TYPE_UINT64))
    variant = g_variant_new_uint64 (value);
  else
    g_return_if_reached ();

  g_variant_ref_sink (variant);
  g_settings_set_value (self->settings, self->key, variant);
  g_clear_pointer (&variant, g_variant_unref);
}

static void
dzl_preferences_spin_button_setting_changed (DzlPreferencesSpinButton *self,
                                             const gchar              *key,
                                             GSettings                *settings)
{
  GtkAdjustment *adj;
  GVariant *value;

  g_assert (DZL_IS_PREFERENCES_SPIN_BUTTON (self));
  g_assert (key != NULL);
  g_assert (G_IS_SETTINGS (settings));

  if (self->updating)
    return;

  self->updating = TRUE;

  adj = gtk_spin_button_get_adjustment (self->spin_button);

  value = g_settings_get_value (settings, key);
  apply_value (adj, value, "value");
  g_variant_unref (value);

  self->updating = FALSE;
}

static void
dzl_preferences_spin_button_connect (DzlPreferencesBin *bin,
                                     GSettings         *settings)
{
  DzlPreferencesSpinButton *self = (DzlPreferencesSpinButton *)bin;
  GSettingsSchema *schema = NULL;
  GSettingsSchemaKey *key = NULL;
  GVariant *range = NULL;
  GVariant *values = NULL;
  GVariant *lower = NULL;
  GVariant *upper = NULL;
  gchar *type = NULL;
  gchar *signal_detail = NULL;
  GtkAdjustment *adj;
  GVariantIter iter;

  g_assert (DZL_IS_PREFERENCES_SPIN_BUTTON (self));

  self->settings = g_object_ref (settings);

  g_object_get (self->settings, "settings-schema", &schema, NULL);

  adj = gtk_spin_button_get_adjustment (self->spin_button);
  key = g_settings_schema_get_key (schema, self->key);
  range = g_settings_schema_key_get_range (key);

  g_variant_get (range, "(sv)", &type, &values);

  if (!dzl_str_equal0 (type, "range") || (2 != g_variant_iter_init (&iter, values)))
    {
      gtk_widget_set_sensitive (GTK_WIDGET (self), FALSE);
      goto cleanup;
    }

  lower = g_variant_iter_next_value (&iter);
  upper = g_variant_iter_next_value (&iter);

  self->type = g_variant_get_type (lower);

  apply_value (adj, lower, "lower");
  apply_value (adj, upper, "upper");

  signal_detail = g_strdup_printf ("changed::%s", self->key);

  self->handler =
    g_signal_connect_object (self->settings,
                             signal_detail,
                             G_CALLBACK (dzl_preferences_spin_button_setting_changed),
                             self,
                             G_CONNECT_SWAPPED);

  dzl_preferences_spin_button_setting_changed (self, self->key, self->settings);

cleanup:
  g_clear_pointer (&key, g_settings_schema_key_unref);
  g_clear_pointer (&type, g_free);
  g_clear_pointer (&signal_detail, g_free);
  g_clear_pointer (&range, g_variant_unref);
  g_clear_pointer (&values, g_variant_unref);
  g_clear_pointer (&lower, g_variant_unref);
  g_clear_pointer (&upper, g_variant_unref);
  g_clear_pointer (&schema, g_settings_schema_unref);
}

static void
dzl_preferences_spin_button_disconnect (DzlPreferencesBin *bin,
                                        GSettings         *settings)
{
  DzlPreferencesSpinButton *self = (DzlPreferencesSpinButton *)bin;

  g_assert (DZL_IS_PREFERENCES_SPIN_BUTTON (self));

  g_signal_handler_disconnect (settings, self->handler);
  self->handler = 0;
}

static gboolean
dzl_preferences_spin_button_matches (DzlPreferencesBin *bin,
                                     DzlPatternSpec    *spec)
{
  DzlPreferencesSpinButton *self = (DzlPreferencesSpinButton *)bin;
  const gchar *tmp;

  g_assert (DZL_IS_PREFERENCES_SPIN_BUTTON (self));
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
dzl_preferences_spin_button_finalize (GObject *object)
{
  DzlPreferencesSpinButton *self = (DzlPreferencesSpinButton *)object;

  g_clear_pointer (&self->key, g_free);
  g_clear_object (&self->settings);

  G_OBJECT_CLASS (dzl_preferences_spin_button_parent_class)->finalize (object);
}

static void
dzl_preferences_spin_button_get_property (GObject    *object,
                                          guint       prop_id,
                                          GValue     *value,
                                          GParamSpec *pspec)
{
  DzlPreferencesSpinButton *self = DZL_PREFERENCES_SPIN_BUTTON (object);

  switch (prop_id)
    {
    case PROP_KEY:
      g_value_set_string (value, self->key);
      break;

    case PROP_SUBTITLE:
      g_value_set_string (value, gtk_label_get_label (self->subtitle));
      break;

    case PROP_TITLE:
      g_value_set_string (value, gtk_label_get_label (self->title));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_spin_button_set_property (GObject      *object,
                                          guint         prop_id,
                                          const GValue *value,
                                          GParamSpec   *pspec)
{
  DzlPreferencesSpinButton *self = DZL_PREFERENCES_SPIN_BUTTON (object);

  switch (prop_id)
    {
    case PROP_KEY:
      self->key = g_value_dup_string (value);
      break;

    case PROP_SUBTITLE:
      gtk_label_set_label (self->subtitle, g_value_get_string (value));
      break;

    case PROP_TITLE:
      gtk_label_set_label (self->title, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_spin_button_class_init (DzlPreferencesSpinButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  DzlPreferencesBinClass *bin_class = DZL_PREFERENCES_BIN_CLASS (klass);

  object_class->finalize = dzl_preferences_spin_button_finalize;
  object_class->get_property = dzl_preferences_spin_button_get_property;
  object_class->set_property = dzl_preferences_spin_button_set_property;

  bin_class->connect = dzl_preferences_spin_button_connect;
  bin_class->disconnect = dzl_preferences_spin_button_disconnect;
  bin_class->matches = dzl_preferences_spin_button_matches;

  signals [ACTIVATE] =
    g_signal_new_class_handler ("activate",
                                G_TYPE_FROM_CLASS (klass),
                                G_SIGNAL_RUN_LAST,
                                G_CALLBACK (dzl_preferences_spin_button_activate),
                                NULL, NULL, NULL, G_TYPE_NONE, 0);

  widget_class->activate_signal = signals [ACTIVATE];

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/dazzle/ui/dzl-preferences-spin-button.ui");
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesSpinButton, spin_button);
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesSpinButton, subtitle);
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesSpinButton, title);

  properties [PROP_KEY] =
    g_param_spec_string ("key",
                         "Key",
                         "Key",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_SUBTITLE] =
    g_param_spec_string ("subtitle",
                         "subtitle",
                         "subtitle",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_TITLE] =
    g_param_spec_string ("title",
                         "title",
                         "title",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
dzl_preferences_spin_button_init (DzlPreferencesSpinButton *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_object_set (gtk_spin_button_get_adjustment (self->spin_button),
                "value", 0.0,
                "lower", 0.0,
                "upper", 0.0,
                "step-increment", 1.0,
                "page-increment", 10.0,
                "page-size", 10.0,
                NULL);

  g_signal_connect_object (self->spin_button,
                           "notify::value",
                           G_CALLBACK (dzl_preferences_spin_button_value_changed),
                           self,
                           G_CONNECT_SWAPPED);
}

/**
 * dzl_preferences_spin_button_get_spin_button:
 *
 * Returns: (transfer none): The actual spin button widget.
 */
GtkWidget *
dzl_preferences_spin_button_get_spin_button (DzlPreferencesSpinButton *self)
{
  g_return_val_if_fail (DZL_IS_PREFERENCES_SPIN_BUTTON (self), NULL);

  return GTK_WIDGET (self->spin_button);
}
