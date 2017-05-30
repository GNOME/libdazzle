tree_headers = [
  'dzl-tree-builder.h',
  'dzl-tree-node.h',
  'dzl-tree-types.h',
  'dzl-tree.h',
]

tree_sources = [
  'dzl-tree-builder.c',
  'dzl-tree.c',
  'dzl-tree-node.c',
]

libdazzle_public_headers += files(tree_headers)
libdazzle_public_sources += files(tree_sources)

install_headers(tree_headers, subdir: join_paths(libdazzle_header_subdir, 'tree'))
