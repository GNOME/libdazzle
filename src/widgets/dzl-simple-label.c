/* dzl-simple-label.c
 *
 * Copyright (C) 2016 Christian Hergert <christian@hergert.me>
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

#define G_LOG_DOMAIN "dzl-simple-labels"

#include <string.h>

#include "util/dzl-macros.h"
#include "widgets/dzl-simple-label.h"

struct _DzlSimpleLabel
{
  GtkWidget    parent_instance;

  gchar       *label;
  gint         label_len;

  gint         width_chars;

  PangoLayout *cached_layout;

  gfloat       xalign;

  gint         cached_width_request;
  gint         cached_height_request;
  gint         real_width;
  gint         real_height;
};

G_DEFINE_TYPE (DzlSimpleLabel, dzl_simple_label, GTK_TYPE_WIDGET)

enum {
  PROP_0,
  PROP_LABEL,
  PROP_WIDTH_CHARS,
  PROP_XALIGN,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
dzl_simple_label_calculate_size (DzlSimpleLabel *self)
{
  PangoContext *context;
  PangoLayout *layout;

  g_assert (DZL_IS_SIMPLE_LABEL (self));

  self->cached_height_request = -1;
  self->cached_width_request = -1;

  if (self->label == NULL && self->width_chars <= 0)
    {
      self->cached_height_request = 0;
      self->cached_width_request = 0;
      self->real_width = 0;
      self->real_height = 0;
      return;
    }

  if (NULL == (context = gtk_widget_get_pango_context (GTK_WIDGET (self))))
    return;

  g_clear_object (&self->cached_layout);

  layout = pango_layout_new (context);

  if (self->width_chars >= 0)
    {
      gchar str[self->width_chars];
      memset (str, '9', self->width_chars);
      pango_layout_set_text (layout, str, self->width_chars);
    }
  else
    {
      pango_layout_set_text (layout, self->label, self->label_len);
    }

  pango_layout_get_pixel_size (layout,
                               &self->cached_width_request,
                               &self->cached_height_request);

  if (self->label != NULL)
    pango_layout_set_text (layout, self->label, self->label_len);
  else
    pango_layout_set_text (layout, "", 0);

  pango_layout_get_pixel_size (layout, &self->real_width, &self->real_height);

  if (self->real_width > self->cached_width_request)
    self->cached_width_request = self->real_width;

  if (self->real_height > self->cached_height_request)
    self->cached_height_request = self->real_height;

  self->cached_layout = layout;
}

static void
dzl_simple_label_get_preferred_width (GtkWidget *widget,
                                      gint      *min_width,
                                      gint      *nat_width)
{
  DzlSimpleLabel *self = (DzlSimpleLabel *)widget;

  g_assert (DZL_IS_SIMPLE_LABEL (self));

  if (self->cached_width_request == -1)
    dzl_simple_label_calculate_size (self);

  *min_width = *nat_width = self->cached_width_request;
}

static void
dzl_simple_label_get_preferred_height (GtkWidget *widget,
                                       gint      *min_height,
                                       gint      *nat_height)
{
  DzlSimpleLabel *self = (DzlSimpleLabel *)widget;

  g_assert (DZL_IS_SIMPLE_LABEL (self));

  if (self->cached_height_request == -1)
    dzl_simple_label_calculate_size (self);

  *min_height = *nat_height = self->cached_height_request;
}

static gboolean
dzl_simple_label_draw (GtkWidget *widget,
                       cairo_t   *cr)
{
  DzlSimpleLabel *self = (DzlSimpleLabel *)widget;
  GtkAllocation alloc;
  gdouble x;
  gdouble y;

  if (self->label == NULL)
    return GDK_EVENT_PROPAGATE;

  gtk_widget_get_allocation (widget, &alloc);

  if (self->cached_width_request == -1 ||
      self->cached_height_request == -1 ||
      self->cached_layout == NULL)
    dzl_simple_label_calculate_size (self);

  x = (alloc.width - self->real_width) * self->xalign;
  y = (alloc.height - self->real_height) / 2;

  /*
   * We should support baseline here, but we don't actually
   * get a real baseline yet where this is used in Builder,
   * so I'm going to punt on it.
   */

  gtk_render_layout (gtk_widget_get_style_context (widget),
                     cr, x, y, self->cached_layout);

  return GDK_EVENT_PROPAGATE;
}

static void
dzl_simple_label_destroy (GtkWidget *widget)
{
  DzlSimpleLabel *self = (DzlSimpleLabel *)widget;

  g_clear_pointer (&self->label, g_free);
  g_clear_object (&self->cached_layout);

  GTK_WIDGET_CLASS (dzl_simple_label_parent_class)->destroy (widget);
}

static void
dzl_simple_label_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  DzlSimpleLabel *self = DZL_SIMPLE_LABEL (object);

  switch (prop_id)
    {
    case PROP_LABEL:
      g_value_set_string (value, self->label);
      break;

    case PROP_WIDTH_CHARS:
      g_value_set_int (value, self->width_chars);
      break;

    case PROP_XALIGN:
      g_value_set_float (value, self->xalign);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_simple_label_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  DzlSimpleLabel *self = DZL_SIMPLE_LABEL (object);

  switch (prop_id)
    {
    case PROP_LABEL:
      dzl_simple_label_set_label (self, g_value_get_string (value));
      break;

    case PROP_WIDTH_CHARS:
      dzl_simple_label_set_width_chars (self, g_value_get_int (value));
      break;

    case PROP_XALIGN:
      dzl_simple_label_set_xalign (self, g_value_get_float (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_simple_label_class_init (DzlSimpleLabelClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = dzl_simple_label_get_property;
  object_class->set_property = dzl_simple_label_set_property;

  widget_class->destroy = dzl_simple_label_destroy;
  widget_class->draw = dzl_simple_label_draw;
  widget_class->get_preferred_width = dzl_simple_label_get_preferred_width;
  widget_class->get_preferred_height = dzl_simple_label_get_preferred_height;

  gtk_widget_class_set_css_name (widget_class, "label");

  properties [PROP_LABEL] =
    g_param_spec_string ("label",
                         NULL,
                         NULL,
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_WIDTH_CHARS] =
    g_param_spec_int ("width-chars",
                      NULL,
                      NULL,
                      -1,
                      1000,
                      -1,
                      (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_XALIGN] =
    g_param_spec_float ("xalign",
                        NULL,
                        NULL,
                        0.0,
                        1.0,
                        0.5,
                        (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_simple_label_init (DzlSimpleLabel *self)
{
  gtk_widget_set_has_window (GTK_WIDGET (self), FALSE);

  self->width_chars = -1;
  self->xalign = 0.5;
}

GtkWidget *
dzl_simple_label_new (const gchar *label)
{
  return g_object_new (DZL_TYPE_SIMPLE_LABEL,
                       "label", label,
                       NULL);
}

const gchar *
dzl_simple_label_get_label (DzlSimpleLabel *self)
{
  g_return_val_if_fail (DZL_IS_SIMPLE_LABEL (self), NULL);

  return self->label;
}

void
dzl_simple_label_set_label (DzlSimpleLabel *self,
                            const gchar    *label)
{
  g_return_if_fail (DZL_IS_SIMPLE_LABEL (self));

  if (g_strcmp0 (label, self->label) != 0)
    {
      gint last_len = self->label_len;

      g_free (self->label);

      self->label = g_strdup (label);
      self->label_len = label ? strlen (label) : 0;

      self->cached_width_request = -1;
      self->cached_height_request = -1;

      /*
       * If width chars is not set, then we always have to calculate the size
       * change.  If we are growing larger, we also might have to relcalculate
       * if the new length is larger than our precalculated length. If we are
       * shrinking from an overgrow position, we also have to resize.
       *
       * But in *most* cases, we can avoid the resize altogether. This is
       * a necessity in the situations where this widget is valuable (such
       * as the cursor coordinate label in Builder).
       */
      if ((self->width_chars < 0) ||
          ((self->label_len > self->width_chars) && (last_len != self->label_len)) ||
          ((last_len > self->width_chars) && (self->label_len <= self->width_chars)))
        gtk_widget_queue_resize (GTK_WIDGET (self));

      gtk_widget_queue_draw (GTK_WIDGET (self));

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_LABEL]);
    }
}

gint
dzl_simple_label_get_width_chars (DzlSimpleLabel *self)
{
  g_return_val_if_fail (DZL_IS_SIMPLE_LABEL (self), -1);

  return self->width_chars;
}

void
dzl_simple_label_set_width_chars (DzlSimpleLabel *self,
                                  gint            width_chars)
{
  g_return_if_fail (DZL_IS_SIMPLE_LABEL (self));
  g_return_if_fail (width_chars >= -1);
  g_return_if_fail (width_chars <= 100);

  if (self->width_chars != width_chars)
    {
      self->width_chars = width_chars;
      self->cached_width_request = -1;
      self->cached_height_request = -1;
      gtk_widget_queue_resize (GTK_WIDGET (self));
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_WIDTH_CHARS]);
    }
}

gfloat
dzl_simple_label_get_xalign (DzlSimpleLabel *self)
{
  g_return_val_if_fail (DZL_IS_SIMPLE_LABEL (self), 0.0);

  return self->xalign;
}

void
dzl_simple_label_set_xalign (DzlSimpleLabel *self,
                             gfloat          xalign)
{
  if (self->xalign != xalign)
    {
      self->xalign = xalign;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_XALIGN]);
    }
}
