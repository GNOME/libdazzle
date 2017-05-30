panel_headers = [
  'dzl-dock-bin-edge.h',
  'dzl-dock-bin.h',
  'dzl-dock-item.h',
  'dzl-dock-manager.h',
  'dzl-dock-overlay-edge.h',
  'dzl-dock-overlay.h',
  'dzl-dock-paned.h',
  'dzl-dock-revealer.h',
  'dzl-dock-stack.h',
  'dzl-dock-transient-grab.h',
  'dzl-dock-types.h',
  'dzl-dock-widget.h',
  'dzl-dock-window.h',
  'dzl-dock.h',
  'dzl-tab-strip.h',
  'dzl-tab.h',
]

panel_sources = [
  'dzl-dock-bin-edge.c',
  'dzl-dock-bin.c',
  'dzl-dock-item.c',
  'dzl-dock-manager.c',
  'dzl-dock-overlay-edge.c',
  'dzl-dock-overlay.c',
  'dzl-dock-paned.c',
  'dzl-dock-revealer.c',
  'dzl-dock-stack.c',
  'dzl-dock-transient-grab.c',
  'dzl-dock-widget.c',
  'dzl-dock-window.c',
  'dzl-dock.c',
  'dzl-tab-strip.c',
  'dzl-tab.c',
]

libdazzle_public_headers += files(panel_headers)
libdazzle_public_sources += files(panel_sources)

install_headers(panel_headers, subdir: join_paths(libdazzle_header_subdir, 'panel'))
