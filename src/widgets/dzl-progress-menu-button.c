/* dzl-progress-menu-button.c
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "dzl-progress-menu-button"

#include "config.h"

#include "animation/dzl-animation.h"
#include "animation/dzl-box-theatric.h"
#include "widgets/dzl-progress-icon.h"
#include "widgets/dzl-progress-menu-button.h"

typedef struct
{
  GtkMenuButton    parent_instance;
  GtkStack        *stack;
  GtkImage        *image;
  DzlProgressIcon *icon;
  const gchar     *theatric_icon_name;
  gdouble          progress;
  guint            transition_duration;
  guint            show_theatric : 1;
  guint            suppress_theatric : 1;
} DzlProgressMenuButtonPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (DzlProgressMenuButton, dzl_progress_menu_button, GTK_TYPE_MENU_BUTTON)

enum {
  PROP_0,
  PROP_PROGRESS,
  PROP_SHOW_PROGRESS,
  PROP_SHOW_THEATRIC,
  PROP_THEATRIC_ICON_NAME,
  PROP_TRANSITION_DURATION,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void dzl_progress_menu_button_begin_theatrics (DzlProgressMenuButton *self);

GtkWidget *
dzl_progress_menu_button_new (void)
{
  return g_object_new (DZL_TYPE_PROGRESS_MENU_BUTTON, NULL);
}

gdouble
dzl_progress_menu_button_get_progress (DzlProgressMenuButton *self)
{
  DzlProgressMenuButtonPrivate *priv = dzl_progress_menu_button_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_PROGRESS_MENU_BUTTON (self), 0.0);

  return priv->progress;
}

void
dzl_progress_menu_button_set_progress (DzlProgressMenuButton *self,
                                       gdouble                progress)
{
  DzlProgressMenuButtonPrivate *priv = dzl_progress_menu_button_get_instance_private (self);

  g_return_if_fail (DZL_IS_PROGRESS_MENU_BUTTON (self));
  g_return_if_fail (progress >= 0.0);
  g_return_if_fail (progress <= 1.0);

  if (progress != priv->progress)
    {
      priv->progress = progress;
      dzl_progress_icon_set_progress (priv->icon, progress);
      if (progress == 1.0)
        dzl_progress_menu_button_begin_theatrics (self);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_PROGRESS]);
    }
}

gboolean
dzl_progress_menu_button_get_show_theatric (DzlProgressMenuButton *self)
{
  DzlProgressMenuButtonPrivate *priv = dzl_progress_menu_button_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_PROGRESS_MENU_BUTTON (self), FALSE);

  return priv->show_theatric;
}

void
dzl_progress_menu_button_set_show_theatric (DzlProgressMenuButton *self,
                                            gboolean               show_theatric)
{
  DzlProgressMenuButtonPrivate *priv = dzl_progress_menu_button_get_instance_private (self);

  g_return_if_fail (DZL_IS_PROGRESS_MENU_BUTTON (self));

  show_theatric = !!show_theatric;

  if (priv->show_theatric != show_theatric)
    {
      priv->show_theatric = show_theatric;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_SHOW_THEATRIC]);
    }
}

static gboolean
begin_theatrics_from_main (gpointer user_data)
{
  DzlProgressMenuButton *self = user_data;
  GtkAllocation rect;

  g_assert (DZL_IS_PROGRESS_MENU_BUTTON (self));

  /* Ignore if still ont allocated */
  gtk_widget_get_allocation (GTK_WIDGET (self), &rect);
  if (rect.x != -1 && rect.y != -1)
    dzl_progress_menu_button_begin_theatrics (self);

  return G_SOURCE_REMOVE;
}

static void
dzl_progress_menu_button_begin_theatrics (DzlProgressMenuButton *self)
{
  DzlProgressMenuButtonPrivate *priv = dzl_progress_menu_button_get_instance_private (self);
  g_autoptr(GIcon) icon = NULL;
  DzlBoxTheatric *theatric;
  GtkAllocation rect;

  g_assert (DZL_IS_PROGRESS_MENU_BUTTON (self));

  if (!priv->show_theatric || priv->transition_duration == 0 || priv->suppress_theatric)
    return;

  gtk_widget_get_allocation (GTK_WIDGET (self), &rect);

  if (rect.x == -1 && rect.y == -1)
    {
      /* Delay this until our widget has been mapped/realized/displayed */
      gdk_threads_add_idle_full (G_PRIORITY_LOW,
                                 begin_theatrics_from_main,
                                 g_object_ref (self), g_object_unref);
      return;
    }

  rect.x = 0;
  rect.y = 0;

  icon = g_themed_icon_new (priv->theatric_icon_name);

  theatric = g_object_new (DZL_TYPE_BOX_THEATRIC,
                           "alpha", 1.0,
                           "height", rect.height,
                           "icon", icon,
                           "target", self,
                           "width", rect.width,
                           "x", rect.x,
                           "y", rect.y,
                           NULL);

  dzl_object_animate_full (theatric,
                           DZL_ANIMATION_EASE_OUT_CUBIC,
                           priv->transition_duration,
                           gtk_widget_get_frame_clock (GTK_WIDGET (self)),
                           g_object_unref,
                           theatric,
                           "x", rect.x - 60,
                           "width", rect.width + 120,
                           "y", rect.y,
                           "height", rect.height + 120,
                           "alpha", 0.0,
                           NULL);

  priv->suppress_theatric = TRUE;
}

static void
dzl_progress_menu_button_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  DzlProgressMenuButton *self = DZL_PROGRESS_MENU_BUTTON (object);
  DzlProgressMenuButtonPrivate *priv = dzl_progress_menu_button_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_PROGRESS:
      g_value_set_double (value, priv->progress);
      break;

    case PROP_SHOW_PROGRESS:
      g_value_set_boolean (value, dzl_progress_menu_button_get_show_progress (self));
      break;

    case PROP_SHOW_THEATRIC:
      g_value_set_boolean (value, priv->show_theatric);
      break;

    case PROP_THEATRIC_ICON_NAME:
      g_value_set_static_string (value, priv->theatric_icon_name);
      break;

    case PROP_TRANSITION_DURATION:
      g_value_set_uint (value, priv->transition_duration);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_progress_menu_button_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  DzlProgressMenuButton *self = DZL_PROGRESS_MENU_BUTTON (object);
  DzlProgressMenuButtonPrivate *priv = dzl_progress_menu_button_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_PROGRESS:
      dzl_progress_menu_button_set_progress (self, g_value_get_double (value));
      break;

    case PROP_SHOW_PROGRESS:
      dzl_progress_menu_button_set_show_progress (self, g_value_get_boolean (value));
      break;

    case PROP_SHOW_THEATRIC:
      dzl_progress_menu_button_set_show_theatric (self, g_value_get_boolean (value));
      break;

    case PROP_THEATRIC_ICON_NAME:
      priv->theatric_icon_name = g_intern_string (g_value_get_string (value));
      break;

    case PROP_TRANSITION_DURATION:
      priv->transition_duration = g_value_get_uint (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_progress_menu_button_class_init (DzlProgressMenuButtonClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = dzl_progress_menu_button_get_property;
  object_class->set_property = dzl_progress_menu_button_set_property;

  properties [PROP_PROGRESS] =
    g_param_spec_double ("progress",
                         "Progress",
                         "Progress",
                         0.0, 1.0, 0.0,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_SHOW_PROGRESS] =
    g_param_spec_boolean ("show-progress",
                          "Show Progress",
                          "Show progress instead of image",
                          TRUE,
                          (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_SHOW_THEATRIC] =
    g_param_spec_boolean ("show-theatric",
                          "Show Theatric",
                          "Show Theatric",
                          TRUE,
                          (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_THEATRIC_ICON_NAME] =
    g_param_spec_string ("theatric-icon-name",
                         "Theatric Icon Name",
                         "Theatric Icon Name",
                         "folder-download-symbolic",
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_TRANSITION_DURATION] =
    g_param_spec_uint ("transition-duration",
                       "Transition Duration",
                       "Transition Duration",
                       0,
                       5000,
                       750,
                       (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_progress_menu_button_init (DzlProgressMenuButton *self)
{
  DzlProgressMenuButtonPrivate *priv = dzl_progress_menu_button_get_instance_private (self);

  priv->theatric_icon_name = g_intern_static_string ("folder-download-symbolic");
  priv->show_theatric = TRUE;
  priv->transition_duration = 750;

  priv->stack = g_object_new (GTK_TYPE_STACK,
                              "homogeneous", FALSE,
                              "visible", TRUE,
                              NULL);
  g_signal_connect (priv->stack,
                    "destroy",
                    G_CALLBACK (gtk_widget_destroyed),
                    &priv->stack);
  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (priv->stack));

  priv->icon = g_object_new (DZL_TYPE_PROGRESS_ICON,
                             "visible", TRUE,
                             NULL);
  g_signal_connect (priv->icon,
                    "destroy",
                    G_CALLBACK (gtk_widget_destroyed),
                    &priv->icon);
  gtk_container_add (GTK_CONTAINER (priv->stack), GTK_WIDGET (priv->icon));

  priv->image = g_object_new (GTK_TYPE_IMAGE,
                              "icon-name", "content-loading-symbolic",
                              "visible", TRUE,
                              NULL);
  g_signal_connect (priv->image,
                    "destroy",
                    G_CALLBACK (gtk_widget_destroyed),
                    &priv->image);
  gtk_container_add (GTK_CONTAINER (priv->stack), GTK_WIDGET (priv->image));
}

/**
 * dzl_progress_menu_button_reset_theatrics:
 * @self: a #DzlProgressMenuButton
 *
 * To avoid suprious animations from the button, you must call this function any
 * time you want to allow animations to continue. This is because animations are
 * automatically started upon reaching a progress of 1.0.
 *
 * If you are performing operations in the background, calling this function
 * every time you add an operation is a good strategry.
 */
void
dzl_progress_menu_button_reset_theatrics (DzlProgressMenuButton *self)
{
  DzlProgressMenuButtonPrivate *priv = dzl_progress_menu_button_get_instance_private (self);

  g_return_if_fail (DZL_IS_PROGRESS_MENU_BUTTON (self));

  priv->suppress_theatric = FALSE;
}

gboolean
dzl_progress_menu_button_get_show_progress (DzlProgressMenuButton *self)
{
  DzlProgressMenuButtonPrivate *priv = dzl_progress_menu_button_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_PROGRESS_MENU_BUTTON (self), FALSE);

  return gtk_stack_get_visible_child (priv->stack) == GTK_WIDGET (priv->icon);
}

void
dzl_progress_menu_button_set_show_progress (DzlProgressMenuButton *self,
                                            gboolean               show_progress)
{
  DzlProgressMenuButtonPrivate *priv = dzl_progress_menu_button_get_instance_private (self);

  g_return_if_fail (DZL_IS_PROGRESS_MENU_BUTTON (self));

  if (show_progress != dzl_progress_menu_button_get_show_progress (self))
    {
      if (show_progress)
        gtk_stack_set_visible_child (priv->stack, GTK_WIDGET (priv->icon));
      else
        gtk_stack_set_visible_child (priv->stack, GTK_WIDGET (priv->image));
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_SHOW_PROGRESS]);
    }
}
