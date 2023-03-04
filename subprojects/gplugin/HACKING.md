## clang-format

GPlugin uses [clang-format][1] to automatically format code. Version 9 (or
higher) is required.  Earlier versions will fail on the options in the
`.clang-format` file.

You can use clang-format in one or more of the following ways:
* Integrate it into your editor. The [clang-format page][1] has examples.
* Use a Mercurial [hook script][2].
* Run `ninja clang-format` from your build directory.
* Worst case, run `clang-format -i SOME_FILE` manually.

### GObject Functions

GObject and GPlugin have functions which take pairs of name/value arguments:
* `g_object_get()`
* `g_object_new()`
* `gplugin_plugin_info_new()`

If code calling one of these functions with such pairs has to be wrapped, the
pairs are easier to read and edit with manual formatting to group the name and
value on the same line (but otherwise following the usual formatting that
clang-format would apply):
```
	/* clang-format off */
	g_object_get(
		G_OBJECT(info),
		"name", &name,
		"summary", &summary,
		NULL);
	/* clang-format on */
```

### Debian

Debian and derivatives (e.g. Ubuntu) ship clang-format version 9 in a package
named `clang-format-9`.  The package named `clang-format` is currently an
older version.  Additionally, the `clang-format-9` package will not place a
`clang-format` in the `$PATH`.  Install as follows:

```
sudo apt install clang-format-9
sudo update-alternatives --set clang-format /usr/bin/clang-format-9
```

## convey

GPlugin uses [Convey][3] for CI.  You can run the integration tests locally.

To initially setup Convey, which requires Docker:

1. Install Convey.  You can use a pre-built binary from [convey's Downloads
   page][4]; put that (or a symlink) in your `$PATH` named `convey`.
   For now, avoid version 0.14.
2. Install Docker.  On Debian systems, use: `sudo apt install docker.io`
3. Add yourself to the `docker` group.  Adding yourself to a group does not
   add that group to your existing session, so the first time you will need to
   get a shell in that group with `newgrp docker` or by rebooting.

To run all of the integration tests, run the "all" metaplan:
```
convey all
```

To run a specific test/platform, run the plan directly:
```
convey alpine-edge-amd64
```

To get a list of metaplans and their plans:
```
convey list metaplans
```

[1]: https://clang.llvm.org/docs/ClangFormat.html
[2]: https://hg.mozilla.org/projects/nss/file/default/coreconf/precommit.clang-format.sh
[3]: https://keep.imfreedom.org/grim/convey
[4]: https://bintray.com/pidgin/releases/convey
