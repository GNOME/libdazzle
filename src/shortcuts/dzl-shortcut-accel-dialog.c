/* dzl-shortcut-accel-dialog.c
 *
 * Copyright (C) 2016 Endless, Inc
 *           (C) 2017 Christian Hergert
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
 *
 * Authors: Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
 *          Christian Hergert <chergert@redhat.com>
 */

#define G_LOG_DOMAIN "dzl-shortcut-accel-dialog"

#include "config.h"

#include <glib/gi18n.h>

#include "shortcuts/dzl-shortcut-accel-dialog.h"
#include "shortcuts/dzl-shortcut-chord.h"
#include "shortcuts/dzl-shortcut-label.h"
#include "util/dzl-macros.h"

struct _DzlShortcutAccelDialog
{
  GtkDialog             parent_instance;

  GtkStack             *stack;
  GtkLabel             *display_label;
  DzlShortcutLabel     *display_shortcut;
  GtkLabel             *selection_label;
  GtkButton            *button_cancel;
  GtkButton            *button_set;

  GdkDevice            *grab_pointer;

  gchar                *shortcut_title;
  DzlShortcutChord     *chord;

  gulong                grab_source;

  guint                 first_modifier;
};

enum {
  PROP_0,
  PROP_ACCELERATOR,
  PROP_SHORTCUT_TITLE,
  N_PROPS
};

G_DEFINE_TYPE (DzlShortcutAccelDialog, dzl_shortcut_accel_dialog, GTK_TYPE_DIALOG)

static GParamSpec *properties [N_PROPS];

/*
 * dzl_shortcut_accel_dialog_begin_grab:
 *
 * This function returns %G_SOURCE_REMOVE so that it may be used as
 * a GSourceFunc when necessary.
 *
 * Returns: %G_SOURCE_REMOVE always.
 */
static gboolean
dzl_shortcut_accel_dialog_begin_grab (DzlShortcutAccelDialog *self)
{
  g_autoptr(GList) seats = NULL;
  GdkWindow *window;
  GdkDisplay *display;
  GdkSeat *first_seat;
  GdkDevice *device;
  GdkDevice *pointer;
  GdkGrabStatus status;

  g_assert (DZL_IS_SHORTCUT_ACCEL_DIALOG (self));

  self->grab_source = 0;

  if (!gtk_widget_get_mapped (GTK_WIDGET (self)))
    return G_SOURCE_REMOVE;

  if (NULL == (window = gtk_widget_get_window (GTK_WIDGET (self))))
    return G_SOURCE_REMOVE;

  display = gtk_widget_get_display (GTK_WIDGET (self));

  if (NULL == (seats = gdk_display_list_seats (display)))
    return G_SOURCE_REMOVE;

  first_seat = seats->data;
  device = gdk_seat_get_keyboard (first_seat);

  if (device == NULL)
    {
      g_warning ("Keyboard grab unsuccessful, no keyboard in seat");
      return G_SOURCE_REMOVE;
    }

  if (gdk_device_get_source (device) == GDK_SOURCE_KEYBOARD)
    pointer = gdk_device_get_associated_device (device);
  else
    pointer = device;

  status = gdk_seat_grab (gdk_device_get_seat (pointer),
                          window,
                          GDK_SEAT_CAPABILITY_KEYBOARD,
                          FALSE,
                          NULL,
                          NULL,
                          NULL,
                          NULL);

  if (status != GDK_GRAB_SUCCESS)
    return G_SOURCE_REMOVE;

  self->grab_pointer = pointer;

  g_debug ("Grab started on %s with device %s",
           G_OBJECT_TYPE_NAME (self),
           G_OBJECT_TYPE_NAME (device));

  gtk_grab_add (GTK_WIDGET (self));

  return G_SOURCE_REMOVE;
}

static void
dzl_shortcut_accel_dialog_release_grab (DzlShortcutAccelDialog *self)
{
  g_assert (DZL_IS_SHORTCUT_ACCEL_DIALOG (self));

  if (self->grab_pointer != NULL)
    {
      gdk_seat_ungrab (gdk_device_get_seat (self->grab_pointer));
      self->grab_pointer = NULL;
      gtk_grab_remove (GTK_WIDGET (self));
    }
}

static void
dzl_shortcut_accel_dialog_map (GtkWidget *widget)
{
  DzlShortcutAccelDialog *self = (DzlShortcutAccelDialog *)widget;

  g_assert (DZL_IS_SHORTCUT_ACCEL_DIALOG (self));

  GTK_WIDGET_CLASS (dzl_shortcut_accel_dialog_parent_class)->map (widget);

  self->grab_source =
    g_timeout_add_full (G_PRIORITY_LOW,
                        100,
                        (GSourceFunc) dzl_shortcut_accel_dialog_begin_grab,
                        g_object_ref (self),
                        g_object_unref);
}

static void
dzl_shortcut_accel_dialog_unmap (GtkWidget *widget)
{
  DzlShortcutAccelDialog *self = (DzlShortcutAccelDialog *)widget;

  g_assert (DZL_IS_SHORTCUT_ACCEL_DIALOG (self));

  dzl_shortcut_accel_dialog_release_grab (self);

  GTK_WIDGET_CLASS (dzl_shortcut_accel_dialog_parent_class)->unmap (widget);
}

static gboolean
dzl_shortcut_accel_dialog_is_editing (DzlShortcutAccelDialog *self)
{
  g_assert (DZL_IS_SHORTCUT_ACCEL_DIALOG (self));

  return self->grab_pointer != NULL;
}

static void
dzl_shortcut_accel_dialog_apply_state (DzlShortcutAccelDialog *self)
{
  g_assert (DZL_IS_SHORTCUT_ACCEL_DIALOG (self));

  if (self->chord != NULL)
    {
      gtk_stack_set_visible_child_name (self->stack, "display");
      gtk_dialog_set_response_sensitive (GTK_DIALOG (self), GTK_RESPONSE_ACCEPT, TRUE);
    }
  else
    {
      gtk_stack_set_visible_child_name (self->stack, "selection");
      gtk_dialog_set_response_sensitive (GTK_DIALOG (self), GTK_RESPONSE_ACCEPT, FALSE);
    }
}

static gboolean
dzl_shortcut_accel_dialog_key_press_event (GtkWidget   *widget,
                                           GdkEventKey *key)
{
  DzlShortcutAccelDialog *self = (DzlShortcutAccelDialog *)widget;

  g_assert (DZL_IS_SHORTCUT_ACCEL_DIALOG (self));
  g_assert (key != NULL);

  if (dzl_shortcut_accel_dialog_is_editing (self))
    {
      GdkModifierType real_mask;
      guint keyval_lower;

      if (key->is_modifier)
        {
          /*
           * If we are just starting a chord, we need to stash the modifier
           * so that we know when we have finished the sequence.
           */
          if (self->chord == NULL && self->first_modifier == 0)
            self->first_modifier = key->keyval;

          goto chain_up;
        }

      real_mask = key->state & gtk_accelerator_get_default_mod_mask ();
      keyval_lower = gdk_keyval_to_lower (key->keyval);

      /* Normalize <Tab> */
      if (keyval_lower == GDK_KEY_ISO_Left_Tab)
        keyval_lower = GDK_KEY_Tab;

      /* Put shift back if it changed the case of the key */
      if (keyval_lower != key->keyval)
        real_mask |= GDK_SHIFT_MASK;

      /* We don't want to use SysRq as a keybinding but we do
       * want Alt+Print), so we avoid translation from Alt+Print to SysRq
       */
      if (keyval_lower == GDK_KEY_Sys_Req && (real_mask & GDK_MOD1_MASK) != 0)
        keyval_lower = GDK_KEY_Print;

      /* A single Escape press cancels the editing */
      if (!key->is_modifier && real_mask == 0 && keyval_lower == GDK_KEY_Escape)
        {
          dzl_shortcut_accel_dialog_release_grab (self);
          gtk_dialog_response (GTK_DIALOG (self), GTK_RESPONSE_CANCEL);
          return GDK_EVENT_STOP;
        }

      /* Backspace disables the current shortcut */
      if (real_mask == 0 && keyval_lower == GDK_KEY_BackSpace)
        {
          dzl_shortcut_accel_dialog_set_accelerator (self, NULL);
          gtk_dialog_response (GTK_DIALOG (self), GTK_RESPONSE_ACCEPT);
          return GDK_EVENT_STOP;
        }

      if (self->chord == NULL)
        self->chord = dzl_shortcut_chord_new_from_event (key);
      else
        dzl_shortcut_chord_append_event (self->chord, key);

      dzl_shortcut_accel_dialog_apply_state (self);

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_ACCELERATOR]);

      return GDK_EVENT_STOP;
    }

chain_up:
  return GTK_WIDGET_CLASS (dzl_shortcut_accel_dialog_parent_class)->key_press_event (widget, key);
}

static gboolean
dzl_shortcut_accel_dialog_key_release_event (GtkWidget   *widget,
                                             GdkEventKey *key)
{
  DzlShortcutAccelDialog *self = (DzlShortcutAccelDialog *)widget;

  g_assert (DZL_IS_SHORTCUT_ACCEL_DIALOG (self));
  g_assert (key != NULL);

  if (self->chord != NULL)
    {
      /*
       * If we have a chord defined and there was no modifier,
       * then any key release should be enough for us to cancel
       * our grab.
       */
      if (!dzl_shortcut_chord_has_modifier (self->chord))
        {
          dzl_shortcut_accel_dialog_release_grab (self);
          goto chain_up;
        }

      /*
       * If we started our sequence with a modifier, we want to
       * release our grab when that modifier has been released.
       */
      if (key->is_modifier &&
          self->first_modifier != 0 &&
          self->first_modifier == key->keyval)
        {
          self->first_modifier = 0;
          dzl_shortcut_accel_dialog_release_grab (self);
          goto chain_up;
        }
    }

  /* Clear modifier if it was released before a chord was made */
  if (self->first_modifier == key->keyval)
    self->first_modifier = 0;

chain_up:
  return GTK_WIDGET_CLASS (dzl_shortcut_accel_dialog_parent_class)->key_release_event (widget, key);
}

static void
dzl_shortcut_accel_dialog_destroy (GtkWidget *widget)
{
  DzlShortcutAccelDialog *self = (DzlShortcutAccelDialog *)widget;

  g_assert (DZL_IS_SHORTCUT_ACCEL_DIALOG (self));

  if (self->grab_source != 0)
    {
      g_source_remove (self->grab_source);
      self->grab_source = 0;
    }

  GTK_WIDGET_CLASS (dzl_shortcut_accel_dialog_parent_class)->destroy (widget);
}

static void
dzl_shortcut_accel_dialog_finalize (GObject *object)
{
  DzlShortcutAccelDialog *self = (DzlShortcutAccelDialog *)object;

  g_clear_pointer (&self->shortcut_title, g_free);
  g_clear_pointer (&self->chord, dzl_shortcut_chord_free);

  G_OBJECT_CLASS (dzl_shortcut_accel_dialog_parent_class)->finalize (object);
}

static void
dzl_shortcut_accel_dialog_get_property (GObject    *object,
                                        guint       prop_id,
                                        GValue     *value,
                                        GParamSpec *pspec)
{
  DzlShortcutAccelDialog *self = DZL_SHORTCUT_ACCEL_DIALOG (object);

  switch (prop_id)
    {
    case PROP_ACCELERATOR:
      g_value_take_string (value, dzl_shortcut_accel_dialog_get_accelerator (self));
      break;

    case PROP_SHORTCUT_TITLE:
      g_value_set_string (value, dzl_shortcut_accel_dialog_get_shortcut_title (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_accel_dialog_set_property (GObject      *object,
                                        guint         prop_id,
                                        const GValue *value,
                                        GParamSpec   *pspec)
{
  DzlShortcutAccelDialog *self = DZL_SHORTCUT_ACCEL_DIALOG (object);

  switch (prop_id)
    {
    case PROP_ACCELERATOR:
      dzl_shortcut_accel_dialog_set_accelerator (self, g_value_get_string (value));
      break;

    case PROP_SHORTCUT_TITLE:
      dzl_shortcut_accel_dialog_set_shortcut_title (self, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcut_accel_dialog_class_init (DzlShortcutAccelDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = dzl_shortcut_accel_dialog_finalize;
  object_class->get_property = dzl_shortcut_accel_dialog_get_property;
  object_class->set_property = dzl_shortcut_accel_dialog_set_property;

  widget_class->destroy = dzl_shortcut_accel_dialog_destroy;
  widget_class->map = dzl_shortcut_accel_dialog_map;
  widget_class->unmap = dzl_shortcut_accel_dialog_unmap;
  widget_class->key_press_event = dzl_shortcut_accel_dialog_key_press_event;
  widget_class->key_release_event = dzl_shortcut_accel_dialog_key_release_event;

  properties [PROP_ACCELERATOR] =
    g_param_spec_string ("accelerator",
                         "Accelerator",
                         "Accelerator",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_SHORTCUT_TITLE] =
    g_param_spec_string ("shortcut-title",
                         "Title",
                         "Title",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/dazzle/ui/dzl-shortcut-accel-dialog.ui");

  gtk_widget_class_bind_template_child (widget_class, DzlShortcutAccelDialog, stack);
  gtk_widget_class_bind_template_child (widget_class, DzlShortcutAccelDialog, selection_label);
  gtk_widget_class_bind_template_child (widget_class, DzlShortcutAccelDialog, display_label);
  gtk_widget_class_bind_template_child (widget_class, DzlShortcutAccelDialog, display_shortcut);
  gtk_widget_class_bind_template_child (widget_class, DzlShortcutAccelDialog, button_cancel);
  gtk_widget_class_bind_template_child (widget_class, DzlShortcutAccelDialog, button_set);

  g_type_ensure (DZL_TYPE_SHORTCUT_LABEL);
}

static void
dzl_shortcut_accel_dialog_init (DzlShortcutAccelDialog *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_dialog_add_buttons (GTK_DIALOG (self),
                          _("Cancel"), GTK_RESPONSE_CANCEL,
                          _("Set"), GTK_RESPONSE_ACCEPT,
                          NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (self), GTK_RESPONSE_ACCEPT);

  gtk_dialog_set_response_sensitive (GTK_DIALOG (self), GTK_RESPONSE_ACCEPT, FALSE);

  g_object_bind_property (self, "accelerator",
                          self->display_shortcut, "accelerator",
                          G_BINDING_SYNC_CREATE);
}

gchar *
dzl_shortcut_accel_dialog_get_accelerator (DzlShortcutAccelDialog *self)
{
  g_return_val_if_fail (DZL_IS_SHORTCUT_ACCEL_DIALOG (self), NULL);

  if (self->chord == NULL)
    return NULL;

  return dzl_shortcut_chord_to_string (self->chord);
}

void
dzl_shortcut_accel_dialog_set_accelerator (DzlShortcutAccelDialog *self,
                                           const gchar            *accelerator)
{
  g_autoptr(DzlShortcutChord) chord = NULL;

  g_return_if_fail (DZL_IS_SHORTCUT_ACCEL_DIALOG (self));

  if (accelerator)
    chord = dzl_shortcut_chord_new_from_string (accelerator);

  if (!dzl_shortcut_chord_equal (chord, self->chord))
    {
      dzl_shortcut_chord_free (self->chord);
      self->chord = g_steal_pointer (&chord);
      gtk_dialog_set_response_sensitive (GTK_DIALOG (self),
                                         GTK_RESPONSE_ACCEPT,
                                         self->chord != NULL);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_ACCELERATOR]);
    }
}

void
dzl_shortcut_accel_dialog_set_shortcut_title (DzlShortcutAccelDialog *self,
                                              const gchar            *shortcut_title)
{
  g_return_if_fail (DZL_IS_SHORTCUT_ACCEL_DIALOG (self));

  if (g_strcmp0 (shortcut_title, self->shortcut_title) != 0)
    {
      g_autofree gchar *label = NULL;

      if (shortcut_title != NULL)
        {
          /* Translators: <b>%s</b> is used to show the provided text in bold */
          label = g_strdup_printf (_("Enter new shortcut to change <b>%s</b>."), shortcut_title);
        }

      gtk_label_set_label (self->selection_label, label);
      gtk_label_set_label (self->display_label, label);

      g_free (self->shortcut_title);
      self->shortcut_title = g_strdup (shortcut_title);

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_SHORTCUT_TITLE]);
    }
}

const gchar *
dzl_shortcut_accel_dialog_get_shortcut_title (DzlShortcutAccelDialog *self)
{
  g_return_val_if_fail (DZL_IS_SHORTCUT_ACCEL_DIALOG (self), NULL);

  return self->shortcut_title;
}

const DzlShortcutChord *
dzl_shortcut_accel_dialog_get_chord (DzlShortcutAccelDialog *self)
{
  g_return_val_if_fail (DZL_IS_SHORTCUT_ACCEL_DIALOG (self), NULL);

  return self->chord;
}

GtkWidget *
dzl_shortcut_accel_dialog_new (void)
{
  return g_object_new (DZL_TYPE_SHORTCUT_ACCEL_DIALOG, NULL);
}
