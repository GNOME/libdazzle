/* dzl-suggestion-row.c
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
 *
 * This file is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define G_LOG_DOMAIN "dzl-suggestion-row"

#include "config.h"

#include "dzl-suggestion-row.h"

typedef struct
{
  DzlSuggestion *suggestion;

  GtkImage      *image;
  GtkLabel      *title;
  GtkLabel      *separator;
  GtkLabel      *subtitle;
} DzlSuggestionRowPrivate;

enum {
  PROP_0,
  PROP_SUGGESTION,
  N_PROPS
};

G_DEFINE_TYPE_WITH_PRIVATE (DzlSuggestionRow, dzl_suggestion_row, DZL_TYPE_LIST_BOX_ROW)

static GParamSpec *properties [N_PROPS];

static void
dzl_suggestion_row_disconnect (DzlSuggestionRow *self)
{
  DzlSuggestionRowPrivate *priv = dzl_suggestion_row_get_instance_private (self);

  g_return_if_fail (DZL_IS_SUGGESTION_ROW (self));

  if (priv->suggestion == NULL)
    return;

  g_object_set (priv->image, "icon-name", NULL, NULL);
  gtk_label_set_label (priv->title, NULL);
  gtk_label_set_label (priv->subtitle, NULL);
}

static void
dzl_suggestion_row_connect (DzlSuggestionRow *self)
{
  DzlSuggestionRowPrivate *priv = dzl_suggestion_row_get_instance_private (self);
  const gchar *icon_name;
  const gchar *subtitle;

  g_return_if_fail (DZL_IS_SUGGESTION_ROW (self));
  g_return_if_fail (priv->suggestion != NULL);

  icon_name = dzl_suggestion_get_icon_name (priv->suggestion);
  g_object_set (priv->image, "icon-name", icon_name, NULL);

  gtk_label_set_label (priv->title, dzl_suggestion_get_title (priv->suggestion));

  subtitle = dzl_suggestion_get_subtitle (priv->suggestion);
  gtk_label_set_label (priv->subtitle, subtitle);
  gtk_widget_set_visible (GTK_WIDGET (priv->separator), !!subtitle);
}

static void
dzl_suggestion_row_finalize (GObject *object)
{
  DzlSuggestionRow *self = (DzlSuggestionRow *)object;
  DzlSuggestionRowPrivate *priv = dzl_suggestion_row_get_instance_private (self);

  g_clear_object (&priv->suggestion);

  G_OBJECT_CLASS (dzl_suggestion_row_parent_class)->finalize (object);
}

static void
dzl_suggestion_row_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  DzlSuggestionRow *self = DZL_SUGGESTION_ROW (object);

  switch (prop_id)
    {
    case PROP_SUGGESTION:
      g_value_set_object (value, dzl_suggestion_row_get_suggestion (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_suggestion_row_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  DzlSuggestionRow *self = DZL_SUGGESTION_ROW (object);

  switch (prop_id)
    {
    case PROP_SUGGESTION:
      dzl_suggestion_row_set_suggestion (self, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_suggestion_row_class_init (DzlSuggestionRowClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = dzl_suggestion_row_finalize;
  object_class->get_property = dzl_suggestion_row_get_property;
  object_class->set_property = dzl_suggestion_row_set_property;

  properties [PROP_SUGGESTION] =
    g_param_spec_object ("suggestion",
                         "Suggestion",
                         "The suggestion to display",
                         DZL_TYPE_SUGGESTION,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/dazzle/ui/dzl-suggestion-row.ui");
  gtk_widget_class_bind_template_child_private (widget_class, DzlSuggestionRow, image);
  gtk_widget_class_bind_template_child_private (widget_class, DzlSuggestionRow, title);
  gtk_widget_class_bind_template_child_private (widget_class, DzlSuggestionRow, subtitle);
  gtk_widget_class_bind_template_child_private (widget_class, DzlSuggestionRow, separator);
}

static void
dzl_suggestion_row_init (DzlSuggestionRow *self)
{
  GtkStyleContext *context;

  gtk_widget_init_template (GTK_WIDGET (self));

  context = gtk_widget_get_style_context (GTK_WIDGET (self));
  gtk_style_context_add_class (context, "suggestion");
}

/**
 * dzl_suggestion_row_get_suggestion:
 * @self: a #DzlSuggestionRow
 *
 * Gets the suggestion to be displayed.
 *
 * Returns: (transfer none): An #DzlSuggestion
 */
DzlSuggestion *
dzl_suggestion_row_get_suggestion (DzlSuggestionRow *self)
{
  DzlSuggestionRowPrivate *priv = dzl_suggestion_row_get_instance_private (self);

  g_return_val_if_fail (DZL_IS_SUGGESTION_ROW (self), NULL);

  return priv->suggestion;
}

void
dzl_suggestion_row_set_suggestion (DzlSuggestionRow *self,
                                   DzlSuggestion    *suggestion)
{
  DzlSuggestionRowPrivate *priv = dzl_suggestion_row_get_instance_private (self);

  g_return_if_fail (DZL_IS_SUGGESTION_ROW (self));
  g_return_if_fail (!suggestion || DZL_IS_SUGGESTION (suggestion));

  if (priv->suggestion != suggestion)
    {
      if (priv->suggestion != NULL)
        {
          dzl_suggestion_row_disconnect (self);
          g_clear_object (&priv->suggestion);
        }

      if (suggestion != NULL)
        {
          priv->suggestion = g_object_ref (suggestion);
          dzl_suggestion_row_connect (self);
        }

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_SUGGESTION]);
    }
}

void
_dzl_suggestion_row_set_ellipsize (DzlSuggestionRow   *self,
                                   PangoEllipsizeMode  title,
                                   PangoEllipsizeMode  subtitle)
{
  DzlSuggestionRowPrivate *priv = dzl_suggestion_row_get_instance_private (self);

  g_assert (DZL_IS_SUGGESTION_ROW (self));

  gtk_label_set_ellipsize (priv->title, title);
  gtk_label_set_ellipsize (priv->subtitle, subtitle);
}
