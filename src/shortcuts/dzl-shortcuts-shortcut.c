/* dzl-shortcuts-shortcut.c
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

#include "shortcuts/dzl-shortcut-label.h"
#include "shortcuts/dzl-shortcuts-shortcut.h"
#include "shortcuts/dzl-shortcuts-window-private.h"

/**
 * SECTION:dzl-shortcuts-shortcut
 * @Title: DzlShortcutsShortcut
 * @Short_description: Represents a keyboard shortcut in a DzlShortcutsWindow
 *
 * A DzlShortcutsShortcut represents a single keyboard shortcut or gesture
 * with a short text. This widget is only meant to be used with #DzlShortcutsWindow.
 */

struct _DzlShortcutsShortcut
{
  GtkBox            parent_instance;

  GtkImage         *image;
  DzlShortcutLabel *accelerator;
  GtkLabel         *title;
  GtkLabel         *subtitle;
  GtkLabel         *title_box;

  GtkSizeGroup     *accel_size_group;
  GtkSizeGroup     *title_size_group;

  gboolean          subtitle_set;
  gboolean          icon_set;
  GtkTextDirection  direction;
  gchar            *action_name;
  GtkShortcutType   shortcut_type;
};

struct _DzlShortcutsShortcutClass
{
  GtkBoxClass parent_class;
};

G_DEFINE_TYPE (DzlShortcutsShortcut, dzl_shortcuts_shortcut, GTK_TYPE_BOX)

enum {
  PROP_0,
  PROP_ACCELERATOR,
  PROP_ICON,
  PROP_ICON_SET,
  PROP_TITLE,
  PROP_SUBTITLE,
  PROP_SUBTITLE_SET,
  PROP_ACCEL_SIZE_GROUP,
  PROP_TITLE_SIZE_GROUP,
  PROP_DIRECTION,
  PROP_SHORTCUT_TYPE,
  PROP_ACTION_NAME,
  LAST_PROP
};

static GParamSpec *properties[LAST_PROP];

static void
dzl_shortcuts_shortcut_set_accelerator (DzlShortcutsShortcut *self,
                                        const gchar          *accelerator)
{
  dzl_shortcut_label_set_accelerator (self->accelerator, accelerator);
}

static void
dzl_shortcuts_shortcut_set_accel_size_group (DzlShortcutsShortcut *self,
                                             GtkSizeGroup         *group)
{
  if (self->accel_size_group)
    {
      gtk_size_group_remove_widget (self->accel_size_group, GTK_WIDGET (self->accelerator));
      gtk_size_group_remove_widget (self->accel_size_group, GTK_WIDGET (self->image));
    }

  if (group)
    {
      gtk_size_group_add_widget (group, GTK_WIDGET (self->accelerator));
      gtk_size_group_add_widget (group, GTK_WIDGET (self->image));
    }

  g_set_object (&self->accel_size_group, group);
}

static void
dzl_shortcuts_shortcut_set_title_size_group (DzlShortcutsShortcut *self,
                                             GtkSizeGroup         *group)
{
  if (self->title_size_group)
    gtk_size_group_remove_widget (self->title_size_group, GTK_WIDGET (self->title_box));
  if (group)
    gtk_size_group_add_widget (group, GTK_WIDGET (self->title_box));

  g_set_object (&self->title_size_group, group);
}

static void
update_subtitle_from_type (DzlShortcutsShortcut *self)
{
  const gchar *subtitle;

  if (self->subtitle_set)
    return;

  switch (self->shortcut_type)
    {
    case GTK_SHORTCUT_ACCELERATOR:
    case GTK_SHORTCUT_GESTURE:
      subtitle = NULL;
      break;

    case GTK_SHORTCUT_GESTURE_PINCH:
      subtitle = _("Two finger pinch");
      break;

    case GTK_SHORTCUT_GESTURE_STRETCH:
      subtitle = _("Two finger stretch");
      break;

    case GTK_SHORTCUT_GESTURE_ROTATE_CLOCKWISE:
      subtitle = _("Rotate clockwise");
      break;

    case GTK_SHORTCUT_GESTURE_ROTATE_COUNTERCLOCKWISE:
      subtitle = _("Rotate counterclockwise");
      break;

    case GTK_SHORTCUT_GESTURE_TWO_FINGER_SWIPE_LEFT:
      subtitle = _("Two finger swipe left");
      break;

    case GTK_SHORTCUT_GESTURE_TWO_FINGER_SWIPE_RIGHT:
      subtitle = _("Two finger swipe right");
      break;

    default:
      subtitle = NULL;
      break;
    }

  gtk_label_set_label (self->subtitle, subtitle);
  gtk_widget_set_visible (GTK_WIDGET (self->subtitle), subtitle != NULL);
  g_object_notify (G_OBJECT (self), "subtitle");
}

static void
dzl_shortcuts_shortcut_set_subtitle_set (DzlShortcutsShortcut *self,
                                         gboolean              subtitle_set)
{
  if (self->subtitle_set != subtitle_set)
    {
      self->subtitle_set = subtitle_set;
      g_object_notify (G_OBJECT (self), "subtitle-set");
    }
  update_subtitle_from_type (self);
}

static void
dzl_shortcuts_shortcut_set_subtitle (DzlShortcutsShortcut *self,
                                     const gchar          *subtitle)
{
  gtk_label_set_label (self->subtitle, subtitle);
  gtk_widget_set_visible (GTK_WIDGET (self->subtitle), subtitle && subtitle[0]);
  dzl_shortcuts_shortcut_set_subtitle_set (self, subtitle && subtitle[0]);

  g_object_notify (G_OBJECT (self), "subtitle");
}

static void
update_icon_from_type (DzlShortcutsShortcut *self)
{
  GIcon *icon;

  if (self->icon_set)
    return;

  switch (self->shortcut_type)
    {
    case GTK_SHORTCUT_GESTURE_PINCH:
      icon = g_themed_icon_new ("gesture-pinch-symbolic");
      break;

    case GTK_SHORTCUT_GESTURE_STRETCH:
      icon = g_themed_icon_new ("gesture-stretch-symbolic");
      break;

    case GTK_SHORTCUT_GESTURE_ROTATE_CLOCKWISE:
      icon = g_themed_icon_new ("gesture-rotate-clockwise-symbolic");
      break;

    case GTK_SHORTCUT_GESTURE_ROTATE_COUNTERCLOCKWISE:
      icon = g_themed_icon_new ("gesture-rotate-anticlockwise-symbolic");
      break;

    case GTK_SHORTCUT_GESTURE_TWO_FINGER_SWIPE_LEFT:
      icon = g_themed_icon_new ("gesture-two-finger-swipe-left-symbolic");
      break;

    case GTK_SHORTCUT_GESTURE_TWO_FINGER_SWIPE_RIGHT:
      icon = g_themed_icon_new ("gesture-two-finger-swipe-right-symbolic");
      break;

    case GTK_SHORTCUT_ACCELERATOR:
    case GTK_SHORTCUT_GESTURE:
    default: ;
      icon = NULL;
      break;
    }

  if (icon)
    {
      gtk_image_set_from_gicon (self->image, icon, GTK_ICON_SIZE_DIALOG);
      gtk_image_set_pixel_size (self->image, 64);
      g_object_unref (icon);
    }
}

static void
dzl_shortcuts_shortcut_set_icon_set (DzlShortcutsShortcut *self,
                                     gboolean              icon_set)
{
  if (self->icon_set != icon_set)
    {
      self->icon_set = icon_set;
      g_object_notify (G_OBJECT (self), "icon-set");
    }
  update_icon_from_type (self);
}

static void
dzl_shortcuts_shortcut_set_icon (DzlShortcutsShortcut *self,
                                 GIcon                *gicon)
{
  gtk_image_set_from_gicon (self->image, gicon, GTK_ICON_SIZE_DIALOG);
  dzl_shortcuts_shortcut_set_icon_set (self, gicon != NULL);
  g_object_notify (G_OBJECT (self), "icon");
}

static void
update_visible_from_direction (DzlShortcutsShortcut *self)
{
  if (self->direction == GTK_TEXT_DIR_NONE ||
      self->direction == gtk_widget_get_direction (GTK_WIDGET (self)))
    {
      gtk_widget_set_visible (GTK_WIDGET (self), TRUE);
      /* When porting to gtk4, we'll have to update all this
       * code anyway, so fine to disable warnings.
       */
      G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
      gtk_widget_set_no_show_all (GTK_WIDGET (self), FALSE);
      G_GNUC_END_IGNORE_DEPRECATIONS;
    }
  else
    {
      gtk_widget_set_visible (GTK_WIDGET (self), FALSE);
      G_GNUC_BEGIN_IGNORE_DEPRECATIONS;
      gtk_widget_set_no_show_all (GTK_WIDGET (self), TRUE);
      G_GNUC_END_IGNORE_DEPRECATIONS;
    }
}

static void
dzl_shortcuts_shortcut_set_direction (DzlShortcutsShortcut *self,
                                      GtkTextDirection      direction)
{
  if (self->direction == direction)
    return;

  self->direction = direction;

  update_visible_from_direction (self);

  g_object_notify (G_OBJECT (self), "direction");
}

static void
dzl_shortcuts_shortcut_direction_changed (GtkWidget        *widget,
                                          GtkTextDirection  previous_dir)
{
  update_visible_from_direction (DZL_SHORTCUTS_SHORTCUT (widget));

  GTK_WIDGET_CLASS (dzl_shortcuts_shortcut_parent_class)->direction_changed (widget, previous_dir);
}

static void
dzl_shortcuts_shortcut_set_type (DzlShortcutsShortcut *self,
                                 GtkShortcutType       type)
{
  if (self->shortcut_type == type)
    return;

  self->shortcut_type = type;

  update_subtitle_from_type (self);
  update_icon_from_type (self);

  gtk_widget_set_visible (GTK_WIDGET (self->accelerator), type == GTK_SHORTCUT_ACCELERATOR);
  gtk_widget_set_visible (GTK_WIDGET (self->image), type != GTK_SHORTCUT_ACCELERATOR);


  g_object_notify (G_OBJECT (self), "shortcut-type");
}

static void
dzl_shortcuts_shortcut_set_action_name (DzlShortcutsShortcut *self,
                                        const gchar          *action_name)
{
  g_free (self->action_name);
  self->action_name = g_strdup (action_name);

  g_object_notify (G_OBJECT (self), "action-name");
}

static void
dzl_shortcuts_shortcut_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  DzlShortcutsShortcut *self = DZL_SHORTCUTS_SHORTCUT (object);

  switch (prop_id)
    {
    case PROP_TITLE:
      g_value_set_string (value, gtk_label_get_label (self->title));
      break;

    case PROP_SUBTITLE:
      g_value_set_string (value, gtk_label_get_label (self->subtitle));
      break;

    case PROP_SUBTITLE_SET:
      g_value_set_boolean (value, self->subtitle_set);
      break;

    case PROP_ACCELERATOR:
      g_value_set_string (value, dzl_shortcut_label_get_accelerator (self->accelerator));
      break;

    case PROP_ICON:
      {
        GIcon *icon;

        gtk_image_get_gicon (self->image, &icon, NULL);
        g_value_set_object (value, icon);
      }
      break;

    case PROP_ICON_SET:
      g_value_set_boolean (value, self->icon_set);
      break;

    case PROP_DIRECTION:
      g_value_set_enum (value, self->direction);
      break;

    case PROP_SHORTCUT_TYPE:
      g_value_set_enum (value, self->shortcut_type);
      break;

    case PROP_ACTION_NAME:
      g_value_set_string (value, self->action_name);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dzl_shortcuts_shortcut_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  DzlShortcutsShortcut *self = DZL_SHORTCUTS_SHORTCUT (object);

  switch (prop_id)
    {
    case PROP_ACCELERATOR:
      dzl_shortcuts_shortcut_set_accelerator (self, g_value_get_string (value));
      break;

    case PROP_ICON:
      dzl_shortcuts_shortcut_set_icon (self, g_value_get_object (value));
      break;

    case PROP_ICON_SET:
      dzl_shortcuts_shortcut_set_icon_set (self, g_value_get_boolean (value));
      break;

    case PROP_ACCEL_SIZE_GROUP:
      dzl_shortcuts_shortcut_set_accel_size_group (self, GTK_SIZE_GROUP (g_value_get_object (value)));
      break;

    case PROP_TITLE:
      gtk_label_set_label (self->title, g_value_get_string (value));
      break;

    case PROP_SUBTITLE:
      dzl_shortcuts_shortcut_set_subtitle (self, g_value_get_string (value));
      break;

    case PROP_SUBTITLE_SET:
      dzl_shortcuts_shortcut_set_subtitle_set (self, g_value_get_boolean (value));
      break;

    case PROP_TITLE_SIZE_GROUP:
      dzl_shortcuts_shortcut_set_title_size_group (self, GTK_SIZE_GROUP (g_value_get_object (value)));
      break;

    case PROP_DIRECTION:
      dzl_shortcuts_shortcut_set_direction (self, g_value_get_enum (value));
      break;

    case PROP_SHORTCUT_TYPE:
      dzl_shortcuts_shortcut_set_type (self, g_value_get_enum (value));
      break;

    case PROP_ACTION_NAME:
      dzl_shortcuts_shortcut_set_action_name (self, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
dzl_shortcuts_shortcut_finalize (GObject *object)
{
  DzlShortcutsShortcut *self = DZL_SHORTCUTS_SHORTCUT (object);

  g_clear_object (&self->accel_size_group);
  g_clear_object (&self->title_size_group);
  g_free (self->action_name);

  G_OBJECT_CLASS (dzl_shortcuts_shortcut_parent_class)->finalize (object);
}

static void
dzl_shortcuts_shortcut_add (GtkContainer *container,
                            GtkWidget    *widget)
{
  g_warning ("Can't add children to %s", G_OBJECT_TYPE_NAME (container));
}

static GType
dzl_shortcuts_shortcut_child_type (GtkContainer *container)
{
  return G_TYPE_NONE;
}

void
dzl_shortcuts_shortcut_update_accel (DzlShortcutsShortcut *self,
                                     GtkWindow            *window)
{
  GtkApplication *app;
  gchar **accels;
  gchar *str;

  if (self->action_name == NULL)
    return;

  app = gtk_window_get_application (window);
  if (app == NULL)
    return;

  accels = gtk_application_get_accels_for_action (app, self->action_name);
  str = g_strjoinv (" ", accels);

  dzl_shortcuts_shortcut_set_accelerator (self, str);

  g_free (str);
  g_strfreev (accels);
}

static void
dzl_shortcuts_shortcut_class_init (DzlShortcutsShortcutClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->finalize = dzl_shortcuts_shortcut_finalize;
  object_class->get_property = dzl_shortcuts_shortcut_get_property;
  object_class->set_property = dzl_shortcuts_shortcut_set_property;

  widget_class->direction_changed = dzl_shortcuts_shortcut_direction_changed;

  container_class->add = dzl_shortcuts_shortcut_add;
  container_class->child_type = dzl_shortcuts_shortcut_child_type;

  /**
   * DzlShortcutsShortcut:accelerator:
   *
   * The accelerator(s) represented by this object. This property is used
   * if #DzlShortcutsShortcut:shortcut-type is set to #GTK_SHORTCUT_ACCELERATOR.
   *
   * The syntax of this property is (an extension of) the syntax understood by
   * gtk_accelerator_parse(). Multiple accelerators can be specified by separating
   * them with a space, but keep in mind that the available width is limited.
   * It is also possible to specify ranges of shortcuts, using ... between the keys.
   * Sequences of keys can be specified using a + or & between the keys.
   *
   * Examples:
   * - A single shortcut: &lt;ctl&gt;&lt;alt&gt;delete
   * - Two alternative shortcuts: &lt;shift&gt;a Home
   * - A range of shortcuts: &lt;alt&gt;1...&lt;alt&gt;9
   * - Several keys pressed together: Control_L&Control_R
   * - A sequence of shortcuts or keys: &lt;ctl&gt;c+&lt;ctl&gt;x
   *
   * Use + instead of & when the keys may (or have to be) pressed sequentially (e.g
   * use t+t for 'press the t key twice').
   *
   * Note that <, > and & need to be escaped as &lt;, &gt; and &amp; when used
   * in .ui files.
   */
  properties[PROP_ACCELERATOR] =
    g_param_spec_string ("accelerator",
                         "Accelerator",
                         "The accelerator keys for shortcuts of type 'Accelerator'",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * DzlShortcutsShortcut:icon:
   *
   * An icon to represent the shortcut or gesture. This property is used if
   * #DzlShortcutsShortcut:shortcut-type is set to #GTK_SHORTCUT_GESTURE.
   * For the other predefined gesture types, GTK+ provides an icon on its own.
   */
  properties[PROP_ICON] =
    g_param_spec_object ("icon",
                         "Icon",
                         "The icon to show for shortcuts of type 'Other Gesture'",
                         G_TYPE_ICON,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * DzlShortcutsShortcut:icon-set:
   *
   * %TRUE if an icon has been set.
   */
  properties[PROP_ICON_SET] =
    g_param_spec_boolean ("icon-set",
                          "Icon Set",
                          "Whether an icon has been set",
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * DzlShortcutsShortcut:title:
   *
   * The textual description for the shortcut or gesture represented by
   * this object. This should be a short string that can fit in a single line.
   */
  properties[PROP_TITLE] =
    g_param_spec_string ("title",
                         "Title",
                         "A short description for the shortcut",
                         "",
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * DzlShortcutsShortcut:subtitle:
   *
   * The subtitle for the shortcut or gesture.
   *
   * This is typically used for gestures and should be a short, one-line
   * text that describes the gesture itself. For the predefined gesture
   * types, GTK+ provides a subtitle on its own.
   */
  properties[PROP_SUBTITLE] =
    g_param_spec_string ("subtitle",
                         "Subtitle",
                         "A short description for the gesture",
                         "",
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * DzlShortcutsShortcut:subtitle-set:
   *
   * %TRUE if a subtitle has been set.
   */
  properties[PROP_SUBTITLE_SET] =
    g_param_spec_boolean ("subtitle-set",
                          "Subtitle Set",
                          "Whether a subtitle has been set",
                          FALSE,
                          (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  /**
   * DzlShortcutsShortcut:accel-size-group:
   *
   * The size group for the accelerator portion of this shortcut.
   *
   * This is used internally by GTK+, and must not be modified by applications.
   */
  properties[PROP_ACCEL_SIZE_GROUP] =
    g_param_spec_object ("accel-size-group",
                         "Accelerator Size Group",
                         "Accelerator Size Group",
                         GTK_TYPE_SIZE_GROUP,
                         (G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  /**
   * DzlShortcutsShortcut:title-size-group:
   *
   * The size group for the textual portion of this shortcut.
   *
   * This is used internally by GTK+, and must not be modified by applications.
   */
  properties[PROP_TITLE_SIZE_GROUP] =
    g_param_spec_object ("title-size-group",
                         "Title Size Group",
                         "Title Size Group",
                         GTK_TYPE_SIZE_GROUP,
                         (G_PARAM_WRITABLE | G_PARAM_STATIC_STRINGS));

  /**
   * DzlShortcutsShortcut:direction:
   *
   * The text direction for which this shortcut is active. If the shortcut
   * is used regardless of the text direction, set this property to
   * #GTK_TEXT_DIR_NONE.
   */
  properties[PROP_DIRECTION] =
    g_param_spec_enum ("direction",
                       "Direction",
                       "Text direction for which this shortcut is active",
                       GTK_TYPE_TEXT_DIRECTION,
                       GTK_TEXT_DIR_NONE,
                       (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

  /**
   * DzlShortcutsShortcut:shortcut-type:
   *
   * The type of shortcut that is represented.
   */
  properties[PROP_SHORTCUT_TYPE] =
    g_param_spec_enum ("shortcut-type",
                       "Shortcut Type",
                       "The type of shortcut that is represented",
                       GTK_TYPE_SHORTCUT_TYPE,
                       GTK_SHORTCUT_ACCELERATOR,
                       (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY));

  /**
   * DzlShortcutsShortcut:action-name:
   *
   * A detailed action name. If this is set for a shortcut
   * of type %GTK_SHORTCUT_ACCELERATOR, then GTK+ will use
   * the accelerators that are associated with the action
   * via gtk_application_set_accels_for_action(), and setting
   * #DzlShortcutsShortcut::accelerator is not necessary.
   *
   * Since: 3.22
   */
  properties[PROP_ACTION_NAME] =
    g_param_spec_string ("action-name",
                         "Action Name",
                         "The name of the action",
                         NULL,
                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (object_class, LAST_PROP, properties);
  gtk_widget_class_set_css_name (widget_class, "shortcut");
}

static void
dzl_shortcuts_shortcut_init (DzlShortcutsShortcut *self)
{
  gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_HORIZONTAL);
  gtk_box_set_spacing (GTK_BOX (self), 12);

  self->direction = GTK_TEXT_DIR_NONE;
  self->shortcut_type = GTK_SHORTCUT_ACCELERATOR;

  self->image = g_object_new (GTK_TYPE_IMAGE,
                              "visible", FALSE,
                              "valign", GTK_ALIGN_CENTER,
                              "no-show-all", TRUE,
                              NULL);
  GTK_CONTAINER_CLASS (dzl_shortcuts_shortcut_parent_class)->add (GTK_CONTAINER (self), GTK_WIDGET (self->image));

  self->accelerator = g_object_new (DZL_TYPE_SHORTCUT_LABEL,
                                    "visible", TRUE,
                                    "valign", GTK_ALIGN_CENTER,
                                    "no-show-all", TRUE,
                                    NULL);
  GTK_CONTAINER_CLASS (dzl_shortcuts_shortcut_parent_class)->add (GTK_CONTAINER (self), GTK_WIDGET (self->accelerator));

  self->title_box = g_object_new (GTK_TYPE_BOX,
                                  "visible", TRUE,
                                  "valign", GTK_ALIGN_CENTER,
                                  "hexpand", TRUE,
                                  "orientation", GTK_ORIENTATION_VERTICAL,
                                  NULL);
  GTK_CONTAINER_CLASS (dzl_shortcuts_shortcut_parent_class)->add (GTK_CONTAINER (self), GTK_WIDGET (self->title_box));

  self->title = g_object_new (GTK_TYPE_LABEL,
                              "visible", TRUE,
                              "xalign", 0.0f,
                              NULL);
  gtk_container_add (GTK_CONTAINER (self->title_box), GTK_WIDGET (self->title));

  self->subtitle = g_object_new (GTK_TYPE_LABEL,
                                 "visible", FALSE,
                                 "no-show-all", TRUE,
                                 "xalign", 0.0f,
                                 NULL);
  gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (self->subtitle)),
                               GTK_STYLE_CLASS_DIM_LABEL);
  gtk_container_add (GTK_CONTAINER (self->title_box), GTK_WIDGET (self->subtitle));
}
