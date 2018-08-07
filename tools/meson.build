if get_option('enable_tools')

# Counters are UNIX-systems only currently
if host_machine.system() != 'windows'
dazzle_list_counters = executable('dazzle-list-counters', 'dazzle-list-counters.c',
  dependencies: libdazzle_deps + [libdazzle_dep],
       install: true,
   install_dir: bindir
)
endif

endif
