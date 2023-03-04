# GPlugin

[ ![Download](https://api.bintray.com/packages/pidgin/releases/GPlugin/images/download.svg) ](https://bintray.com/pidgin/releases/GPlugin/_latestVersion)
[ ![Issues](https://img.shields.io/badge/Issues-YouTrack-ee3b8b.svg) ](https://issues.imfreedom.org/issues/GPLUGIN)



GPlugin is a GObject based library that implements a reusable plugin system.
It supports loading plugins in multiple other languages via loaders.  It relies
heavily on [GObjectIntrospection](https://gi.readthedocs.io/) to expose its API
to the other languages.

It has a simple API which makes it very easy to use in your application.
For more information on using GPlugin in your application, please see the
[embedding](https://docs.pidgin.im/gplugin/latest/chapter-embedding.html) page.

## History

GPlugin has a bit of history, you can read more about it in
[HISTORY.md](HISTORY.md)

## Language Support

GPlugin currently supports plugins written in C/C++, Lua, Python3, and Vala.

## API Reference

The in-development API reference for the development branch can be found at
[docs.imfreedom.org/gplugin/](https://docs.imfreedom.org/gplugin/) for the core
library and
[docs.imfreedom.org/gplugin-gtk4/](https://docs.imfreedom.org/gplugin-gtk4/) for
the GTK4 integration.

