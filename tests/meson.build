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
