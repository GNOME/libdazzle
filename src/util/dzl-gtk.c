/* dzl-gtk.c
 *
 * Copyright (C) 2015-2017 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "dzl-gtk"

#include "animation/dzl-animation.h"
#include "util/dzl-gtk.h"

gboolean
dzl_gtk_widget_action (GtkWidget   *widget,
                       const gchar *prefix,
                       const gchar *action_name,
                       GVariant    *parameter)
{
  GtkWidget *toplevel;
  GApplication *app;
  GActionGroup *group = NULL;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
  g_return_val_if_fail (prefix, FALSE);
  g_return_val_if_fail (action_name, FALSE);

  app = g_application_get_default ();
  toplevel = gtk_widget_get_toplevel (widget);

  while ((group == NULL) && (widget != NULL))
    {
      group = gtk_widget_get_action_group (widget, prefix);

      if G_UNLIKELY (GTK_IS_POPOVER (widget))
        {
          GtkWidget *relative_to;

          relative_to = gtk_popover_get_relative_to (GTK_POPOVER (widget));

          if (relative_to != NULL)
            widget = relative_to;
          else
            widget = gtk_widget_get_parent (widget);
        }
      else
        {
          widget = gtk_widget_get_parent (widget);
        }
    }

  if (!group && g_str_equal (prefix, "win") && G_IS_ACTION_GROUP (toplevel))
    group = G_ACTION_GROUP (toplevel);

  if (!group && g_str_equal (prefix, "app") && G_IS_ACTION_GROUP (app))
    group = G_ACTION_GROUP (app);

  if (group && g_action_group_has_action (group, action_name))
    {
      g_action_group_activate_action (group, action_name, parameter);
      return TRUE;
    }

  if (parameter && g_variant_is_floating (parameter))
    {
      parameter = g_variant_ref_sink (parameter);
      g_variant_unref (parameter);
    }

  g_warning ("Failed to locate action %s.%s", prefix, action_name);

  return FALSE;
}

gboolean
dzl_gtk_widget_action_with_string (GtkWidget   *widget,
                                   const gchar *group,
                                   const gchar *name,
                                   const gchar *param)
{
  GVariant *variant = NULL;
  gboolean ret;

  g_return_val_if_fail (GTK_IS_WIDGET (widget), FALSE);
  g_return_val_if_fail (group != NULL, FALSE);
  g_return_val_if_fail (name != NULL, FALSE);

  if (param == NULL)
    param = "";

  if (*param != 0)
    {
      g_autoptr(GError) error = NULL;

      variant = g_variant_parse (NULL, param, NULL, NULL, &error);

      if (variant == NULL)
        {
          g_warning ("can't parse keybinding parameters \"%s\": %s",
                     param, error->message);
          return FALSE;
        }
    }

  ret = dzl_gtk_widget_action (widget, group, name, variant);

  return ret;
}

static void
show_callback (gpointer data)
{
  g_object_set_data (data, "DZL_FADE_ANIMATION", NULL);
  g_object_unref (data);
}

static void
hide_callback (gpointer data)
{
  GtkWidget *widget = data;

  g_object_set_data (data, "DZL_FADE_ANIMATION", NULL);
  gtk_widget_hide (widget);
  gtk_widget_set_opacity (widget, 1.0);
  g_object_unref (widget);
}

void
dzl_gtk_widget_hide_with_fade (GtkWidget *widget)
{
  GdkFrameClock *frame_clock;
  DzlAnimation *anim;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (gtk_widget_get_visible (widget))
    {
      anim = g_object_get_data (G_OBJECT (widget), "DZL_FADE_ANIMATION");
      if (anim != NULL)
        dzl_animation_stop (anim);

      frame_clock = gtk_widget_get_frame_clock (widget);
      anim = dzl_object_animate_full (widget,
                                      DZL_ANIMATION_LINEAR,
                                      1000,
                                      frame_clock,
                                      hide_callback,
                                      g_object_ref (widget),
                                      "opacity", 0.0,
                                      NULL);
      g_object_set_data_full (G_OBJECT (widget), "DZL_FADE_ANIMATION",
                              g_object_ref (anim), g_object_unref);
    }
}

void
dzl_gtk_widget_show_with_fade (GtkWidget *widget)
{
  GdkFrameClock *frame_clock;
  DzlAnimation *anim;

  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (!gtk_widget_get_visible (widget))
    {
      anim = g_object_get_data (G_OBJECT (widget), "DZL_FADE_ANIMATION");
      if (anim != NULL)
        dzl_animation_stop (anim);

      frame_clock = gtk_widget_get_frame_clock (widget);
      gtk_widget_set_opacity (widget, 0.0);
      gtk_widget_show (widget);
      anim = dzl_object_animate_full (widget,
                                      DZL_ANIMATION_LINEAR,
                                      500,
                                      frame_clock,
                                      show_callback,
                                      g_object_ref (widget),
                                      "opacity", 1.0,
                                      NULL);
      g_object_set_data_full (G_OBJECT (widget), "DZL_FADE_ANIMATION",
                              g_object_ref (anim), g_object_unref);
    }
}

static void
dzl_gtk_widget_find_child_typed_cb (GtkWidget *widget,
                                    gpointer   user_data)
{
  struct {
    gpointer ret;
    GType type;
  } *state = user_data;

  if (state->ret != NULL)
    return;

  if (g_type_is_a (G_OBJECT_TYPE (widget), state->type))
    {
      state->ret = widget;
    }
  else if (GTK_IS_CONTAINER (widget))
    {
      gtk_container_foreach (GTK_CONTAINER (widget),
                             dzl_gtk_widget_find_child_typed_cb,
                             state);
    }
}

gpointer
dzl_gtk_widget_find_child_typed (GtkWidget *widget,
                                 GType      child_type)
{
  struct {
    gpointer ret;
    GType type;
  } state;

  g_return_val_if_fail (GTK_IS_CONTAINER (widget), NULL);
  g_return_val_if_fail (g_type_is_a (child_type, GTK_TYPE_WIDGET), NULL);

  state.ret = NULL;
  state.type = child_type;

  gtk_container_foreach (GTK_CONTAINER (widget),
                         dzl_gtk_widget_find_child_typed_cb,
                         &state);

  return state.ret;
}

/**
 * dzl_gtk_text_buffer_remove_tag:
 *
 * Like gtk_text_buffer_remove_tag() but allows specifying that the tags
 * should be removed one at a time to avoid over-damaging the views
 * displaying @buffer.
 */
void
dzl_gtk_text_buffer_remove_tag (GtkTextBuffer     *buffer,
                                GtkTextTag        *tag,
                                const GtkTextIter *start,
                                const GtkTextIter *end,
                                gboolean           minimal_damage)
{
  GtkTextIter tag_begin;
  GtkTextIter tag_end;

  g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
  g_return_if_fail (GTK_IS_TEXT_TAG (tag));
  g_return_if_fail (start != NULL);
  g_return_if_fail (end != NULL);

  if (!minimal_damage)
    {
      gtk_text_buffer_remove_tag (buffer, tag, start, end);
      return;
    }

  tag_begin = *start;

  if (!gtk_text_iter_starts_tag (&tag_begin, tag))
    {
      if (!gtk_text_iter_forward_to_tag_toggle (&tag_begin, tag))
        return;
    }

  while (gtk_text_iter_starts_tag (&tag_begin, tag) &&
         gtk_text_iter_compare (&tag_begin, end) < 0)
    {
      gint count = 1;

      tag_end = tag_begin;

      /*
       * We might have found the start of another tag embedded
       * inside this tag. So keep scanning forward until we have
       * reached the right number of end tags.
       */

      while (gtk_text_iter_forward_to_tag_toggle (&tag_end, tag))
        {
          if (gtk_text_iter_starts_tag (&tag_end, tag))
            count++;
          else if (gtk_text_iter_ends_tag (&tag_end, tag))
            count--;

          if (count == 0)
            break;
        }

      if (gtk_text_iter_ends_tag (&tag_end, tag))
        gtk_text_buffer_remove_tag (buffer, tag, &tag_begin, &tag_end);

      tag_begin = tag_end;

      /*
       * Move to the next start tag. It's possible to have an overlapped
       * end tag, which would be non-ideal, but possible.
       */
      if (!gtk_text_iter_starts_tag (&tag_begin, tag))
        {
          while (gtk_text_iter_forward_to_tag_toggle (&tag_begin, tag))
            {
              if (gtk_text_iter_starts_tag (&tag_begin, tag))
                break;
            }
        }
    }
}

void
dzl_gtk_widget_add_style_class (GtkWidget   *widget,
                                const gchar *class_name)
{
  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (class_name != NULL);

  gtk_style_context_add_class (gtk_widget_get_style_context (widget), class_name);
}

void
dzl_gtk_widget_action_set (GtkWidget   *widget,
                           const gchar *group,
                           const gchar *name,
                           const gchar *first_property,
                           ...)
{
  GAction *action = NULL;
  va_list args;

  g_return_if_fail (GTK_IS_WIDGET (widget));
  g_return_if_fail (group != NULL);
  g_return_if_fail (name != NULL);
  g_return_if_fail (first_property != NULL);

  for (; widget; widget = gtk_widget_get_parent (widget))
    {
      GActionGroup *actions = gtk_widget_get_action_group (widget, group);

      if (G_IS_ACTION_MAP (actions))
        {
          action = g_action_map_lookup_action (G_ACTION_MAP (actions), name);

          if (action != NULL)
            break;
        }
    }

  if (action == NULL)
    {
      g_warning ("Failed to locate action %s.%s", group, name);
      return;
    }

  va_start (args, first_property);
  g_object_set_valist (G_OBJECT (action), first_property, args);
  va_end (args);
}
