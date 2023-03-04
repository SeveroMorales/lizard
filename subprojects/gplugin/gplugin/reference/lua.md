Title: Lua Plugins
Slug: lua

## Lua Plugins

> You **MUST** have the Lua loader plugin installed and working as well as the
> gobject-introspection package for GPlugin instance to use Lua plugins.

### Example Lua Plugin

Like all plugins in GPlugin, Lua plugins must also implement the
`gplugin_query`, `gplugin_load`, and `gplugin_unload` functions.

The following is a basic Lua plugin.

```lua
local lgi = require "lgi"
local GPlugin = lgi.GPlugin

-- gplugin_plugin_query is called when searching for plugins. The only thing
-- this function should do, is return a GPlugin.PluginInfo instance.
function gplugin_query()
    return GPlugin.PluginInfo {
        id = "gplugin-lua/basic-plugin",
        abi_version = 0x01020304,
        name = "basic plugin",
        category = "test",
        version = "0.0.10",
        license_id = "license-id",
        summary = "basic lua plugin",
        description = "description of the basic lua plugin",
        authors = { "Gary Kramlich &lt;grim@reaperworld.com&gt;" },
        website = "https://keep.imfreedom.org/gplugin/gplugin/"
    }
end

-- gplugin_load is called when your plugin is loaded in the application. If
-- something isn't quite right, you can return false or call error() to signify
-- that something went wrong and stop your plugin from being loaded.
function gplugin_load(plugin)
    return true
end

-- gplugin_plugin_unload is called when your plugin is unloaded in the
-- application. The shutdown parameter tells your plugin whether or not the
-- application is shutting down. For example, if a user unloads your plugin,
-- shutdown will be false, but if the program is shutting down, shutdown will be
-- true.
--
-- If something went wrong with the unload or the plugin isn't ready to be
-- unloaded, you can return false here to stop it from being unloaded. Note if
-- shutdown is true, the return value is not honored.
function gplugin_unload(plugin, shutdown)
    return true
end
```

