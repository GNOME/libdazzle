search_headers = [
  'dzl-fuzzy-index-builder.h',
  'dzl-fuzzy-index-cursor.h',
  'dzl-fuzzy-index.h',
  'dzl-fuzzy-index-match.h',
  'dzl-fuzzy-mutable-index.h',
  'dzl-levenshtein.h',
  'dzl-pattern-spec.h',
  'dzl-trie.h',
]

search_sources = [
  'dzl-fuzzy-index-builder.c',
  'dzl-fuzzy-index-cursor.c',
  'dzl-fuzzy-index.c',
  'dzl-fuzzy-index-match.c',
  'dzl-fuzzy-mutable-index.c',
  'dzl-levenshtein.c',
  'dzl-pattern-spec.c',
  'dzl-trie.c',
]

libdazzle_public_headers += files(search_headers)
libdazzle_public_sources += files(search_sources)

install_headers(search_headers, subdir: join_paths(libdazzle_header_subdir, 'search'))
