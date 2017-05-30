libdazzle_api_version = '1.0'

libdazzle_resources = gnome.compile_resources(
  'dzl-resources',
  'dazzle.gresources.xml',

  c_name: 'dzl',
)

libdazzle_generated_headers = []

libdazzle_public_headers = [
  'dazzle.h',

  'actions/dzl-child-property-action.h',
  'actions/dzl-settings-flag-action.h',
  'actions/dzl-widget-action-group.h',

  'animation/dzl-animation.h',
  'animation/dzl-box-theatric.h',

  'bindings/dzl-binding-group.h',
  'bindings/dzl-signal-group.h',

  'cache/dzl-task-cache.h',

  'files/dzl-directory-reaper.h',

  'menus/dzl-menu-manager.h',

  'panel/dzl-dock-bin-edge.h',
  'panel/dzl-dock-bin.h',
  'panel/dzl-dock-item.h',
  'panel/dzl-dock-manager.h',
  'panel/dzl-dock-overlay-edge.h',
  'panel/dzl-dock-overlay.h',
  'panel/dzl-dock-paned.h',
  'panel/dzl-dock-revealer.h',
  'panel/dzl-dock-stack.h',
  'panel/dzl-dock-transient-grab.h',
  'panel/dzl-dock-types.h',
  'panel/dzl-dock-widget.h',
  'panel/dzl-dock-window.h',
  'panel/dzl-dock.h',
  'panel/dzl-tab-strip.h',
  'panel/dzl-tab.h',

  'settings/dzl-settings-sandwich.h',

  'shortcuts/dzl-shortcut-accel-dialog.h',
  'shortcuts/dzl-shortcut-chord.h',
  'shortcuts/dzl-shortcut-context.h',
  'shortcuts/dzl-shortcut-controller.h',
  'shortcuts/dzl-shortcut-label.h',
  'shortcuts/dzl-shortcut-manager.h',
  'shortcuts/dzl-shortcut-model.h',
  'shortcuts/dzl-shortcut-theme-editor.h',
  'shortcuts/dzl-shortcut-theme.h',
  'shortcuts/dzl-shortcuts-group.h',
  'shortcuts/dzl-shortcuts-section.h',
  'shortcuts/dzl-shortcuts-shortcut.h',
  'shortcuts/dzl-shortcuts-window.h',

  'statemachine/dzl-state-machine-buildable.h',
  'statemachine/dzl-state-machine.h',

  'suggestions/dzl-suggestion-entry-buffer.h',
  'suggestions/dzl-suggestion-entry.h',
  'suggestions/dzl-suggestion-popover.h',
  'suggestions/dzl-suggestion-row.h',
  'suggestions/dzl-suggestion.h',

  'theming/dzl-css-provider.h',
  'theming/dzl-theme-manager.h',

  'util/dzl-cairo.h',
  'util/dzl-date-time.h',
  'util/dzl-dnd.h',
  'util/dzl-heap.h',
  'util/dzl-pango.h',
  'util/dzl-rgba.h',

  'widgets/dzl-bin.h',
  'widgets/dzl-box.h',
  'widgets/dzl-centering-bin.h',
  'widgets/dzl-column-layout.h',
  'widgets/dzl-elastic-bin.h',
  'widgets/dzl-empty-state.h',
  'widgets/dzl-entry-box.h',
  'widgets/dzl-file-chooser-entry.h',
  'widgets/dzl-list-box.h',
  'widgets/dzl-multi-paned.h',
  'widgets/dzl-pill-box.h',
  'widgets/dzl-priority-box.h',
  'widgets/dzl-progress-button.h',
  'widgets/dzl-radio-box.h',
  'widgets/dzl-scrolled-window.h',
  'widgets/dzl-search-bar.h',
  'widgets/dzl-simple-label.h',
  'widgets/dzl-simple-popover.h',
  'widgets/dzl-slider.h',
  'widgets/dzl-stack-list.h',
  'widgets/dzl-three-grid.h',
]

libdazzle_public_sources = [
  'actions/dzl-child-property-action.c',
  'actions/dzl-settings-flag-action.c',
  'actions/dzl-widget-action-group.c',

  'animation/dzl-animation.c',
  'animation/dzl-box-theatric.c',

  'bindings/dzl-binding-group.c',
  'bindings/dzl-signal-group.c',

  'cache/dzl-task-cache.c',

  'files/dzl-directory-reaper.c',

  'menus/dzl-menu-manager.c',

  'panel/dzl-dock-bin-edge.c',
  'panel/dzl-dock-bin.c',
  'panel/dzl-dock-item.c',
  'panel/dzl-dock-manager.c',
  'panel/dzl-dock-overlay-edge.c',
  'panel/dzl-dock-overlay.c',
  'panel/dzl-dock-paned.c',
  'panel/dzl-dock-revealer.c',
  'panel/dzl-dock-stack.c',
  'panel/dzl-dock-transient-grab.c',
  'panel/dzl-dock-widget.c',
  'panel/dzl-dock-window.c',
  'panel/dzl-dock.c',
  'panel/dzl-tab-strip.c',
  'panel/dzl-tab.c',

  'settings/dzl-settings-sandwich.c',

  'shortcuts/dzl-shortcut-accel-dialog.c',
  'shortcuts/dzl-shortcut-chord.c',
  'shortcuts/dzl-shortcut-context.c',
  'shortcuts/dzl-shortcut-controller.c',
  'shortcuts/dzl-shortcut-label.c',
  'shortcuts/dzl-shortcut-manager.c',
  'shortcuts/dzl-shortcut-model.c',
  'shortcuts/dzl-shortcut-theme-editor.c',
  'shortcuts/dzl-shortcut-theme-load.c',
  'shortcuts/dzl-shortcut-theme-save.c',
  'shortcuts/dzl-shortcut-theme.c',
  'shortcuts/dzl-shortcuts-group.c',
  'shortcuts/dzl-shortcuts-section.c',
  'shortcuts/dzl-shortcuts-shortcut.c',
  'shortcuts/dzl-shortcuts-window.c',

  'statemachine/dzl-state-machine-buildable.c',
  'statemachine/dzl-state-machine.c',

  'suggestions/dzl-suggestion-entry-buffer.c',
  'suggestions/dzl-suggestion-entry.c',
  'suggestions/dzl-suggestion-popover.c',
  'suggestions/dzl-suggestion-row.c',
  'suggestions/dzl-suggestion.c',

  'util/dzl-cairo.c',
  'util/dzl-date-time.c',
  'util/dzl-dnd.c',
  'util/dzl-heap.c',
  'util/dzl-pango.c',
  'util/dzl-rgba.c',
  'util/dzl-util.c',

  'theming/dzl-css-provider.c',
  'theming/dzl-theme-manager.c',

  'widgets/dzl-bin.c',
  'widgets/dzl-box.c',
  'widgets/dzl-centering-bin.c',
  'widgets/dzl-column-layout.c',
  'widgets/dzl-elastic-bin.c',
  'widgets/dzl-empty-state.c',
  'widgets/dzl-entry-box.c',
  'widgets/dzl-file-chooser-entry.c',
  'widgets/dzl-list-box.c',
  'widgets/dzl-multi-paned.c',
  'widgets/dzl-pill-box.c',
  'widgets/dzl-priority-box.c',
  'widgets/dzl-progress-button.c',
  'widgets/dzl-radio-box.c',
  'widgets/dzl-scrolled-window.c',
  'widgets/dzl-search-bar.c',
  'widgets/dzl-simple-label.c',
  'widgets/dzl-simple-popover.c',
  'widgets/dzl-slider.c',
  'widgets/dzl-stack-list.c',
  'widgets/dzl-three-grid.c',
]

libdazzle_sources = [
  libdazzle_generated_headers,
  libdazzle_public_headers,
  libdazzle_public_sources,

  'animation/dzl-frame-source.c',
  'animation/dzl-frame-source.h',

  'panel/dzl-dock-bin-edge-private.h',
  'panel/dzl-dock-paned-private.h',
  'panel/dzl-tab-private.h',

  'shortcuts/dzl-shortcut-closure-chain.c',
  'shortcuts/dzl-shortcut-closure-chain.h',
  'shortcuts/dzl-shortcut-private.h',
  'shortcuts/dzl-shortcuts-shortcut-private.h',
  'shortcuts/dzl-shortcuts-window-private.h',

  'util/dzl-counter.c',
  'util/dzl-counter.h',
  'util/dzl-util-private.h',

  'widgets/dzl-rect-helper.c',
  'widgets/dzl-rect-helper.h',

  # Other dependencies
  libdazzle_resources,
]

libdazzle_deps = [
  dependency('gio-2.0'),
  dependency('gmodule-2.0'),
  dependency('gtk+-3.0'),
  dependency('libpeas-1.0'),
  cc.find_library('m', required: false),
  cc.find_library('rt', required: false),
]

libdazzle_args = []
if get_option('enable_rdtscp')
  libdazzle_args += '-DEGG_HAVE_RDTSCP'
endif

libdazzle = shared_library(
  'libdazzle-' + libdazzle_api_version,
  libdazzle_sources,

        c_args: libdazzle_args,
  dependencies: libdazzle_deps,
       install: true,
)

libdazzle_dep = declare_dependency(
              sources: libdazzle_sources,
         dependencies: libdazzle_deps,
            link_with: libdazzle,
  include_directories: include_directories('.'),
)

if get_option('with_introspection')

  libdazzle_gir = gnome.generate_gir(libdazzle,
    sources: libdazzle_generated_headers + libdazzle_public_headers + libdazzle_public_sources,
    nsversion: libdazzle_api_version,
    namespace: 'Dazzle',
    symbol_prefix: 'dzl',
    identifier_prefix: 'Dzl',
    link_with: libdazzle,
    includes: ['Gio-2.0', 'Gtk-3.0'],
    install: true,
    install_dir_gir: girdir,
    install_dir_typelib: typelibdir,
    extra_args: [
      '--c-include=dazzle.h',
    ]
  )

  if get_option('with_vapi')

    libdazzle_vapi = gnome.generate_vapi('libdazzle-' + libdazzle_api_version,
      sources: libdazzle_gir[0],
      packages: [ 'gio-2.0', 'gtk+-3.0' ],
      install: true,
      install_dir: vapidir,
    )

  endif
endif
