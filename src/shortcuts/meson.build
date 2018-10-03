shortcuts_headers = [
  'dzl-shortcut-accel-dialog.h',
  'dzl-shortcut-chord.h',
  'dzl-shortcut-context.h',
  'dzl-shortcut-controller.h',
  'dzl-shortcut-label.h',
  'dzl-shortcut-manager.h',
  'dzl-shortcut-model.h',
  'dzl-shortcut-phase.h',
  'dzl-shortcut-simple-label.h',
  'dzl-shortcut-theme-editor.h',
  'dzl-shortcut-theme.h',
  'dzl-shortcut-tooltip.h',
  'dzl-shortcuts-group.h',
  'dzl-shortcuts-section.h',
  'dzl-shortcuts-shortcut.h',
  'dzl-shortcuts-window.h',
]

shortcuts_sources = [
  'dzl-shortcut-accel-dialog.c',
  'dzl-shortcut-chord.c',
  'dzl-shortcut-context.c',
  'dzl-shortcut-controller.c',
  'dzl-shortcut-label.c',
  'dzl-shortcut-manager.c',
  'dzl-shortcut-model.c',
  'dzl-shortcut-phase.c',
  'dzl-shortcut-simple-label.c',
  'dzl-shortcut-theme-editor.c',
  'dzl-shortcut-theme-load.c',
  'dzl-shortcut-theme-save.c',
  'dzl-shortcut-theme.c',
  'dzl-shortcut-tooltip.c',
  'dzl-shortcuts-group.c',
  'dzl-shortcuts-section.c',
  'dzl-shortcuts-shortcut.c',
  'dzl-shortcuts-window.c',
]

libdazzle_public_headers += files(shortcuts_headers)
libdazzle_public_sources += files(shortcuts_sources)
libdazzle_private_sources += files('dzl-shortcut-closure-chain.c')

install_headers(shortcuts_headers, subdir: join_paths(libdazzle_header_subdir, 'shortcuts'))
