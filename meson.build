project('libdazzle', 'c',
          version: '3.42.0',
          license: 'GPLv3+',
    meson_version: '>= 0.50.0',
  default_options: [ 'warning_level=1', 'buildtype=debugoptimized', 'c_std=gnu11' ],
)

version_arr = meson.project_version().split('.')
dazzle_version_major = version_arr[0].to_int()
dazzle_version_minor = version_arr[1].to_int()
dazzle_version_micro = version_arr[2].to_int()

apiversion = '1.0'
soversion = 0

if dazzle_version_minor.is_odd()
  dazzle_interface_age = 0
else
  dazzle_interface_age = dazzle_version_micro
endif

# maintaining compatibility with the previous libtool versioning
# current = minor * 100 + micro - interface
# revision = interface
current = dazzle_version_minor * 100 + dazzle_version_micro - dazzle_interface_age
revision = dazzle_interface_age
libversion = '@0@.@1@.@2@'.format(soversion, current, revision)

darwin_versions = [current + 1, '@0@.@1@'.format(current + 1, revision)]

package_string = 'libdazzle-@0@'.format(apiversion)

config_h = configuration_data()
config_h.set_quoted('GETTEXT_PACKAGE', package_string)
config_h.set_quoted('LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))

add_project_arguments([
  '-DHAVE_CONFIG_H',
  '-DDAZZLE_COMPILATION',
], language: 'c')

if host_machine.system() == 'darwin'
  add_project_arguments([
    '-DPLATFORM_OSX',
  ], language: 'c')
endif

root_inc = include_directories('.')
src_inc = include_directories('src')

cc = meson.get_compiler('c')

global_c_args = []
test_c_args = [
  '-Wcast-align',
  '-Wdeclaration-after-statement',
  '-Werror=address',
  '-Werror=array-bounds',
  '-Werror=empty-body',
  '-Werror=implicit',
  '-Werror=implicit-function-declaration',
  # fix with mkenums
  # '-Werror=incompatible-pointer-types',
  '-Werror=init-self',
  '-Werror=int-conversion',
  '-Werror=int-to-pointer-cast',
  '-Werror=main',
  '-Werror=misleading-indentation',
  '-Werror=missing-braces',
  '-Werror=missing-include-dirs',
  '-Werror=nonnull',
  '-Werror=overflow',
  '-Werror=parenthesis',
  '-Werror=pointer-arith',
  '-Werror=pointer-to-int-cast',
  '-Werror=redundant-decls',
  '-Werror=return-type',
  '-Werror=sequence-point',
  '-Werror=shadow',
  '-Werror=strict-prototypes',
  '-Werror=trigraphs',
  '-Werror=write-strings',
  '-Wformat-nonliteral',
  ['-Werror=format-security', '-Werror=format=2' ],
  '-Wignored-qualifiers',
  '-Wimplicit-function-declaration',
  '-Wlogical-op',
  # '-Wmissing-declarations',
  '-Wmissing-format-attribute',
  '-Wmissing-include-dirs',
  '-Wmissing-noreturn',
  '-Wnested-externs',
  '-Wno-cast-function-type',
  '-Wno-missing-field-initializers',
  '-Wno-sign-compare',
  '-Wno-unused-parameter',
  '-Wold-style-definition',
  '-Wpointer-arith',
  '-Wredundant-decls',
  '-Wstrict-prototypes',
  '-Wswitch-default',
  '-Wswitch-enum',
  '-Wuninitialized',
  '-Wunused',
  '-fno-strict-aliasing',
]
if get_option('buildtype') != 'plain'
  test_c_args += '-fstack-protector-strong'
endif
if get_option('enable_profiling')
  test_c_args += '-pg'
endif

foreach arg: test_c_args
  if cc.has_multi_arguments(arg)
    global_c_args += arg
  endif
endforeach

# Detect and set symbol visibility
hidden_visibility_args = []
if get_option('default_library') != 'static'
  if host_machine.system() == 'windows'
    config_h.set('DLL_EXPORT', true)
    if cc.get_id() == 'msvc' or cc.get_id() == 'clang-cl'
      config_h.set('_DZL_EXTERN', '__declspec(dllexport) extern')
    elif cc.has_argument('-fvisibility=hidden')
      config_h.set('_DZL_EXTERN', '__attribute__((visibility("default"))) __declspec(dllexport) extern')
      hidden_visibility_args = ['-fvisibility=hidden']
    endif
  elif cc.has_argument('-fvisibility=hidden')
    config_h.set('_DZL_EXTERN', '__attribute__((visibility("default"))) extern')
    hidden_visibility_args = ['-fvisibility=hidden']
  endif
endif

add_project_arguments(global_c_args, language: 'c')

release_args = []
global_link_args = []
test_link_args = [
  '-Wl,-z,relro',
  '-Wl,-z,now',
]
if not get_option('buildtype').startswith('debug')
  # TODO: Maybe reuse 'b_ndebug' option
  release_args += [ '-DG_DISABLE_ASSERT', '-DG_DISABLE_CAST_CHECKS' ]
  test_link_args += [
    '-Wl,-Bsymbolic',
    '-fno-plt',
  ]
endif
foreach link_arg: test_link_args
  if cc.links('int main () { return 0; }', name: link_arg, args: link_arg)
    global_link_args += link_arg
  endif
endforeach
add_project_link_arguments(global_link_args, language: 'c')

# Setup various paths that subdirectory meson.build files need
package_subdir = get_option('package_subdir') # When used as subproject
libdir = join_paths(get_option('libdir'), package_subdir)
girdir = join_paths(get_option('datadir'), package_subdir, 'gir-1.0')
typelibdir = join_paths(get_option('libdir'), package_subdir, 'girepository-1.0')
if package_subdir != ''
  # Use libexecdir to avoid subdirectories under bindir
  bindir = join_paths(get_option('libexecdir'), package_subdir)
  vapidir = join_paths(get_option('datadir'), package_subdir, 'vapi')
else
  bindir = get_option('bindir')
  vapidir = join_paths(get_option('datadir'), 'vala', 'vapi')
endif

configure_file(
         output: 'config.h',
  configuration: config_h,
)

i18n = import('i18n')
gnome = import('gnome')

subdir('src')
subdir('tools')
subdir('tests')
subdir('po')
subdir('examples/app')

if get_option('enable_gtk_doc')
  subdir('doc')
endif
