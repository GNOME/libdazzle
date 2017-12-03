files_headers = [
  'dzl-directory-model.h',
  'dzl-directory-reaper.h',
  'dzl-file-transfer.h',
  'dzl-recursive-file-monitor.h',
]

files_sources = [
  'dzl-directory-model.c',
  'dzl-directory-reaper.c',
  'dzl-file-transfer.c',
  'dzl-recursive-file-monitor.c',
]

files_enums_headers = [
  'dzl-file-transfer.h',
]

libdazzle_public_headers += files(files_headers)
libdazzle_public_sources += files(files_sources)
dzl_enum_headers += files(files_enums_headers)

install_headers(files_headers, subdir: join_paths(libdazzle_header_subdir, 'files'))
