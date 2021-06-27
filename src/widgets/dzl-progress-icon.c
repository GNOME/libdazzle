/* dzl-progress-icon.c
 *
 * Copyright (C) 2015 Igalia S.L.
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

#define G_LOG_DOMAIN "dzl-progress-icon"

#include "config.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include "dzl-progress-icon.h"

struct _DzlProgressIcon
{
  GtkDrawingArea parent_instance;
  gdouble        progress;
};

G_DEFINE_TYPE (DzlProgressIcon, dzl_progress_icon, GTK_TYPE_DRAWING_AREA)

enum {
  PROP_0,
  PROP_PROGRESS,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static gboolean
dzl_progress_icon_draw (GtkWidget *widget,
                        cairo_t   *cr)
{
  DzlProgressIcon *self = (DzlProgressIcon *)widget;
  GtkStyleContext *style_context;
  GtkAllocation alloc;
  GdkRGBA rgba;
  GtkStateFlags state;
  gdouble alpha;

  g_assert (DZL_IS_PROGRESS_ICON (self));
  g_assert (cr != NULL);

  gtk_widget_get_allocation (widget, &alloc);

  style_context = gtk_widget_get_style_context (widget);
  state = gtk_style_context_get_state (style_context);
  gtk_style_context_get_color (style_context, state, &rgba);

  alpha = rgba.alpha;
  rgba.alpha = 0.15;
  gdk_cairo_set_source_rgba (cr, &rgba);

  cairo_arc (cr,
             alloc.width / 2,
             alloc.height / 2,
             alloc.width / 2,
             0.0,
             2 * M_PI);
  cairo_fill (cr);

  if (self->progress > 0.0)
    {
      rgba.alpha = alpha;
      gdk_cairo_set_source_rgba (cr, &rgba);

      cairo_arc (cr,
                 alloc.width / 2,
                 alloc.height / 2,
                 alloc.width / 2,
                 (-.5 * M_PI),
                 (2 * self->progress * M_PI) - (.5 * M_PI));

      if (self->progress != 1.0)
        {
          cairo_line_to (cr, alloc.width / 2, alloc.height / 2);
          cairo_line_to (cr, alloc.width / 2, 0);
        }

      cairo_fill (cr);
    }

  return GDK_EVENT_PROPAGATE;
}

static void
dzl_progress_icon_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  DzlProgressIcon *self = DZL_PROGRESS_ICON (object);

  switch (prop_id)
    {
    case PROP_PROGRESS:
      g_value_set_double (value, dzl_progress_icon_get_progress (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_progress_icon_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  DzlProgressIcon *self = DZL_PROGRESS_ICON (object);

  switch (prop_id)
    {
    case PROP_PROGRESS:
      dzl_progress_icon_set_progress (self, g_value_get_double (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_progress_icon_class_init (DzlProgressIconClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = dzl_progress_icon_get_property;
  object_class->set_property = dzl_progress_icon_set_property;

  widget_class->draw = dzl_progress_icon_draw;

  properties [PROP_PROGRESS] =
    g_param_spec_double ("progress",
                         "Progress",
                         "Progress",
                         0.0,
                         1.0,
                         0.0,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_progress_icon_init (DzlProgressIcon *icon)
{
  g_object_set (icon, "width-request", 16, "height-request", 16, NULL);
  gtk_widget_set_valign (GTK_WIDGET (icon), GTK_ALIGN_CENTER);
  gtk_widget_set_halign (GTK_WIDGET (icon), GTK_ALIGN_CENTER);
}

GtkWidget *
dzl_progress_icon_new (void)
{
  return g_object_new (DZL_TYPE_PROGRESS_ICON, NULL);
}

gdouble
dzl_progress_icon_get_progress (DzlProgressIcon *self)
{
  g_return_val_if_fail (DZL_IS_PROGRESS_ICON (self), 0.0);

  return self->progress;
}

void
dzl_progress_icon_set_progress (DzlProgressIcon *self,
                                gdouble          progress)
{
  g_return_if_fail (DZL_IS_PROGRESS_ICON (self));

  progress = CLAMP (progress, 0.0, 1.0);

  if (self->progress != progress)
    {
      self->progress = progress;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_PROGRESS]);
      gtk_widget_queue_draw (GTK_WIDGET (self));
    }
}
