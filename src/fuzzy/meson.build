fuzzy_headers = [
  'dzl-fuzzy-index-builder.h',
  'dzl-fuzzy-index-cursor.h',
  'dzl-fuzzy-index-match.h',
  'dzl-fuzzy-index.h',
]

fuzzy_sources = [
  'dzl-fuzzy-index-builder.c',
  'dzl-fuzzy-index-cursor.c',
  'dzl-fuzzy-index-match.c',
  'dzl-fuzzy-index.c',
]

libdazzle_public_headers += files(fuzzy_headers)
libdazzle_public_sources += files(fuzzy_sources)
libdazzle_private_sources += files('dzl-fuzzy-util.c')

install_headers(fuzzy_headers, subdir: join_paths(libdazzle_header_subdir, 'fuzzy'))
