/* dzl-preferences-group.c
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

#define G_LOG_DOMAIN "dzl-preferences-group"

#include "config.h"

#include "prefs/dzl-preferences-bin.h"
#include "prefs/dzl-preferences-bin-private.h"
#include "prefs/dzl-preferences-entry.h"
#include "prefs/dzl-preferences-group.h"
#include "prefs/dzl-preferences-group-private.h"
#include "util/dzl-macros.h"

G_DEFINE_TYPE (DzlPreferencesGroup, dzl_preferences_group, GTK_TYPE_BIN)

#define COLUMN_WIDTH 500

enum {
  PROP_0,
  PROP_IS_LIST,
  PROP_MODE,
  PROP_PRIORITY,
  PROP_TITLE,
  LAST_PROP
};

static GParamSpec *properties [LAST_PROP];

gint
dzl_preferences_group_get_priority (DzlPreferencesGroup *self)
{
  g_return_val_if_fail (DZL_IS_PREFERENCES_GROUP (self), 0);

  return self->priority;
}

static void
dzl_preferences_group_widget_destroy (DzlPreferencesGroup *self,
                                      GtkWidget           *widget)
{
  g_assert (DZL_IS_PREFERENCES_GROUP (self));
  g_assert (GTK_IS_WIDGET (widget));

  g_ptr_array_remove (self->widgets, widget);
}

static void
dzl_preferences_group_row_activated (DzlPreferencesGroup *self,
                                     GtkListBoxRow       *row,
                                     GtkListBox          *list_box)
{
  GtkWidget *child;

  g_assert (DZL_IS_PREFERENCES_GROUP (self));
  g_assert (GTK_IS_LIST_BOX_ROW (row));
  g_assert (GTK_IS_LIST_BOX (list_box));

  child = gtk_bin_get_child (GTK_BIN (row));
  if (child != NULL)
    gtk_widget_activate (child);
}

static void
dzl_preferences_group_row_selected (DzlPreferencesGroup *self,
                                    GtkListBoxRow       *row,
                                    GtkListBox          *list_box)
{
  g_assert (DZL_IS_PREFERENCES_GROUP (self));
  g_assert (!row || GTK_IS_LIST_BOX_ROW (row));
  g_assert (GTK_IS_LIST_BOX (list_box));

  if (gtk_list_box_get_selection_mode (list_box) == GTK_SELECTION_SINGLE &&
      GTK_IS_LIST_BOX_ROW (row) &&
      gtk_list_box_row_get_activatable (row))
    dzl_preferences_group_row_activated (self, row, list_box);
}

const gchar *
dzl_preferences_group_get_title (DzlPreferencesGroup *self)
{
  const gchar *title;

  g_return_val_if_fail (DZL_IS_PREFERENCES_GROUP (self), NULL);

  title = gtk_label_get_label (self->title);

  return (!title || !*title) ? NULL : title;
}

static void
dzl_preferences_group_get_preferred_width (GtkWidget *widget,
                                           gint      *min_width,
                                           gint      *nat_width)
{
  *min_width = *nat_width = COLUMN_WIDTH;
}

static void
dzl_preferences_group_get_preferred_height_for_width (GtkWidget *widget,
                                                      gint       width,
                                                      gint      *min_height,
                                                      gint      *nat_height)
{
  g_assert (GTK_IS_WIDGET (widget));
  g_assert (min_height != NULL);
  g_assert (nat_height != NULL);

  GTK_WIDGET_CLASS (dzl_preferences_group_parent_class)->get_preferred_height_for_width (widget, width, min_height, nat_height);
}

static GtkSizeRequestMode
dzl_preferences_group_get_request_mode (GtkWidget *widget)
{
  return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

static void
dzl_preferences_group_finalize (GObject *object)
{
  DzlPreferencesGroup *self = (DzlPreferencesGroup *)object;

  g_clear_pointer (&self->widgets, g_ptr_array_unref);
  g_clear_pointer (&self->size_groups, g_hash_table_unref);

  G_OBJECT_CLASS (dzl_preferences_group_parent_class)->finalize (object);
}

static void
dzl_preferences_group_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  DzlPreferencesGroup *self = DZL_PREFERENCES_GROUP (object);

  switch (prop_id)
    {
    case PROP_MODE:
      g_value_set_enum (value, gtk_list_box_get_selection_mode (self->list_box));
      break;

    case PROP_IS_LIST:
      g_value_set_boolean (value, self->is_list);
      break;

    case PROP_PRIORITY:
      g_value_set_int (value, self->priority);
      break;

    case PROP_TITLE:
      g_value_set_string (value, gtk_label_get_label (self->title));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_group_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  DzlPreferencesGroup *self = DZL_PREFERENCES_GROUP (object);

  switch (prop_id)
    {
    case PROP_MODE:
      gtk_list_box_set_selection_mode (self->list_box, g_value_get_enum (value));
      break;

    case PROP_IS_LIST:
      self->is_list = g_value_get_boolean (value);
      gtk_widget_set_visible (GTK_WIDGET (self->box), !self->is_list);
      gtk_widget_set_visible (GTK_WIDGET (self->list_box_frame), self->is_list);
      break;

    case PROP_PRIORITY:
      self->priority = g_value_get_int (value);
      break;

    case PROP_TITLE:
      gtk_label_set_label (self->title, g_value_get_string (value));
      gtk_widget_set_visible (GTK_WIDGET (self->title), !!g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_preferences_group_class_init (DzlPreferencesGroupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = dzl_preferences_group_finalize;
  object_class->get_property = dzl_preferences_group_get_property;
  object_class->set_property = dzl_preferences_group_set_property;

  widget_class->get_preferred_width = dzl_preferences_group_get_preferred_width;
  widget_class->get_preferred_height_for_width = dzl_preferences_group_get_preferred_height_for_width;
  widget_class->get_request_mode = dzl_preferences_group_get_request_mode;

  properties [PROP_MODE] =
    g_param_spec_enum ("mode",
                       NULL,
                       NULL,
                       GTK_TYPE_SELECTION_MODE,
                       GTK_SELECTION_NONE,
                       (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_IS_LIST] =
    g_param_spec_boolean ("is-list",
                          "Is List",
                          "If the group should be rendered as a listbox.",
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_PRIORITY] =
    g_param_spec_int ("priority",
                      "Priority",
                      "Priority",
                      G_MININT,
                      G_MAXINT,
                      0,
                      (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "Title",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);

  gtk_widget_class_set_template_from_resource (widget_class, "/org/gnome/dazzle/ui/dzl-preferences-group.ui");
  gtk_widget_class_set_css_name (widget_class, "dzlpreferencesgroup");
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesGroup, box);
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesGroup, list_box);
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesGroup, list_box_frame);
  gtk_widget_class_bind_template_child (widget_class, DzlPreferencesGroup, title);
}

static void
dzl_preferences_group_init (DzlPreferencesGroup *self)
{
  self->widgets = g_ptr_array_new ();

  gtk_widget_init_template (GTK_WIDGET (self));

  g_signal_connect_object (self->list_box,
                           "row-activated",
                           G_CALLBACK (dzl_preferences_group_row_activated),
                           self,
                           G_CONNECT_SWAPPED);

  g_signal_connect_object (self->list_box,
                           "row-selected",
                           G_CALLBACK (dzl_preferences_group_row_selected),
                           self,
                           G_CONNECT_SWAPPED);
}

static gboolean
dzl_preferences_group_row_focus (DzlPreferencesGroup *self,
                                 GtkDirectionType     dir,
                                 GtkListBoxRow       *row)
{
  GtkWidget *child;
  GtkWidget *entry;

  self->last_focused_tab_backward = (dir == GTK_DIR_TAB_BACKWARD);

  child = gtk_bin_get_child (GTK_BIN (row));

  if (DZL_IS_PREFERENCES_ENTRY (child))
    {
      entry = dzl_preferences_entry_get_entry_widget ( DZL_PREFERENCES_ENTRY (child));
      if (GTK_IS_ENTRY (entry) &&
          gtk_widget_is_focus (entry) &&
          dir == GTK_DIR_TAB_BACKWARD)
        gtk_widget_grab_focus (GTK_WIDGET (row));
    }

  return GDK_EVENT_PROPAGATE;
}

static void
dzl_preferences_group_row_grab_focus (DzlPreferencesGroup *self,
                                      GtkListBoxRow       *row)
{
  GtkWidget *child;
  GtkListBoxRow *last_focused;

  last_focused = self->last_focused;
  child = gtk_bin_get_child (GTK_BIN (row));
  if (DZL_IS_PREFERENCES_ENTRY (child))
    {
      self->last_focused = row;
      if (row != last_focused || !self->last_focused_tab_backward)
        gtk_widget_activate (child);

      return;
    }

  self->last_focused = NULL;
}

void
dzl_preferences_group_add (DzlPreferencesGroup *self,
                           GtkWidget           *widget)
{
  gint position = -1;

  g_return_if_fail (DZL_IS_PREFERENCES_GROUP (self));
  g_return_if_fail (DZL_IS_PREFERENCES_BIN (widget));

  g_ptr_array_add (self->widgets, widget);

  g_signal_connect_object (widget,
                           "destroy",
                           G_CALLBACK (dzl_preferences_group_widget_destroy),
                           self,
                           G_CONNECT_SWAPPED);

  if (self->is_list)
    {
      GtkWidget *row;

      if (GTK_IS_LIST_BOX_ROW (widget))
        row = widget;
      else
        row = g_object_new (GTK_TYPE_LIST_BOX_ROW,
                            "child", widget,
                            "visible", TRUE,
                            NULL);

      gtk_container_add (GTK_CONTAINER (self->list_box), row);
      g_signal_connect_object (row,
                               "focus",
                               G_CALLBACK (dzl_preferences_group_row_focus),
                               self,
                               G_CONNECT_SWAPPED);
      g_signal_connect_object (row,
                               "grab-focus",
                               G_CALLBACK (dzl_preferences_group_row_grab_focus),
                               self,
                               G_CONNECT_AFTER | G_CONNECT_SWAPPED);
    }
  else
    {
      gtk_container_add_with_properties (GTK_CONTAINER (self->box), widget,
                                         "position", position,
                                         NULL);
    }
}

void
dzl_preferences_group_set_map (DzlPreferencesGroup *self,
                               GHashTable          *map)
{
  guint i;

  g_return_if_fail (DZL_IS_PREFERENCES_GROUP (self));

  for (i = 0; i < self->widgets->len; i++)
    {
      GtkWidget *widget = g_ptr_array_index (self->widgets, i);

      if (DZL_IS_PREFERENCES_BIN (widget))
        _dzl_preferences_bin_set_map (DZL_PREFERENCES_BIN (widget), map);
    }
}

static void
dzl_preferences_group_refilter_cb (GtkWidget *widget,
                                   gpointer   user_data)
{
  DzlPreferencesBin *bin = NULL;
  struct {
    DzlPatternSpec *spec;
    guint matches;
  } *lookup = user_data;
  gboolean matches;

  if (DZL_IS_PREFERENCES_BIN (widget))
    bin = DZL_PREFERENCES_BIN (widget);
  else if (GTK_IS_BIN (widget) && DZL_IS_PREFERENCES_BIN (gtk_bin_get_child (GTK_BIN (widget))))
    bin = DZL_PREFERENCES_BIN (gtk_bin_get_child (GTK_BIN (widget)));
  else
    return;

  if (lookup->spec == NULL)
    matches = TRUE;
  else
    matches = _dzl_preferences_bin_matches (bin, lookup->spec);

  gtk_widget_set_visible (widget, matches);

  lookup->matches += matches;
}

guint
dzl_preferences_group_refilter (DzlPreferencesGroup *self,
                                DzlPatternSpec      *spec)
{
  struct {
    DzlPatternSpec *spec;
    guint matches;
  } lookup = { spec, 0 };
  const gchar *tmp;

  g_return_val_if_fail (DZL_IS_PREFERENCES_GROUP (self), 0);

  tmp = gtk_label_get_label (self->title);
  if (spec && tmp && dzl_pattern_spec_match (spec, tmp))
    lookup.spec = NULL;

  gtk_container_foreach (GTK_CONTAINER (self->list_box),
                         dzl_preferences_group_refilter_cb,
                         &lookup);
  gtk_container_foreach (GTK_CONTAINER (self->box),
                         dzl_preferences_group_refilter_cb,
                         &lookup);

  gtk_widget_set_visible (GTK_WIDGET (self), lookup.matches > 0);

  return lookup.matches;
}

/**
 * dzl_preferences_group_get_size_group:
 * @self: a #DzlPreferencesGroup
 *
 * Gets a size group that can be used to organize items in
 * a group based on columns.
 *
 * Returns: (not nullable) (transfer none): a #GtkSizeGroup
 */
GtkSizeGroup *
dzl_preferences_group_get_size_group (DzlPreferencesGroup *self,
                                      guint                column)
{
  GtkSizeGroup *ret;

  g_return_val_if_fail (DZL_IS_PREFERENCES_GROUP (self), NULL);

  if (self->size_groups == NULL)
    self->size_groups = g_hash_table_new_full (g_direct_hash,
                                               g_direct_equal,
                                               NULL,
                                               g_object_unref);

  if (!(ret = g_hash_table_lookup (self->size_groups, GUINT_TO_POINTER (column))))
    {
      ret = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
      g_hash_table_insert (self->size_groups, GUINT_TO_POINTER (column), ret);
    }

  return ret;
}
