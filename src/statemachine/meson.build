statemachine_headers = [
  'dzl-state-machine-buildable.h',
  'dzl-state-machine.h',
]

statemachine_sources = [
  'dzl-state-machine-buildable.c',
  'dzl-state-machine.c',
]

libdazzle_public_headers += files(statemachine_headers)
libdazzle_public_sources += files(statemachine_sources)

install_headers(statemachine_headers, subdir: join_paths(libdazzle_header_subdir, 'statemachine'))
