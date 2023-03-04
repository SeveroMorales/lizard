Title: Plugin IDs
Slug: plugin-ids

## Plugin IDs

### Introduction

Every plugin contains a unique identifier. Third-party plugins (that is,
plugins written by anyone who is not a libpurple, Pidgin, or Finch developer)
are expected to use a plugin ID that follows a specific format. This format
categorizes plugins and makes duplicate IDs highly unlikely.

### Format

The basic format of a plugin ID is as follows:

```
type-username-pluginname
```

The ***type*** indicator specifies the type of plugin. This must be one
of the following:

#### Types Of Plugins

core
: A core libpurple plugin, capable of being loaded in any program using
libpurple. Core plugins may not contain any UI-specific code.

prpl
: A protocol plugin. This is a core plugin which provides libpurple the ability
to connect to another IM or chat network.

gtk
: A GTK+ (a.k.a. Pidgin) plugin. These plugins may use GTK+ code, but may not
use window toolkit code, such as X11, Win32, Cocoa, or Carbon.

gtk-x11
: A GTK+ plugin that uses X11 code. These plugins may use both GTK+ code and
X11 code, allowing to hook into features specific to X11.

gtk-win32
: A GTK+ plugin that uses Win32 code. These plugins may use both GTK+ code and
Win32 code, allowing to hook into features available on Windows.

gnt
: A GNT (a.k.a. Finch) plugin. These plugins may use GNT code.

The ***username*** must be a unique identifier for you. It
***should*** be your https://developer.pidgin.im Trac user ID. Failing that, you
could use your SourceForge user ID or your Libera.chat IRC nickname, if you
have either. The https://developer.pidgin.im Trac user ID is preferred.
Do ***not*** leave this field blank!

The ***pluginname*** is the name of your plugin. It is usually all
lowercase letters and matches the static plugin ID (the first argument to
the GPLUGIN_NATIVE_PLUGIN_DECLARE() macro call), although it can be anything you
like. Do ***not*** include version information in the plugin ID--the
`PurplePluginInfo` object already has a property for this.

### One Last Rule For Plugin IDs

Plugin IDs may ***NOT*** contain spaces. If you need a space, use another
hyphen (-).

### Exceptions To The Rule

As with any rule there are exceptions. If you browse through the source
tree you will see that the plugins we distribute with the Pidgin source
do not contain a username field. This is because while one developer may
have written each specific plugin, the plugins are maintained
collectively by the entire development team. This lack of a username
field is also an indicator that the plugin is one of our plugins and not
a third-party plugin.

Another exception to the rule is the
[Purple Plugin Pack](https://keep.imfreedom.org/pidgin/purple-plugin-pack/).
All plugins whose lives started in the Purple Plugin Pack use
`"plugin_pack"` for the username field to indicate origination in
the Purple Plugin Pack.

These two exceptions are mentioned here for completeness. We don't
encourage breaking the conventions set forth by the rules outlined above.

### Examples Of Well-Chosen Plugin IDs

The following is a list of well-chosen Plugin IDs listing a few good examples.

gtk-amc_grim-guifications
: This is the plugin ID for the Guifications 2.x plugin.

gtk-rlaager-album
: This is the plugin ID for the Album plugin, which is now part of the
Purple Plugin Pack. Its ID follows the rules because its life started prior
to its inclusion in the Plugin Pack.

core-rlaager-irchelper
: This is the plugin ID for the IRC Helper plugin, which is now part of the
Purple Plugin Pack. Its ID follows the rules because its life started prior
to its inclusion in the Plugin Pack.

### Plugin Database

Although it doesn't exist yet, in time there will be a plugin database
on the Pidgin website, where users can download and install new plugins.
Plugins will be accessed by your plugin ID, which is one reason why it
must be unique. 
