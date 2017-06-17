example_resources = gnome.compile_resources(
  'example-resources',
  'example.gresources.xml',
  c_name: 'example',
)

example_application_sources = [
  'example-application.c',
  'example-application.h',
  'example-document.c',
  'example-document.h',
  'example-document-view.c',
  'example-document-view.h',
  'example-window.c',
  'example-window.h',
  'main.c',
  example_resources,
]

example_application = executable('example-application', example_application_sources,
  dependencies: libdazzle_dep,
)
