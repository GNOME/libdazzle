/* dzl-shortcut-label.c
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

#define G_LOG_DOMAIN "dzl-shortcut-label"

#include "config.h"

#include "shortcuts/dzl-shortcut-label.h"
#include "util/dzl-macros.h"

struct _DzlShortcutLabel
{
  GtkBox            parent_instance;
  DzlShortcutChord *chord;
};

enum {
  PROP_0,
  PROP_ACCELERATOR,
  PROP_CHORD,
  N_PROPS
};

G_DEFINE_TYPE (DzlShortcutLabel, dzl_shortcut_label, GTK_TYPE_BOX)

static GParamSpec *properties [N_PROPS];

static void
dzl_shortcut_label_finalize (GObject *object)
{
  DzlShortcutLabel *self = (DzlShortcutLabel *)object;

  g_clear_pointer (&self->chord, dzl_shortcut_chord_free);

  G_OBJECT_CLASS (dzl_shortcut_label_parent_class)->finalize (object);
}

static void
dzl_shortcut_label_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  DzlShortcutLabel *self = DZL_SHORTCUT_LABEL (object);

  switch (prop_id)
    {
    case PROP_ACCELERATOR:
      g_value_take_string (value, dzl_shortcut_label_get_accelerator (self));
      break;

    case PROP_CHORD:
      g_value_set_boxed (value, dzl_shortcut_label_get_chord (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_label_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  DzlShortcutLabel *self = DZL_SHORTCUT_LABEL (object);

  switch (prop_id)
    {
    case PROP_ACCELERATOR:
      dzl_shortcut_label_set_accelerator (self, g_value_get_string (value));
      break;

    case PROP_CHORD:
      dzl_shortcut_label_set_chord (self, g_value_get_boxed (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_label_class_init (DzlShortcutLabelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dzl_shortcut_label_finalize;
  object_class->get_property = dzl_shortcut_label_get_property;
  object_class->set_property = dzl_shortcut_label_set_property;

  properties [PROP_ACCELERATOR] =
    g_param_spec_string ("accelerator",
                         "Accelerator",
                         "The accelerator for the label",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_CHORD] =
    g_param_spec_boxed ("chord",
                         "Chord",
                         "The chord for the label",
                         DZL_TYPE_SHORTCUT_CHORD,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_shortcut_label_init (DzlShortcutLabel *self)
{
  gtk_box_set_spacing (GTK_BOX (self), 12);
}

GtkWidget *
dzl_shortcut_label_new (void)
{
  return g_object_new (DZL_TYPE_SHORTCUT_LABEL, NULL);
}

gchar *
dzl_shortcut_label_get_accelerator (DzlShortcutLabel *self)
{
  g_return_val_if_fail (DZL_IS_SHORTCUT_LABEL (self), NULL);

  if (self->chord == NULL)
    return NULL;

  return dzl_shortcut_chord_to_string (self->chord);
}

void
dzl_shortcut_label_set_accelerator (DzlShortcutLabel *self,
                                    const gchar      *accelerator)
{
  g_autoptr(DzlShortcutChord) chord = NULL;

  g_return_if_fail (DZL_IS_SHORTCUT_LABEL (self));

  if (accelerator != NULL)
    chord = dzl_shortcut_chord_new_from_string (accelerator);

  dzl_shortcut_label_set_chord (self, chord);
}

void
dzl_shortcut_label_set_chord (DzlShortcutLabel       *self,
                              const DzlShortcutChord *chord)
{
  g_return_if_fail (DZL_IS_SHORTCUT_LABEL (self));

  if (!dzl_shortcut_chord_equal (chord, self->chord))
    {
      dzl_shortcut_chord_free (self->chord);
      self->chord = dzl_shortcut_chord_copy (chord);

      gtk_container_foreach (GTK_CONTAINER (self), (GtkCallback) gtk_widget_destroy, NULL);

      if (chord != NULL)
        {
          GdkModifierType first_mod = 0;
          guint len;

          len = dzl_shortcut_chord_get_length (chord);

          dzl_shortcut_chord_get_nth_key (chord, 0, NULL, &first_mod);

          for (guint i = 0; i < len; i++)
            {
              g_autofree gchar *accel = NULL;
              GtkWidget *label;
              GdkModifierType mod = 0;
              guint keyval = 0;

              dzl_shortcut_chord_get_nth_key (chord, i, &keyval, &mod);

              if (i > 0 && (mod & first_mod) == first_mod)
                accel = gtk_accelerator_name (keyval, mod & ~first_mod);
              else
                accel = gtk_accelerator_name (keyval, mod);

              label = g_object_new (GTK_TYPE_SHORTCUT_LABEL,
                                    "accelerator", accel,
                                    "visible", TRUE,
                                    NULL);
              gtk_container_add (GTK_CONTAINER (self), label);
            }
        }

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_ACCELERATOR]);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_CHORD]);
    }
}

/**
 * dzl_shortcut_label_get_chord:
 * @self: a #DzlShortcutLabel
 *
 * Gets the chord for the label, or %NULL.
 *
 * Returns: (transfer none) (nullable): A #DzlShortcutChord or %NULL
 */
const DzlShortcutChord *
dzl_shortcut_label_get_chord (DzlShortcutLabel *self)
{
  return self->chord;
}
