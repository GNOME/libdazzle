action_headers = [
  'dzl-action-group.h',
  'dzl-child-property-action.h',
  'dzl-properties-group.h',
  'dzl-settings-flag-action.h',
  'dzl-widget-action-group.h',
]

action_sources = [
  'dzl-child-property-action.c',
  'dzl-properties-group.c',
  'dzl-settings-flag-action.c',
  'dzl-widget-action-group.c',
]

libdazzle_public_headers += files(action_headers)
libdazzle_public_sources += files(action_sources)

install_headers(action_headers, subdir: join_paths(libdazzle_header_subdir, 'actions'))
