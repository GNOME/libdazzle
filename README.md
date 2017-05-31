# Dazzle

The libdazzle library is a companion library to GObject and Gtk+.
It provides various features that we wish were in the underlying library but cannot for various reasons.
In most cases, they are wildly out of scope for those libraries.
In other cases, our design isn't quite generic enough to work for everyone.

While we don't want to blind our users, we do think of dazzle as something you haven't seen before.
As we improve our implementations in libdazzle, we do think that bits of libdazzle can be migrated into upstream projects.

Currently, the primary consumer of libdazzle is the Builder IDE.
Most of this code was extracted from Builder so that it could be used by others.

The libdazzle project is heavily opinionated, and tends to gravitate towards design that matches the GNOME 3 human interface guidelines.

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

