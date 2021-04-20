util_headers = [
  'dzl-cairo.h',
  'dzl-cancellable.h',
  'dzl-date-time.h',
  'dzl-dnd.h',
  'dzl-file-manager.h',
  'dzl-gdk.h',
  'dzl-gtk.h',
  'dzl-heap.h',
  'dzl-int-pair.h',
  'dzl-list-model-filter.h',
  'dzl-macros.h',
  'dzl-pango.h',
  'dzl-read-only-list-model.h',
  'dzl-rgba.h',
  'dzl-ring.h',
  'dzl-variant.h',
]

util_sources = [
  'dzl-cairo.c',
  'dzl-cancellable.c',
  'dzl-date-time.c',
  'dzl-dnd.c',
  'dzl-file-manager.c',
  'dzl-gdk.c',
  'dzl-gtk.c',
  'dzl-heap.c',
  'dzl-list-model-filter.c',
  'dzl-pango.c',
  'dzl-read-only-list-model.c',
  'dzl-rgba.c',
  'dzl-ring.c',
  'dzl-util.c',
  'dzl-variant.c',
]

# No counters if building on win32 for now.
# When avaiable remove G_OS_WIN32 checks in src/dazzel.h.
# They need explicit porting to that platform and should
# probably just wrap the eventtrace API or something.
if host_machine.system() != 'windows'
  util_headers += ['dzl-counter.h']
  util_sources += ['dzl-counter.c']
endif

libdazzle_public_headers += files(util_headers)
libdazzle_public_sources += files(util_sources)

install_headers(util_headers, subdir: join_paths(libdazzle_header_subdir, 'util'))
