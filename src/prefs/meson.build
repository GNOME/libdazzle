prefs_headers = [
  'dzl-preferences-bin.h',
  'dzl-preferences-entry.h',
  'dzl-preferences-file-chooser-button.h',
  'dzl-preferences-flow-box.h',
  'dzl-preferences-font-button.h',
  'dzl-preferences-group.h',
  'dzl-preferences-page.h',
  'dzl-preferences-spin-button.h',
  'dzl-preferences-switch.h',
  'dzl-preferences-view.h',
  'dzl-preferences.h',
]

prefs_sources = [
  'dzl-preferences-bin.c',
  'dzl-preferences-entry.c',
  'dzl-preferences-file-chooser-button.c',
  'dzl-preferences-flow-box.c',
  'dzl-preferences-font-button.c',
  'dzl-preferences-group.c',
  'dzl-preferences-page.c',
  'dzl-preferences-spin-button.c',
  'dzl-preferences-switch.c',
  'dzl-preferences-view.c',
  'dzl-preferences.c',
]

libdazzle_public_headers += files(prefs_headers)
libdazzle_public_sources += files(prefs_sources)

install_headers(prefs_headers, subdir: join_paths(libdazzle_header_subdir, 'prefs'))
