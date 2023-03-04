Title: C Plugin Tutorial
Slug: c-plugin-tutorial

## C Plugin Tutorial

### Introduction

C plugins are native plugins. They have complete access to all of the API,
and can do basically whatever they want. All of the protocol plugins are
also written in C.

### Getting Started

To develop a plugin you need to have the libpurple and (for UI plugins) the
Pidgin/Finch source code or development headers. It is generally a good idea
to compile against the same version of Pidgin that you are running. You may
also want to develop against the code in our Mercurial repository if you need
to use a new feature. Please do not abuse our Mercurial repository, however.

### An Example

I know every tutorial has a hello world, so why should libpurple be any
different?

```c
#include <purple.h>

static GPluginPluginInfo *
hello_world_query(GError **error)
{
	const gchar * const authors[] = {
		"Author Name <e@mail>",
		NULL
	};

	/* For specific notes on the meanings of each of these members, consult the
	   C Plugin Howto on the website. */
	return purple_plugin_info_new (
		"name",         "Hello World!",
		"version",      VERSION,
		"category",     "Example",
		"summary",      "Hello World Plugin",
		"description",  "Hello World Plugin",
		"authors",      authors,
		"website",      "http://helloworld.tld",
		"abi-version",  PURPLE_ABI_VERSION,
		NULL
	);
}

static gboolean
hello_world_load(GPluginPlugin *plugin, GError **error)
{
	purple_notify_message(plugin, PURPLE_NOTIFY_MSG_INFO, "Hello World!",
                        "This is the Hello World! plugin :)",
                        NULL, NULL, NULL, NULL);

	return TRUE;
}

static gboolean
hello_world_unload(GPluginPlugin *plugin, gboolean shutdown, GError **error)
{
	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(hello_world)
```

Okay, so what does all this mean?  We start off by including purple.h.  This
file includes all the libpurple header files.

`hello_world_query`, `hello_world_load` and `hello_world_unload` are functions
that must be implemented in every plugin.  These functions are named according
to the value passed to `GPLUGIN_NATIVE_PLUGIN_DECLARE` which is described
below.

`hello_world_query` is called when the plugin is queried by the plugin system,
and returns various information about the plugin in form of a newly created
instance of `GPluginPluginInfo` or a subclass.  For a list of all available
properties, see `purple_plugin_info_new()`.

If anything should go wrong during the query you can return an error by using
`g_set_error()` on the `error` argument.

`hello_world_load` is called when the plugin is loaded. That is the user has
enabled the plugin or libpurple is loading a plugin that was previously loaded.
You can initialize any variables, register dynamic types, and so on in this
function.  Plugins may also want to add their preferences to the pref
tree--more about that later. In this plugin we'll just use it to display a
message. Just like `hello_world_query` if there are any errors that arise, you
can call `g_set_error()` on the `error` argument and return `FALSE`.

`hello_world_unload` is called when the plugin is unloaded. That is, when the
user has manually unloaded the plugin or the program is shutting down. We can
use it to wrap up everything, and free our variables. If the program is shutting
down, the `shutdown` argument will be `TRUE`. Again, if there are any errors, you
can call `g_set_error()` on the `error` argument and return `FALSE`.

Finally we have `GPLUGIN_NATIVE_PLUGIN_DECLARE()`. It is a helper macro that
makes creating plugins easier. It has a single argument which is the prefix
used for the `_query`, `_load`, and `_unload` functions.
