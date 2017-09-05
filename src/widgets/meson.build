widgets_headers = [
  'dzl-bin.h',
  'dzl-bolding-label.h',
  'dzl-box.h',
  'dzl-centering-bin.h',
  'dzl-column-layout.h',
  'dzl-elastic-bin.h',
  'dzl-empty-state.h',
  'dzl-entry-box.h',
  'dzl-file-chooser-entry.h',
  'dzl-list-box.h',
  'dzl-list-box-row.h',
  'dzl-multi-paned.h',
  'dzl-pill-box.h',
  'dzl-priority-box.h',
  'dzl-progress-button.h',
  'dzl-progress-menu-button.h',
  'dzl-progress-icon.h',
  'dzl-radio-box.h',
  'dzl-scrolled-window.h',
  'dzl-search-bar.h',
  'dzl-simple-label.h',
  'dzl-simple-popover.h',
  'dzl-slider.h',
  'dzl-stack-list.h',
  'dzl-three-grid.h',
]

widgets_sources = [
  'dzl-bin.c',
  'dzl-bolding-label.c',
  'dzl-box.c',
  'dzl-centering-bin.c',
  'dzl-column-layout.c',
  'dzl-elastic-bin.c',
  'dzl-empty-state.c',
  'dzl-entry-box.c',
  'dzl-file-chooser-entry.c',
  'dzl-list-box.c',
  'dzl-list-box-row.c',
  'dzl-multi-paned.c',
  'dzl-pill-box.c',
  'dzl-priority-box.c',
  'dzl-progress-button.c',
  'dzl-progress-menu-button.c',
  'dzl-progress-icon.c',
  'dzl-radio-box.c',
  'dzl-scrolled-window.c',
  'dzl-search-bar.c',
  'dzl-simple-label.c',
  'dzl-simple-popover.c',
  'dzl-slider.c',
  'dzl-stack-list.c',
  'dzl-three-grid.c',
]

if host_machine.system() != 'windows'
  widgets_headers += ['dzl-counters-window.h']
  widgets_sources += ['dzl-counters-window.c']
endif

libdazzle_public_headers += files(widgets_headers)
libdazzle_public_sources += files(widgets_sources)
libdazzle_private_sources += files('dzl-rect-helper.c')

install_headers(widgets_headers, subdir: join_paths(libdazzle_header_subdir, 'widgets'))
