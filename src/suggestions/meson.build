suggestions_headers = [
  'dzl-suggestion-button.h',
  'dzl-suggestion-entry-buffer.h',
  'dzl-suggestion-entry.h',
  'dzl-suggestion-popover.h',
  'dzl-suggestion-row.h',
  'dzl-suggestion.h',
]

suggestions_sources = [
  'dzl-suggestion-button.c',
  'dzl-suggestion-entry-buffer.c',
  'dzl-suggestion-entry.c',
  'dzl-suggestion-popover.c',
  'dzl-suggestion-row.c',
  'dzl-suggestion.c',
]

libdazzle_public_headers += files(suggestions_headers)
libdazzle_public_sources += files(suggestions_sources)

install_headers(suggestions_headers, subdir: join_paths(libdazzle_header_subdir, 'suggestions'))
