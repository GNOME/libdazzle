animation_headers = [
  'dzl-animation.h',
  'dzl-box-theatric.h',
]

animation_sources = [
  'dzl-animation.c',
  'dzl-box-theatric.c',
]

libdazzle_public_headers += files(animation_headers)
libdazzle_public_sources += files(animation_sources)
libdazzle_private_sources += files('dzl-frame-source.c')

install_headers(animation_headers, subdir: join_paths(libdazzle_header_subdir, 'animation'))
