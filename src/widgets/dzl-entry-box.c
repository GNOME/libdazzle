/* dzl-entry-box.c
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

#include "config.h"

#include "dzl-entry-box.h"

struct _DzlEntryBox
{
  GtkBox parent_instance;

  gint max_width_chars;
};

G_DEFINE_TYPE (DzlEntryBox, dzl_entry_box, GTK_TYPE_BOX)

enum {
  PROP_0,
  PROP_MAX_WIDTH_CHARS,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
dzl_entry_box_get_preferred_width (GtkWidget *widget,
                                   gint      *min_width,
                                   gint      *nat_width)
{
  DzlEntryBox *self = (DzlEntryBox *)widget;

  g_assert (DZL_IS_ENTRY_BOX (self));
  g_assert (min_width != NULL);
  g_assert (nat_width != NULL);

  GTK_WIDGET_CLASS (dzl_entry_box_parent_class)->get_preferred_width (widget, min_width, nat_width);

  if (self->max_width_chars > 0)
    {
      PangoContext *context;
      PangoFontMetrics *metrics;
      gint char_width;
      gint digit_width;
      gint width;

      context = gtk_widget_get_pango_context (widget);
      metrics = pango_context_get_metrics (context,
                                           pango_context_get_font_description (context),
                                           pango_context_get_language (context));

      char_width = pango_font_metrics_get_approximate_char_width (metrics);
      digit_width = pango_font_metrics_get_approximate_digit_width (metrics);
      width = MAX (char_width, digit_width) * self->max_width_chars / PANGO_SCALE;

      if (width > *nat_width)
        *nat_width = width;

      pango_font_metrics_unref (metrics);
    }
}

static void
dzl_entry_box_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  DzlEntryBox *self = DZL_ENTRY_BOX (object);

  switch (prop_id)
    {
    case PROP_MAX_WIDTH_CHARS:
      g_value_set_int (value, self->max_width_chars);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_entry_box_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  DzlEntryBox *self = DZL_ENTRY_BOX (object);

  switch (prop_id)
    {
    case PROP_MAX_WIDTH_CHARS:
      self->max_width_chars = g_value_get_int (value);
      gtk_widget_queue_resize (GTK_WIDGET (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_entry_box_class_init (DzlEntryBoxClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = dzl_entry_box_get_property;
  object_class->set_property = dzl_entry_box_set_property;

  widget_class->get_preferred_width = dzl_entry_box_get_preferred_width;

  properties [PROP_MAX_WIDTH_CHARS] =
    g_param_spec_int ("max-width-chars",
                      "Max Width Chars",
                      "Max Width Chars",
                      -1,
                      G_MAXINT,
                      -1,
                      (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_css_name (widget_class, "entry");
}

static void
dzl_entry_box_init (DzlEntryBox *self)
{
  self->max_width_chars = -1;

  G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
  gtk_container_set_reallocate_redraws (GTK_CONTAINER (self), TRUE);
  G_GNUC_END_IGNORE_DEPRECATIONS;
}

GtkWidget *
dzl_entry_box_new (void)
{
  return g_object_new (DZL_TYPE_ENTRY_BOX, NULL);
}
