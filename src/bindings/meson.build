bindings_headers = [
  'dzl-binding-group.h',
  'dzl-signal-group.h',
]

bindings_sources = [
  'dzl-binding-group.c',
  'dzl-signal-group.c',
]

libdazzle_public_headers += files(bindings_headers)
libdazzle_public_sources += files(bindings_sources)

install_headers(bindings_headers, subdir: join_paths(libdazzle_header_subdir, 'bindings'))
