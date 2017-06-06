test_env = [
  'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
  'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
  'G_DEBUG=gc-friendly',
  'GSETTINGS_BACKEND=memory',
  'PYTHONDONTWRITEBYTECODE=yes',
  'MALLOC_CHECK_=2',
#  'MALLOC_PERTURB_=$((${RANDOM:-256} % 256))',
]

test_cflags = [
  '-fPIE',
  '-DTEST_DATA_DIR="@0@/data"'.format(meson.current_source_dir()),
]

test_link_args = [
  '-fPIC',
]

test_application = executable('test-application', 'test-application.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_menu_manager = executable('test-menu-manager', 'test-menu-manager.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_state_machine = executable('test-state-machine', 'test-state-machine.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_binding_group = executable('test-binding-group', 'test-binding-group.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_signal_group = executable('test-signal-group', 'test-signal-group.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_task_cache = executable('test-task-cache', 'test-task-cache.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

test_heap = executable('test-heap', 'test-heap.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

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

test_fuzzy_index = executable('test-fuzzy-index', 'test-fuzzy-index.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

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

test_shortcut_theme = executable('test-shortcut-theme', 'test-shortcut-theme.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

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

test_levenshtein = executable('test-levenshtein', 'test-levenshtein.c',
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_deps + [libdazzle_dep],
)

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
