/* dzl-shortcuts-shortcutprivate.h
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

#ifndef DZL_SHORTCUTS_SHORTCUT_H
#define DZL_SHORTCUTS_SHORTCUT_H

#include <gtk/gtk.h>

#include "dzl-version-macros.h"

G_BEGIN_DECLS

#define DZL_TYPE_SHORTCUTS_SHORTCUT (dzl_shortcuts_shortcut_get_type())
#define DZL_SHORTCUTS_SHORTCUT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), DZL_TYPE_SHORTCUTS_SHORTCUT, DzlShortcutsShortcut))
#define DZL_SHORTCUTS_SHORTCUT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DZL_TYPE_SHORTCUTS_SHORTCUT, DzlShortcutsShortcutClass))
#define DZL_IS_SHORTCUTS_SHORTCUT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DZL_TYPE_SHORTCUTS_SHORTCUT))
#define DZL_IS_SHORTCUTS_SHORTCUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DZL_TYPE_SHORTCUTS_SHORTCUT))
#define DZL_SHORTCUTS_SHORTCUT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DZL_TYPE_SHORTCUTS_SHORTCUT, DzlShortcutsShortcutClass))


typedef struct _DzlShortcutsShortcut      DzlShortcutsShortcut;
typedef struct _DzlShortcutsShortcutClass DzlShortcutsShortcutClass;

/**
 * DzlShortcutType:
 * @DZL_SHORTCUT_ACCELERATOR:
 *   The shortcut is a keyboard accelerator. The #DzlShortcutsShortcut:accelerator
 *   property will be used.
 * @DZL_SHORTCUT_GESTURE_PINCH:
 *   The shortcut is a pinch gesture. GTK+ provides an icon and subtitle.
 * @DZL_SHORTCUT_GESTURE_STRETCH:
 *   The shortcut is a stretch gesture. GTK+ provides an icon and subtitle.
 * @DZL_SHORTCUT_GESTURE_ROTATE_CLOCKWISE:
 *   The shortcut is a clockwise rotation gesture. GTK+ provides an icon and subtitle.
 * @DZL_SHORTCUT_GESTURE_ROTATE_COUNTERCLOCKWISE:
 *   The shortcut is a counterclockwise rotation gesture. GTK+ provides an icon and subtitle.
 * @DZL_SHORTCUT_GESTURE_TWO_FINGER_SWIPE_LEFT:
 *   The shortcut is a two-finger swipe gesture. GTK+ provides an icon and subtitle.
 * @DZL_SHORTCUT_GESTURE_TWO_FINGER_SWIPE_RIGHT:
 *   The shortcut is a two-finger swipe gesture. GTK+ provides an icon and subtitle.
 * @DZL_SHORTCUT_GESTURE:
 *   The shortcut is a gesture. The #DzlShortcutsShortcut:icon property will be
 *   used.
 *
 * DzlShortcutType specifies the kind of shortcut that is being described.
 * More values may be added to this enumeration over time.
 *
 * Since: 3.20
 */
typedef enum {
  DZL_SHORTCUT_ACCELERATOR,
  DZL_SHORTCUT_GESTURE_PINCH,
  DZL_SHORTCUT_GESTURE_STRETCH,
  DZL_SHORTCUT_GESTURE_ROTATE_CLOCKWISE,
  DZL_SHORTCUT_GESTURE_ROTATE_COUNTERCLOCKWISE,
  DZL_SHORTCUT_GESTURE_TWO_FINGER_SWIPE_LEFT,
  DZL_SHORTCUT_GESTURE_TWO_FINGER_SWIPE_RIGHT,
  DZL_SHORTCUT_GESTURE
} DzlShortcutType;

DZL_AVAILABLE_IN_ALL
GType dzl_shortcuts_shortcut_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* DZL_SHORTCUTS_SHORTCUT_H */
