Title: Embedding GPlugin
Slug: embedding

## Embedding GPlugin

You can embed GPlugin into any language that has GObject-Introspection support,
but in this example we're going to look at embedding GPlugin into a C based
project.

GPlugin was designed to be simple to implement and use. Initialization and
teardown examples can be found below.

### Initialization

During the start up of your application you need to add the following

```c
/* Create a variable to hold the default manager instance. */
GPluginManager *manager = NULL;

/* Initialize the GPlugin library */
gplugin_init();

/* Get a pointer to the default GPluginManager instance. */
manager = gplugin_manager_get_default();

/* Tell GPlugin to look for plugins in its default paths */
gplugin_manager_add_default_paths(manager);

/* Optionally tell GPlugin to look for plugins in application specific
 * paths.  This will add `$PREFIX/lib/application`.
 */
gplugin_manager_add_app_paths(manager, PREFIX, "application");

/* Once you're ready to find/load plugins call g_plugin_manager_refresh. */
gplugin_manager_refresh(manager);
```

### Shutdown

When your application is shutting down you need to uninitialize GPlugin by
calling

```c
gplugin_uninit();
```

