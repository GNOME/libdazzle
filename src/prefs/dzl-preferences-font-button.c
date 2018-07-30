/* dzl-preferences-font-button.c
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

#define G_LOG_DOMAIN "dzl-preferences-font-button"

#include "config.h"

#include "prefs/dzl-preferences-font-button.h"
#include "util/dzl-macros.h"

struct _DzlPreferencesFontButton
{
  GtkBin                parent_instance;

  gulong                handler;

  GSettings            *settings;
  gchar                *key;

  GtkLabel             *title;
  GtkLabel             *font_family;
  GtkLabel             *font_size;
  GtkPopover           *popover;
  GtkButton            *confirm;
  GtkFontChooserWidget *chooser;
};

G_DEFINE_TYPE (DzlPreferencesFontButton, dzl_preferences_font_button, DZL_TYPE_PREFERENCES_BIN)

enum {
  PROP_0,
  PROP_KEY,
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
dzl_preferences_font_button_show (DzlPreferencesFontButton *self)
{
  gchar *font = NULL;

  g_assert (DZL_IS_PREFERENCES_FONT_BUTTON (self));

  font = g_settings_get_string (self->settings, self->key);
  g_object_set (self->chooser, "font", font, NULL);
  g_free (font);

  gtk_popover_popup (self->popover);
}

static void
dzl_preferences_font_button_activate (DzlPreferencesFontButton *self)
{
  g_assert (DZL_IS_PREFERENCES_FONT_BUTTON (self));

  if (!gtk_widget_get_visible (GTK_WIDGET (self->popover)))
    dzl_preferences_font_button_show (self);
}

static void
dzl_preferences_font_button_changed (DzlPreferencesFontButton *self,
                                     const gchar              *key,
                                     GSettings                *settings)
{
  PangoFontDescription *font_desc;
  gchar *name;

  g_assert (DZL_IS_PREFERENCES_FONT_BUTTON (self));
  g_assert (key != NULL);
  g_assert (G_IS_SETTINGS (settings));

  name = g_settings_get_string (settings, key);
  font_desc = pango_font_description_from_string (name);

  if (font_desc != NULL)
    {
      gchar *font_size;

      gtk_label_set_label (self->font_family, pango_font_description_get_family (font_desc));
      font_size = g_strdup_printf ("%d", pango_font_description_get_size (font_desc) / PANGO_SCALE);
      gtk_label_set_label (self->font_size, font_size);
      g_free (font_size);
    }

  g_clear_pointer (&font_desc, pango_font_description_free);
  g_free (name);
}

static void
dzl_preferences_font_button_connect (DzlPreferencesBin *bin,
                                     GSettings         *settings)
{
  DzlPreferencesFontButton *self = (DzlPreferencesFontButton *)bin;
  g_autofree gchar *signal_detail = NULL;

  g_assert (DZL_IS_PREFERENCES_FONT_BUTTON (self));

  signal_detail = g_strdup_printf ("changed::%s", self->key);

  self->settings = g_object_ref (settings);

  self->handler =
    g_signal_connect_object (settings,
                             signal_detail,
                             G_CALLBACK (dzl_preferences_font_button_changed),
                             self,
                             G_CONNECT_SWAPPED);

  dzl_preferences_font_button_changed (self, self->key, settings);
}

static void
dzl_preferences_font_button_disconnect (DzlPreferencesBin *bin,
                                        GSettings         *settings)
{
  DzlPreferencesFontButton *self = (DzlPreferencesFontButton *)bin;

  g_assert (DZL_IS_PREFERENCES_FONT_BUTTON (self));

  g_signal_handler_disconnect (settings, self->handler);
  self->handler = 0;
}

static gboolean
dzl_preferences_font_button_matches (DzlPreferencesBin *bin,
                                     DzlPatternSpec    *spec)
{
  DzlPreferencesFontButton *self = (DzlPreferencesFontButton *)bin;
  const gchar *tmp;

  g_assert (DZL_IS_PREFERENCES_FONT_BUTTON (self));
  g_assert (spec != NULL);

  tmp = gtk_label_get_label (self->title);
  if (tmp && dzl_pattern_spec_match (spec, tmp))
    return TRUE;

  tmp = gtk_label_get_label (self->font_family);
  if (tmp && dzl_pattern_spec_match (spec, tmp))
    return TRUE;

  return FALSE;
}

static void
dzl_preferences_font_button_finalize (GObject *object)
{
  DzlPreferencesFontButton *self = (DzlPreferencesFontButton *)object;

  g_clear_object (&self->settings);
  g_clear_pointer (&self->key, g_free);

  G_OBJECT_CLASS (dzl_preferences_font_button_parent_class)->finalize (object);
}

static void
dzl_preferences_font_button_get_property (GObject    *object,
                                          guint       prop_id,
                                          GValue     *value,
                                          GParamSpec *pspec)
{
  DzlPreferencesFontButton *self = DZL_PREFERENCES_FONT_BUTTON (object);

  switch (prop_id)
    {
    case PROP_KEY:
      g_value_set_string (value, self->key);
      break;

    case PROP_TITLE:
      g_value_set_string (value, gtk_label_get_label (self->title));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_font_button_set_property (GObject      *object,
                                          guint         prop_id,
                                          const GValue *value,
                                          GParamSpec   *pspec)
{
  DzlPreferencesFontButton *self = DZL_PREFERENCES_FONT_BUTTON (object);

  switch (prop_id)
    {
    case PROP_KEY:
      self->key = g_value_dup_string (value);
      break;

    case PROP_TITLE:
      gtk_label_set_label (self->title, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_font_button_class_init (DzlPreferencesFontButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  DzlPreferencesBinClass *bin_class = DZL_PREFERENCES_BIN_CLASS (klass);

  object_class->finalize = dzl_preferences_font_button_finalize;
  object_class->get_property = dzl_preferences_font_button_get_property;
  object_class->set_property = dzl_preferences_font_button_set_property;

  bin_class->connect = dzl_preferences_font_button_connect;
  bin_class->disconnect = dzl_preferences_font_button_disconnect;
  bin_class->matches = dzl_preferences_font_button_matches;

  signals [ACTIVATE] =
    g_signal_new_class_handler ("activate",
                                G_TYPE_FROM_CLASS (klass),
                                G_SIGNAL_RUN_LAST,
                                G_CALLBACK (dzl_preferences_font_button_activate),
                                NULL, NULL, NULL, G_TYPE_NONE, 0);

  widget_class->activate_signal = signals [ACTIVATE];

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
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/dazzle/ui/dzl-preferences-font-button.ui");
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesFontButton, chooser);
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesFontButton, confirm);
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesFontButton, font_family);
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesFontButton, font_size);
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesFontButton, popover);
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesFontButton, title);
}

static gboolean
transform_to (GBinding     *binding,
              const GValue *value,
              GValue       *to_value,
              gpointer      user_data)
{
  g_value_set_boolean (to_value, !!g_value_get_boxed (value));
  return TRUE;
}

static void
dzl_preferences_font_button_clicked (DzlPreferencesFontButton *self,
                                     GtkButton                *button)
{
  g_autofree gchar *font = NULL;

  g_assert (DZL_IS_PREFERENCES_FONT_BUTTON (self));
  g_assert (GTK_IS_BUTTON (button));

  g_object_get (self->chooser, "font", &font, NULL);
  g_settings_set_string (self->settings, self->key, font);
  gtk_popover_popdown (self->popover);
}

static void
dzl_preferences_font_button_font_activated (DzlPreferencesFontButton *self,
                                            const gchar              *font,
                                            GtkFontChooser           *chooser)
{
  g_assert (DZL_IS_PREFERENCES_FONT_BUTTON (self));
  g_assert (GTK_IS_FONT_CHOOSER (chooser));

  g_settings_set_string (self->settings, self->key, font);
  gtk_popover_popdown (self->popover);
}

static void
dzl_preferences_font_button_init (DzlPreferencesFontButton *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  g_object_bind_property_full (self->chooser, "font-desc",
                               self->confirm, "sensitive",
                               G_BINDING_SYNC_CREATE,
                               transform_to,
                               NULL, NULL, NULL);

  g_signal_connect_object (self->chooser,
                           "font-activated",
                           G_CALLBACK (dzl_preferences_font_button_font_activated),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (self->confirm,
                           "clicked",
                           G_CALLBACK (dzl_preferences_font_button_clicked),
                           self,
                           G_CONNECT_SWAPPED);
}
