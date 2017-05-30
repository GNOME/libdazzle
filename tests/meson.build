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

test_suggestion = executable('test-suggestion', 'test-suggestion.c', 'fuzzy.c', 'fuzzy.h',
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
