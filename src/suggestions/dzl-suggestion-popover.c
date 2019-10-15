/* dzl-suggestion-popover.c
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

#define G_LOG_DOMAIN "dzl-suggestion-popover"

#include "config.h"

#include <glib/gi18n.h>

#include "dzl-debug.h"

#include "animation/dzl-animation.h"
#include "shortcuts/dzl-shortcut-controller.h"
#include "shortcuts/dzl-shortcut-private.h"
#include "suggestions/dzl-suggestion.h"
#include "suggestions/dzl-suggestion-entry.h"
#include "suggestions/dzl-suggestion-popover.h"
#include "suggestions/dzl-suggestion-private.h"
#include "suggestions/dzl-suggestion-row.h"
#include "util/dzl-util-private.h"
#include "widgets/dzl-elastic-bin.h"
#include "widgets/dzl-list-box.h"
#include "widgets/dzl-list-box-private.h"

#define DELAYED_POPDOWN_MSEC 100

struct _DzlSuggestionPopover
{
  GtkWindow           parent_instance;

  GtkWidget          *relative_to;
  GtkWindow          *transient_for;
  GtkRevealer        *revealer;
  GtkScrolledWindow  *scrolled_window;
  DzlListBox         *list_box;
  GtkBox             *top_box;

  /* Used for listbox workaround with selection ordering */
  DzlSuggestionRow   *tmp_selected_row;

  DzlAnimation       *scroll_anim;

  GListModel         *model;

  GdkDevice          *grab_device;

  GType               row_type;

  gulong              delete_event_handler;
  gulong              configure_event_handler;
  gulong              size_allocate_handler;
  gulong              items_changed_handler;
  gulong              destroy_handler;

  guint               queued_popdown;

  PangoEllipsizeMode  subtitle_ellipsize;
  PangoEllipsizeMode  title_ellipsize;

  guint               popup_requested : 1;
  guint               entry_focused : 1;
  guint               has_grab : 1;
  guint               compact : 1;
};

enum {
  PROP_0,
  PROP_MODEL,
  PROP_RELATIVE_TO,
  PROP_SELECTED,
  PROP_SUBTITLE_ELLIPSIZE,
  PROP_TITLE_ELLIPSIZE,
  N_PROPS
};

enum {
  SUGGESTION_ACTIVATED,
  N_SIGNALS
};

static void dzl_suggestion_popover_set_transient_for (DzlSuggestionPopover *self,
                                                      GtkWindow            *window);

G_DEFINE_TYPE (DzlSuggestionPopover, dzl_suggestion_popover, GTK_TYPE_WINDOW)

static GParamSpec *properties [N_PROPS];
static guint signals [N_SIGNALS];

static gdouble
check_range (gdouble lower,
             gdouble page_size,
             gdouble y1,
             gdouble y2)
{
  gdouble upper = lower + page_size;

  /*
   * The goal here is to determine if y1 and y2 fall within lower and upper. We
   * will determine the new value to set to ensure the minimum amount to scroll
   * to show the rows.
   */

  /* Do nothing if we are all visible */
  if (y1 >= lower && y2 <= upper)
    return lower;

  /* We might need to scroll y2 into view */
  if (y2 > upper)
    return y2 - page_size;

  return y1;
}

static void
dzl_suggestion_popover_select_row (DzlSuggestionPopover *self,
                                   GtkListBoxRow        *row)
{
  GtkAdjustment *adj;
  GtkAllocation alloc;
  gdouble y1;
  gdouble y2;
  gdouble page_size;
  gdouble value;
  gdouble new_value;

  g_assert (DZL_IS_SUGGESTION_POPOVER (self));
  g_assert (GTK_IS_LIST_BOX_ROW (row));

  gtk_list_box_select_row (GTK_LIST_BOX (self->list_box), row);

  gtk_widget_get_allocation (GTK_WIDGET (row), &alloc);

  /* If there is no allocation yet, ignore things */
  if (alloc.y < 0)
    return;

  /*
   * We want to ensure that Y1 and Y2 are both within the
   * page of the scrolled window. If not, we need to animate
   * our scroll position to include that row.
   */

  y1 = alloc.y;
  y2 = alloc.y + alloc.height;

  adj = gtk_scrolled_window_get_vadjustment (self->scrolled_window);
  page_size = gtk_adjustment_get_page_size (adj);
  value = gtk_adjustment_get_value (adj);

  new_value = check_range (value, page_size, y1, y2);

  if (new_value != value)
    {
      DzlAnimation *anim;
      guint duration = 167;

      if (self->scroll_anim != NULL)
        {
          dzl_animation_stop (self->scroll_anim);

          /* Speed up the animation because we are scrolling while already
           * scrolling. We don't want to fall behind.
           */
          duration = 84;
        }

      anim = dzl_object_animate (adj,
                                 DZL_ANIMATION_EASE_IN_OUT_CUBIC,
                                 duration,
                                 gtk_widget_get_frame_clock (GTK_WIDGET (self->scrolled_window)),
                                 "value", new_value,
                                 NULL);
      dzl_set_weak_pointer (&self->scroll_anim, anim);
    }
}

static void
dzl_suggestion_popover_reposition (DzlSuggestionPopover *self)
{
  g_assert (DZL_IS_SUGGESTION_POPOVER (self));

  if (DZL_IS_SUGGESTION_ENTRY (self->relative_to))
    _dzl_suggestion_entry_reposition (DZL_SUGGESTION_ENTRY (self->relative_to), self);
}

/**
 * dzl_suggestion_popover_get_relative_to:
 * @self: a #DzlSuggestionPopover
 *
 * Returns: (transfer none) (nullable): A #GtkWidget or %NULL.
 */
GtkWidget *
dzl_suggestion_popover_get_relative_to (DzlSuggestionPopover *self)
{

  g_return_val_if_fail (DZL_IS_SUGGESTION_POPOVER (self), NULL);

  return self->relative_to;
}

void
dzl_suggestion_popover_set_relative_to (DzlSuggestionPopover *self,
                                        GtkWidget            *relative_to)
{
  g_return_if_fail (DZL_IS_SUGGESTION_POPOVER (self));
  g_return_if_fail (!relative_to || GTK_IS_WIDGET (relative_to));

  if (self->relative_to != relative_to)
    {
      if (self->relative_to != NULL)
        {
          g_signal_handlers_disconnect_by_func (self->relative_to,
                                                G_CALLBACK (gtk_widget_destroyed),
                                                &self->relative_to);
          self->relative_to = NULL;
        }

      if (relative_to != NULL)
        {
          self->relative_to = relative_to;
          g_signal_connect (self->relative_to,
                            "destroy",
                            G_CALLBACK (gtk_widget_destroyed),
                            &self->relative_to);
        }

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_RELATIVE_TO]);
    }
}

static void
dzl_suggestion_popover_hide (GtkWidget *widget)
{
  DzlSuggestionPopover *self = (DzlSuggestionPopover *)widget;

  g_return_if_fail (DZL_IS_SUGGESTION_POPOVER (self));

  dzl_suggestion_popover_set_transient_for (self, NULL);

  GTK_WIDGET_CLASS (dzl_suggestion_popover_parent_class)->hide (widget);
}

static void
dzl_suggestion_popover_transient_for_size_allocate (DzlSuggestionPopover *self,
                                                    GtkAllocation        *allocation,
                                                    GtkWindow            *toplevel)
{
  g_assert (DZL_IS_SUGGESTION_POPOVER (self));
  g_assert (allocation != NULL);
  g_assert (GTK_IS_WINDOW (toplevel));

  dzl_suggestion_popover_reposition (self);
}

static gboolean
dzl_suggestion_popover_transient_for_delete_event (DzlSuggestionPopover *self,
                                                   GdkEvent             *event,
                                                   GtkWindow            *toplevel)
{
  g_assert (DZL_IS_SUGGESTION_POPOVER (self));
  g_assert (event != NULL);
  g_assert (GTK_IS_WINDOW (toplevel));

  gtk_widget_hide (GTK_WIDGET (self));

  return FALSE;
}

static gboolean
dzl_suggestion_popover_transient_for_configure_event (DzlSuggestionPopover *self,
                                                      GdkEvent             *event,
                                                      GtkWindow            *toplevel)
{
  g_assert (DZL_IS_SUGGESTION_POPOVER (self));
  g_assert (event != NULL);
  g_assert (GTK_IS_WINDOW (toplevel));

  gtk_widget_hide (GTK_WIDGET (self));

  return FALSE;
}

static void
dzl_suggestion_popover_set_transient_for (DzlSuggestionPopover *self,
                                          GtkWindow            *window)
{
  g_assert (DZL_IS_SUGGESTION_POPOVER (self));
  g_assert (!window || GTK_IS_WINDOW (window));

  if (self->transient_for == window)
    return;

  if (self->transient_for != NULL)
    {
      g_signal_handler_disconnect (self->transient_for, self->size_allocate_handler);
      g_signal_handler_disconnect (self->transient_for, self->configure_event_handler);
      g_signal_handler_disconnect (self->transient_for, self->delete_event_handler);
      g_signal_handler_disconnect (self->transient_for, self->destroy_handler);

      self->size_allocate_handler = 0;
      self->configure_event_handler = 0;
      self->delete_event_handler = 0;
      self->destroy_handler = 0;
    }

  self->transient_for = window;

  if (self->transient_for != NULL)
    {
      GtkWindowGroup *group = gtk_window_get_group (self->transient_for);

      gtk_window_group_add_window (group, GTK_WINDOW (self));

      self->destroy_handler =
        g_signal_connect (self->transient_for,
                          "destroy",
                          G_CALLBACK (gtk_widget_destroyed),
                          &self->transient_for);
      self->delete_event_handler =
        g_signal_connect_object (self->transient_for,
                                 "delete-event",
                                 G_CALLBACK (dzl_suggestion_popover_transient_for_delete_event),
                                 self,
                                 G_CONNECT_SWAPPED);
      self->size_allocate_handler =
        g_signal_connect_object (self->transient_for,
                                 "size-allocate",
                                 G_CALLBACK (dzl_suggestion_popover_transient_for_size_allocate),
                                 self,
                                 G_CONNECT_SWAPPED | G_CONNECT_AFTER);
      self->configure_event_handler =
        g_signal_connect_object (self->transient_for,
                                 "configure-event",
                                 G_CALLBACK (dzl_suggestion_popover_transient_for_configure_event),
                                 self,
                                 G_CONNECT_SWAPPED);
    }
}

static void
dzl_suggestion_popover_show (GtkWidget *widget)
{
  DzlSuggestionPopover *self = (DzlSuggestionPopover *)widget;

  g_return_if_fail (DZL_IS_SUGGESTION_POPOVER (self));

  if (self->relative_to != NULL)
    {
      GtkWidget *toplevel = gtk_widget_get_ancestor (self->relative_to, GTK_TYPE_WINDOW);

      if (toplevel != NULL)
        {
          dzl_suggestion_popover_set_transient_for (self, GTK_WINDOW (toplevel));
          dzl_suggestion_popover_reposition (self);
        }
    }

  GTK_WIDGET_CLASS (dzl_suggestion_popover_parent_class)->show (widget);
}

static void
dzl_suggestion_popover_screen_changed (GtkWidget *widget,
                                       GdkScreen *previous_screen)
{
  GdkScreen *screen;
  GdkVisual *visual;

  GTK_WIDGET_CLASS (dzl_suggestion_popover_parent_class)->screen_changed (widget, previous_screen);

  screen = gtk_widget_get_screen (widget);
  visual = gdk_screen_get_rgba_visual (screen);

  if (visual != NULL)
    gtk_widget_set_visual (widget, visual);
}

static void
dzl_suggestion_popover_realize (GtkWidget *widget)
{
  DzlSuggestionPopover *self = (DzlSuggestionPopover *)widget;
  GdkScreen *screen;
  GdkVisual *visual;

  g_assert (DZL_IS_SUGGESTION_POPOVER (self));

  screen = gtk_widget_get_screen (widget);
  visual = gdk_screen_get_rgba_visual (screen);

  if (visual != NULL)
    gtk_widget_set_visual (widget, visual);

  GTK_WIDGET_CLASS (dzl_suggestion_popover_parent_class)->realize (widget);

  dzl_suggestion_popover_reposition (self);
}

static void
dzl_suggestion_popover_notify_child_revealed (DzlSuggestionPopover *self,
                                              GParamSpec           *pspec,
                                              GtkRevealer          *revealer)
{
  g_assert (DZL_IS_SUGGESTION_POPOVER (self));
  g_assert (GTK_IS_REVEALER (revealer));

  if (!gtk_revealer_get_reveal_child (self->revealer))
    gtk_widget_hide (GTK_WIDGET (self));
}

static void
dzl_suggestion_popover_list_box_row_activated (DzlSuggestionPopover *self,
                                               DzlSuggestionRow     *row,
                                               GtkListBox           *list_box)
{
  DzlSuggestion *suggestion;

  g_assert (DZL_IS_SUGGESTION_POPOVER (self));
  g_assert (DZL_IS_SUGGESTION_ROW (row));
  g_assert (GTK_IS_LIST_BOX (list_box));

  suggestion = dzl_suggestion_row_get_suggestion (row);
  g_signal_emit (self, signals [SUGGESTION_ACTIVATED], 0, suggestion);
}

static void
dzl_suggestion_popover_list_box_row_selected (DzlSuggestionPopover *self,
                                              DzlSuggestionRow     *row,
                                              GtkListBox           *list_box)
{
  g_assert (DZL_IS_SUGGESTION_POPOVER (self));
  g_assert (!row || DZL_IS_SUGGESTION_ROW (row));
  g_assert (GTK_IS_LIST_BOX (list_box));

  /* GtkListBox doesn't necessarily give us @row back when we call
   * gtk_list_box_get_selected_row(), so this workaround allows us
   * to continue using that API after checking for this pointer.
   */
  self->tmp_selected_row = DZL_SUGGESTION_ROW (row);
  g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_SELECTED]);
  self->tmp_selected_row = NULL;
}

static void
update_ellipsize_cb (GtkWidget *widget,
                     gpointer   user_data)
{
  DzlSuggestionPopover *self = user_data;

  _dzl_suggestion_row_set_ellipsize (DZL_SUGGESTION_ROW (widget),
                                     self->title_ellipsize,
                                     self->subtitle_ellipsize);
}

static void
dzl_suggestion_popover_set_title_ellipsize (DzlSuggestionPopover *self,
                                            PangoEllipsizeMode    title_ellipsize)
{
  g_assert (DZL_IS_SUGGESTION_POPOVER (self));

  self->title_ellipsize = title_ellipsize;

  _dzl_list_box_forall (self->list_box, update_ellipsize_cb, self);
}

static void
dzl_suggestion_popover_set_subtitle_ellipsize (DzlSuggestionPopover *self,
                                               PangoEllipsizeMode    subtitle_ellipsize)
{
  g_assert (DZL_IS_SUGGESTION_POPOVER (self));

  self->subtitle_ellipsize = subtitle_ellipsize;

  _dzl_list_box_forall (self->list_box, update_ellipsize_cb, self);
}

static void
attach_cb (DzlListBox    *list_box,
           DzlListBoxRow *row,
           gpointer       user_data)
{
  DzlSuggestionPopover *self = user_data;

  g_assert (DZL_IS_SUGGESTION_POPOVER (self));
  g_assert (DZL_IS_SUGGESTION_ROW (row));
  g_assert (DZL_IS_LIST_BOX (list_box));

  _dzl_suggestion_row_set_ellipsize (DZL_SUGGESTION_ROW (row),
                                     self->title_ellipsize,
                                     self->subtitle_ellipsize);
  gtk_orientable_set_orientation (GTK_ORIENTABLE (row), self->compact ? GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL);
}

static gboolean
dzl_suggestion_popover_key_press_event (GtkWidget   *widget,
                                        GdkEventKey *event)
{
  DzlSuggestionPopover *self = (DzlSuggestionPopover *)widget;

  g_assert (DZL_IS_SUGGESTION_POPOVER (self));
  g_assert (event != NULL);

  if (self->relative_to != NULL)
    {
      DzlShortcutController *controller = dzl_shortcut_controller_try_find (self->relative_to);
      g_autoptr(DzlShortcutChord) chord = NULL;

      /* NOTE: This only allows for shortcuts on the target entry, not any
       *       global shortcut. That is similar to how GtkEntryCompletion works
       *       and ensures that we don't have state taken during global dispatch.
       */

      if (controller != NULL &&
          (chord = dzl_shortcut_chord_new_from_event (event)))
        {
          DzlShortcutMatch match;

          match = _dzl_shortcut_controller_handle (controller,
                                                   event,
                                                   chord,
                                                   DZL_SHORTCUT_PHASE_DISPATCH,
                                                   self->relative_to);

          if (match == DZL_SHORTCUT_MATCH_EQUAL)
            return TRUE;
        }

      return gtk_widget_event (self->relative_to, (GdkEvent *)event);
    }

  return GTK_WIDGET_CLASS (dzl_suggestion_popover_parent_class)->key_press_event (widget, event);
}

static gboolean
dzl_suggestion_popover_key_release_event (GtkWidget   *widget,
                                          GdkEventKey *event)
{
  DzlSuggestionPopover *self = (DzlSuggestionPopover *)widget;

  g_assert (DZL_IS_SUGGESTION_POPOVER (self));
  g_assert (event != NULL);

  if (self->relative_to != NULL)
    return gtk_widget_event (self->relative_to, (GdkEvent *)event);

  return GTK_WIDGET_CLASS (dzl_suggestion_popover_parent_class)->key_release_event (widget, event);
}

static gboolean
dzl_suggestion_popover_grab_broken_event (GtkWidget          *widget,
                                          GdkEventGrabBroken *event)
{
  DzlSuggestionPopover *self = (DzlSuggestionPopover *)widget;

  g_assert (DZL_IS_SUGGESTION_POPOVER (self));
  g_assert (event != NULL);

  dzl_suggestion_popover_popdown (self);

  return GTK_WIDGET_CLASS (dzl_suggestion_popover_parent_class)->grab_broken_event (widget, event);
}

static gboolean
dzl_suggestion_popover_button_release_event (GtkWidget      *widget,
                                             GdkEventButton *event)
{
  DzlSuggestionPopover *self = (DzlSuggestionPopover *)widget;
  gboolean ret;

  g_assert (DZL_IS_SUGGESTION_POPOVER (self));
  g_assert (event != NULL);

  ret = GTK_WIDGET_CLASS (dzl_suggestion_popover_parent_class)->button_release_event (widget, event);

  dzl_suggestion_popover_popdown (self);

  /* Force immediate hide */
  gtk_widget_hide (GTK_WIDGET (self));

  return ret;
}

static void
dzl_suggestion_popover_destroy (GtkWidget *widget)
{
  DzlSuggestionPopover *self = (DzlSuggestionPopover *)widget;

  dzl_clear_source (&self->queued_popdown);
  g_clear_object (&self->grab_device);

  dzl_suggestion_popover_set_transient_for (self, NULL);

  if (self->scroll_anim != NULL)
    {
      dzl_animation_stop (self->scroll_anim);
      dzl_clear_weak_pointer (&self->scroll_anim);
    }

  g_clear_object (&self->model);

  dzl_suggestion_popover_set_relative_to (self, NULL);

  GTK_WIDGET_CLASS (dzl_suggestion_popover_parent_class)->destroy (widget);
}

static void
dzl_suggestion_popover_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  DzlSuggestionPopover *self = DZL_SUGGESTION_POPOVER (object);

  switch (prop_id)
    {
    case PROP_MODEL:
      g_value_set_object (value, dzl_suggestion_popover_get_model (self));
      break;

    case PROP_RELATIVE_TO:
      g_value_set_object (value, dzl_suggestion_popover_get_relative_to (self));
      break;

    case PROP_SELECTED:
      g_value_set_object (value, dzl_suggestion_popover_get_selected (self));
      break;

    case PROP_SUBTITLE_ELLIPSIZE:
      g_value_set_enum (value, self->subtitle_ellipsize);
      break;

    case PROP_TITLE_ELLIPSIZE:
      g_value_set_enum (value, self->title_ellipsize);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_suggestion_popover_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  DzlSuggestionPopover *self = DZL_SUGGESTION_POPOVER (object);

  switch (prop_id)
    {
    case PROP_MODEL:
      dzl_suggestion_popover_set_model (self, g_value_get_object (value));
      break;

    case PROP_RELATIVE_TO:
      dzl_suggestion_popover_set_relative_to (self, g_value_get_object (value));
      break;

    case PROP_SELECTED:
      dzl_suggestion_popover_set_selected (self, g_value_get_object (value));
      break;

    case PROP_SUBTITLE_ELLIPSIZE:
      dzl_suggestion_popover_set_subtitle_ellipsize (self, g_value_get_enum (value));
      break;

    case PROP_TITLE_ELLIPSIZE:
      dzl_suggestion_popover_set_title_ellipsize (self, g_value_get_enum (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_suggestion_popover_class_init (DzlSuggestionPopoverClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->get_property = dzl_suggestion_popover_get_property;
  object_class->set_property = dzl_suggestion_popover_set_property;

  widget_class->destroy = dzl_suggestion_popover_destroy;
  widget_class->hide = dzl_suggestion_popover_hide;
  widget_class->screen_changed = dzl_suggestion_popover_screen_changed;
  widget_class->realize = dzl_suggestion_popover_realize;
  widget_class->show = dzl_suggestion_popover_show;
  widget_class->button_release_event = dzl_suggestion_popover_button_release_event;
  widget_class->key_press_event = dzl_suggestion_popover_key_press_event;
  widget_class->key_release_event = dzl_suggestion_popover_key_release_event;
  widget_class->grab_broken_event = dzl_suggestion_popover_grab_broken_event;

  properties [PROP_MODEL] =
    g_param_spec_object ("model",
                         "Model",
                         "The model to be visualized",
                         DZL_TYPE_SUGGESTION,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_RELATIVE_TO] =
    g_param_spec_object ("relative-to",
                         "Relative To",
                         "The widget to be relative to",
                         GTK_TYPE_WIDGET,
                         (G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS));

  properties [PROP_SELECTED] =
    g_param_spec_object ("selected",
                         "Selected",
                         "The selected suggestion",
                         DZL_TYPE_SUGGESTION,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_SUBTITLE_ELLIPSIZE] =
    g_param_spec_enum ("subtitle-ellipsize",
                       "Subtitle Ellipsize",
                       "How to use ellipsis with subtitle",
                       PANGO_TYPE_ELLIPSIZE_MODE,
                       PANGO_ELLIPSIZE_END,
                       (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_TITLE_ELLIPSIZE] =
    g_param_spec_enum ("title-ellipsize",
                       "Title Ellipsize",
                       "How to use ellipsis with title",
                       PANGO_TYPE_ELLIPSIZE_MODE,
                       PANGO_ELLIPSIZE_END,
                       (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);

  signals [SUGGESTION_ACTIVATED] =
    g_signal_new ("suggestion-activated",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL, NULL,
                  G_TYPE_NONE, 1, DZL_TYPE_SUGGESTION);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/dazzle/ui/dzl-suggestion-popover.ui");
  gtk_widget_class_bind_template_child (widget_class, DzlSuggestionPopover, revealer);
  gtk_widget_class_bind_template_child (widget_class, DzlSuggestionPopover, list_box);
  gtk_widget_class_bind_template_child (widget_class, DzlSuggestionPopover, scrolled_window);
  gtk_widget_class_bind_template_child (widget_class, DzlSuggestionPopover, top_box);

  gtk_widget_class_set_css_name (widget_class, "dzlsuggestionpopover");

  g_type_ensure (DZL_TYPE_ELASTIC_BIN);
  g_type_ensure (DZL_TYPE_LIST_BOX);
  g_type_ensure (DZL_TYPE_SUGGESTION_ROW);
}

static void
dzl_suggestion_popover_init (DzlSuggestionPopover *self)
{
  self->row_type = DZL_TYPE_SUGGESTION_ROW;
  self->title_ellipsize = PANGO_ELLIPSIZE_END;
  self->subtitle_ellipsize = PANGO_ELLIPSIZE_END;

  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_window_set_type_hint (GTK_WINDOW (self), GDK_WINDOW_TYPE_HINT_COMBO);
  gtk_window_set_skip_pager_hint (GTK_WINDOW (self), TRUE);
  gtk_window_set_skip_taskbar_hint (GTK_WINDOW (self), TRUE);
  gtk_window_set_decorated (GTK_WINDOW (self), FALSE);
  gtk_window_set_resizable (GTK_WINDOW (self), FALSE);

  g_signal_connect_object (self->revealer,
                           "notify::child-revealed",
                           G_CALLBACK (dzl_suggestion_popover_notify_child_revealed),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (self->list_box,
                           "row-activated",
                           G_CALLBACK (dzl_suggestion_popover_list_box_row_activated),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (self->list_box,
                           "row-selected",
                           G_CALLBACK (dzl_suggestion_popover_list_box_row_selected),
                           self,
                           G_CONNECT_SWAPPED);

  _dzl_list_box_set_attach_func (self->list_box, attach_cb, self);

  dzl_list_box_set_recycle_max (self->list_box, 50);
}

GtkWidget *
dzl_suggestion_popover_new (void)
{
  return g_object_new (DZL_TYPE_SUGGESTION_POPOVER, NULL);
}

void
dzl_suggestion_popover_popup (DzlSuggestionPopover *self)
{
  GdkScreen *screen = NULL;
  guint duration = 250;
  guint n_items;

  g_assert (DZL_IS_SUGGESTION_POPOVER (self));

  if (self->model == NULL || 0 == (n_items = g_list_model_get_n_items (self->model)))
    {
      self->popup_requested = TRUE;
      return;
    }

  if (gtk_widget_get_mapped (GTK_WIDGET (self)) &&
      gtk_revealer_get_reveal_child (self->revealer))
    return;

  if (self->relative_to != NULL)
    {
      GdkDisplay *display;
      GdkMonitor *monitor;
      GdkWindow *window;
      GtkAllocation alloc;
      gint min_height;
      gint nat_height;

      display = gtk_widget_get_display (GTK_WIDGET (self->relative_to));
      window = gtk_widget_get_window (GTK_WIDGET (self->relative_to));
      monitor = gdk_display_get_monitor_at_window (display, window);
      screen = gtk_widget_get_screen (GTK_WIDGET (self->relative_to));

      gtk_window_set_screen (GTK_WINDOW (self), screen);

      gtk_widget_get_preferred_height (GTK_WIDGET (self), &min_height, &nat_height);
      gtk_widget_get_allocation (GTK_WIDGET (self), &alloc);

      duration = dzl_animation_calculate_duration (monitor, alloc.height, nat_height);
    }

  gtk_widget_grab_focus (GTK_WIDGET (self));
  gtk_widget_show (GTK_WIDGET (self));

  if (!self->has_grab && self->grab_device != NULL)
    {
      self->has_grab = TRUE;
      gtk_grab_add (GTK_WIDGET (self));
      gdk_seat_grab (gdk_device_get_seat (self->grab_device),
                     gtk_widget_get_window (GTK_WIDGET (self)),
                     GDK_SEAT_CAPABILITY_POINTER | GDK_SEAT_CAPABILITY_TOUCH,
                     TRUE, NULL, NULL, NULL, NULL);
    }

  gtk_revealer_set_transition_duration (self->revealer, duration);
  gtk_revealer_set_reveal_child (self->revealer, TRUE);
}

void
dzl_suggestion_popover_popdown (DzlSuggestionPopover *self)
{
  GtkAllocation alloc;
  GdkDisplay *display;
  GdkMonitor *monitor;
  GdkWindow *window;
  guint duration;

  g_assert (DZL_IS_SUGGESTION_POPOVER (self));

  self->popup_requested = FALSE;

  if (self->has_grab)
    {
      self->has_grab = FALSE;
      gtk_grab_remove (GTK_WIDGET (self));

      if (self->grab_device != NULL)
        {
          gdk_seat_ungrab (gdk_device_get_seat (self->grab_device));
          g_clear_object (&self->grab_device);
        }
    }


  if (!gtk_widget_get_mapped (GTK_WIDGET (self)))
    return;

  display = gtk_widget_get_display (GTK_WIDGET (self->relative_to));
  window = gtk_widget_get_window (GTK_WIDGET (self->relative_to));
  monitor = gdk_display_get_monitor_at_window (display, window);

  gtk_widget_get_allocation (GTK_WIDGET (self), &alloc);

  duration = dzl_animation_calculate_duration (monitor, alloc.height, 0);

  gtk_revealer_set_transition_duration (self->revealer, duration);
  gtk_revealer_set_reveal_child (self->revealer, FALSE);
}

static gboolean
dzl_suggestion_popover_do_queued_popdown (gpointer data)
{
  DzlSuggestionPopover *self = data;

  self->queued_popdown = 0;

  if (self->model != NULL && g_list_model_get_n_items (self->model) == 0)
    dzl_suggestion_popover_popdown (self);

  return G_SOURCE_REMOVE;
}

static void
dzl_suggestion_popover_queue_popdown (DzlSuggestionPopover *self)
{
  g_assert (DZL_IS_SUGGESTION_POPOVER (self));

  dzl_clear_source (&self->queued_popdown);
  self->queued_popdown = gdk_threads_add_timeout (DELAYED_POPDOWN_MSEC,
                                                  dzl_suggestion_popover_do_queued_popdown,
                                                  self);
}

static void
dzl_suggestion_popover_items_changed (DzlSuggestionPopover *self,
                                      guint                 position,
                                      guint                 removed,
                                      guint                 added,
                                      GListModel           *model)
{
  DZL_ENTRY;

  g_assert (DZL_IS_SUGGESTION_POPOVER (self));
  g_assert (G_IS_LIST_MODEL (model));

  DZL_TRACE_MSG ("removed=%d, added=%d, requested=%d",
                 removed, added, self->popup_requested);

  if (g_list_model_get_n_items (model) == 0)
    {
      /* do not immediately dismiss, because this might be an
       * intermittant event only to be followed by an addition
       * of new items to the GListModel.
       */
      dzl_suggestion_popover_queue_popdown (self);
      DZL_EXIT;
    }

  dzl_clear_source (&self->queued_popdown);

  if (self->popup_requested)
    {
      dzl_suggestion_popover_popup (self);
      self->popup_requested = FALSE;
      DZL_EXIT;
    }

  if (gtk_widget_get_mapped (GTK_WIDGET (self)) &&
      gtk_revealer_get_child_revealed (self->revealer) &&
      gtk_revealer_get_reveal_child (self->revealer))
    DZL_EXIT;

  /*
   * If we are currently animating in the initial view of the popover,
   * then we might need to cancel that animation and rely on the elastic
   * bin for smooth resizing.
   */
  if (gtk_revealer_get_reveal_child (self->revealer) &&
      !gtk_revealer_get_child_revealed (self->revealer) &&
      (removed || added))
    {
      g_signal_handlers_block_by_func (self->revealer,
                                       G_CALLBACK (dzl_suggestion_popover_notify_child_revealed),
                                       self);
      gtk_revealer_set_transition_duration (self->revealer, 0);
      gtk_revealer_set_reveal_child (self->revealer, FALSE);
      gtk_revealer_set_reveal_child (self->revealer, TRUE);
      g_signal_handlers_unblock_by_func (self->revealer,
                                         G_CALLBACK (dzl_suggestion_popover_notify_child_revealed),
                                         self);
    }
  else if (self->entry_focused)
    {
      dzl_suggestion_popover_popup (self);
      self->popup_requested = FALSE;
      DZL_EXIT;
    }

  DZL_EXIT;
}

static void
dzl_suggestion_popover_connect (DzlSuggestionPopover *self)
{
  g_assert (DZL_IS_SUGGESTION_POPOVER (self));

  if (self->model == NULL)
    return;

  dzl_list_box_set_model (self->list_box, self->model);

  self->items_changed_handler =
    g_signal_connect_object (self->model,
                             "items-changed",
                             G_CALLBACK (dzl_suggestion_popover_items_changed),
                             self,
                             G_CONNECT_AFTER | G_CONNECT_SWAPPED);

  if (g_list_model_get_n_items (self->model) == 0)
    {
      dzl_suggestion_popover_popdown (self);
      return;
    }

  /* select the first row */
  dzl_suggestion_popover_move_by (self, 1);

  /* If we started animating, cancel it to avoid such jarring movement. */
  if (self->scroll_anim != NULL)
    {
      dzl_animation_stop (self->scroll_anim);
      dzl_clear_weak_pointer (&self->scroll_anim);
    }

  gtk_adjustment_set_value (gtk_scrolled_window_get_vadjustment (self->scrolled_window), 0.0);
}

static void
dzl_suggestion_popover_disconnect (DzlSuggestionPopover *self)
{
  g_assert (DZL_IS_SUGGESTION_POPOVER (self));

  if (self->model == NULL)
    return;

  g_signal_handler_disconnect (self->model, self->items_changed_handler);
  self->items_changed_handler = 0;

  dzl_list_box_set_model (self->list_box, NULL);
}

void
dzl_suggestion_popover_set_model (DzlSuggestionPopover *self,
                                  GListModel           *model)
{
  g_return_if_fail (DZL_IS_SUGGESTION_POPOVER (self));
  g_return_if_fail (!model || G_IS_LIST_MODEL (model));
  g_return_if_fail (!model || g_type_is_a (g_list_model_get_item_type (model), DZL_TYPE_SUGGESTION));

  if (self->model != model)
    {
      if (self->model != NULL)
        {
          dzl_suggestion_popover_disconnect (self);
          g_clear_object (&self->model);
        }

      if (model != NULL)
        {
          self->model = g_object_ref (model);
          dzl_suggestion_popover_connect (self);

          if (self->popup_requested)
            dzl_suggestion_popover_popup (self);
        }

      self->popup_requested = FALSE;

      g_object_notify_by_pspec (G_OBJECT (self), properties [PROP_MODEL]);
    }
}

/**
 * dzl_suggestion_popover_get_model:
 * @self: a #DzlSuggestionPopover
 *
 * Gets the model being visualized.
 *
 * Returns: (nullable) (transfer none): A #GListModel or %NULL.
 */
GListModel *
dzl_suggestion_popover_get_model (DzlSuggestionPopover *self)
{
  g_return_val_if_fail (DZL_IS_SUGGESTION_POPOVER (self), NULL);

  return self->model;
}

static void
find_index_of_row (GtkWidget *widget,
                   gpointer   user_data)
{
  struct {
    GtkWidget *row;
    gint       index;
    gint       counter;
  } *row_lookup = user_data;

  if (widget == row_lookup->row)
    row_lookup->index = row_lookup->counter;

  row_lookup->counter++;
}

void
dzl_suggestion_popover_move_by (DzlSuggestionPopover *self,
                                gint                  amount)
{
  GtkListBoxRow *row;
  struct {
    GtkWidget *row;
    gint       index;
    gint       counter;
  } row_lookup;

  g_return_if_fail (DZL_IS_SUGGESTION_POPOVER (self));

  if (!(row = gtk_list_box_get_row_at_index (GTK_LIST_BOX (self->list_box), 0)))
    return;

  if (!gtk_list_box_get_selected_row (GTK_LIST_BOX (self->list_box)))
    {
      if (amount < 0)
        {
          guint n_items = g_list_model_get_n_items (self->model);

          if (n_items > 0)
            row = gtk_list_box_get_row_at_index (GTK_LIST_BOX (self->list_box), n_items - 1);
        }

      dzl_suggestion_popover_select_row (self, row);
      return;
    }

  /*
   * It would be nice if we have a bit better API to have control over
   * this from GtkListBox. move-cursor isn't really sufficient for our
   * control over position without updating the focus.
   *
   * We could look at doing focus redirection to the popover first,
   * but that isn't exactly a clean solution either and suggests that
   * we subclass the listbox too.
   *
   * This is really inefficient, but in general we won't have that
   * many results, becuase showin the user a ton of results is not
   * exactly useful.
   *
   * If we do decide to reuse this class in something autocompletion
   * in a text editor, we'll want to restrategize (including avoiding
   * the creation of unnecessary rows and row reuse).
   */
  row = gtk_list_box_get_selected_row (GTK_LIST_BOX (self->list_box));

  row_lookup.row = GTK_WIDGET (row);
  row_lookup.counter = 0;
  row_lookup.index = -1;

  gtk_container_foreach (GTK_CONTAINER (self->list_box),
                         find_index_of_row,
                         &row_lookup);

  row_lookup.index += amount;
  row_lookup.index = CLAMP (row_lookup.index, -0, (gint)g_list_model_get_n_items (self->model) - 1);

  row = gtk_list_box_get_row_at_index (GTK_LIST_BOX (self->list_box), row_lookup.index);
  dzl_suggestion_popover_select_row (self, row);
}

static void
find_suggestion_row (GtkWidget *widget,
                     gpointer   user_data)
{
  DzlSuggestionRow *row = DZL_SUGGESTION_ROW (widget);
  DzlSuggestion *suggestion = dzl_suggestion_row_get_suggestion (row);
  struct {
    DzlSuggestion  *suggestion;
    GtkListBoxRow **row;
  } *lookup = user_data;

  if (suggestion == lookup->suggestion)
    *lookup->row = GTK_LIST_BOX_ROW (row);
}

void
dzl_suggestion_popover_set_selected (DzlSuggestionPopover *self,
                                     DzlSuggestion        *suggestion)
{
  GtkListBoxRow *row = NULL;
  struct {
    DzlSuggestion  *suggestion;
    GtkListBoxRow **row;
  } lookup = { suggestion, &row };

  g_return_if_fail (DZL_IS_SUGGESTION_POPOVER (self));
  g_return_if_fail (!suggestion || DZL_IS_SUGGESTION (suggestion));

  if (suggestion == NULL)
    row = gtk_list_box_get_row_at_index (GTK_LIST_BOX (self->list_box), 0);
  else
    gtk_container_foreach (GTK_CONTAINER (self->list_box), find_suggestion_row, &lookup);

  if (row != NULL)
    dzl_suggestion_popover_select_row (self, row);
}

/**
 * dzl_suggestion_popover_get_selected:
 * @self: a #DzlSuggestionPopover
 *
 * Gets the currently selected suggestion.
 *
 * Returns: (transfer none) (nullable): An #DzlSuggestion or %NULL.
 */
DzlSuggestion *
dzl_suggestion_popover_get_selected (DzlSuggestionPopover *self)
{
  DzlSuggestionRow *row;

  g_return_val_if_fail (DZL_IS_SUGGESTION_POPOVER (self), NULL);

  /* Work around row selection wonkiness in GtkListBox */
  if (self->tmp_selected_row)
    row = self->tmp_selected_row;
  else
    row = DZL_SUGGESTION_ROW (gtk_list_box_get_selected_row (GTK_LIST_BOX (self->list_box)));

  if (row != NULL)
    return dzl_suggestion_row_get_suggestion (row);

  return NULL;
}

void
dzl_suggestion_popover_activate_selected (DzlSuggestionPopover *self)
{
  DzlSuggestion *suggestion;

  g_return_if_fail (DZL_IS_SUGGESTION_POPOVER (self));

  if (NULL != (suggestion = dzl_suggestion_popover_get_selected (self)))
    g_signal_emit (self, signals [SUGGESTION_ACTIVATED], 0, suggestion);
}

void
_dzl_suggestion_popover_set_max_height (DzlSuggestionPopover *self,
                                        gint                  max_height)
{
  GdkWindow *window;
  gint clip_height = 3000; /* something near 4k'ish */

  g_return_if_fail (DZL_IS_SUGGESTION_POPOVER (self));

  window = gtk_widget_get_window (GTK_WIDGET (self));

  if (window != NULL)
    {
      GdkDisplay *display = gtk_widget_get_display (GTK_WIDGET (self));
      GdkMonitor *monitor = gdk_display_get_monitor_at_window (display, window);

      if (monitor != NULL)
        {
          GdkRectangle geom;

          gdk_monitor_get_geometry (monitor, &geom);
          clip_height = geom.height;
        }
    }

  max_height = CLAMP (max_height, -1, clip_height);

  g_object_set (self->scrolled_window,
                "max-content-height", max_height,
                NULL);
}

void
_dzl_suggestion_popover_adjust_margin (DzlSuggestionPopover *self,
                                       GdkRectangle         *area)
{
  GtkStyleContext *style_context;
  GtkBorder margin;

  g_return_if_fail (DZL_IS_SUGGESTION_POPOVER (self));
  g_return_if_fail (area != NULL);

  /*
   * This is our last chance to adjust the allocation.  We need to subtract any
   * margins needed by the box for theming. We style the shadow in the box so
   * that we can do the show/hide with the revealer and have things come out
   * looking correct.
   */

  style_context = gtk_widget_get_style_context (GTK_WIDGET (self->top_box));
  gtk_style_context_get_margin (style_context,
                                gtk_style_context_get_state (style_context),
                                &margin);

  area->x -= margin.right;
  area->y -= margin.top;
  area->width += margin.right + margin.left;
  area->height += margin.top + margin.bottom;
}

void
_dzl_suggestion_popover_set_click_mode (DzlSuggestionPopover *self,
                                        gboolean              single_click)
{
  g_assert (DZL_IS_SUGGESTION_POPOVER (self));

  gtk_list_box_set_activate_on_single_click (GTK_LIST_BOX (self->list_box), single_click);
}

void
_dzl_suggestion_popover_set_focused (DzlSuggestionPopover *self,
                                     gboolean              entry_focused)
{
  g_return_if_fail (DZL_IS_SUGGESTION_POPOVER (self));

  self->entry_focused = !!entry_focused;
  if (!entry_focused)
    self->popup_requested = FALSE;
}

void
_dzl_suggestion_popover_set_device (DzlSuggestionPopover *self,
                                    GdkDevice            *device)
{
  g_return_if_fail (DZL_IS_SUGGESTION_POPOVER (self));
  g_return_if_fail (!device || GDK_IS_DEVICE (device));

  if (device != self->grab_device)
    {
      if (self->has_grab && self->grab_device != NULL)
        gdk_seat_ungrab (gdk_device_get_seat (self->grab_device));
      g_set_object (&self->grab_device, device);
    }
}

static void
make_rows_vertical (GtkWidget *row,
                    gpointer   user_data)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (row), GTK_ORIENTATION_VERTICAL);
}

static void
make_rows_horizontal (GtkWidget *row,
                      gpointer   user_data)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (row), GTK_ORIENTATION_HORIZONTAL);
}

void
_dzl_suggestion_popover_set_compact (DzlSuggestionPopover *self,
                                     gboolean              compact)
{
  g_return_if_fail (DZL_IS_SUGGESTION_POPOVER (self));

  compact = !!compact;

  if (compact != self->compact)
    {
      self->compact = compact;

      if (compact)
        gtk_container_foreach (GTK_CONTAINER (self->list_box), make_rows_vertical, NULL);
      else
        gtk_container_foreach (GTK_CONTAINER (self->list_box), make_rows_horizontal, NULL);
    }
}
