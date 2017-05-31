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
 */

struct _DzlBoldingLabel
{
  GtkLabel parent_instance;
};

G_DEFINE_TYPE (DzlBoldingLabel, dzl_bolding_label, GTK_TYPE_LABEL)

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
      gint height;
      gint width;

      text = gtk_label_get_text (GTK_LABEL (widget));
      layout = gtk_widget_create_pango_layout (widget, text);

      if (font_desc != NULL)
        font_desc_copy = pango_font_description_copy (font_desc);
      else
        font_desc_copy = pango_font_description_new ();

      pango_font_description_set_weight (font_desc_copy, PANGO_WEIGHT_BOLD);
      pango_layout_set_font_description (layout, font_desc_copy);
      pango_layout_get_pixel_size (layout, &width, &height);

      if (width > *min_width)
        *min_width = width;

      if (width > *nat_width)
        *nat_width = width;

      pango_font_description_free (font_desc_copy);
      g_object_unref (layout);
    }
}

static void
dzl_bolding_label_class_init (DzlBoldingLabelClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  widget_class->get_preferred_width = dzl_bolding_label_get_preferred_width;
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
  pango_attr_list_filter (copy, remove_weights, attr);
  pango_attr_list_insert (copy, attr);
  gtk_label_set_attributes (GTK_LABEL (self), copy);
  gtk_widget_queue_draw (GTK_WIDGET (self));
  pango_attr_list_unref (copy);
}
