
libdazzle_header_subdir = 'libdazzle-' + apiversion
libdazzle_header_dir = join_paths(get_option('includedir'), libdazzle_header_subdir)

libdazzle_resources = gnome.compile_resources(
  'dzl-resources',
  'dazzle.gresources.xml',

  c_name: 'dzl',
)

version_data = configuration_data()
version_data.set('DZL_MAJOR_VERSION', dazzle_version_major)
version_data.set('DZL_MINOR_VERSION', dazzle_version_minor)
version_data.set('DZL_MICRO_VERSION', dazzle_version_micro)
version_data.set('DZL_VERSION', meson.project_version())

dzl_version_h = configure_file(
          input: 'dzl-version.h.in',
         output: 'dzl-version.h',
    install_dir: libdazzle_header_dir,
        install: true,
  configuration: version_data)

libdazzle_generated_headers = [
  dzl_version_h,
]

libdazzle_public_headers = [
  'dazzle.h',

  'actions/dzl-child-property-action.h',
  'actions/dzl-settings-flag-action.h',
  'actions/dzl-widget-action-group.h',

  'animation/dzl-animation.h',
  'animation/dzl-box-theatric.h',

  'app/dzl-application.h',

  'bindings/dzl-binding-group.h',
  'bindings/dzl-signal-group.h',

  'cache/dzl-task-cache.h',

  'files/dzl-directory-model.h',
  'files/dzl-directory-reaper.h',

  'graphing/dzl-cpu-graph.h',
  'graphing/dzl-cpu-model.h',
  'graphing/dzl-graph-column.h',
  'graphing/dzl-graph-line-renderer.h',
  'graphing/dzl-graph-model.h',
  'graphing/dzl-graph-renderer.h',
  'graphing/dzl-graph-view.h',

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

  'prefs/dzl-preferences-bin.h',
  'prefs/dzl-preferences-entry.h',
  'prefs/dzl-preferences-file-chooser-button.h',
  'prefs/dzl-preferences-flow-box.h',
  'prefs/dzl-preferences-font-button.h',
  'prefs/dzl-preferences-group.h',
  'prefs/dzl-preferences-page.h',
  'prefs/dzl-preferences-spin-button.h',
  'prefs/dzl-preferences-switch.h',
  'prefs/dzl-preferences-view.h',
  'prefs/dzl-preferences.h',

  'search/dzl-fuzzy-index-builder.h',
  'search/dzl-fuzzy-index-cursor.h',
  'search/dzl-fuzzy-index-match.h',
  'search/dzl-fuzzy-index.h',
  'search/dzl-fuzzy-mutable-index.h',
  'search/dzl-levenshtein.h',
  'search/dzl-pattern-spec.h',
  'search/dzl-trie.h',

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

  'tree/dzl-tree-builder.h',
  'tree/dzl-tree-node.h',
  'tree/dzl-tree-types.h',
  'tree/dzl-tree.h',

  'util/dzl-cairo.h',
  'util/dzl-counter.h',
  'util/dzl-date-time.h',
  'util/dzl-dnd.h',
  'util/dzl-file-manager.h',
  'util/dzl-gdk.h',
  'util/dzl-gtk.h',
  'util/dzl-heap.h',
  'util/dzl-pango.h',
  'util/dzl-rgba.h',
  'util/dzl-ring.h',
  'util/dzl-variant.h',

  'widgets/dzl-bin.h',
  'widgets/dzl-bolding-label.h',
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
  'widgets/dzl-progress-menu-button.h',
  'widgets/dzl-progress-icon.h',
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

  'app/dzl-application.c',

  'bindings/dzl-binding-group.c',
  'bindings/dzl-signal-group.c',

  'cache/dzl-task-cache.c',

  'files/dzl-directory-model.c',
  'files/dzl-directory-reaper.c',

  'graphing/dzl-cpu-graph.c',
  'graphing/dzl-cpu-model.c',
  'graphing/dzl-graph-column.c',
  'graphing/dzl-graph-line-renderer.c',
  'graphing/dzl-graph-model.c',
  'graphing/dzl-graph-renderer.c',
  'graphing/dzl-graph-view.c',

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

  'prefs/dzl-preferences-bin.c',
  'prefs/dzl-preferences-entry.c',
  'prefs/dzl-preferences-file-chooser-button.c',
  'prefs/dzl-preferences-flow-box.c',
  'prefs/dzl-preferences-font-button.c',
  'prefs/dzl-preferences-group.c',
  'prefs/dzl-preferences-page.c',
  'prefs/dzl-preferences-spin-button.c',
  'prefs/dzl-preferences-switch.c',
  'prefs/dzl-preferences-view.c',
  'prefs/dzl-preferences.c',

  'search/dzl-fuzzy-index-builder.c',
  'search/dzl-fuzzy-index-cursor.c',
  'search/dzl-fuzzy-index-match.c',
  'search/dzl-fuzzy-index.c',
  'search/dzl-fuzzy-mutable-index.c',
  'search/dzl-levenshtein.c',
  'search/dzl-pattern-spec.c',
  'search/dzl-trie.c',

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

  'tree/dzl-tree-builder.c',
  'tree/dzl-tree.c',
  'tree/dzl-tree-node.c',

  'util/dzl-cairo.c',
  'util/dzl-counter.c',
  'util/dzl-date-time.c',
  'util/dzl-dnd.c',
  'util/dzl-file-manager.c',
  'util/dzl-gdk.c',
  'util/dzl-gtk.c',
  'util/dzl-heap.c',
  'util/dzl-pango.c',
  'util/dzl-rgba.c',
  'util/dzl-ring.c',
  'util/dzl-util.c',
  'util/dzl-variant.c',

  'theming/dzl-css-provider.c',
  'theming/dzl-theme-manager.c',

  'widgets/dzl-bin.c',
  'widgets/dzl-bolding-label.c',
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
  'widgets/dzl-progress-menu-button.c',
  'widgets/dzl-progress-icon.c',
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

  'graphing/dzl-column-private.h',
  'graphing/dzl-graph-column-private.h',

  'panel/dzl-dock-bin-edge-private.h',
  'panel/dzl-dock-paned-private.h',
  'panel/dzl-tab-private.h',

  'prefs/dzl-preferences-bin-private.h',
  'prefs/dzl-preferences-group-private.h',
  'prefs/dzl-preferences-page-private.h',

  'search/dzl-fuzzy-index-private.h',

  'shortcuts/dzl-shortcut-closure-chain.c',
  'shortcuts/dzl-shortcut-closure-chain.h',
  'shortcuts/dzl-shortcut-private.h',
  'shortcuts/dzl-shortcuts-shortcut-private.h',
  'shortcuts/dzl-shortcuts-window-private.h',

  'tree/dzl-tree-private.h',

  'util/dzl-util-private.h',

  'widgets/dzl-rect-helper.c',
  'widgets/dzl-rect-helper.h',

  libdazzle_resources,
]

libdazzle_deps = [
  dependency('gio-2.0'),
  dependency('gmodule-2.0'),
  dependency('gtk+-3.0'),
  cc.find_library('m', required: false),
  cc.find_library('rt', required: false),
]

libdazzle_args = []
if get_option('enable_rdtscp')
  libdazzle_args += '-DDZL_HAVE_RDTSCP'
endif

libdazzle = library(
  'dazzle-' + apiversion,
  libdazzle_sources,

  link_depends: 'dazzle.map',
        c_args: libdazzle_args,
     link_args: [ '-Wl,--version-script,' + join_paths(meson.current_source_dir(), 'dazzle.map') ],
  dependencies: libdazzle_deps,
     soversion: soversion,
       version: libversion,
       install: true,
)

libdazzle_dep = declare_dependency(
              sources: libdazzle_generated_headers,
         dependencies: libdazzle_deps,
            link_with: libdazzle,
  include_directories: include_directories('.'),
)

if get_option('with_introspection')

  libdazzle_gir = gnome.generate_gir(libdazzle,
                sources: libdazzle_generated_headers + libdazzle_public_headers + libdazzle_public_sources,
              nsversion: apiversion,
              namespace: 'Dazzle',
          symbol_prefix: 'dzl',
      identifier_prefix: 'Dzl',
              link_with: libdazzle,
               includes: ['Gio-2.0', 'Gtk-3.0'],
                install: true,
        install_dir_gir: girdir,
    install_dir_typelib: typelibdir,
             extra_args: [ '--c-include=dazzle.h' ],
  )

  if get_option('with_vapi')

    libdazzle_vapi = gnome.generate_vapi('libdazzle-' + apiversion,
            sources: libdazzle_gir[0],
           packages: [ 'gio-2.0', 'gtk+-3.0' ],
            install: true,
        install_dir: vapidir,
      metadata_dirs: [ meson.current_source_dir() ],
    )

  endif
endif

# TODO: This is ugly and should be handled upstream
foreach header: libdazzle_public_headers
  header_split = header.split('/')
  if header_split.length() == 2
    header_path = join_paths(libdazzle_header_subdir, header_split[0])
    install_headers(header, subdir: header_path)
  else
    install_headers(header, subdir: libdazzle_header_subdir)
  endif
endforeach

pkgg = import('pkgconfig')

pkgg.generate(
    libraries: [libdazzle],
      subdirs: 'libdazzle-@0@'.format(apiversion),
      version: meson.project_version(),
         name: 'Dazzle',
     filebase: 'libdazzle-@0@'.format(apiversion),
  description: 'Razzle Dazzle for Gtk+ 3.x applications',
     requires: 'gtk+-3.0',
)

