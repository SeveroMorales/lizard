Title: Python3 Plugins
Slut: python3

## Python3 Plugins

> You **MUST** have the Python3 loader plugin installed and working as well as
> the gobject-introspection package for GPlugin installed to use Python3
> plugins.

### Example Python Plugin

Like all plugins in GPlugin, Python plugins must also implement the
`gplugin_query`, `gplugin_load`, and `gplugin_unload` functions.

The following is a basic Python plugin.

```python
import gi

gi.require_version("GPlugin", "0.0")
from gi.repository import GPlugin

# gplugin_plugin_query is called when searching for plugins. The only thing this
# function should do, is return a GPlugin.PluginInfo instance.
def gplugin_plugin_query():
    return GPlugin.PluginInfo(
        id="gplugin-python/basic-plugin",
        abi_version=0x01020304,
        name="basic plugin",
        authors=["author1"],
        category="test",
        version="version",
        license_id="license",
        summary="summary",
        website="website",
        description="description",
    )

# gplugin_plugin_load is called when your plugin is loaded in the application.
# If something isn't quite right, you can return False or throw an exception to
# signify that something went wrong and stop your plugin from being loaded.
def gplugin_plugin_load(plugin):
    return True

# gplugin_plugin_unload is called when your plugin is unloaded in the
# application. The shutdown parameter tells your plugin whether or not the
# application is shutting down. For example, if a user unloads your plugin,
# shutdown will be False, but if the program is shutting down, shutdown will be
# True.
#
# If something went wrong with the unload or the plugin isn't ready to be
# unloaded, you can return False here to stop it from being unloaded. Note if
# shutdown is True, the return value is not honored.
def gplugin_plugin_unload(plugin, shutdown):
    return True
```

