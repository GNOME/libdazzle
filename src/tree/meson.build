tree_headers = [
  'dzl-tree-builder.h',
  'dzl-tree-node.h',
  'dzl-tree-types.h',
  'dzl-tree.h',
  'dzl-list-store-adapter.h',
]

tree_sources = [
  'dzl-tree-builder.c',
  'dzl-tree.c',
  'dzl-tree-node.c',
  'dzl-tree-store.c',
  'dzl-list-store-adapter.c',
]

tree_enums_headers = [
  'dzl-tree-types.h',
]

libdazzle_public_headers += files(tree_headers)
libdazzle_public_sources += files(tree_sources)
dzl_enum_headers += files(tree_enums_headers)

install_headers(tree_headers, subdir: join_paths(libdazzle_header_subdir, 'tree'))
