suggestions_headers = [
  'dzl-suggestion-entry-buffer.h',
  'dzl-suggestion-entry.h',
  'dzl-suggestion-popover.h',
  'dzl-suggestion-row.h',
  'dzl-suggestion.h',
]

foreach h: suggestions_headers
  libdazzle_public_headers += files(h)
endforeach

install_headers(suggestions_headers, subdir: join_paths(libdazzle_header_subdir, 'suggestions'))
