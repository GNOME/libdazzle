/* dzl-shortcuts-group.c
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>

#include "config.h"

#include "shortcuts/dzl-shortcuts-group.h"
#include "shortcuts/dzl-shortcuts-shortcut.h"

/**
 * SECTION:dzl-shortcuts-group
 * @Title: DzlShortcutsGroup
 * @Short_description: Represents a group of shortcuts in a DzlShortcutsWindow
 *
 * A DzlShortcutsGroup represents a group of related keyboard shortcuts
 * or gestures. The group has a title. It may optionally be associated with
 * a view of the application, which can be used to show only relevant shortcuts
 * depending on the application context.
 *
 * This widget is only meant to be used with #DzlShortcutsWindow.
 */

struct _DzlShortcutsGroup
{
  GtkBox    parent_instance;

  GtkLabel *title;
  gchar    *view;
  guint     height;

  GtkSizeGroup *accel_size_group;
  GtkSizeGroup *title_size_group;
};

struct _DzlShortcutsGroupClass
{
  GtkBoxClass parent_class;
};

G_DEFINE_TYPE (DzlShortcutsGroup, dzl_shortcuts_group, GTK_TYPE_BOX)

enum {
  PROP_0,
  PROP_TITLE,
  PROP_VIEW,
  PROP_ACCEL_SIZE_GROUP,
  PROP_TITLE_SIZE_GROUP,
  PROP_HEIGHT,
  LAST_PROP
};

static GParamSpec *properties[LAST_PROP];

static void
dzl_shortcuts_group_apply_accel_size_group (DzlShortcutsGroup *group,
                                            GtkWidget         *child)
{
  if (DZL_IS_SHORTCUTS_SHORTCUT (child))
    g_object_set (child, "accel-size-group", group->accel_size_group, NULL);
}

static void
dzl_shortcuts_group_apply_title_size_group (DzlShortcutsGroup *group,
                                            GtkWidget         *child)
{
  if (DZL_IS_SHORTCUTS_SHORTCUT (child))
    g_object_set (child, "title-size-group", group->title_size_group, NULL);
}

static void
dzl_shortcuts_group_set_accel_size_group (DzlShortcutsGroup *group,
                                          GtkSizeGroup      *size_group)
{
  GList *children, *l;

  g_set_object (&group->accel_size_group, size_group);

  children = gtk_container_get_children (GTK_CONTAINER (group));
  for (l = children; l; l = l->next)
    dzl_shortcuts_group_apply_accel_size_group (group, GTK_WIDGET (l->data));
  g_list_free (children);
}

static void
dzl_shortcuts_group_set_title_size_group (DzlShortcutsGroup *group,
                                          GtkSizeGroup      *size_group)
{
  GList *children, *l;

  g_set_object (&group->title_size_group, size_group);

  children = gtk_container_get_children (GTK_CONTAINER (group));
  for (l = children; l; l = l->next)
    dzl_shortcuts_group_apply_title_size_group (group, GTK_WIDGET (l->data));
  g_list_free (children);
}

static guint
dzl_shortcuts_group_get_height (DzlShortcutsGroup *group)
{
  GList *children, *l;
  guint height;

  height = 1;

  children = gtk_container_get_children (GTK_CONTAINER (group));
  for (l = children; l; l = l->next)
    {
      GtkWidget *child = l->data;

      if (!gtk_widget_get_visible (child))
        continue;
      else if (DZL_IS_SHORTCUTS_SHORTCUT (child))
        height += 1;
    }
  g_list_free (children);

  return height;
}

static void
dzl_shortcuts_group_add (GtkContainer *container,
                         GtkWidget    *widget)
{
  if (DZL_IS_SHORTCUTS_SHORTCUT (widget))
    {
      GTK_CONTAINER_CLASS (dzl_shortcuts_group_parent_class)->add (container, widget);
      dzl_shortcuts_group_apply_accel_size_group (DZL_SHORTCUTS_GROUP (container), widget);
      dzl_shortcuts_group_apply_title_size_group (DZL_SHORTCUTS_GROUP (container), widget);
    }
  else
    g_warning ("Can't add children of type %s to %s",
               G_OBJECT_TYPE_NAME (widget),
               G_OBJECT_TYPE_NAME (container));
}

typedef struct {
  GtkCallback callback;
  gpointer data;
  gboolean include_internal;
} CallbackData;

static void
forall_cb (GtkWidget *widget, gpointer data)
{
  DzlShortcutsGroup *self;
  CallbackData *cbdata = data;

  self = DZL_SHORTCUTS_GROUP (gtk_widget_get_parent (widget));
  if (cbdata->include_internal || widget != (GtkWidget*)self->title)
    cbdata->callback (widget, cbdata->data);
}

static void
dzl_shortcuts_group_forall (GtkContainer *container,
                            gboolean      include_internal,
                            GtkCallback   callback,
                            gpointer      callback_data)
{
  CallbackData cbdata;

  cbdata.include_internal = include_internal;
  cbdata.callback = callback;
  cbdata.data = callback_data;

  GTK_CONTAINER_CLASS (dzl_shortcuts_group_parent_class)->forall (container, include_internal, forall_cb, &cbdata);
}

static void
dzl_shortcuts_group_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  DzlShortcutsGroup *self = DZL_SHORTCUTS_GROUP (object);

  switch (prop_id)
    {
    case PROP_TITLE:
      g_value_set_string (value, gtk_label_get_label (self->title));
      break;

    case PROP_VIEW:
      g_value_set_string (value, self->view);
      break;

    case PROP_HEIGHT:
      g_value_set_uint (value, dzl_shortcuts_group_get_height (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcuts_group_direction_changed (GtkWidget        *widget,
                                       GtkTextDirection  previous_dir)
{
  GTK_WIDGET_CLASS (dzl_shortcuts_group_parent_class)->direction_changed (widget, previous_dir);
  g_object_notify (G_OBJECT (widget), "height");
}

static void
dzl_shortcuts_group_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  DzlShortcutsGroup *self = DZL_SHORTCUTS_GROUP (object);

  switch (prop_id)
    {
    case PROP_TITLE:
      gtk_label_set_label (self->title, g_value_get_string (value));
      break;

    case PROP_VIEW:
      g_free (self->view);
      self->view = g_value_dup_string (value);
      break;

    case PROP_ACCEL_SIZE_GROUP:
      dzl_shortcuts_group_set_accel_size_group (self, GTK_SIZE_GROUP (g_value_get_object (value)));
      break;

    case PROP_TITLE_SIZE_GROUP:
      dzl_shortcuts_group_set_title_size_group (self, GTK_SIZE_GROUP (g_value_get_object (value)));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcuts_group_finalize (GObject *object)
{
  DzlShortcutsGroup *self = DZL_SHORTCUTS_GROUP (object);

  g_free (self->view);
  g_set_object (&self->accel_size_group, NULL);
  g_set_object (&self->title_size_group, NULL);

  G_OBJECT_CLASS (dzl_shortcuts_group_parent_class)->finalize (object);
}

static void
dzl_shortcuts_group_dispose (GObject *object)
{
  DzlShortcutsGroup *self = DZL_SHORTCUTS_GROUP (object);

  /*
   * Since we overload forall(), the inherited destroy() won't work as normal.
   * Remove internal widgets ourself.
   */
  if (self->title)
    {
      gtk_widget_destroy (GTK_WIDGET (self->title));
      self->title = NULL;
    }

  G_OBJECT_CLASS (dzl_shortcuts_group_parent_class)->dispose (object);
}

static void
dzl_shortcuts_group_class_init (DzlShortcutsGroupClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->finalize = dzl_shortcuts_group_finalize;
  object_class->get_property = dzl_shortcuts_group_get_property;
  object_class->set_property = dzl_shortcuts_group_set_property;
  object_class->dispose = dzl_shortcuts_group_dispose;

  widget_class->direction_changed = dzl_shortcuts_group_direction_changed;
  container_class->add = dzl_shortcuts_group_add;
  container_class->forall = dzl_shortcuts_group_forall;

  /**
   * DzlShortcutsGroup:title:
   *
   * The title for this group of shortcuts.
   */
  properties[PROP_TITLE] =
    g_param_spec_string ("title", _("Title"), _("Title"),
                         "",
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * DzlShortcutsGroup:view:
   *
   * An optional view that the shortcuts in this group are relevant for.
   * The group will be hidden if the #DzlShortcutsWindow:view-name property
   * does not match the view of this group.
   *
   * Set this to %NULL to make the group always visible.
   */
  properties[PROP_VIEW] =
    g_param_spec_string ("view", _("View"), _("View"),
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * DzlShortcutsGroup:accel-size-group:
   *
   * The size group for the accelerator portion of shortcuts in this group.
   *
   * This is used internally by GTK+, and must not be modified by applications.
   */
  properties[PROP_ACCEL_SIZE_GROUP] =
    g_param_spec_object ("accel-size-group",
                         _("Accelerator Size Group"),
                         _("Accelerator Size Group"),
                         GTK_TYPE_SIZE_GROUP,
                         (G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  /**
   * DzlShortcutsGroup:title-size-group:
   *
   * The size group for the textual portion of shortcuts in this group.
   *
   * This is used internally by GTK+, and must not be modified by applications.
   */
  properties[PROP_TITLE_SIZE_GROUP] =
    g_param_spec_object ("title-size-group",
                         _("Title Size Group"),
                         _("Title Size Group"),
                         GTK_TYPE_SIZE_GROUP,
                         (G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  /**
   * DzlShortcutsGroup:height:
   *
   * A rough measure for the number of lines in this group.
   *
   * This is used internally by GTK+, and is not useful for applications.
   */
  properties[PROP_HEIGHT] =
    g_param_spec_uint ("height", _("Height"), _("Height"),
                       0, G_MAXUINT, 1,
                       (G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
dzl_shortcuts_group_init (DzlShortcutsGroup *self)
{
  PangoAttrList *attrs;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_VERTICAL);
  gtk_box_set_spacing (GTK_BOX (self), 10);

  attrs = pango_attr_list_new ();
  pango_attr_list_insert (attrs, pango_attr_weight_new (PANGO_WEIGHT_BOLD));
  self->title = g_object_new (GTK_TYPE_LABEL,
                              "attributes", attrs,
                              "visible", TRUE,
                              "xalign", 0.0f,
                              NULL);
  pango_attr_list_unref (attrs);

  GTK_CONTAINER_CLASS (dzl_shortcuts_group_parent_class)->add (GTK_CONTAINER (self), GTK_WIDGET (self->title));
}
