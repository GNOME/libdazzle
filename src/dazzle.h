/* dazzle.h
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
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

#ifndef DAZZLE_H
#define DAZZLE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#if !GTK_CHECK_VERSION(3, 22, 0)
# error "libdazzle requires gtk+-3.0 >= 3.22.15"
#endif

#if !GLIB_CHECK_VERSION(2, 50, 0)
# error "libdazzle requires glib-2.0 >= 2.52.0"
#endif

#define DAZZLE_INSIDE
#include "dzl-version.h"
#include "dzl-enums.h"
#include "actions/dzl-action-group.h"
#include "actions/dzl-child-property-action.h"
#include "actions/dzl-properties-group.h"
#include "actions/dzl-settings-flag-action.h"
#include "actions/dzl-widget-action-group.h"
#include "animation/dzl-animation.h"
#include "animation/dzl-box-theatric.h"
#include "animation/dzl-frame-source.h"
#include "app/dzl-application.h"
#include "app/dzl-application-window.h"
#include "bindings/dzl-binding-group.h"
#include "bindings/dzl-signal-group.h"
#include "cache/dzl-task-cache.h"
#include "files/dzl-directory-model.h"
#include "files/dzl-directory-reaper.h"
#include "files/dzl-file-transfer.h"
#include "files/dzl-recursive-file-monitor.h"
#include "graphing/dzl-cpu-graph.h"
#include "graphing/dzl-cpu-model.h"
#include "graphing/dzl-graph-column.h"
#include "graphing/dzl-graph-line-renderer.h"
#include "graphing/dzl-graph-model.h"
#include "graphing/dzl-graph-renderer.h"
#include "graphing/dzl-graph-view.h"
#include "menus/dzl-joined-menu.h"
#include "menus/dzl-menu-button.h"
#include "menus/dzl-menu-manager.h"
#include "panel/dzl-dock-bin-edge.h"
#include "panel/dzl-dock-bin.h"
#include "panel/dzl-dock-item.h"
#include "panel/dzl-dock-manager.h"
#include "panel/dzl-dock-overlay-edge.h"
#include "panel/dzl-dock-overlay.h"
#include "panel/dzl-dock-paned.h"
#include "panel/dzl-dock-revealer.h"
#include "panel/dzl-dock-stack.h"
#include "panel/dzl-dock-transient-grab.h"
#include "panel/dzl-dock-types.h"
#include "panel/dzl-dock-widget.h"
#include "panel/dzl-dock-window.h"
#include "panel/dzl-dock.h"
#include "panel/dzl-tab-strip.h"
#include "panel/dzl-tab.h"
#include "pathbar/dzl-path.h"
#include "pathbar/dzl-path-bar.h"
#include "pathbar/dzl-path-element.h"
#include "prefs/dzl-preferences-bin.h"
#include "prefs/dzl-preferences-entry.h"
#include "prefs/dzl-preferences-file-chooser-button.h"
#include "prefs/dzl-preferences-flow-box.h"
#include "prefs/dzl-preferences-font-button.h"
#include "prefs/dzl-preferences-group.h"
#include "prefs/dzl-preferences-page.h"
#include "prefs/dzl-preferences-spin-button.h"
#include "prefs/dzl-preferences-switch.h"
#include "prefs/dzl-preferences-view.h"
#include "prefs/dzl-preferences.h"
#include "search/dzl-fuzzy-index.h"
#include "search/dzl-fuzzy-index-builder.h"
#include "search/dzl-fuzzy-index-cursor.h"
#include "search/dzl-fuzzy-index-match.h"
#include "search/dzl-fuzzy-mutable-index.h"
#include "search/dzl-levenshtein.h"
#include "search/dzl-pattern-spec.h"
#include "search/dzl-trie.h"
#include "settings/dzl-settings-sandwich.h"
#include "shortcuts/dzl-shortcut-accel-dialog.h"
#include "shortcuts/dzl-shortcut-chord.h"
#include "shortcuts/dzl-shortcut-context.h"
#include "shortcuts/dzl-shortcut-controller.h"
#include "shortcuts/dzl-shortcut-label.h"
#include "shortcuts/dzl-shortcut-manager.h"
#include "shortcuts/dzl-shortcut-model.h"
#include "shortcuts/dzl-shortcut-phase.h"
#include "shortcuts/dzl-shortcut-simple-label.h"
#include "shortcuts/dzl-shortcut-theme-editor.h"
#include "shortcuts/dzl-shortcut-theme.h"
#include "shortcuts/dzl-shortcut-tooltip.h"
#include "shortcuts/dzl-shortcuts-group.h"
#include "shortcuts/dzl-shortcuts-section.h"
#include "shortcuts/dzl-shortcuts-shortcut.h"
#include "shortcuts/dzl-shortcuts-window.h"
#include "statemachine/dzl-state-machine-buildable.h"
#include "statemachine/dzl-state-machine.h"
#include "suggestions/dzl-suggestion-button.h"
#include "suggestions/dzl-suggestion-entry-buffer.h"
#include "suggestions/dzl-suggestion-entry.h"
#include "suggestions/dzl-suggestion-popover.h"
#include "suggestions/dzl-suggestion-row.h"
#include "suggestions/dzl-suggestion.h"
#include "theming/dzl-css-provider.h"
#include "theming/dzl-theme-manager.h"
#include "tree/dzl-list-store-adapter.h"
#include "tree/dzl-tree.h"
#include "tree/dzl-tree-builder.h"
#include "tree/dzl-tree-node.h"
#include "tree/dzl-tree-types.h"
#include "util/dzl-cairo.h"
#include "util/dzl-cancellable.h"
#ifndef G_OS_WIN32
# include "util/dzl-counter.h"
#endif
#include "util/dzl-date-time.h"
#include "util/dzl-dnd.h"
#include "util/dzl-file-manager.h"
#include "util/dzl-gdk.h"
#include "util/dzl-gtk.h"
#include "util/dzl-heap.h"
#include "util/dzl-int-pair.h"
#include "util/dzl-list-model-filter.h"
#include "util/dzl-macros.h"
#include "util/dzl-pango.h"
#include "util/dzl-read-only-list-model.h"
#include "util/dzl-rgba.h"
#include "util/dzl-ring.h"
#include "util/dzl-variant.h"
#include "widgets/dzl-bin.h"
#include "widgets/dzl-bolding-label.h"
#include "widgets/dzl-box.h"
#include "widgets/dzl-centering-bin.h"
#include "widgets/dzl-column-layout.h"
#ifndef G_OS_WIN32
# include "widgets/dzl-counters-window.h"
#endif
#include "widgets/dzl-elastic-bin.h"
#include "widgets/dzl-empty-state.h"
#include "widgets/dzl-entry-box.h"
#include "widgets/dzl-file-chooser-entry.h"
#include "widgets/dzl-list-box.h"
#include "widgets/dzl-multi-paned.h"
#include "widgets/dzl-pill-box.h"
#include "widgets/dzl-priority-box.h"
#include "widgets/dzl-progress-button.h"
#include "widgets/dzl-progress-menu-button.h"
#include "widgets/dzl-progress-icon.h"
#include "widgets/dzl-radio-box.h"
#include "widgets/dzl-scrolled-window.h"
#include "widgets/dzl-search-bar.h"
#include "widgets/dzl-simple-label.h"
#include "widgets/dzl-simple-popover.h"
#include "widgets/dzl-slider.h"
#include "widgets/dzl-stack-list.h"
#include "widgets/dzl-three-grid.h"
#undef DAZZLE_INSIDE

G_END_DECLS

#endif /* DAZZLE_H */
