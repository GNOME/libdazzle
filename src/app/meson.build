app_headers = [
  'dzl-application.h',
  'dzl-application-window.h',
]

app_sources = [
  'dzl-application.c',
  'dzl-application-window.c',
]

dzl_enum_headers += files([
  'dzl-application-window.h',
])

libdazzle_public_headers += files(app_headers)
libdazzle_public_sources += files(app_sources)

install_headers(app_headers, subdir: join_paths(libdazzle_header_subdir, 'app'))
