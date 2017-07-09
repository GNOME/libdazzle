/* dzl-shortcut-model.h
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
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

#ifndef DZL_SHORTCUT_MODEL_H
#define DZL_SHORTCUT_MODEL_H

#include <gtk/gtk.h>

#include "dzl-shortcut-chord.h"
#include "dzl-shortcut-manager.h"
#include "dzl-shortcut-theme.h"

G_BEGIN_DECLS

#define DZL_TYPE_SHORTCUT_MODEL (dzl_shortcut_model_get_type())

G_DECLARE_FINAL_TYPE (DzlShortcutModel, dzl_shortcut_model, DZL, SHORTCUT_MODEL, GtkTreeStore)

GtkTreeModel       *dzl_shortcut_model_new         (void);
DzlShortcutManager *dzl_shortcut_model_get_manager (DzlShortcutModel       *self);
void                dzl_shortcut_model_set_manager (DzlShortcutModel       *self,
                                                    DzlShortcutManager     *manager);
DzlShortcutTheme   *dzl_shortcut_model_get_theme   (DzlShortcutModel       *self);
void                dzl_shortcut_model_set_theme   (DzlShortcutModel       *self,
                                                    DzlShortcutTheme       *theme);
void                dzl_shortcut_model_set_chord   (DzlShortcutModel       *self,
                                                    GtkTreeIter            *iter,
                                                    const DzlShortcutChord *chord);
void                dzl_shortcut_model_rebuild     (DzlShortcutModel       *self);

G_END_DECLS

#endif /* DZL_SHORTCUT_MODEL_H */
