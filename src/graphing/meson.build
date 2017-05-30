graphing_headers = [
  'dzl-cpu-graph.h',
  'dzl-cpu-model.h',
  'dzl-graph-column.h',
  'dzl-graph-line-renderer.h',
  'dzl-graph-model.h',
  'dzl-graph-renderer.h',
  'dzl-graph-view.h',
]

graphing_sources = [
  'dzl-cpu-graph.c',
  'dzl-cpu-model.c',
  'dzl-graph-column.c',
  'dzl-graph-line-renderer.c',
  'dzl-graph-model.c',
  'dzl-graph-renderer.c',
  'dzl-graph-view.c',
]

libdazzle_public_headers += files(graphing_headers)
libdazzle_public_sources += files(graphing_sources)

install_headers(graphing_headers, subdir: join_paths(libdazzle_header_subdir, 'graphing'))
