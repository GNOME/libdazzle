if get_option('enable_tests')

test_env = [
  'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
  'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
  'G_DEBUG=gc-friendly',
  'GSETTINGS_BACKEND=memory',
  'PYTHONDONTWRITEBYTECODE=yes',
  'MALLOC_CHECK_=2',
  'NO_AT_BRIDGE=1',
#  'MALLOC_PERTURB_=$((${RANDOM:-256} % 256))',
]

test_cflags = [
  '-DTEST_DATA_DIR="@0@/data"'.format(meson.current_source_dir()),
]

test_link_args = [
]

test_application = executable('test-application', 'test-application.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-application', test_application, env: test_env)

test_menu_manager = executable('test-menu-manager', 'test-menu-manager.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_menu_manager2 = executable('test-menu-manager2', 'test-menu-manager2.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-menu-manager2', test_menu_manager2, env: test_env)

test_state_machine = executable('test-state-machine', 'test-state-machine.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-state-machine', test_state_machine, env: test_env)

test_binding_group = executable('test-binding-group', 'test-binding-group.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-binding-group', test_binding_group, env: test_env)

test_signal_group = executable('test-signal-group', 'test-signal-group.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-signal-group', test_binding_group, env: test_env)

test_task_cache = executable('test-task-cache', 'test-task-cache.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-task-cache', test_task_cache, env: test_env)

test_heap = executable('test-heap', 'test-heap.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-heap', test_heap, env: test_env)

test_radio_box = executable('test-radio-box', 'test-radio-box.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_slider = executable('test-slider', 'test-slider.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_file_chooser_entry = executable('test-file-chooser-entry', 'test-file-chooser-entry.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_elastic_bin = executable('test-elastic-bin', 'test-elastic-bin.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_stack_list = executable('test-stack-list', 'test-stack-list.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_suggestion = executable('test-suggestion', 'test-suggestion.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_suggestion_buffer = executable('test-suggestion-buffer', 'test-suggestion-buffer.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-suggestion-buffer', test_suggestion_buffer, env: test_env)

test_fuzzy_index = executable('test-fuzzy-index', 'test-fuzzy-index.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-fuzzy-index', test_fuzzy_index, env: test_env)

test_bin = executable('test-bin', 'test-bin.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_multi_paned = executable('test-multi-paned', 'test-multi-paned.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_tab_strip = executable('test-tab-strip', 'test-tab-strip.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_panel = executable('test-panel', 'test-panel.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_shortcut_chord = executable('test-shortcut-chord', 'test-shortcut-chord.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-shortcut-chord', test_shortcut_chord, env: test_env)

test_shortcut_overlays = executable('test-shortcut-overlays', 'test-shortcut-overlays.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-shortcut-overlays', test_shortcut_overlays, env: test_env)

test_shortcut_theme = executable('test-shortcut-theme', 'test-shortcut-theme.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-shortcut-theme', test_shortcut_theme, env: test_env)

test_shortcuts = executable('test-shortcuts', 'test-shortcuts.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_progress_button = executable('test-progress-button', 'test-progress-button.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_progress_menu_button = executable('test-progress-menu-button', 'test-progress-menu-button.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_progress_icon = executable('test-progress-icon', 'test-progress-icon.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_simple_popover = executable('test-simple-popover', 'test-simple-popover.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_entry_box = executable('test-entry-box', 'test-entry-box.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_bolding_label = executable('test-bolding-label', 'test-bolding-label.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_file_manager = executable('test-file-manager', 'test-file-manager.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_empty_state = executable('test-empty-state', 'test-empty-state.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_fuzzy_mutable_index = executable('test-fuzzy-mutable-index', 'test-fuzzy-mutable-index.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_desktop_index = executable('test-desktop-index', 'test-desktop-index.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_cpu_graph = executable('test-cpu-graph', 'test-cpu-graph.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_trie = executable('test-trie', 'test-trie.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-trie', test_trie, env: test_env)

test_levenshtein = executable('test-levenshtein', 'test-levenshtein.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-levenshtein', test_levenshtein, env: test_env)

test_pill_box = executable('test-pill-box', 'test-pill-box.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_preferences = executable('test-preferences', 'test-preferences.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_int_pair = executable('test-int-pair', 'test-int-pair.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-int-pair', test_int_pair, env: test_env)

test_path_bar = executable('test-path-bar', 'test-path-bar.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_ring = executable('test-ring', 'test-ring.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-ring', test_ring, env: test_env)

# Counters only run on UNIX-like systems currently
if host_machine.system() != 'windows'
test_counters_window = executable('test-counters-window', 'test-counters-window.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
endif

test_list_store = executable('test-list-store', 'test-list-store.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-list-store', test_list_store, env: test_env)

test_joined_menu = executable('test-joined-menu', 'test-joined-menu.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_box = executable('test-box', 'test-box.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_directory_reaper = executable('test-directory-reaper', 'test-directory-reaper.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-directory-reaper', test_directory_reaper, env: test_env)

test_list_store_adapter = executable('test-list-store-adapter', 'test-list-store-adapter.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-list-store-adapter', test_list_store_adapter, env: test_env)

test_util = executable('test-util', ['test-util.c', '../src/util/dzl-util.c'],
               c_args: test_cflags,
            link_args: test_link_args,
  include_directories: [include_directories('.'), root_inc ],
         dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-util', test_util, env: test_env)

test_pattern_spec = executable('test-pattern-spec', 'test-pattern-spec.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-pattern-spec', test_pattern_spec, env: test_env)

test_recursive_monitor = executable('test-recursive-monitor', 'test-recursive-monitor.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-recursive-monitor', test_recursive_monitor, env: test_env)

test_file_transfer = executable('test-file-transfer', 'test-file-transfer.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-file-transfer', test_file_transfer, env: test_env)

test_tree = executable('test-tree', 'test-tree.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_cancellable = executable('test-cancellable', 'test-cancellable.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-cancellable', test_cancellable, env: test_env)

test_graph_model = executable('test-graph-model', 'test-graph-model.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-graph-model', test_graph_model, env: test_env)

test_read_only_list_model = executable('test-read-only-list-model', 'test-read-only-list-model.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-read-only-list-model', test_read_only_list_model, env: test_env)

test_model_filter = executable('test-model-filter', 'test-model-filter.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-model-filter', test_model_filter, env: test_env)

test_shortcut_tooltip = executable('test-shortcut-tooltip', 'test-shortcut-tooltip.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_fuzzy_highlight = executable('test-fuzzy-highlight', 'test-fuzzy-highlight.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-fuzzy-highlight', test_fuzzy_highlight, env: test_env)

test_mutable_scoring = executable('test-mutable-scoring', 'test-mutable-scoring.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)
test('test-mutable-scoring', test_mutable_scoring, env: test_env)

endif
