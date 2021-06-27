
libdazzle_header_subdir = join_paths(package_subdir, 'libdazzle-' + apiversion)
libdazzle_header_dir = join_paths(get_option('includedir'), libdazzle_header_subdir)

libdazzle_resources = gnome.compile_resources(
  'dzl-resources',
  'dazzle.gresources.xml',

  c_name: 'dzl',
)

dzl_enum_headers = []

version_data = configuration_data()
version_data.set('DZL_MAJOR_VERSION', dazzle_version_major)
version_data.set('DZL_MINOR_VERSION', dazzle_version_minor)
version_data.set('DZL_MICRO_VERSION', dazzle_version_micro)
version_data.set('DZL_VERSION', meson.project_version())

dzl_version_h = configure_file(
          input: 'dzl-version.h.in',
         output: 'dzl-version.h',
    install_dir: libdazzle_header_dir,
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

install_headers(['dazzle.h', 'dzl-version-macros.h'],
  subdir: libdazzle_header_subdir)

# Filled out in the subdirs
libdazzle_public_headers = []
libdazzle_public_sources = []
libdazzle_private_sources = []

subdir('actions')
subdir('animation')
subdir('app')
subdir('bindings')
subdir('cache')
subdir('files')
subdir('graphing')
subdir('menus')
subdir('panel')
subdir('pathbar')
subdir('prefs')
subdir('search')
subdir('settings')
subdir('shortcuts')
subdir('statemachine')
subdir('suggestions')
subdir('theming')
subdir('tree')
subdir('util')
subdir('widgets')

dzl_enums = gnome.mkenums('dzl-enums',
      h_template: 'dzl-enums.h.in',
      c_template: 'dzl-enums.c.in',
         sources: dzl_enum_headers,
  install_header: true,
     install_dir: libdazzle_header_dir,
)
libdazzle_public_sources += [dzl_enums[0]]
libdazzle_generated_headers += [dzl_enums[1]]

libdazzle_sources = [
  libdazzle_generated_headers,
  libdazzle_public_sources,
  libdazzle_private_sources,
  libdazzle_resources,
  'dzl-init.c',
]

libdazzle_deps = [
  dependency('gio-2.0', version: '>=2.50.0'),
  dependency('gmodule-2.0'),
  dependency('gtk+-3.0', version: '>=3.22.0'),
  cc.find_library('m', required: false),
  cc.find_library('rt', required: false),
]

libdazzle_args = []
libdazzle_args += hidden_visibility_args
if get_option('enable_rdtscp')
  libdazzle_args += '-DDZL_HAVE_RDTSCP'
endif

if host_machine.system() == 'darwin'
  libdazzle_args += '-xobjective-c'
endif

libdazzle = shared_library(
  'dazzle-' + apiversion,
  libdazzle_sources,
      darwin_versions: darwin_versions,
            soversion: 0,
               c_args: libdazzle_args + release_args,
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

libdazzle_package = 'libdazzle-@0@'.format(apiversion)

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
        export_packages: libdazzle_package,
             extra_args: [ '--c-include=dazzle.h', '--quiet', '-DDAZZLE_COMPILATION', ],
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
     filebase: libdazzle_package,
  description: 'Razzle Dazzle for Gtk+ 3.x applications',
     requires: 'gtk+-3.0',
  install_dir: join_paths(libdir, 'pkgconfig'),
)
