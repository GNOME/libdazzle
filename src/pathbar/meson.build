pathbar_headers = [
  'dzl-path-bar.h',
  'dzl-path-element.h',
  'dzl-path.h',
]

pathbar_sources = [
  'dzl-path-bar.c',
  'dzl-path.c',
  'dzl-path-element.c',
]

libdazzle_public_headers += files(pathbar_headers)
libdazzle_public_sources += files(pathbar_sources)

install_headers(pathbar_headers, subdir: join_paths(libdazzle_header_subdir, 'pathbar'))
