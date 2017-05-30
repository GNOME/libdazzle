menus_headers = [
  'dzl-joined-menu.h',
  'dzl-menu-button-item.h',
  'dzl-menu-button-section.h',
  'dzl-menu-button.h',
  'dzl-menu-manager.h',
]

menus_sources = [
  'dzl-joined-menu.c',
  'dzl-menu-button-item.c',
  'dzl-menu-button-section.c',
  'dzl-menu-button.c',
  'dzl-menu-manager.c',
]

libdazzle_public_headers += files(menus_headers)
libdazzle_public_sources += files(menus_sources)

install_headers(menus_headers, subdir: join_paths(libdazzle_header_subdir, 'menus'))
