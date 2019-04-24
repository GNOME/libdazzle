/* dzl-bolding-label.c
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

#include <glib/gi18n.h>

#include "dzl-bolding-label.h"

/**
 * SECTION:dzl-bolding-label
 *
 * This is a GtkLabel widget that will allocate extra space if necessary
 * so that the size request will not change when the contents of the
 * label are bolded.
 *
 * This might be useful when you want to change a label based on some
 * selection state without it affecting the size request or layout.
 *
 * Remember that bold and non-bold variants of fonts can often be very
 * different in terms of styling. To reduce the chances that you see
 * shifts in placement, you may want to set the #GtkLabel:xalign or
 * #GtkLabel:halign properties to 0.0 or %GTK_ALIGN_START respectively.
 */

struct _DzlBoldingLabel
{
  GtkLabel parent_instance;
};

G_DEFINE_TYPE (DzlBoldingLabel, dzl_bolding_label, GTK_TYPE_LABEL)

enum {
  PROP_0,
  PROP_BOLD,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

DzlBoldingLabel *
dzl_bolding_label_new (const gchar *str,
                       gboolean     bold)
{
  DzlBoldingLabel *label;

  label = g_object_new (DZL_TYPE_BOLDING_LABEL,
                        "bold", bold,
                        NULL);

  if (str && *str)
    gtk_label_set_text (GTK_LABEL (label), str);

  return label;
}

DzlBoldingLabel *
dzl_bolding_label_new_with_mnemonic (const gchar *str,
                                     gboolean     bold)
{
  DzlBoldingLabel *label;

  label = g_object_new (DZL_TYPE_BOLDING_LABEL,
                        "bold", bold,
                        NULL);

  if (str && *str)
    gtk_label_set_text_with_mnemonic (GTK_LABEL (label), str);

  return label;
}

static void
dzl_bolding_label_get_preferred_width (GtkWidget *widget,
                                       gint      *min_width,
                                       gint      *nat_width)
{
  const PangoFontDescription *font_desc;
  PangoContext *context;

  g_assert (DZL_IS_BOLDING_LABEL (widget));
  g_assert (min_width);
  g_assert (nat_width);

  GTK_WIDGET_CLASS (dzl_bolding_label_parent_class)->get_preferred_width (widget, min_width, nat_width);

  if (NULL == (context = gtk_widget_get_pango_context (widget)) ||
      NULL == (font_desc = pango_context_get_font_description (context)))
    return;

  if (pango_font_description_get_weight (font_desc) != PANGO_WEIGHT_BOLD)
    {
      PangoFontDescription *font_desc_copy;
      PangoLayout *layout;
      const gchar *text;
      PangoEllipsizeMode ellipsize;
      gint height;
      gint width;

      text = gtk_label_get_text (GTK_LABEL (widget));
      layout = gtk_widget_create_pango_layout (widget, text);
      font_desc_copy = pango_font_description_copy (font_desc);
      ellipsize = gtk_label_get_ellipsize (GTK_LABEL (widget));

      pango_font_description_set_weight (font_desc_copy, PANGO_WEIGHT_BOLD);
      pango_layout_set_font_description (layout, font_desc_copy);
      pango_layout_set_ellipsize (layout, ellipsize);
      pango_layout_get_pixel_size (layout, &width, &height);

      if (ellipsize == PANGO_ELLIPSIZE_NONE)
        {
          /*
           * Only apply min_width if we cannot ellipsize, if we can, that
           * effects things differently.
           */
          if (width > *min_width)
            *min_width = width;
        }

      if (width > *nat_width)
        *nat_width = width;

      pango_font_description_free (font_desc_copy);
      g_object_unref (layout);
    }
}

static void
dzl_bolding_label_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  DzlBoldingLabel *self = DZL_BOLDING_LABEL (object);

  switch (prop_id)
    {
    case PROP_BOLD:
      dzl_bolding_label_set_bold (self, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_bolding_label_class_init (DzlBoldingLabelClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = dzl_bolding_label_set_property;

  widget_class->get_preferred_width = dzl_bolding_label_get_preferred_width;

  properties [PROP_BOLD] =
    g_param_spec_boolean ("bold",
                          "Bold",
                          "Set the bold weight for the label",
                          FALSE,
                          (G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dzl_bolding_label_init (DzlBoldingLabel *self)
{
}

static gboolean
remove_weights (PangoAttribute *attr,
                gpointer        user_data)
{
  return attr->klass->type == ((PangoAttribute *)user_data)->klass->type;
}

void
dzl_bolding_label_set_weight (DzlBoldingLabel *self,
                              PangoWeight      weight)
{
  PangoAttrList *filtered;
  PangoAttrList *attrs;
  PangoAttrList *copy;
  PangoAttribute *attr;

  g_return_if_fail (DZL_IS_BOLDING_LABEL (self));

  attrs = gtk_label_get_attributes (GTK_LABEL (self));
  if (attrs)
    copy = pango_attr_list_copy (attrs);
  else
    copy = pango_attr_list_new ();
  attr = pango_attr_weight_new (weight);
  filtered = pango_attr_list_filter (copy, remove_weights, attr);
  pango_attr_list_insert (copy, attr);
  gtk_label_set_attributes (GTK_LABEL (self), copy);
  gtk_widget_queue_draw (GTK_WIDGET (self));
  pango_attr_list_unref (filtered);
  pango_attr_list_unref (copy);
}

void
dzl_bolding_label_set_bold (DzlBoldingLabel *self,
                            gboolean         bold)
{
  g_return_if_fail (DZL_IS_BOLDING_LABEL (self));

  dzl_bolding_label_set_weight (self, bold ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL);
}
