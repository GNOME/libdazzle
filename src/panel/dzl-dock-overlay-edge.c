/* dzl-dock-overlay-edge.c
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

#define G_LOG_DOMAIN "dzl-dock-overlay-edge"

#include "config.h"

#include "panel/dzl-dock-item.h"
#include "panel/dzl-dock-overlay-edge.h"
#include "panel/dzl-dock-paned.h"
#include "panel/dzl-dock-paned-private.h"
#include "panel/dzl-dock-stack.h"
#include "util/dzl-util-private.h"

struct _DzlDockOverlayEdge
{
  DzlBin          parent;
  GtkPositionType edge : 2;
  gint            position;
};

G_DEFINE_TYPE_EXTENDED (DzlDockOverlayEdge, dzl_dock_overlay_edge, DZL_TYPE_BIN, 0,
                        G_IMPLEMENT_INTERFACE (DZL_TYPE_DOCK_ITEM, NULL))

enum {
  PROP_0,
  PROP_EDGE,
  PROP_POSITION,
  N_PROPS
};

enum {
  STYLE_PROP_0,
  STYLE_PROP_OVERLAP_SIZE,
  STYLE_PROP_MNEMONIC_OVERLAP_SIZE,
  N_STYLE_PROPS
};

static GParamSpec *properties [N_PROPS];
static GParamSpec *style_properties [N_STYLE_PROPS];

static void
dzl_dock_overlay_edge_update_edge (DzlDockOverlayEdge *self)
{
  GtkWidget *child;
  GtkPositionType edge;
  GtkStyleContext *style_context;
  GtkOrientation orientation;
  const gchar *style_class;

  g_assert (DZL_IS_DOCK_OVERLAY_EDGE (self));

  style_context = gtk_widget_get_style_context (GTK_WIDGET (self));

  gtk_style_context_remove_class (style_context, "left");
  gtk_style_context_remove_class (style_context, "right");
  gtk_style_context_remove_class (style_context, "top");
  gtk_style_context_remove_class (style_context, "bottom");

  switch (self->edge)
    {
    case GTK_POS_TOP:
      edge = GTK_POS_BOTTOM;
      orientation = GTK_ORIENTATION_HORIZONTAL;
      style_class = "top";
      break;

    case GTK_POS_BOTTOM:
      edge = GTK_POS_TOP;
      orientation = GTK_ORIENTATION_HORIZONTAL;
      style_class = "bottom";
      break;

    case GTK_POS_LEFT:
      edge = GTK_POS_RIGHT;
      orientation = GTK_ORIENTATION_VERTICAL;
      style_class = "left";
      break;

    case GTK_POS_RIGHT:
      edge = GTK_POS_LEFT;
      orientation = GTK_ORIENTATION_VERTICAL;
      style_class = "right";
      break;

    default:
      g_assert_not_reached ();
      return;
    }

  gtk_style_context_add_class (style_context, style_class);

  child = gtk_bin_get_child (GTK_BIN (self));

  if (DZL_IS_DOCK_PANED (child))
    {
      gtk_orientable_set_orientation (GTK_ORIENTABLE (child), orientation);
      dzl_dock_paned_set_child_edge (DZL_DOCK_PANED (child), edge);
    }
  else if (DZL_IS_DOCK_STACK (child))
    {
      dzl_dock_stack_set_edge (DZL_DOCK_STACK (child), edge);
    }
}

static void
dzl_dock_overlay_edge_add (GtkContainer *container,
                           GtkWidget    *child)
{
  DzlDockOverlayEdge *self = (DzlDockOverlayEdge *)container;

  g_assert (DZL_IS_DOCK_OVERLAY_EDGE (self));
  g_assert (GTK_IS_WIDGET (child));

  GTK_CONTAINER_CLASS (dzl_dock_overlay_edge_parent_class)->add (container, child);

  dzl_dock_overlay_edge_update_edge (self);

  if (DZL_IS_DOCK_ITEM (child))
    dzl_dock_item_adopt (DZL_DOCK_ITEM (self), DZL_DOCK_ITEM (child));
}

static void
dzl_dock_overlay_edge_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  DzlDockOverlayEdge *self = DZL_DOCK_OVERLAY_EDGE (object);

  switch (prop_id)
    {
    case PROP_EDGE:
      g_value_set_enum (value, dzl_dock_overlay_edge_get_edge (self));
      break;

    case PROP_POSITION:
      g_value_set_int (value, dzl_dock_overlay_edge_get_position (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_dock_overlay_edge_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  DzlDockOverlayEdge *self = DZL_DOCK_OVERLAY_EDGE (object);

  switch (prop_id)
    {
    case PROP_EDGE:
      dzl_dock_overlay_edge_set_edge (self, g_value_get_enum (value));
      break;

    case PROP_POSITION:
      dzl_dock_overlay_edge_set_position (self, g_value_get_int (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_dock_overlay_edge_class_init (DzlDockOverlayEdgeClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = dzl_dock_overlay_edge_get_property;
  object_class->set_property = dzl_dock_overlay_edge_set_property;

  container_class->add = dzl_dock_overlay_edge_add;

  properties [PROP_EDGE] =
    g_param_spec_enum ("edge",
                       "Edge",
                       "Edge",
                       GTK_TYPE_POSITION_TYPE,
                       GTK_POS_LEFT,
                       (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_POSITION] =
    g_param_spec_int ("position",
                      "Position",
                      "The size of the edge",
                      0,
                      G_MAXINT,
                      0,
                      (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  style_properties [STYLE_PROP_MNEMONIC_OVERLAP_SIZE] =
    g_param_spec_int ("mnemonic-overlap-size",
                      "Mnemonic Overlap Size",
                      "The amount of pixels to overlap when mnemonics are visible",
                      0,
                      G_MAXINT,
                      30,
                      (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  gtk_widget_class_install_style_property (widget_class,
                                           style_properties [STYLE_PROP_MNEMONIC_OVERLAP_SIZE]);

  style_properties [STYLE_PROP_OVERLAP_SIZE] =
    g_param_spec_int ("overlap-size",
                      "Overlap Size",
                      "The amount of pixels to overlap when hidden",
                      0,
                      G_MAXINT,
                      5,
                      (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  gtk_widget_class_install_style_property (widget_class,
                                           style_properties [STYLE_PROP_OVERLAP_SIZE]);

  gtk_widget_class_set_css_name (widget_class, "dzldockoverlayedge");
}

static void
dzl_dock_overlay_edge_init (DzlDockOverlayEdge *self)
{
}

gint
dzl_dock_overlay_edge_get_position (DzlDockOverlayEdge *self)
{
  g_return_val_if_fail (DZL_IS_DOCK_OVERLAY_EDGE (self), 0);

  return self->position;
}

void
dzl_dock_overlay_edge_set_position (DzlDockOverlayEdge *self,
                                    gint                position)
{
  g_return_if_fail (DZL_IS_DOCK_OVERLAY_EDGE (self));
  g_return_if_fail (position >= 0);

  if (position != self->position)
    {
      self->position = position;
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_POSITION]);
    }
}

GtkPositionType
dzl_dock_overlay_edge_get_edge (DzlDockOverlayEdge *self)
{
  g_return_val_if_fail (DZL_IS_DOCK_OVERLAY_EDGE (self), 0);

  return self->edge;
}

void
dzl_dock_overlay_edge_set_edge (DzlDockOverlayEdge *self,
                                GtkPositionType     edge)
{
  g_return_if_fail (DZL_IS_DOCK_OVERLAY_EDGE (self));
  g_return_if_fail (edge >= 0);
  g_return_if_fail (edge <= 3);

  if (edge != self->edge)
    {
      self->edge = edge;
      dzl_dock_overlay_edge_update_edge (self);
      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_EDGE]);
    }
}
