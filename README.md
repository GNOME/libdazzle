# Dazzle

The libdazzle library is a companion library to GObject and Gtk+.

** This project is basically end-of-life **

It was never really meant to be used outside of Builder and Sysprof projects as it was a convenient spot to share code.

Ideally, GTK 4 has what you need.

If you find yourself using things like search, action helpers, etc you should just copy that code into your project.
If you need paneling, https://gitlab.gnome.org/chergert/libpanel has some of that, albeit has not made an official release.
If you need shortcuts, GTK 4 has better solutions built in.
If you need suggestion/searching, you can just us popovers in GTK 4.

## Language Support

libdazzle, as you might imagine, is written in C.
We find C the most convenient language when it comes down to interoperability with other language runtimes.
libdazzle supports GObject Introspection and vapi meaning you can use libdazzle from a wide variety of languages including:

 - Python 2.7 or 3.x
 - JavaScript (using Gjs/Spidermonkey)
 - Lua/Luajit
 - Perl
 - Rust
 - Vala

and many others that implement binding support for GObject Introspection.

## License

libdazzle is licensed under the GPLv3+.
We *DO NOT* require copyright attribution to contribute to libdazzle.

## Building

We use the meson (and thereby Ninja) build system for libdazzle.
The quickest way to get going is to do the following:

```sh
meson . build
cd build
ninja
ninja install
```

If you need control over installation paths, see `meson --help`.
Here is a fairly common way to configure libdazzle.

```sh
meson --prefix=/opt/gnome --libdir=/opt/gnome/lib . build
```

## Components

libdazzle has a wide range of components from utilities for `GIO`, widgets for `Gtk`, an animation framework, state machines, paneling and high-performance counters.

### TODO: overview of components

Until we have an overrview here, check out the src/ directory.

