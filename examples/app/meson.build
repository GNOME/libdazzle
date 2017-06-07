example_resources = gnome.compile_resources(
  'example-resources',
  'example.gresources.xml',
  c_name: 'example',
)

example_application_sources = [
  'example-application.c',
  'example-application.h',
  'example-window.c',
  'example-window.h',
  'main.c',
  example_resources,
]

example_application = executable('example-application', example_application_sources,
        c_args: test_cflags,
     link_args: test_link_args,
  dependencies: libdazzle_dep,
)
