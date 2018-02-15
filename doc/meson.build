subdir('xml')

private_headers = ['config.h']

private_src_headers = [
  'dazzle.h',
  'graphing/dzl-column-private.h',
  'graphing/dzl-graph-column-private.h',
  'menus/dzl-menu-button-section.h',
  'panel/dzl-dock-bin-edge-private.h',
  'panel/dzl-dock-paned-private.h',
  'panel/dzl-tab-private.h',
  'prefs/dzl-preferences-bin-private.h',
  'prefs/dzl-preferences-group-private.h',
  'prefs/dzl-preferences-page-private.h',
  'search/dzl-fuzzy-index-private.h',
  'shortcuts/dzl-shortcut-private.h',
  'shortcuts/dzl-shortcuts-shortcut-private.h',
  'shortcuts/dzl-shortcuts-window-private.h',
  'tree/dzl-tree-private.h',
  'util/dzl-util-private.h',
  'widgets/dzl-list-box-private.h',
  'widgets/dzl-rect-helper.h',
]

foreach private_src_header : private_src_headers
  private_headers += [join_paths(meson.source_root(), 'src', private_src_header)]
endforeach

glib_prefix = dependency('glib-2.0').get_pkgconfig_variable('prefix')
glib_docpath = join_paths(glib_prefix, 'share', 'gtk-doc', 'html')
docpath = join_paths(get_option('datadir'), 'gtk-doc', 'html')

gnome.gtkdoc('libdazzle',
           main_xml: 'dazzle-docs.sgml',
            src_dir: [
                       join_paths(meson.source_root(), 'src'),
                       join_paths(meson.build_root(), 'src'),
                     ],
       dependencies: libdazzle_dep,
  gobject_typesfile: 'libdazzle.types',
          scan_args: [
                       '--rebuild-types',
                       '--ignore-headers=' + ' '.join(private_headers),
                     ],
       fixxref_args: [
                       '--html-dir=@0@'.format(docpath),
                       '--extra-dir=@0@'.format(join_paths(glib_docpath, 'glib')),
                       '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gobject')),
                       '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gio')),
                       '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gi')),
                       '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gtk3')),
                     ],
        install_dir: 'libdazzle',
            install: true)
