/* dzl-menu-button-item.c
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

#define G_LOG_DOMAIN "dzl-menu-button-item"

#include "config.h"

#include "menus/dzl-menu-button.h"
#include "menus/dzl-menu-button-item.h"
#include "shortcuts/dzl-shortcut-label.h"
#include "shortcuts/dzl-shortcut-simple-label.h"
#include "shortcuts/dzl-shortcut-private.h"
#include "util/dzl-gtk.h"

struct _DzlMenuButtonItem
{
  GtkCheckButton          parent_instance;

  const gchar            *action_name;

  /* Template references */
  DzlShortcutSimpleLabel *accel;
  GtkImage               *image;

  /* -1 is for unset, otherwise GtkButtonRole */
  gint                    role;

  guint                   has_icon : 1;
  guint                   show_image : 1;
};

enum {
  PROP_0,
  PROP_ACCEL,
  PROP_ICON_NAME,
  PROP_ROLE,
  PROP_SHOW_ACCEL,
  PROP_SHOW_IMAGE,
  PROP_TEXT_SIZE_GROUP,
  PROP_TEXT,
  N_PROPS,
};

G_DEFINE_TYPE (DzlMenuButtonItem, dzl_menu_button_item, GTK_TYPE_CHECK_BUTTON)

static GParamSpec *properties [N_PROPS];

static void
dzl_menu_button_item_clicked (DzlMenuButtonItem *self)
{
  gboolean transitions_enabled = FALSE;
  GtkWidget *button;
  GtkWidget *popover;

  g_assert (DZL_IS_MENU_BUTTON_ITEM (self));

  button = dzl_gtk_widget_get_relative (GTK_WIDGET (self), DZL_TYPE_MENU_BUTTON);
  if (button != NULL)
    g_object_get (button, "transitions-enabled", &transitions_enabled, NULL);

  popover = gtk_widget_get_ancestor (GTK_WIDGET (self), GTK_TYPE_POPOVER);

  if (transitions_enabled)
    gtk_popover_popdown (GTK_POPOVER (popover));
  else
    gtk_widget_hide (popover);
}

static gboolean
action_is_stateful (GtkWidget   *widget,
                    const gchar *group,
                    const gchar *name)
{
  GActionGroup *actions = gtk_widget_get_action_group (widget, group);
  GtkWidget *parent;

  if (actions != NULL)
    {
      if (g_action_group_has_action (actions, name) &&
          g_action_group_get_action_state_type (actions, name) != NULL)
        return TRUE;
    }

  if (GTK_IS_POPOVER (widget))
    parent = gtk_popover_get_relative_to (GTK_POPOVER (widget));
  else
    parent = gtk_widget_get_parent (widget);

  if (parent)
    return action_is_stateful (parent, group, name);

  return FALSE;
}

static void
dzl_menu_button_item_notify_action_name (DzlMenuButtonItem *self,
                                         GParamSpec        *pspec)
{
  const gchar *action_name;
  g_auto(GStrv) parts = NULL;
  gboolean draw = FALSE;

  g_assert (DZL_IS_MENU_BUTTON_ITEM (self));

  action_name = gtk_actionable_get_action_name (GTK_ACTIONABLE (self));

  if (action_name)
    parts = g_strsplit (action_name, ".", 2);

  if (parts && parts[0] && parts[1])
    draw = action_is_stateful (GTK_WIDGET (self), parts[0], parts[1]);

  g_object_set (self, "draw-indicator", draw, NULL);
}

static void
dzl_menu_button_item_hierarchy_changed (GtkWidget *widget,
                                        GtkWidget *old_toplevel)
{
  DzlMenuButtonItem *self = (DzlMenuButtonItem *)widget;

  g_assert (DZL_IS_MENU_BUTTON_ITEM (self));

  if (self->role == -1)
    dzl_menu_button_item_notify_action_name (self, NULL);
}

static void
dzl_menu_button_item_set_property (GObject      *object,
                                   guint         prop_id,
                                   const GValue *value,
                                   GParamSpec   *pspec)
{
  DzlMenuButtonItem *self = DZL_MENU_BUTTON_ITEM (object);

  switch (prop_id)
    {
    case PROP_ACCEL:
      dzl_shortcut_simple_label_set_accel (self->accel, g_value_get_string (value));
      break;

    case PROP_ICON_NAME:
      self->has_icon = !!g_value_get_string (value);
      g_object_set_property (G_OBJECT (self->image), "icon-name", value);
      gtk_widget_set_visible (GTK_WIDGET (self->image), self->has_icon && self->show_image);
      break;

    case PROP_ROLE:
      self->role = g_value_get_int (value);
      if (self->role == GTK_BUTTON_ROLE_CHECK)
        g_object_set (self, "draw-indicator", TRUE, NULL);
      else
        {
          g_object_set (self, "draw-indicator", FALSE, NULL);
          if (self->role == -1)
            dzl_menu_button_item_hierarchy_changed (GTK_WIDGET (self), NULL);
        }
      break;

    case PROP_SHOW_ACCEL:
      g_object_set_property (G_OBJECT (self->accel), "show-accel", value);
      break;

    case PROP_SHOW_IMAGE:
      self->show_image = g_value_get_boolean (value);
      gtk_widget_set_visible (GTK_WIDGET (self->image), self->has_icon && self->show_image);
      break;

    case PROP_TEXT:
      dzl_shortcut_simple_label_set_title (self->accel, g_value_get_string (value));
      break;

    case PROP_TEXT_SIZE_GROUP:
      _dzl_shortcut_simple_label_set_size_group (self->accel, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_menu_button_item_class_init (DzlMenuButtonItemClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->set_property = dzl_menu_button_item_set_property;

  widget_class->hierarchy_changed = dzl_menu_button_item_hierarchy_changed;

  properties [PROP_ACCEL] =
    g_param_spec_string ("accel",
                         "Accel",
                         "The accelerator for the item",
                         NULL,
                         (G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  properties [PROP_ICON_NAME] =
    g_param_spec_string ("icon-name",
                         "Icon Name",
                         "The icon to display with the item",
                         NULL,
                         (G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  properties [PROP_ROLE] =
    g_param_spec_int ("role", NULL, NULL,
                      -1, GTK_BUTTON_ROLE_RADIO, -1,
                      (G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_SHOW_ACCEL] =
    g_param_spec_boolean ("show-accel",
                          "Show Accel",
                          "If the accel label should be shown",
                          FALSE,
                          (G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  properties [PROP_SHOW_IMAGE] =
    g_param_spec_boolean ("show-image",
                          "Show Image",
                          "If the image should be shown",
                          FALSE,
                          (G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  properties [PROP_TEXT] =
    g_param_spec_string ("text",
                         "Text",
                         "The text for the menu item",
                         NULL,
                         (G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  properties [PROP_TEXT_SIZE_GROUP] =
    g_param_spec_object ("text-size-group", NULL, NULL,
                         GTK_TYPE_SIZE_GROUP,
                         (G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_menu_button_item_init (DzlMenuButtonItem *self)
{
  GtkTextDirection dir;
  GtkBox *box;

  self->role = -1;

  dzl_gtk_widget_add_style_class (GTK_WIDGET (self), "dzlmenubuttonitem");

  g_signal_connect (self,
                    "clicked",
                    G_CALLBACK (dzl_menu_button_item_clicked),
                    NULL);

  g_signal_connect (self,
                    "notify::action-name",
                    G_CALLBACK (dzl_menu_button_item_notify_action_name),
                    NULL);

  /* flip the location of the checkbutton */
  dir = gtk_widget_get_direction (GTK_WIDGET (self));
  if (dir != GTK_TEXT_DIR_LTR)
    dir = GTK_TEXT_DIR_LTR;
  else
    dir = GTK_TEXT_DIR_RTL;
  gtk_widget_set_direction (GTK_WIDGET (self), dir);

  g_object_set (self, "draw-indicator", FALSE, NULL);

  box = g_object_new (GTK_TYPE_BOX,
                      "visible", TRUE,
                      NULL);
  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (box));

  self->image = g_object_new (GTK_TYPE_IMAGE,
                              "hexpand", FALSE,
                              NULL);
  gtk_container_add_with_properties (GTK_CONTAINER (box), GTK_WIDGET (self->image),
                                     "pack-type", GTK_PACK_START,
                                     "position", 0,
                                     NULL);

  self->accel = g_object_new (DZL_TYPE_SHORTCUT_SIMPLE_LABEL,
                              "hexpand", TRUE,
                              "visible", TRUE,
                              NULL);
  gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (self->accel));
}
