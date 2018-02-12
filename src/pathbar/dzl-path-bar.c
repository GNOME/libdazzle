/* dzl-path-bar.c
 *
 * Copyright (C) 2016-2017 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "dzl-path-bar"

#include "config.h"

#include <dazzle.h>
#include <string.h>

#include "dzl-path.h"
#include "dzl-path-bar.h"
#include "dzl-path-element.h"

struct _DzlPathBar
{
  GtkBox    parent_instance;
  DzlPath *path;
};

G_DEFINE_TYPE (DzlPathBar, dzl_path_bar, GTK_TYPE_BOX)

enum {
  PROP_0,
  PROP_PATH,
  N_PROPS
};

enum {
  ELEMENT_SELECTED,
  POPULATE_MENU,
  N_SIGNALS
};

static GParamSpec *properties [N_PROPS];
static guint signals [N_SIGNALS];

static void dzl_path_bar_button_clicked (DzlPathBar     *self,
                                          GtkToggleButton *button);

GtkWidget *
dzl_path_bar_new (void)
{
  return g_object_new (DZL_TYPE_PATH_BAR, NULL);
}

static void
dzl_path_bar_finalize (GObject *object)
{
  DzlPathBar *self = (DzlPathBar *)object;

  g_clear_object (&self->path);

  G_OBJECT_CLASS (dzl_path_bar_parent_class)->finalize (object);
}

static void
dzl_path_bar_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  DzlPathBar *self = DZL_PATH_BAR (object);

  switch (prop_id)
    {
    case PROP_PATH:
      g_value_set_object (value, self->path);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_path_bar_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  DzlPathBar *self = DZL_PATH_BAR (object);

  switch (prop_id)
    {
    case PROP_PATH:
      dzl_path_bar_set_path (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_path_bar_class_init (DzlPathBarClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = dzl_path_bar_finalize;
  object_class->get_property = dzl_path_bar_get_property;
  object_class->set_property = dzl_path_bar_set_property;

  properties [PROP_PATH] =
    g_param_spec_object ("path",
                         "Path",
                         "Path",
                         DZL_TYPE_PATH,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  signals [ELEMENT_SELECTED] =
    g_signal_new ("element-selected",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL,
                  G_TYPE_NONE, 2, DZL_TYPE_PATH, DZL_TYPE_PATH_ELEMENT);

  signals [POPULATE_MENU] =
    g_signal_new ("populate-menu",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL,
                  G_TYPE_NONE, 3, DZL_TYPE_PATH, DZL_TYPE_PATH_ELEMENT, G_TYPE_MENU);

  gtk_widget_class_set_css_name (widget_class, "dzlpathbar");
}

static void
dzl_path_bar_init (DzlPathBar *self)
{
  gtk_box_set_spacing (GTK_BOX (self), 12);
}

static void
dzl_path_bar_buttons_foreach_cb (GtkWidget *widget,
                                 gpointer   user_data)
{
  struct {
    GtkCallback callback;
    gpointer user_data;
  } *closure = user_data;

  g_assert (GTK_IS_WIDGET (widget));
  g_assert (closure != NULL);
  g_assert (closure->callback != NULL);

  if (GTK_IS_BOX (widget))
    gtk_container_foreach (GTK_CONTAINER (widget),
                           dzl_path_bar_buttons_foreach_cb,
                           closure);
  else if (GTK_IS_TOGGLE_BUTTON (widget))
    closure->callback (widget, closure->user_data);
}

static void
dzl_path_bar_buttons_foreach (DzlPathBar  *self,
                              GtkCallback  callback,
                              gpointer     user_data)
{
  struct {
    GtkCallback callback;
    gpointer user_data;
  } closure = { callback, user_data };

  g_assert (DZL_IS_PATH_BAR (self));
  g_assert (callback != NULL);

  gtk_container_foreach (GTK_CONTAINER (self),
                         dzl_path_bar_buttons_foreach_cb,
                         &closure);
}

static void
dzl_path_bar_set_blocked_cb (GtkWidget *widget,
                             gpointer   user_data)
{
  struct {
    DzlPathBar *self;
    gboolean     block;
  } *block_data = user_data;

  g_assert (GTK_IS_WIDGET (widget));
  g_assert (block_data != NULL);
  g_assert (DZL_IS_PATH_BAR (block_data->self));

  if (block_data->block)
    g_signal_handlers_block_by_func (widget,
                                     dzl_path_bar_button_clicked,
                                     block_data->self);
  else
    g_signal_handlers_unblock_by_func (widget,
                                       dzl_path_bar_button_clicked,
                                       block_data->self);
}

static void
dzl_path_bar_set_blocked (DzlPathBar *self,
                          gboolean    blocked)
{
  struct {
    DzlPathBar *self;
    gboolean     block;
  } block_data = { self, blocked };

  g_assert (DZL_IS_PATH_BAR (self));

  dzl_path_bar_buttons_foreach (self,
                                 dzl_path_bar_set_blocked_cb,
                                 &block_data);
}

static void
dzl_path_bar_set_active_cb (GtkWidget *widget,
                            gpointer   user_data)
{
  GtkWidget *active = user_data;

  g_assert (GTK_IS_TOGGLE_BUTTON (widget));
  g_assert (GTK_IS_TOGGLE_BUTTON (active));

  if (active != widget)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), FALSE);
}

static void
dzl_path_bar_button_clicked (DzlPathBar      *self,
                             GtkToggleButton *button)
{
  g_autoptr(GMenu) menu = NULL;
  DzlPathElement *element;

  g_assert (DZL_IS_PATH_BAR (self));
  g_assert (GTK_IS_TOGGLE_BUTTON (button));

  /* block all other clicked events while we process */
  dzl_path_bar_set_blocked (self, TRUE);

  /* don't allow unselecting the item */
  if (!gtk_toggle_button_get_active (button))
    gtk_toggle_button_set_active (button, TRUE);

  /* set all the other items as disabled */
  dzl_path_bar_buttons_foreach (self,
                                 dzl_path_bar_set_active_cb,
                                 button);

  /* possibly show a menu for this item */
  menu = g_menu_new ();
  element = g_object_get_data (G_OBJECT (button), "DZL_PATH_ELEMENT");
  g_assert (DZL_IS_PATH_ELEMENT (element));
  g_signal_emit (self, signals [POPULATE_MENU], 0, self->path, element, menu);
  if (g_menu_model_get_n_items (G_MENU_MODEL (menu)) != 0)
    {
      GtkPopover *popover;

      popover = g_object_new (GTK_TYPE_POPOVER,
                              "modal", TRUE,
                              "relative-to", button,
                              "position", GTK_POS_BOTTOM,
                              NULL);
      gtk_popover_bind_model (popover, G_MENU_MODEL (menu), NULL);
      g_signal_connect (popover,
                        "closed",
                        G_CALLBACK (gtk_widget_destroy),
                        NULL);
      gtk_widget_show (GTK_WIDGET (popover));
    }

  /* unblock clicked events again */
  dzl_path_bar_set_blocked (self, FALSE);

  /* notify handlers that an item was selected */
  g_signal_emit (self, signals [ELEMENT_SELECTED], 0, self->path, element);
}

static GtkWidget *
dzl_path_bar_create_element (DzlPathBar     *self,
                             DzlPathElement *element,
                             gboolean        is_last)
{
  const gchar *title;
  const gchar *icon_name;
  GtkToggleButton *button;
  GtkLabel *slash;
  GtkLabel *label;
  GtkImage *image;
  GtkBox *box;
  GtkBox *box2;

  g_assert (DZL_IS_PATH_BAR (self));
  g_assert (DZL_IS_PATH_ELEMENT (element));

  title = dzl_path_element_get_title (element);
  icon_name = dzl_path_element_get_icon_name (element);

  box = g_object_new (GTK_TYPE_BOX,
                      "orientation", GTK_ORIENTATION_HORIZONTAL,
                      "spacing", 12,
                      "visible", TRUE,
                      "valign", GTK_ALIGN_BASELINE,
                      NULL);

  button = g_object_new (GTK_TYPE_TOGGLE_BUTTON,
                         "active", is_last == TRUE,
                         "focus-on-click", FALSE,
                         "visible", TRUE,
                         "valign", GTK_ALIGN_BASELINE,
                         NULL);
  g_object_set_data_full (G_OBJECT (button),
                          "DZL_PATH_ELEMENT",
                          g_object_ref (element),
                          g_object_unref);
  g_signal_connect_object (button,
                           "clicked",
                           G_CALLBACK (dzl_path_bar_button_clicked),
                           self,
                           G_CONNECT_SWAPPED | G_CONNECT_AFTER);
  gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (button));

  slash = g_object_new (GTK_TYPE_LABEL,
                        "label", "/",
                        "valign", GTK_ALIGN_BASELINE,
                        "visible", is_last == FALSE,
                        NULL);
  gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (slash)), "separator");
  gtk_container_add (GTK_CONTAINER (box), GTK_WIDGET (slash));

  box2 = g_object_new (GTK_TYPE_BOX,
                       "orientation", GTK_ORIENTATION_HORIZONTAL,
                       "spacing", 6,
                       "valign", GTK_ALIGN_BASELINE,
                       "visible", TRUE,
                       NULL);
  gtk_container_add (GTK_CONTAINER (button), GTK_WIDGET (box2));

  if (icon_name != NULL)
    {
      image = g_object_new (GTK_TYPE_IMAGE,
                            "icon-name", icon_name,
                            "pixel-size", 16,
                            "valign", GTK_ALIGN_BASELINE,
                            "visible", TRUE,
                            NULL);
      gtk_container_add (GTK_CONTAINER (box2), GTK_WIDGET (image));
    }

  label = g_object_new (DZL_TYPE_BOLDING_LABEL,
                        "label", title,
                        "valign", GTK_ALIGN_BASELINE,
                        "visible", TRUE,
                        NULL);
  gtk_container_add (GTK_CONTAINER (box2), GTK_WIDGET (label));

  return GTK_WIDGET (box);
}

/**
 * dzl_path_bar_get_path:
 *
 * Gets the path for the view.
 *
 * Returns: (transfer none): A #DzlPath.
 */
DzlPath *
dzl_path_bar_get_path (DzlPathBar *self)
{
  g_return_val_if_fail (DZL_IS_PATH_BAR (self), NULL);

  return self->path;
}

void
dzl_path_bar_set_path (DzlPathBar *self,
                       DzlPath    *path)
{
  g_return_if_fail (DZL_IS_PATH_BAR (self));

  if (g_set_object (&self->path, path))
    {
      gtk_container_foreach (GTK_CONTAINER (self), (GtkCallback)gtk_widget_destroy, NULL);

      if (path != NULL)
        {
          GList *items;

          items = dzl_path_get_elements (path);

          for (; items != NULL; items = items->next)
            {
              DzlPathElement *element = items->data;
              GtkWidget *widget;

              widget = dzl_path_bar_create_element (self, element, items->next == NULL);
              gtk_container_add (GTK_CONTAINER (self), widget);
            }
        }
    }
}

static void
dzl_path_bar_set_selected_index_cb (GtkWidget *widget,
                                    gpointer   user_data)
{
  struct {
    GtkWidget *button;
    guint      index;
  } *lookup = user_data;

  g_assert (GTK_IS_WIDGET (widget));
  g_assert (lookup != NULL);

  if (lookup->button == NULL && lookup->index == 0)
    lookup->button = widget;
  else
    lookup->index--;
}

void
dzl_path_bar_set_selected_index (DzlPathBar *self,
                                 guint       index)
{
  struct {
    GtkWidget *button;
    guint      index;
  } lookup = { NULL, index };

  g_return_if_fail (DZL_IS_PATH_BAR (self));

  dzl_path_bar_buttons_foreach (self,
                                 dzl_path_bar_set_selected_index_cb,
                                 &lookup);

  if (lookup.button)
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (lookup.button), TRUE);
}
