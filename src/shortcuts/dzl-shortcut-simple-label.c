/* dzl-shortcut-simple-label.c
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

#define G_LOG_DOMAIN "dzl-shortcut-simple-label"

#include "config.h"

#include "shortcuts/dzl-shortcut-chord.h"
#include "shortcuts/dzl-shortcut-simple-label.h"
#include "util/dzl-gtk.h"

struct _DzlShortcutSimpleLabel
{
  GtkBox       parent_instance;

  GtkLabel    *accel_label;
  GtkLabel    *title;

  const gchar *accel;
  const gchar *action;
  const gchar *command;
};

enum {
  PROP_0,
  PROP_ACCEL,
  PROP_ACTION,
  PROP_COMMAND,
  PROP_SHOW_ACCEL,
  PROP_TITLE,
  N_PROPS
};

G_DEFINE_TYPE (DzlShortcutSimpleLabel, dzl_shortcut_simple_label, GTK_TYPE_BOX)

static GParamSpec *properties [N_PROPS];

static void
dzl_shortcut_simple_label_get_property (GObject    *object,
                                        guint       prop_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
  DzlShortcutSimpleLabel *self = DZL_SHORTCUT_SIMPLE_LABEL (object);

  switch (prop_id)
    {
    case PROP_ACCEL:
      g_value_set_static_string (value, dzl_shortcut_simple_label_get_accel (self));
      break;

    case PROP_ACTION:
      g_value_set_static_string (value, dzl_shortcut_simple_label_get_action (self));
      break;

    case PROP_COMMAND:
      g_value_set_static_string (value, dzl_shortcut_simple_label_get_command (self));
      break;

    case PROP_SHOW_ACCEL:
      g_object_get_property (G_OBJECT (self->accel_label), "visible", value);
      break;

    case PROP_TITLE:
      g_value_set_string (value, dzl_shortcut_simple_label_get_title (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_simple_label_set_property (GObject      *object,
                                        guint         prop_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  DzlShortcutSimpleLabel *self = DZL_SHORTCUT_SIMPLE_LABEL (object);

  switch (prop_id)
    {
    case PROP_ACCEL:
      dzl_shortcut_simple_label_set_accel (self, g_value_get_string (value));
      break;

    case PROP_ACTION:
      dzl_shortcut_simple_label_set_action (self, g_value_get_string (value));
      break;

    case PROP_COMMAND:
      dzl_shortcut_simple_label_set_command (self, g_value_get_string (value));
      break;

    case PROP_SHOW_ACCEL:
      g_object_set_property (G_OBJECT (self->accel_label), "visible", value);
      break;

    case PROP_TITLE:
      dzl_shortcut_simple_label_set_title (self, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_simple_label_class_init (DzlShortcutSimpleLabelClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = dzl_shortcut_simple_label_get_property;
  object_class->set_property = dzl_shortcut_simple_label_set_property;

  properties [PROP_ACTION] =
    g_param_spec_string ("action",
                         "Action",
                         "Action",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_ACCEL] =
    g_param_spec_string ("accel",
                         "Accel",
                         "The accel label to override the discovered accel",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_COMMAND] =
    g_param_spec_string ("command",
                         "Command",
                         "Command",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_SHOW_ACCEL] =
    g_param_spec_boolean ("show-accel", NULL, NULL,
                          TRUE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "Title",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_shortcut_simple_label_init (DzlShortcutSimpleLabel *self)
{
  self->title = g_object_new (GTK_TYPE_LABEL,
                              "visible", TRUE,
                              "use-underline", TRUE,
                              "xalign", 0.0f,
                              NULL);
  dzl_gtk_widget_add_style_class (GTK_WIDGET (self->title), "title");
  gtk_container_add_with_properties (GTK_CONTAINER (self), GTK_WIDGET (self->title),
                                     "fill", TRUE,
                                     "pack-type", GTK_PACK_START,
                                     NULL);

  self->accel_label = g_object_new (GTK_TYPE_LABEL,
                                    "hexpand", TRUE,
                                    "halign", GTK_ALIGN_END,
                                    "margin-start", 12,
                                    "visible", TRUE,
                                    "xalign", 1.0f,
                                    NULL);
  dzl_gtk_widget_add_style_class (GTK_WIDGET (self->accel_label), "dim-label");
  gtk_container_add_with_properties (GTK_CONTAINER (self), GTK_WIDGET (self->accel_label),
                                     "fill", TRUE,
                                     "pack-type", GTK_PACK_END,
                                     NULL);
}

GtkWidget *
dzl_shortcut_simple_label_new (void)
{
  return g_object_new (DZL_TYPE_SHORTCUT_SIMPLE_LABEL, NULL);
}

const gchar *
dzl_shortcut_simple_label_get_accel (DzlShortcutSimpleLabel *self)
{
  g_return_val_if_fail (DZL_IS_SHORTCUT_SIMPLE_LABEL (self), NULL);

  return self->accel;
}

const gchar *
dzl_shortcut_simple_label_get_action (DzlShortcutSimpleLabel *self)
{
  g_return_val_if_fail (DZL_IS_SHORTCUT_SIMPLE_LABEL (self), NULL);

  return self->action;
}

const gchar *
dzl_shortcut_simple_label_get_command (DzlShortcutSimpleLabel *self)
{
  g_return_val_if_fail (DZL_IS_SHORTCUT_SIMPLE_LABEL (self), NULL);

  return self->command;
}

const gchar *
dzl_shortcut_simple_label_get_title (DzlShortcutSimpleLabel *self)
{
  g_return_val_if_fail (DZL_IS_SHORTCUT_SIMPLE_LABEL (self), NULL);

  return gtk_label_get_label (self->title);
}

void
dzl_shortcut_simple_label_set_accel (DzlShortcutSimpleLabel *self,
                                     const gchar            *accel)
{
  g_return_if_fail (DZL_IS_SHORTCUT_SIMPLE_LABEL (self));

  accel = g_intern_string (accel);

  if (accel != self->accel)
    {
      g_autofree gchar *label = NULL;

      self->accel = accel;

      if (accel != NULL)
        {
          g_autoptr(DzlShortcutChord) chord = NULL;

          chord = dzl_shortcut_chord_new_from_string (accel);
          label = dzl_shortcut_chord_get_label (chord);
        }

      gtk_label_set_label (self->accel_label, label);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_ACCEL]);
    }
}

void
dzl_shortcut_simple_label_set_action (DzlShortcutSimpleLabel *self,
                                      const gchar            *action)
{
  g_return_if_fail (DZL_IS_SHORTCUT_SIMPLE_LABEL (self));

  action = g_intern_string (action);

  if (action != self->action)
    {
      self->action = action;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_ACTION]);
    }
}

void
dzl_shortcut_simple_label_set_command (DzlShortcutSimpleLabel *self,
                                       const gchar            *command)
{
  g_return_if_fail (DZL_IS_SHORTCUT_SIMPLE_LABEL (self));

  command = g_intern_string (command);

  if (command != self->command)
    {
      self->command = command;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_COMMAND]);
    }
}

void
dzl_shortcut_simple_label_set_title (DzlShortcutSimpleLabel *self,
                                     const gchar            *title)
{
  g_return_if_fail (DZL_IS_SHORTCUT_SIMPLE_LABEL (self));

  gtk_label_set_label (self->title, title);
  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_TITLE]);
}

void
_dzl_shortcut_simple_label_set_size_group(DzlShortcutSimpleLabel *self,
                                          GtkSizeGroup           *size_group)
{
  g_return_if_fail (DZL_IS_SHORTCUT_SIMPLE_LABEL (self));

  if (size_group != NULL)
    gtk_size_group_add_widget (size_group, GTK_WIDGET (self->title));
}
