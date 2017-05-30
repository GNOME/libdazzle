
libdazzle_header_subdir = join_paths(package_subdir, 'libdazzle-' + apiversion)
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

debug_data = configuration_data()
debug_data.set10('ENABLE_TRACING', get_option('enable_tracing'))

dzl_debug_h = configure_file(
          input: 'dzl-debug.h.in',
         output: 'dzl-debug.h',
  configuration: debug_data)

libdazzle_generated_headers = [
  dzl_debug_h,
  dzl_version_h,
]

non_introspectable_public_headers = [
  'util/dzl-int-pair.h',
]

libdazzle_public_headers = [
  'dazzle.h',
]

subdir('actions')
subdir('animation')
subdir('app')
subdir('bindings')
subdir('cache')
subdir('files')
subdir('graphing')
subdir('menus')
subdir('panel')
subdir('search')
subdir('settings')
subdir('shortcuts')
subdir('statemachine')
subdir('suggestions')
subdir('theming')
subdir('tree')
subdir('util')
subdir('widgets')

libdazzle_public_sources = [
  'actions/dzl-child-property-action.c',
  'actions/dzl-properties-group.c',
  'actions/dzl-settings-flag-action.c',
  'actions/dzl-widget-action-group.c',

  'animation/dzl-animation.c',
  'animation/dzl-box-theatric.c',

  'app/dzl-application.c',
  'app/dzl-application-window.c',

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

  'menus/dzl-joined-menu.c',
  'menus/dzl-menu-button.c',
  'menus/dzl-menu-button-item.c',
  'menus/dzl-menu-button-section.c',
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

  'pathbar/dzl-path.c',
  'pathbar/dzl-path-bar.c',
  'pathbar/dzl-path-element.c',

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
  'widgets/dzl-counters-window.c',
  'widgets/dzl-elastic-bin.c',
  'widgets/dzl-empty-state.c',
  'widgets/dzl-entry-box.c',
  'widgets/dzl-file-chooser-entry.c',
  'widgets/dzl-list-box.c',
  'widgets/dzl-list-box-row.c',
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
  non_introspectable_public_headers,
  libdazzle_generated_headers,
  libdazzle_public_sources,

  'animation/dzl-frame-source.c',

  'graphing/dzl-graph-column-private.h',

  'panel/dzl-dock-bin-edge-private.h',
  'panel/dzl-dock-paned-private.h',
  'panel/dzl-tab-private.h',

  'prefs/dzl-preferences-bin-private.h',
  'prefs/dzl-preferences-group-private.h',
  'prefs/dzl-preferences-page-private.h',

  'search/dzl-fuzzy-index-private.h',

  'shortcuts/dzl-shortcut-closure-chain.c',

  'tree/dzl-tree-private.h',

  'util/dzl-util-private.h',

  'widgets/dzl-list-box-private.h',
  'widgets/dzl-rect-helper.c',

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

libdazzle_map = join_paths(meson.current_source_dir(), 'dazzle.map')

libdazzle = shared_library(
  'dazzle-' + apiversion,
  libdazzle_sources,

         link_depends: libdazzle_map,
               c_args: libdazzle_args,
            link_args: [ '-Wl,--version-script,' + libdazzle_map ],
         dependencies: libdazzle_deps,
  include_directories: [ root_inc, src_inc ],
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
             extra_args: [ '--c-include=dazzle.h', '--quiet' ],
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

pkgg = import('pkgconfig')

pkgg.generate(
    libraries: [libdazzle],
      subdirs: libdazzle_header_subdir,
      version: meson.project_version(),
         name: 'Dazzle',
     filebase: 'libdazzle-@0@'.format(apiversion),
  description: 'Razzle Dazzle for Gtk+ 3.x applications',
     requires: 'gtk+-3.0',
  install_dir: join_paths(libdir, 'pkgconfig'),
)
