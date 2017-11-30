files_headers = [
  'dzl-directory-model.h',
  'dzl-directory-reaper.h',
  'dzl-recursive-file-monitor.h',
]

files_sources = [
  'dzl-directory-model.c',
  'dzl-directory-reaper.c',
  'dzl-recursive-file-monitor.c',
]

libdazzle_public_headers += files(files_headers)
libdazzle_public_sources += files(files_sources)

install_headers(files_headers, subdir: join_paths(libdazzle_header_subdir, 'files'))
