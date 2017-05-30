statemachine_headers = [
  'dzl-state-machine-buildable.h',
  'dzl-state-machine.h',
]

foreach h: statemachine_headers
  libdazzle_public_headers += files(h)
endforeach

install_headers(statemachine_headers, subdir: join_paths(libdazzle_header_subdir, 'statemachine'))
