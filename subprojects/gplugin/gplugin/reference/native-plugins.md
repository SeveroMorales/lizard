Title: Native Plugins
Slug: native

## Native Plugins

Writing Native plugins is pretty simple, but since it's C/C++ it's a bit more
complicated.

There are currently no C++ bindings and no intention to write them, but the C
API is still usable from C++.

```c
#include <gplugin.h>
#include <gplugin-native.h>

/* This function is called by the native plugin loader to get information about
 * the plugin. It returns a #GPluginPluginInfo instance that describes the
 * plugin. If anything goes wrong during this function, @error should be set
 * via g_set_error() and %NULL should be returned. It must match the
 * signature of GPluginNativePluginQueryFunc, and it will be passed to
 * GPLUGIN_NATIVE_PLUGIN_DECLARE which will export it properly.
 */
static GPluginPluginInfo *
gplugin_native_basic_plugin_query(GError **error) {
	/* Authors is a list of authors who worked on the plugin.  Generally
	 * these are in the "Name Surname <user@domain.com>" format.
	 */
	const gchar * const authors[] = {
		"Author O <author@example.com>",
		NULL
	};

	/* gplugin_plugin_info_new only requires that the id be set, and the
	 * rest are here for demonstration purposes.
	 */
	return gplugin_plugin_info_new(
		"gplugin/native-basic-plugin",
		GPLUGIN_NATIVE_PLUGIN_ABI_VERSION,
		"name", "name",
		"version", "version",
		"summary", "summary",
		"description", "description",
		"authors", authors,
		"website", "website",
		NULL);
}

/* This function is called by the native plugin loader when the plugin should
 * be loaded. It should do any setup that the plugin requires and return %TRUE
 * if everything was successful. If not, it should set @error with
 * g_set_error() and return %FALSE. This function needs to match the
 * signature of GPluginNativePluginLoadFunc, and it will be passed to
 * GPLUGIN_NATIVE_PLUGIN_DECLARE which will export it properly.
 */
static gboolean
gplugin_native_basic_plugin_load(GPluginPlugin *plugin, GError **error) {
	return TRUE;
}

/* This function is called by the native plugin loader when the plugin should
 * be unloaded. It should do any clean up that the plugin requires and return
 * %TRUE if everything was successful. If not, it should set @error with
 * g_set_error() and return %FALSE. This function needs to match the signature
 * of GPluginNativePluginUnloadFunc, and it will be passed to
 * GPLUGIN_NATIVE_PLUGIN_DECLARE which will export it properly.
 *
 * The shutdown parameter is set to TRUE when the application is shutting down.
 * This is useful if you're using a library where you can only initialize it
 * once. In this case, you'd typically always return FALSE here, but with the
 * shutdown parameter, you can return FALSE when shutdown is FALSE, but then
 * properly uninitialize the library when shutdown is TRUE.
 */
static gboolean
gplugin_native_basic_plugin_unload(GPluginPlugin *plugin, gboolean shutdown, GError *error) {
	return TRUE;
}

/* This macro does the heavy lifting of making sure to export all of the
 * symbols correctly as well as add some future proofing for features like
 * statically linking plugins. It is highly recommended to use this macro
 * instead of manually exporting the symbols yourself.
 */
GPLUGIN_NATIVE_PLUGIN_DECLARE(gplugin_native_basic_plugin)
```

