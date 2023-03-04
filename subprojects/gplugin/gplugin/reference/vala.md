Title: Vala Plugin Example
Slug: vala

## Vala Plugins

> You **MUST** have the Vala bindings installed on your system for this to
> work. They are built by the default GPlugin build.

### Example Vala Plugin

Due to the way `GPlugin.PluginInfo` info works, you must subclass it and set
your values in the new constructor. It is recommended that you define this
class in a namespace to avoid collisions with other plugins.

Like all plugins in GPlugin, Vala plugins must also implement the
`gplugin_query`, `gplugin_load`, and `gplugin_unload` functions. These
functions must be in the global namespace.

The following is a basic Vala plugin.

```vala
using GPlugin;

namespace BasicPlugin {

/* You need to create a class for your plugin info. This is pretty simple as you
 * can see in the example below.
 */
public class Info : GPlugin.PluginInfo {
	public Info() {
		string[] authors = {"author1"};

		Object(
			id: "gplugin/vala-basic-plugin",
			abi_version: 0x01020304,
			name: "basic plugin",
			authors: authors,
			category: "test",
			version: "version",
			license_id: "license",
			summary: "summary",
			website: "website",
			description: "description"
		);
	}
}

}

/* gplugin_query is called when searching for plugins. The function should
 * return an instance of the PluginInfo class you created above. If something
 * went wrong, you can set error with an error message.
 */
public GPlugin.PluginInfo gplugin_query(out Error error) {
	error = null;

	return new BasicPlugin.Info();
}

/* gplugin_load is called when your plugin is loaded in the application. If
 * something isn't quite right, you can return false and optionally set error,
 * to signify that something went wrong and stop your plugin from being loaded.
 */
public bool gplugin_load(GPlugin.Plugin plugin, out Error error) {
	error = null;

	return true;
}

/* gplugin_plugin_unload is called when your plugin is unloaded in the
 * application. The shutdown parameter tells your plugin whether or not the
 * application is shutting down. For example, if a user unloads your plugin,
 * shutdown will be false, but if the program is shutting down, shutdown will be
 * true.
 *
 * If something went wrong with the unload or the plugin isn't ready to be
 * unloaded, you can return false, and optionally set error, here to stop it
 * from being unloaded. Note if shutdown is true, the return value is not
 * honored.
 */
public bool gplugin_unload(GPlugin.Plugin plugin, bool shutdown, out Error error) {
	error = null;

	return true;
}
```

