GNT: The GLib Ncurses Toolkit
=============================

[ ![Download](https://api.bintray.com/packages/pidgin/releases/libgnt/images/download.svg) ](https://bintray.com/pidgin/releases/libgnt/_latestVersion)
[ ![Issues](https://img.shields.io/badge/Issues-YouTrack-ee3b8b.svg) ](https://issues.imfreedom.org/issues/LIBGNT)

GNT is an ncurses toolkit for creating text-mode graphical user interfaces in a
fast and easy way. It is based on [GLib](https://wiki.gnome.org/Projects/GLib)
and [ncurses](https://www.gnu.org/software/ncurses/ncurses.html).

It was born out of the console-based UI, **Finch**, for the [libpurple
project](https://developer.pidgin.im/wiki/WhatIsLibpurple), but has now been
split into its own independent repository.

Building GNT
------------

To build, you will need [Meson](https://mesonbuild.com/),
[GLib](https://wiki.gnome.org/Projects/GLib),
[ncurses](https://www.gnu.org/software/ncurses/ncurses.html),
[libxml2](http://xmlsoft.org/), and [Python 3](https://www.python.org/). Exact
versions can be determined from the `meson.build` in the top-level directory.

On Debian-based systems, install `meson` `ninja-build` `gobject-introspection`
`libgirepository1.0-dev` `gtk-doc-tools` `libglib2.0-dev` `libxml2-dev`
`libncurses-dev` `libpython3-dev`.

On Fedora-based systems, install `meson` `ninja-build` `gobject-introspection`
`gtk-doc` `glib2-devel` `libxml2-devel` `ncurses-devel` `python3-devel`.

You can then run Meson and Ninja as usual (passing any `-D` options if necessary):

```bash
$ meson build
$ ninja -C build
$ ninja -C build install
```

Notes About Versioning
----------------------

This code was originally contained in the Pidgin/Finch/libpurple repository.
Versions were set by the top-level `configure.ac` in that repository. The GNT
version was independent of Pidgin and Finch and the tag. This repository
contains the original tags and so you may see multiple tags pointing to the
same commit. The GNT version *as it was released* is noted in the second column
of the table below.

A second `configure.ac` was included in the `libgnt` subdirectory that was
extracted to produce this repository. Unfortunately, we were not consistent
with updating the version in that copy. It was deemed too much work for little
gain to go back through the history to correct this error when producing this
split repository. As such, the third column of the table below contains the
version *as recorded in `configure.ac`* at the time of a tag.

Beginning with version 2.14.0, the code has been moved to a separate repository
and so the tag and the version will remain in sync.

Tagged   | Released   | Recorded
-------- | ---------- | ----------
v2.13.0  | 2.8.10     | 2.8.0devel
v2.12.0  | 2.8.10     | 2.8.0devel
v2.11.0  | 2.8.10     | 2.8.0devel
v2.10.12 | 2.8.10     | 2.8.0devel
v2.10.11 | 2.8.10     | 2.8.0devel
v2.10.10 | 2.8.10     | 2.8.0devel
v2.10.9  | 2.8.10     | 2.8.0devel
v2.10.8  | 2.8.10     | 2.8.0devel
v2.10.7  | 2.8.9      | 2.8.0devel
v2.10.6  | 2.8.9      | 2.8.0devel
v2.10.5  | 2.8.9      | 2.8.0devel
v2.10.4  | 2.8.9devel | 2.8.0devel
v2.10.3  | 2.8.9      | 2.8.0devel
v2.10.2  | 2.8.9      | 2.8.0devel
v2.10.1  | 2.8.9      | 2.8.0devel
v2.10.0  | 2.8.9      | 2.8.0devel
v2.9.0   | 2.8.9      | 2.8.0devel
v2.8.0   | 2.8.8      | 2.8.0devel
v2.7.11  | 2.8.7      | 2.8.0devel
v2.7.10  | 2.8.6      | 2.8.0devel
v2.7.9   | 2.8.5      | 2.8.0devel
v2.7.8   | 2.8.5      | 2.8.0devel
v2.7.7   | 2.8.4      | 2.8.0devel
v2.7.6   | 2.8.3      | 2.8.0devel
v2.7.5   | 2.8.2      | 2.8.0devel
v2.7.4   | 2.8.1      | 2.8.0devel
v2.7.3   | 2.8.0      | 2.8.0devel
v2.7.2   | 2.7.2      | 2.7.0devel
v2.7.1   | 2.7.1      | 2.7.0devel
v2.7.0   | 2.7.0      | 2.7.0devel
v2.6.6   | 2.6.6      | 2.6.2devel
v2.6.5   | 2.6.5      | 2.6.2devel
v2.6.4   | 2.6.4      | 2.6.2devel
v2.6.3   | 2.6.3      | 2.6.2devel
v2.6.2   | 2.6.2      | 2.6.2devel
v2.6.1   | 2.6.1      | 2.5.0
v2.6.0   | 2.6.0      | 2.5.0
v2.5.9   | 2.5.9      | 2.5.6
v2.5.8   | 2.5.8      | 2.5.6
v2.5.7   | 2.5.7      | 2.5.6
v2.5.6   | 2.5.6      | 2.5.6
v2.5.5   | 2.5.5      | 2.5.0
v2.5.4   | 2.5.4      | 2.5.0
v2.5.3a  | 2.5.3      | 2.5.0
v2.5.3   | 2.5.3devel | 2.5.0
v2.5.2   | 2.5.2      | 2.5.0
v2.5.1   | 2.5.1      | 2.5.0
v2.5.0   | 2.5.0      | 2.5.0
v2.4.3   | 2.4.3      | 2.4.2
v2.4.2   | 2.4.2      | 2.4.2
v2.4.1   | 2.4.1      | 2.4.1
v2.4.0   | 2.4.0      | 2.4.0devel
v2.3.1   | 2.3.1      | 2.3.1
v2.3.0   | 2.3.0      | 2.3.0
v2.2.2   | 2.2.2      | 2.2.0devel
v2.2.1   | 2.2.1      | 2.2.0devel
v2.2.0   | 2.2.0      | 2.2.0devel
v2.1.1   | 2.1.0      | 2.1.0devel
v2.1.0   | 2.0.0      | 2.0.0devel
v2.0.2   | 1.0.2      | 1.0.0beta7
v2.0.1   | 1.0.1      | 1.0.0beta7
v2.0.0   | 1.0.0      | 1.0.0beta7
