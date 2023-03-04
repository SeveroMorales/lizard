# Hasl

The Hassle-free Authentication and Security Layer client library

## About

This project grew out of necessity for [Pidgin 3](https://pidgin.im). We ran
into a number of issues with both cyrus-sasl and gsasl and finally decided we
would just write our own library.

Another part of this, was that we need to easily add additional SASL
mechanisms, and while both cyrus-sasl and gsasl allow this, their
implementations are very different from the rest of our GObject based code
base.

The name came from a Twitch viewer of
[grim's stream](https://twitch.tv/rw_grim) by the name of taniwha3.

## Building

Standard [meson](https://mesonbuild.com) project build:

```sh
meson setup build
meson compile -C build
meson install -C build
```

### Configuration

There are configuration options for a few things, but you'll probably only be
interested in `doc` and `introspection`. Please note that `doc` requires
`introspection`.

## Documentation

Documentation and API reference for the project can be found at
[docs.imfreedom.org/hasl](https://docs.imfreedom.org/hasl/).
