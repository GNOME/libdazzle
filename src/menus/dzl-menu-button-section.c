/* dzl-menu-button-section.c
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

#define G_LOG_DOMAIN "dzl-menu-button-section"

#include "config.h"

#include "bindings/dzl-signal-group.h"
#include "menus/dzl-menu-button-section.h"
#include "menus/dzl-menu-button-item.h"
#include "widgets/dzl-box.h"
#include "util/dzl-util-private.h"

struct _DzlMenuButtonSection
{
  GtkBox          parent_instance;

  /* Owned references */
  DzlSignalGroup *menu_signals;
  GtkSizeGroup   *text_size_group;

  /* Template references */
  GtkLabel       *label;
  DzlBox         *items_box;
  GtkSeparator   *separator;

  guint           show_accels : 1;
  guint           show_icons : 1;
};

enum {
  PROP_0,
  PROP_LABEL,
  PROP_MODEL,
  PROP_SHOW_ACCELS,
  PROP_SHOW_ICONS,
  PROP_TEXT_SIZE_GROUP,
  N_PROPS
};

G_DEFINE_TYPE (DzlMenuButtonSection, dzl_menu_button_section, GTK_TYPE_BOX)

static GParamSpec *properties [N_PROPS];

static void
update_show_accel (GtkWidget            *widget,
                   DzlMenuButtonSection *self)
{
  if (DZL_IS_MENU_BUTTON_ITEM (widget))
    g_object_set (widget, "show-accel", self->show_accels, NULL);
}

static void
dzl_menu_button_section_set_show_accels (DzlMenuButtonSection *self,
                                         gboolean              show_accels)
{
  g_assert (DZL_IS_MENU_BUTTON_SECTION (self));

  self->show_accels = !!show_accels;
  gtk_container_foreach (GTK_CONTAINER (self->items_box),
                         (GtkCallback) update_show_accel,
                         self);
}

static void
update_show_icon (GtkWidget            *widget,
                  DzlMenuButtonSection *self)
{
  if (DZL_IS_MENU_BUTTON_ITEM (widget))
    g_object_set (widget, "show-image", self->show_icons, NULL);
}

static void
dzl_menu_button_section_set_show_icons (DzlMenuButtonSection *self,
                                        gboolean              show_icons)
{
  g_assert (DZL_IS_MENU_BUTTON_SECTION (self));

  self->show_icons = !!show_icons;
  gtk_container_foreach (GTK_CONTAINER (self->items_box),
                         (GtkCallback) update_show_icon,
                         self);
}

static void
dzl_menu_button_section_items_changed (DzlMenuButtonSection *self,
                                       guint                 position,
                                       guint                 removed,
                                       guint                 added,
                                       GMenuModel           *menu)
{
  g_assert (DZL_IS_MENU_BUTTON_SECTION (self));
  g_assert (G_IS_MENU_MODEL (menu));

  for (guint i = 0; i < removed; i++)
    {
      GtkWidget *child = dzl_box_get_nth_child (self->items_box, position);

      gtk_widget_destroy (child);
    }

  for (guint i = position; i < (position + added); i++)
    {
      DzlMenuButtonItem *item;
      g_autoptr(GVariant) target = NULL;
      g_autofree gchar *accel = NULL;
      g_autofree gchar *action = NULL;
      g_autofree gchar *label = NULL;
      g_autofree gchar *verb_icon_name = NULL;
      g_autofree gchar *rolestr = NULL;
      gint role = -1;

      g_menu_model_get_item_attribute (menu, i, G_MENU_ATTRIBUTE_LABEL, "s", &label);
      g_menu_model_get_item_attribute (menu, i, "verb-icon-name", "s", &verb_icon_name);
      g_menu_model_get_item_attribute (menu, i, "accel", "s", &accel);
      g_menu_model_get_item_attribute (menu, i, "action", "s", &action);
      g_menu_model_get_item_attribute (menu, i, "role", "s", &rolestr);
      target = g_menu_model_get_item_attribute_value (menu, i, "target", NULL);

      if (g_strcmp0 (rolestr, "check") == 0)
        role = GTK_BUTTON_ROLE_CHECK;
      else if (g_strcmp0 (rolestr, "normal") == 0)
        role = GTK_BUTTON_ROLE_NORMAL;

      item = g_object_new (DZL_TYPE_MENU_BUTTON_ITEM,
                           "action-name", action,
                           "action-target", target,
                           "show-image", self->show_icons,
                           "show-accel", self->show_accels,
                           "icon-name", verb_icon_name,
                           "role", role,
                           "text", label,
                           "text-size-group", self->text_size_group,
                           "accel", accel,
                           "visible", TRUE,
                           NULL);
      gtk_container_add_with_properties (GTK_CONTAINER (self->items_box), GTK_WIDGET (item),
                                         "position", i,
                                         NULL);
    }

  if (added || g_menu_model_get_n_items (menu))
    gtk_widget_show (GTK_WIDGET (self->separator));
  else
    gtk_widget_hide (GTK_WIDGET (self->separator));
}

static void
dzl_menu_button_section_bind (DzlMenuButtonSection *self,
                              GMenuModel           *menu,
                              DzlSignalGroup       *menu_signals)
{
  guint n_items;

  g_assert (DZL_IS_MENU_BUTTON_SECTION (self));
  g_assert (G_IS_MENU_MODEL (menu));
  g_assert (DZL_IS_SIGNAL_GROUP (menu_signals));

  /* Remove on bind instead of unbind to avoid data races
   * when destroying the widget.
   */
  gtk_container_foreach (GTK_CONTAINER (self->items_box),
                         (GtkCallback) gtk_widget_destroy,
                         NULL);

  n_items = g_menu_model_get_n_items (menu);
  dzl_menu_button_section_items_changed (self, 0, 0, n_items, menu);
}

static void
dzl_menu_button_section_destroy (GtkWidget *widget)
{
  DzlMenuButtonSection *self = (DzlMenuButtonSection *)widget;

  g_clear_object (&self->menu_signals);
  g_clear_object (&self->text_size_group);

  GTK_WIDGET_CLASS (dzl_menu_button_section_parent_class)->destroy (widget);
}

static void
dzl_menu_button_section_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  DzlMenuButtonSection *self = DZL_MENU_BUTTON_SECTION (object);

  switch (prop_id)
    {
    case PROP_MODEL:
      g_value_set_object (value, dzl_signal_group_get_target (self->menu_signals));
      break;

    case PROP_LABEL:
      g_value_set_string (value, gtk_label_get_label (self->label));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_menu_button_section_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  DzlMenuButtonSection *self = DZL_MENU_BUTTON_SECTION (object);

  switch (prop_id)
    {
    case PROP_MODEL:
      dzl_signal_group_set_target (self->menu_signals, g_value_get_object (value));
      break;

    case PROP_LABEL:
      gtk_label_set_label (self->label, g_value_get_string (value));
      gtk_widget_set_visible (GTK_WIDGET (self->label),
                              !dzl_str_empty0 (g_value_get_string (value)));
      break;

    case PROP_SHOW_ICONS:
      dzl_menu_button_section_set_show_icons (self, g_value_get_boolean (value));
      break;

    case PROP_SHOW_ACCELS:
      dzl_menu_button_section_set_show_accels (self, g_value_get_boolean (value));
      break;

    case PROP_TEXT_SIZE_GROUP:
      self->text_size_group = g_value_dup_object (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_menu_button_section_class_init (DzlMenuButtonSectionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = dzl_menu_button_section_get_property;
  object_class->set_property = dzl_menu_button_section_set_property;

  widget_class->destroy = dzl_menu_button_section_destroy;

  properties [PROP_SHOW_ACCELS] =
    g_param_spec_boolean ("show-accels", NULL, NULL, FALSE,
                          (G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  properties [PROP_SHOW_ICONS] =
    g_param_spec_boolean ("show-icons", NULL, NULL, FALSE,
                          (G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  properties [PROP_MODEL] =
    g_param_spec_object ("model", NULL, NULL,
                         G_TYPE_MENU_MODEL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_LABEL] =
    g_param_spec_string ("label", NULL, NULL, NULL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_TEXT_SIZE_GROUP] =
    g_param_spec_object ("text-size-group", NULL, NULL,
                         GTK_TYPE_SIZE_GROUP,
                         (G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_css_name (widget_class, "dzlmenubuttonsection");
  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/dazzle/ui/dzl-menu-button-section.ui");
  gtk_widget_class_bind_template_child (widget_class, DzlMenuButtonSection, label);
  gtk_widget_class_bind_template_child (widget_class, DzlMenuButtonSection, items_box);
  gtk_widget_class_bind_template_child (widget_class, DzlMenuButtonSection, separator);
}

static void
dzl_menu_button_section_init (DzlMenuButtonSection *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->menu_signals = dzl_signal_group_new (G_TYPE_MENU_MODEL);

  g_signal_connect_swapped (self->menu_signals,
                            "bind",
                            G_CALLBACK (dzl_menu_button_section_bind),
                            self);

  dzl_signal_group_connect_swapped (self->menu_signals,
                                    "items-changed",
                                    G_CALLBACK (dzl_menu_button_section_items_changed),
                                    self);
}

/**
 * dzl_menu_button_section_new:
 *
 * Creates a new #DzlMenuButtonSection.
 *
 * Returns: (transfer full): A #DzlMenuButtonSection
 *
 * Since: 3.26
 */
GtkWidget *
dzl_menu_button_section_new (GMenuModel  *model,
                             const gchar *label)
{
  return g_object_new (DZL_TYPE_MENU_BUTTON_SECTION,
                       "model", model,
                       "label", label,
                       NULL);
}
