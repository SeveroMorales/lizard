# Dependencies

GPlugin depends on the following at a bare minimum:

 * glib-2.0 >= 2.40.0
 * gobject-introspection, libgirepository1.0-dev
 * meson >= 0.42.0
 * gettext
 * help2man
 * a C compiler

A full build (enabled by default) depends on the following:

 * gtk-4
 * python3-dev, python-gi-dev, python3-gi
 * libperl-dev, libglib-perl, libglib-object-introspection-perl
 * liblua5.1-0-dev, lua-lgi
 * valac

All of these packages and their development headers need to be installed
prior to building GPlugin.

# Building

GPlugin uses [meson](https://mesonbuild.com/) as its build system.  As such
compiling is a little bit different than your typical `./configure`, `make`,
`sudo make install`.

Meson requires you to build in a separate directory than your source.  As such,
these instructions use a separate build directory.

To compile you need to run the following commands:

```
meson build
cd build
ninja install
```

If you want/need to tweak the build system (to enable/disable certain loaders)
you can do so at any time by using `meson configure` in the build directory.

