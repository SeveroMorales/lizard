/*
 * Copyright (C) 2011-2021 Gary Kramlich <grim@reaperworld.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>

#include <glib.h>
#include <glib/gi18n-lib.h>

#include <gplugin/gplugin-core.h>
#include <gplugin/gplugin-file-source.h>
#include <gplugin/gplugin-file-tree.h>
#include <gplugin/gplugin-manager.h>
#include <gplugin/gplugin-native-loader.h>
#include <gplugin/gplugin-private.h>
#include <gplugin/gplugin-source.h>

/**
 * GPluginManagerForeachFunc:
 * @id: The id of the plugin.
 * @plugins: (transfer none) (element-type GPlugin.Plugin): A
 *           [struct@GLib.SList] of each plugin that has the id @id.
 * @data: User data passed to [method@GPlugin.Manager.foreach].
 *
 * A callback function for [method@GPlugin.Manager.foreach].
 */

/**
 * GPluginManager:
 *
 * The manager is responsible for querying plugins as well as telling loaders
 * when to load and unload plugins. It also keeps track of paths that should be
 * searched for plugins.
 *
 * Since: 0.32.0
 */

/******************************************************************************
 * Enums
 *****************************************************************************/
enum {
	SIG_LOADING,
	SIG_LOADED,
	SIG_LOAD_FAILED,
	SIG_UNLOADING,
	SIG_UNLOADED,
	SIG_UNLOAD_FAILED,
	SIG_LOADER_REGISTERED,
	SIG_LOADER_UNREGISTERED,
	N_SIGNALS,
};

/******************************************************************************
 * Structs
 *****************************************************************************/
struct _GPluginManager {
	GObject parent;

	GQueue *paths;
	GHashTable *plugins;

	GHashTable *loaders;

	gboolean refresh_needed;
};

G_DEFINE_TYPE(GPluginManager, gplugin_manager, G_TYPE_OBJECT)

/******************************************************************************
 * Globals
 *****************************************************************************/
GPluginManager *default_manager = NULL;
GPluginLoader *native_loader = NULL;
static guint signals[N_SIGNALS] = {
	0,
};
const gchar *dependency_pattern =
	"^(?P<id>.+?)((?P<op>\\<=|\\<|==|=|\\>=|\\>)(?P<version>.+))?$";
GRegex *dependency_regex = NULL;

/******************************************************************************
 * Helpers
 *****************************************************************************/
static gboolean
gplugin_manager_remove_list_value(
	G_GNUC_UNUSED gpointer k,
	gpointer v,
	G_GNUC_UNUSED gpointer d)
{
	GSList *l = NULL;

	for(l = (GSList *)v; l; l = l->next) {
		if(l->data && G_IS_OBJECT(l->data))
			g_object_unref(G_OBJECT(l->data));
	}

	g_slist_free((GSList *)v);

	return TRUE;
}

static void
gplugin_manager_change_paths_from_environment(
	GPluginManager *manager,
	const gchar *name,
	gboolean prepend)
{
	gchar **paths;
	gint i;
	const gchar *from_env;

	from_env = g_getenv(name);
	if(from_env == NULL) {
		return;
	}

	paths = g_strsplit(from_env, G_SEARCHPATH_SEPARATOR_S, 0);
	for(i = 0; paths[i]; i++) {
		if(prepend) {
			gplugin_manager_prepend_path(manager, paths[i]);
		} else {
			gplugin_manager_append_path(manager, paths[i]);
		}
	}
	g_strfreev(paths);
}

static void
gplugin_manager_foreach_unload_plugin(
	gpointer key,
	gpointer value,
	G_GNUC_UNUSED gpointer data)
{
	GList *l = NULL;
	gchar *id = (gchar *)key;

	for(l = (GList *)value; l; l = l->next) {
		GPluginPlugin *plugin = GPLUGIN_PLUGIN(l->data);
		GPluginLoader *loader = NULL;
		GError *error = NULL;

		if(gplugin_plugin_get_state(plugin) != GPLUGIN_PLUGIN_STATE_LOADED) {
			continue;
		}

		loader = gplugin_plugin_get_loader(plugin);
		if(!gplugin_loader_unload_plugin(loader, plugin, TRUE, &error)) {
			g_warning(
				"failed to unload plugin with id %s: %s",
				id,
				error ? error->message : "unknown");
			g_clear_error(&error);
		}
		g_object_unref(G_OBJECT(loader));
	}
}

static gchar *
gplugin_manager_normalize_path(const gchar *path)
{
	if(g_str_has_suffix(path, G_DIR_SEPARATOR_S)) {
		return g_strdup(path);
	}

	return g_strdup_printf("%s%s", path, G_DIR_SEPARATOR_S);
}

static gint
gplugin_manager_compare_paths(gconstpointer a, gconstpointer b)
{
	gchar *keya = NULL, *keyb = NULL;
	gint r = 0;

	keya = g_utf8_collate_key_for_filename((const gchar *)a, -1);
	keyb = g_utf8_collate_key_for_filename((const gchar *)b, -1);

	r = strcmp(keya, keyb);

	g_free(keya);
	g_free(keyb);

	return r;
}

static gboolean
gplugin_manager_load_dependencies(
	GPluginManager *manager,
	GPluginPlugin *plugin,
	G_GNUC_UNUSED GPluginPluginInfo *info,
	GError **error)
{
	GSList *dependencies = NULL, *l = NULL;
	GError *ourerror = NULL;
	gboolean all_loaded = TRUE;

	dependencies =
		gplugin_manager_get_plugin_dependencies(manager, plugin, &ourerror);
	if(ourerror != NULL) {
		g_propagate_error(error, ourerror);

		return FALSE;
	}

	for(l = dependencies; l != NULL; l = l->next) {
		GPluginPlugin *dependency = GPLUGIN_PLUGIN(l->data);
		gboolean loaded = FALSE;

		loaded = gplugin_manager_load_plugin(manager, dependency, &ourerror);

		if(!loaded || ourerror != NULL) {
			if(ourerror != NULL) {
				g_propagate_error(error, ourerror);
			}

			all_loaded = FALSE;
			break;
		}
	}

	g_slist_free_full(dependencies, g_object_unref);

	return all_loaded;
}

/******************************************************************************
 * Manager implementation
 *****************************************************************************/
static gboolean
gplugin_manager_loading_cb(
	G_GNUC_UNUSED GPluginManager *manager,
	G_GNUC_UNUSED GPluginPlugin *plugin,
	G_GNUC_UNUSED GError **error)
{
	return TRUE;
}

static gboolean
gplugin_manager_unloading_cb(
	G_GNUC_UNUSED GPluginManager *manager,
	G_GNUC_UNUSED GPluginPlugin *plugin,
	G_GNUC_UNUSED GError **error)
{
	return TRUE;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
gplugin_manager_finalize(GObject *obj)
{
	GPluginManager *manager = GPLUGIN_MANAGER(obj);

	g_queue_free_full(manager->paths, g_free);
	manager->paths = NULL;

	/* unload all of the loaded plugins */
	g_hash_table_foreach(
		manager->plugins,
		gplugin_manager_foreach_unload_plugin,
		NULL);

	/* free all the data in the plugins hash table and destroy it */
	g_hash_table_foreach_remove(
		manager->plugins,
		gplugin_manager_remove_list_value,
		NULL);
	g_clear_pointer(&manager->plugins, g_hash_table_destroy);

	/* clean up our list of loaders */
	g_clear_pointer(&manager->loaders, g_hash_table_destroy);

	/* call the base class's destructor */
	G_OBJECT_CLASS(gplugin_manager_parent_class)->finalize(obj);
}

static void
gplugin_manager_class_init(GPluginManagerClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = gplugin_manager_finalize;

	/**
	 * GPluginManager::loading-plugin:
	 * @manager: The [class@GPlugin.Manager] instance.
	 * @plugin: The [iface@GPlugin.Plugin] that's about to be loaded.
	 * @error: Return address for a [struct@GLib.Error].
	 *
	 * Emitted before @plugin is loaded.
	 *
	 * Return FALSE to stop loading
	 */
	signals[SIG_LOADING] = g_signal_new_class_handler(
		"loading-plugin",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		G_CALLBACK(gplugin_manager_loading_cb),
		gplugin_boolean_accumulator,
		NULL,
		NULL,
		G_TYPE_BOOLEAN,
		2,
		G_TYPE_OBJECT,
		G_TYPE_POINTER);

	/**
	 * GPluginManager::loaded-plugin:
	 * @manager: The [class@GPlugin.Manager] instance.
	 * @plugin: The [iface@GPlugin.Plugin] that's about to be loaded.
	 *
	 * Emitted after a plugin is loaded.
	 */
	signals[SIG_LOADED] = g_signal_new_class_handler(
		"loaded-plugin",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		G_TYPE_OBJECT);

	/**
	 * GPluginManager::load-plugin-failed:
	 * @manager: The [class@GPlugin.Manager] instance.
	 * @plugin: The [iface@GPlugin.Plugin] that failed to load.
	 * @error: The [struct@GLib.Error] of what went wrong.
	 *
	 * Emitted after a plugin fails to load.
	 */
	signals[SIG_LOAD_FAILED] = g_signal_new_class_handler(
		"load-plugin-failed",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		2,
		G_TYPE_OBJECT,
		G_TYPE_ERROR);

	/**
	 * GPluginManager::unloading-plugin
	 * @manager: The [class@GPlugin.Manager] instance.
	 * @plugin: The [iface@GPlugin.Plugin] that's about to be unloaded.
	 * @error: Return address for a [struct@GLib.Error].
	 *
	 * Emitted before a plugin is unloaded.
	 *
	 * Return FALSE to stop unloading
	 */
	signals[SIG_UNLOADING] = g_signal_new_class_handler(
		"unloading-plugin",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		G_CALLBACK(gplugin_manager_unloading_cb),
		gplugin_boolean_accumulator,
		NULL,
		NULL,
		G_TYPE_BOOLEAN,
		2,
		G_TYPE_OBJECT,
		G_TYPE_POINTER);

	/**
	 * GPluginManager::unloaded-plugin:
	 * @manager: The [class@GPlugin.Manager] instance.
	 * @plugin: The [iface@GPlugin.Plugin] that's about to be loaded.
	 *
	 * emitted after a plugin is successfully unloaded.
	 */
	signals[SIG_UNLOADED] = g_signal_new_class_handler(
		"unloaded-plugin",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		G_TYPE_OBJECT);

	/**
	 * GPluginManager::unload-plugin-failed:
	 * @manager: The [class@GPlugin.Manager] instance.
	 * @plugin: The [iface@GPlugin.Plugin] instance that failed to unload.
	 * @error: A [struct@GLib.Error] instance.
	 *
	 * Emitted when @manager was asked to unload @plugin, but @plugin returned
	 * %FALSE when its unload function was called.
	 */
	signals[SIG_UNLOAD_FAILED] = g_signal_new_class_handler(
		"unload-plugin-failed",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		2,
		G_TYPE_OBJECT,
		G_TYPE_ERROR);

	/**
	 * GPluginManager::loader-registered:
	 * @manager: The [class@GPlugin.Manager] instance.
	 * @loader: The [class@GPlugin.Loader] instance that was registered.
	 *
	 * Emitted when @loader has been registered with @manager via
	 * [method@GPlugin.Manager.register_loader].
	 *
	 * Since: 0.39.0
	 */
	signals[SIG_LOADER_REGISTERED] = g_signal_new_class_handler(
		"loader-registered",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		GPLUGIN_TYPE_LOADER);

	/**
	 * GPluginManager::loader-unregistered:
	 * @manager: The [class@GPlugin.Manager] instance.
	 * @loader: The [class@GPlugin.Loader] instance that was unregistered.
	 *
	 * Emitted when @loader has been unregistered from @manager via
	 * [method@GPlugin.Manager.unregister_loader].
	 *
	 * Since: 0.39.0
	 */
	signals[SIG_LOADER_UNREGISTERED] = g_signal_new_class_handler(
		"loader-unregistered",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		GPLUGIN_TYPE_LOADER);
}

static void
gplugin_manager_init(GPluginManager *manager)
{
	manager->paths = g_queue_new();

	/* the plugins hashtable is keyed on a plugin id and holds a GSList of all
	 * plugins that share that id.
	 */
	manager->plugins =
		g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

	manager->loaders =
		g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
}

/******************************************************************************
 * Private API
 *****************************************************************************/
void
gplugin_manager_private_init(gboolean register_native_loader)
{
	GError *error = NULL;

	if(GPLUGIN_IS_MANAGER(default_manager)) {
		return;
	}

	default_manager = g_object_new(GPLUGIN_TYPE_MANAGER, NULL);

	if(register_native_loader) {
		native_loader = gplugin_native_loader_new();

		if(!gplugin_manager_register_loader(
			   default_manager,
			   native_loader,
			   &error)) {
			if(error != NULL) {
				g_error("failed to register loader: %s", error->message);
				g_error_free(error);
			} else {
				g_error("failed to register loader: unknown failure");
			}
		}
	}

	dependency_regex = g_regex_new(dependency_pattern, 0, 0, NULL);
}

void
gplugin_manager_private_uninit(void)
{
	g_clear_object(&native_loader);

	g_regex_unref(dependency_regex);

	/* g_clear_pointer (and therefore g_clear_object), clears the pointer
	 * before calling the destroy function. So we have to handle this ourself
	 * and clear the pointer after destruction since plugins are unloaded
	 * during destruction and may need to unregister a loader during their
	 * unload.
	 */
	if(default_manager != NULL) {
		g_object_unref(G_OBJECT(default_manager));
		default_manager = NULL;
	}
}

/******************************************************************************
 * API
 *****************************************************************************/

/**
 * gplugin_manager_append_path:
 * @manager: The manager instance.
 * @path: A path to add to the end of the plugin search paths.
 *
 * Adds @path to the end of the list of paths to search for plugins.
 */
void
gplugin_manager_append_path(GPluginManager *manager, const gchar *path)
{
	GList *l = NULL;
	gchar *normalized = NULL;

	g_return_if_fail(GPLUGIN_IS_MANAGER(manager));
	g_return_if_fail(path != NULL);

	normalized = gplugin_manager_normalize_path(path);

	l = g_queue_find_custom(
		manager->paths,
		normalized,
		gplugin_manager_compare_paths);
	if(l == NULL) {
		g_queue_push_tail(manager->paths, normalized);
	} else {
		g_free(normalized);
	}
}

/**
 * gplugin_manager_prepend_path:
 * @manager: The manager instance.
 * @path: A path to add to the beginning of the plugin search paths.
 *
 * Adds @path to the beginning of the list of paths to search for plugins.
 */
void
gplugin_manager_prepend_path(GPluginManager *manager, const gchar *path)
{
	GList *l = NULL;
	gchar *normalized = NULL;

	g_return_if_fail(GPLUGIN_IS_MANAGER(manager));
	g_return_if_fail(path != NULL);

	normalized = gplugin_manager_normalize_path(path);

	l = g_queue_find_custom(
		manager->paths,
		normalized,
		gplugin_manager_compare_paths);
	if(l == NULL) {
		g_queue_push_head(manager->paths, normalized);
	} else {
		g_free(normalized);
	}
}

/**
 * gplugin_manager_remove_path:
 * @manager: The manager instance.
 * @path: A path to remove from the plugin search paths.
 *
 * Removes @path from the list of paths to search for plugins.
 */
void
gplugin_manager_remove_path(GPluginManager *manager, const gchar *path)
{
	GList *l = NULL;
	gchar *normalized = NULL;

	g_return_if_fail(GPLUGIN_IS_MANAGER(manager));
	g_return_if_fail(path != NULL);

	normalized = gplugin_manager_normalize_path(path);

	l = g_queue_find_custom(
		manager->paths,
		normalized,
		gplugin_manager_compare_paths);
	if(l != NULL) {
		g_free(l->data);
		g_queue_delete_link(manager->paths, l);
	}

	g_free(normalized);
}

/**
 * gplugin_manager_remove_paths:
 * @manager: The manager instance.
 *
 * Clears all paths that are set to search for plugins.
 */
void
gplugin_manager_remove_paths(GPluginManager *manager)
{
	g_return_if_fail(GPLUGIN_IS_MANAGER(manager));

	g_queue_clear_full(manager->paths, g_free);
}

/**
 * gplugin_manager_add_default_paths:
 * @manager: The manager instance.
 *
 * Adds the path that GPlugin was installed to to the plugin search path, as
 * well as `${XDG_CONFIG_HOME}/gplugin` so users can install additional loaders
 * themselves.
 */
void
gplugin_manager_add_default_paths(GPluginManager *manager)
{
	gchar *path;

	g_return_if_fail(GPLUGIN_IS_MANAGER(manager));

	path = g_build_filename(PREFIX, LIBDIR, "gplugin", NULL);
	gplugin_manager_prepend_path(manager, path);
	g_free(path);

	path = g_build_filename(g_get_user_config_dir(), "gplugin", NULL);
	gplugin_manager_prepend_path(manager, path);
	g_free(path);
}

/**
 * gplugin_manager_add_app_paths:
 * @manager: The manager instance.
 * @prefix: The installation prefix for the application.
 * @appname: The name of the application whose paths to add.
 *
 * Adds the application installation path for @appname.
 *
 * This will add `{prefix}/{appname}/plugins` to the list as well as
 * `${XDG_CONFIG_HOME}/{appname}/plugins`.
 */
void
gplugin_manager_add_app_paths(
	GPluginManager *manager,
	const gchar *prefix,
	const gchar *appname)
{
	gchar *path;

	g_return_if_fail(GPLUGIN_IS_MANAGER(manager));
	g_return_if_fail(appname != NULL);

	path = g_build_filename(prefix, LIBDIR, appname, NULL);
	gplugin_manager_prepend_path(manager, path);
	g_free(path);

	path = g_build_filename(g_get_user_config_dir(), appname, "plugins", NULL);
	gplugin_manager_prepend_path(manager, path);
	g_free(path);
}

/**
 * gplugin_manager_append_paths_from_environment:
 * @manager: The manager instance.
 * @name: The name of the environment variable containing the paths to add.
 *
 * Append the paths held in the environment variable @name to the list.
 *
 * Since: 0.37.0
 */
void
gplugin_manager_append_paths_from_environment(
	GPluginManager *manager,
	const gchar *name)
{
	gplugin_manager_change_paths_from_environment(manager, name, FALSE);
}

/**
 * gplugin_manager_prepend_paths_from_environment:
 * @manager: The manager instance.
 * @name: The name of the environment variable containing the paths to add.
 *
 * Prepends the paths held in the environment variable @name to the list.
 *
 * Since: 0.37.0
 */
void
gplugin_manager_prepend_paths_from_environment(
	GPluginManager *manager,
	const gchar *name)
{
	gplugin_manager_change_paths_from_environment(manager, name, TRUE);
}

/**
 * gplugin_manager_get_paths:
 * @manager: The manager instance.
 *
 * Gets the list of paths which will be searched for plugins.
 *
 * Returns: (element-type utf8) (transfer none): The [type@GLib.List] of paths
 *          which will be searched for plugins.
 */
GList *
gplugin_manager_get_paths(GPluginManager *manager)
{
	g_return_val_if_fail(GPLUGIN_IS_MANAGER(manager), NULL);

	return manager->paths->head;
}

/**
 * gplugin_manager_register_loader:
 * @manager: The manager instance.
 * @loader: The loader instance to register.
 * @error: (out) (nullable): The return address for a [struct@GLib.Error].
 *
 * Registers @loader as an available loader.
 *
 * Returns: %TRUE if the loader was successfully register, %FALSE otherwise
 *          with @error set.
 */
gboolean
gplugin_manager_register_loader(
	GPluginManager *manager,
	GPluginLoader *loader,
	GError **error)
{
	GPluginLoader *found = NULL;
	const gchar *id = NULL;

	g_return_val_if_fail(GPLUGIN_IS_MANAGER(manager), FALSE);
	g_return_val_if_fail(GPLUGIN_IS_LOADER(loader), FALSE);

	id = gplugin_loader_get_id(loader);
	found = g_hash_table_lookup(manager->loaders, id);
	if(GPLUGIN_IS_LOADER(found)) {
		g_set_error(
			error,
			GPLUGIN_DOMAIN,
			0,
			_("loader %s was already registered"),
			id);
		return FALSE;
	}

	g_hash_table_insert(manager->loaders, g_strdup(id), g_object_ref(loader));

	/* make a note that we need to refresh */
	manager->refresh_needed = TRUE;

	g_signal_emit(manager, signals[SIG_LOADER_REGISTERED], 0, loader);

	return TRUE;
}

/**
 * gplugin_manager_unregister_loader:
 * @manager: The manager instance.
 * @loader: The loader instance to unregister.
 * @error: (out) (nullable): The return address for a [struct@GLib.Error].
 *
 * Unregisters @loader as an available loader.
 *
 * Returns: %TRUE if the loader was successfully unregistered, %FALSE
 *          otherwise with @error set.
 */
gboolean
gplugin_manager_unregister_loader(
	GPluginManager *manager,
	GPluginLoader *loader,
	GError **error)
{
	const gchar *id = NULL;

	g_return_val_if_fail(GPLUGIN_IS_MANAGER(manager), FALSE);
	g_return_val_if_fail(GPLUGIN_IS_LOADER(loader), FALSE);

	id = gplugin_loader_get_id(loader);

	loader = g_hash_table_lookup(manager->loaders, id);
	if(!GPLUGIN_IS_LOADER(loader)) {
		g_set_error(
			error,
			GPLUGIN_DOMAIN,
			0,
			_("loader %s is not registered"),
			id);

		return FALSE;
	}

	/* Temporarily add a reference to loader so we can emit the signal if it
	 * was removed from our table correctly.
	 */
	g_object_ref(loader);

	if(g_hash_table_remove(manager->loaders, id)) {
		g_signal_emit(manager, signals[SIG_LOADER_UNREGISTERED], 0, loader);
	}

	g_clear_object(&loader);

	return TRUE;
}

/**
 * gplugin_manager_get_loaders:
 * @manager: The manager instance.
 *
 * Returns a list of all registered loaders.
 *
 * Returns: (element-type GPlugin.Loader) (transfer container): Returns a list
 *          of all registered loaders.
 */
GList *
gplugin_manager_get_loaders(GPluginManager *manager)
{
	g_return_val_if_fail(GPLUGIN_IS_MANAGER(manager), FALSE);

	return g_hash_table_get_values(manager->loaders);
}

/**
 * gplugin_manager_refresh:
 * @manager: The manager instance.
 *
 * Forces a refresh of all plugins found in the search paths.
 */
void
gplugin_manager_refresh(GPluginManager *manager)
{
	GPluginSource *file_source = NULL;

	g_return_if_fail(GPLUGIN_IS_MANAGER(manager));

	file_source = gplugin_file_source_new(manager);

	manager->refresh_needed = TRUE;

	while(manager->refresh_needed) {
		manager->refresh_needed = FALSE;

		if(gplugin_source_scan(file_source)) {
			manager->refresh_needed = TRUE;
		}
	}

	g_clear_object(&file_source);
}

/**
 * gplugin_manager_foreach:
 * @manager: The manager instance.
 * @func: (scope call): The function to call with each plugin.
 * @data: User data to pass to @func.
 *
 * Calls @func for each plugin that is known.
 */
void
gplugin_manager_foreach(
	GPluginManager *manager,
	GPluginManagerForeachFunc func,
	gpointer data)
{
	GHashTableIter iter;
	gpointer id = NULL, plugins = NULL;

	g_return_if_fail(GPLUGIN_IS_MANAGER(manager));
	g_return_if_fail(func != NULL);

	g_hash_table_iter_init(&iter, manager->plugins);
	while(g_hash_table_iter_next(&iter, &id, &plugins)) {
		func((gchar *)id, (GSList *)plugins, data);
	}
}

/**
 * gplugin_manager_find_plugins:
 * @manager: The manager instance.
 * @id: The ID of the plugin to find.
 *
 * Finds all plugins matching @id.
 *
 * Returns: (element-type GPlugin.Plugin) (transfer full): A [struct@GLib.SList]
 *          of plugins matching @id.
 */
GSList *
gplugin_manager_find_plugins(GPluginManager *manager, const gchar *id)
{
	GSList *plugins_list = NULL, *l;

	g_return_val_if_fail(GPLUGIN_IS_MANAGER(manager), NULL);
	g_return_val_if_fail(id != NULL, NULL);

	l = g_hash_table_lookup(manager->plugins, id);
	plugins_list = g_slist_copy_deep(l, (GCopyFunc)g_object_ref, NULL);

	return plugins_list;
}

/**
 * gplugin_manager_find_plugins_with_version:
 * @manager: The manager instance.
 * @id: The ID of the plugin to find.
 * @op: one of <, <=, =, ==, >=, >.
 * @version: The version to compare against.
 *
 * Similar to [method@GPlugin.Manager.find_plugins] but only returns plugins
 * whose versions match @op and @version.
 *
 * This is primarily used for dependency loading where a plugin may depend on a
 * specific range of versions of another plugin.
 *
 * Returns: (element-type GPlugin.Plugin) (transfer full): A [struct@GLib.SList]
 *          of plugins matching @id and the version constraint.
 */
GSList *
gplugin_manager_find_plugins_with_version(
	GPluginManager *manager,
	const gchar *id,
	const gchar *op,
	const gchar *version)
{
	GSList *plugins = NULL, *filtered = NULL, *l = NULL;

	g_return_val_if_fail(GPLUGIN_IS_MANAGER(manager), NULL);

	plugins = gplugin_manager_find_plugins(manager, id);

	if((op == NULL || *op == '\0') && (version == NULL || *version == '\0')) {
		/* we weren't actually passed an operator and a version so just return
		 * the list we have based on the id.
		 */
		return plugins;
	}

	for(l = plugins; l; l = l->next) {
		GPluginPlugin *plugin = GPLUGIN_PLUGIN(l->data);
		GPluginPluginInfo *info = NULL;
		const gchar *found_version = NULL;
		gint result = 0;
		gboolean keep = FALSE;

		/* get the plugin's version from it's info */
		info = gplugin_plugin_get_info(plugin);
		found_version = gplugin_plugin_info_get_version(info);

		/* now compare the version of the plugin to passed in version.  This
		 * should be done in this order so it's easier to track the operators.
		 * IE: we want to keep the inequality the same.
		 */
		result = gplugin_version_compare(found_version, version);

		/* we need to keep info around until we're done using found_version */
		g_object_unref(G_OBJECT(info));

		if(result < 0) {
			keep = (g_strcmp0(op, "<") == 0 || g_strcmp0(op, "<=") == 0);
		} else if(result == 0) {
			keep =
				(g_strcmp0(op, "=") == 0 || g_strcmp0(op, "==") == 0 ||
				 g_strcmp0(op, "<=") == 0 || g_strcmp0(op, ">=") == 0);
		} else if(result > 0) {
			keep = (g_strcmp0(op, ">") == 0 || g_strcmp0(op, ">=") == 0);
		}

		if(keep) {
			filtered =
				g_slist_prepend(filtered, g_object_ref(G_OBJECT(plugin)));
		}
	}

	g_slist_free_full(plugins, g_object_unref);

	return g_slist_reverse(filtered);
}

/**
 * gplugin_manager_find_plugins_with_state:
 * @manager: The manager instance.
 * @state: The state to look for.
 *
 * Finds all plugins that currently have a state of @state.
 *
 * Returns: (element-type GPlugin.Plugin) (transfer full): A [struct@GLib.SList]
 *          of plugins whose state is @state.
 */
GSList *
gplugin_manager_find_plugins_with_state(
	GPluginManager *manager,
	GPluginPluginState state)
{
	GSList *plugins = NULL;
	GHashTableIter iter;
	gpointer value = NULL;

	g_return_val_if_fail(GPLUGIN_IS_MANAGER(manager), NULL);

	g_hash_table_iter_init(&iter, manager->plugins);
	while(g_hash_table_iter_next(&iter, NULL, &value)) {
		GSList *l = NULL;

		for(l = (GSList *)value; l != NULL; l = l->next) {
			GPluginPlugin *plugin = GPLUGIN_PLUGIN(l->data);

			if(gplugin_plugin_get_state(plugin) == state) {
				plugins =
					g_slist_prepend(plugins, g_object_ref(G_OBJECT(plugin)));
			}
		}
	}

	return plugins;
}

/**
 * gplugin_manager_find_plugin:
 * @manager: The manager instance.
 * @id: The ID of the plugin to find.
 *
 * Finds the first plugin matching @id.
 *
 * This function uses [method@GPlugin.Manager.find_plugins] and returns the
 * first plugin in the list.
 *
 * Returns: (transfer full): A plugin instance or %NULL if no plugin
 *          matching @id was found.
 */
GPluginPlugin *
gplugin_manager_find_plugin(GPluginManager *manager, const gchar *id)
{
	GSList *plugins_list = NULL;
	GPluginPlugin *plugin = NULL;

	g_return_val_if_fail(id != NULL, NULL);

	plugins_list = gplugin_manager_find_plugins(manager, id);
	if(plugins_list == NULL) {
		return NULL;
	}

	plugin = GPLUGIN_PLUGIN(g_object_ref(G_OBJECT(plugins_list->data)));

	g_slist_free_full(plugins_list, g_object_unref);

	return plugin;
}

/**
 * gplugin_manager_find_plugin_with_newest_version:
 * @manager: The manager instance.
 * @id: The ID of the plugin to find.
 *
 * Calls [method@GPlugin.Manager.find_plugins] with @id, and then returns the
 * plugins with the highest version number or %NULL if no plugins with @id are
 * found.
 *
 * Returns: (transfer full): The plugin with an ID of @id that has the highest
 *          version number, or %NULL if no plugins were found with @id.
 */
GPluginPlugin *
gplugin_manager_find_plugin_with_newest_version(
	GPluginManager *manager,
	const gchar *id)
{
	GPluginPlugin *plugin_a = NULL;
	GPluginPluginInfo *info_a = NULL;
	const gchar *version_a = NULL;
	GSList *l = NULL;

	g_return_val_if_fail(id != NULL, NULL);

	l = gplugin_manager_find_plugins(manager, id);
	for(; l != NULL; l = g_slist_delete_link(l, l)) {
		GPluginPlugin *plugin_b = NULL;
		GPluginPluginInfo *info_b = NULL;
		const gchar *version_b = NULL;
		gint cmp = 0;

		if(!GPLUGIN_IS_PLUGIN(l->data)) {
			continue;
		}

		plugin_b = GPLUGIN_PLUGIN(l->data);
		info_b = gplugin_plugin_get_info(plugin_b);

		/* If this is the first plugin we've found, set the plugin_a values and
		 * continue.
		 */
		if(!GPLUGIN_IS_PLUGIN(plugin_a)) {
			plugin_a = plugin_b;
			info_a = info_b;

			version_a = gplugin_plugin_info_get_version(info_a);

			continue;
		}

		/* At this point, we've seen another plugin, so we need to compare
		 * their versions.
		 */
		version_b = gplugin_plugin_info_get_version(info_b);

		cmp = gplugin_version_compare(version_a, version_b);
		if(cmp < 0) {
			/* plugin_b has a newer version, so set the plugin_a pointers to
			 * the plugin_b pointers as well as the version pointers.
			 */
			g_set_object(&plugin_a, plugin_b);
			g_set_object(&info_a, info_b);

			version_a = version_b;
		}

		/* Clean up the plugin_b pointers. */
		g_clear_object(&plugin_b);
		g_clear_object(&info_b);
	}

	g_clear_object(&info_a);

	return plugin_a;
}

/**
 * gplugin_manager_get_plugin_dependencies:
 * @manager: The manager instance.
 * @plugin: The plugin whose dependencies to get.
 * @error: (out) (nullable): Return address for a [struct@GLib.Error].
 *
 * Returns a list of all the plugins that @plugin depends on.
 *
 * Returns: (element-type GPlugin.Plugin) (transfer full): A [struct@GLib.SList]
 *          of plugins that @plugin depends on, or %NULL on error with @error
 *          set.
 */
GSList *
gplugin_manager_get_plugin_dependencies(
	GPluginManager *manager,
	GPluginPlugin *plugin,
	GError **error)
{
	GPluginPluginInfo *info = NULL;
	GSList *ret = NULL;
	const gchar *const *dependencies = NULL;
	gint i = 0;

	g_return_val_if_fail(GPLUGIN_IS_MANAGER(manager), NULL);
	g_return_val_if_fail(GPLUGIN_IS_PLUGIN(plugin), NULL);

	info = gplugin_plugin_get_info(plugin);
	dependencies = gplugin_plugin_info_get_dependencies(info);
	g_object_unref(G_OBJECT(info));

	if(dependencies == NULL) {
		return NULL;
	}

	for(i = 0; dependencies[i] != NULL; i++) {
		gboolean found = FALSE;
		gchar **ors = NULL;
		gint o = 0;

		ors = g_strsplit(dependencies[i], "|", 0);
		for(o = 0; ors[o]; o++) {
			GMatchInfo *match = NULL;
			GSList *matches = NULL;
			gchar *oid = NULL, *oop = NULL, *over = NULL;

			if(!g_regex_match(dependency_regex, ors[o], 0, &match)) {
				continue;
			}

			/* grab the or'd id, op, and version */
			oid = g_match_info_fetch_named(match, "id");
			oop = g_match_info_fetch_named(match, "op");
			over = g_match_info_fetch_named(match, "version");

			/* free the match info */
			g_match_info_free(match);

			/* now look for a plugin matching the id */
			matches = gplugin_manager_find_plugins_with_version(
				manager,
				oid,
				oop,
				over);
			g_free(oid);
			g_free(oop);
			g_free(over);

			if(matches == NULL) {
				continue;
			}

			/* prepend the first found match to our return value */
			ret = g_slist_prepend(ret, g_object_ref(matches->data));
			g_slist_free_full(matches, g_object_unref);

			found = TRUE;

			break;
		}
		g_strfreev(ors);

		if(!found) {
			g_set_error(
				error,
				GPLUGIN_DOMAIN,
				0,
				_("failed to find dependency %s for %s"),
				dependencies[i],
				gplugin_plugin_info_get_id(info));

			g_slist_free_full(ret, g_object_unref);

			return NULL;
		}
	}

	return ret;
}

/**
 * gplugin_manager_load_plugin:
 * @manager: The manager instance.
 * @plugin: The plugin instance.
 * @error: (out) (nullable): Return location for a [struct@GLib.Error] or %NULL.
 *
 * Loads @plugin and all of its dependencies.
 *
 * If a dependency can not be loaded, @plugin will not be loaded either.
 * However, any other plugins that @plugin depends on that were loaded from
 * this call, will not be unloaded.
 *
 * Returns: %TRUE if @plugin was loaded successfully or already loaded, %FALSE
 *          otherwise.
 */
gboolean
gplugin_manager_load_plugin(
	GPluginManager *manager,
	GPluginPlugin *plugin,
	GError **error)
{
	GPluginPluginInfo *info = NULL;
	GPluginLoader *loader = NULL;
	GError *real_error = NULL;
	gboolean ret = TRUE;

	g_return_val_if_fail(GPLUGIN_IS_MANAGER(manager), FALSE);
	g_return_val_if_fail(GPLUGIN_IS_PLUGIN(plugin), FALSE);

	/* if the plugin is already loaded there's nothing for us to do */
	if(gplugin_plugin_get_state(plugin) == GPLUGIN_PLUGIN_STATE_LOADED) {
		return TRUE;
	}

	/* now try to get the plugin info from the plugin */
	info = gplugin_plugin_get_info(plugin);
	if(info == NULL) {
		gchar *filename = gplugin_plugin_get_filename(plugin);

		g_set_error(
			error,
			GPLUGIN_DOMAIN,
			0,
			_("Plugin %s did not return value plugin info"),
			filename);
		g_free(filename);

		gplugin_plugin_set_state(plugin, GPLUGIN_PLUGIN_STATE_LOAD_FAILED);

		return FALSE;
	}

	if(!gplugin_manager_load_dependencies(manager, plugin, info, &real_error)) {
		g_object_unref(G_OBJECT(info));

		g_propagate_error(error, real_error);

		return FALSE;
	}

	g_object_unref(G_OBJECT(info));

	/* now load the actual plugin */
	loader = gplugin_plugin_get_loader(plugin);

	if(!GPLUGIN_IS_LOADER(loader)) {
		gchar *filename = gplugin_plugin_get_filename(plugin);

		g_set_error(
			error,
			GPLUGIN_DOMAIN,
			0,
			_("The loader for %s is not a loader.  This "
			  "should not happened!"),
			filename);
		g_free(filename);

		gplugin_plugin_set_state(plugin, GPLUGIN_PLUGIN_STATE_LOAD_FAILED);
		g_object_unref(G_OBJECT(loader));

		return FALSE;
	}

	g_signal_emit(manager, signals[SIG_LOADING], 0, plugin, &real_error, &ret);
	if(!ret) {
		/* Set the plugin's error. */
		g_object_set(G_OBJECT(plugin), "error", real_error, NULL);

		g_propagate_error(error, real_error);

		gplugin_plugin_set_state(plugin, GPLUGIN_PLUGIN_STATE_LOAD_FAILED);
		g_object_unref(G_OBJECT(loader));

		return ret;
	}

	ret = gplugin_loader_load_plugin(loader, plugin, &real_error);
	if(ret) {
		g_clear_error(&real_error);
		g_signal_emit(manager, signals[SIG_LOADED], 0, plugin);
	} else {
		g_signal_emit(manager, signals[SIG_LOAD_FAILED], 0, plugin, real_error);

		g_propagate_error(error, real_error);
	}

	g_object_unref(G_OBJECT(loader));

	return ret;
}

/**
 * gplugin_manager_unload_plugin:
 * @manager: The manager instance.
 * @plugin: The plugin instance.
 * @error: (out) (nullable): Return location for a [struct@GLib.Error] or %NULL.
 *
 * Unloads @plugin.
 *
 * If @plugin has dependencies, they are not unloaded.
 *
 * Returns: %TRUE if @plugin was unloaded successfully or not loaded, %FALSE
 *          otherwise.
 */
gboolean
gplugin_manager_unload_plugin(
	GPluginManager *manager,
	GPluginPlugin *plugin,
	GError **error)
{
	GPluginLoader *loader = NULL;
	GError *real_error = NULL;
	gboolean ret = TRUE;

	g_return_val_if_fail(GPLUGIN_IS_MANAGER(manager), FALSE);
	g_return_val_if_fail(GPLUGIN_IS_PLUGIN(plugin), FALSE);

	if(gplugin_plugin_get_state(plugin) != GPLUGIN_PLUGIN_STATE_LOADED) {
		return TRUE;
	}

	loader = gplugin_plugin_get_loader(plugin);
	if(!GPLUGIN_IS_LOADER(loader)) {
		g_set_error_literal(
			error,
			GPLUGIN_DOMAIN,
			0,
			_("Plugin loader is not a loader"));
		g_object_unref(G_OBJECT(loader));

		return FALSE;
	}

	g_signal_emit(
		manager,
		signals[SIG_UNLOADING],
		0,
		plugin,
		&real_error,
		&ret);
	if(!ret) {
		/* Set the plugin's error. */
		g_object_set(G_OBJECT(plugin), "error", real_error, NULL);

		g_propagate_error(error, real_error);

		gplugin_plugin_set_state(plugin, GPLUGIN_PLUGIN_STATE_LOAD_FAILED);
		g_object_unref(G_OBJECT(loader));

		return ret;
	}

	ret = gplugin_loader_unload_plugin(loader, plugin, FALSE, &real_error);
	if(ret) {
		g_clear_error(&real_error);
		g_signal_emit(manager, signals[SIG_UNLOADED], 0, plugin);
	} else {
		g_signal_emit(
			manager,
			signals[SIG_UNLOAD_FAILED],
			0,
			plugin,
			real_error);

		g_propagate_error(error, real_error);
	}

	g_object_unref(G_OBJECT(loader));

	return ret;
}

/**
 * gplugin_manager_list_plugins:
 * @manager: The manager instance.
 *
 * Returns a list of all plugin IDs.
 *
 * Each id should be queried directly for more information.
 *
 * Returns: (element-type utf8) (transfer container): A [struct@GLib.List] of
 *          each unique plugin ID.
 */
GList *
gplugin_manager_list_plugins(GPluginManager *manager)
{
	GQueue *queue = NULL;
	GList *ret = NULL;
	GHashTableIter iter;
	gpointer key = NULL;

	g_return_val_if_fail(GPLUGIN_IS_MANAGER(manager), NULL);

	queue = g_queue_new();

	g_hash_table_iter_init(&iter, manager->plugins);
	while(g_hash_table_iter_next(&iter, &key, NULL)) {
		g_queue_push_tail(queue, (gchar *)key);
	}

	ret = g_list_copy(queue->head);

	g_queue_free(queue);

	return ret;
}

/**
 * gplugin_manager_add_plugin:
 * @manager: The instance.
 * @id: The id of the plugin to add.
 * @plugin: The plugin to add.
 *
 * Adds @plugin to @manager with @id. This should only be called by
 * [iface@GPlugin.Source] implementations.
 *
 * Since: 0.39.0
 */
void
gplugin_manager_add_plugin(
	GPluginManager *manager,
	const gchar *id,
	GPluginPlugin *plugin)
{
	GSList *plugins = NULL;

	g_return_if_fail(GPLUGIN_IS_MANAGER(manager));
	g_return_if_fail(id != NULL);
	g_return_if_fail(GPLUGIN_IS_PLUGIN(plugin));

	plugins = g_hash_table_lookup(manager->plugins, id);
	plugins = g_slist_prepend(plugins, g_object_ref(plugin));
	g_hash_table_insert(manager->plugins, g_strdup(id), plugins);
}

/**
 * gplugin_manager_get_default:
 *
 * Gets the default plugin manager in GPlugin.
 *
 * Returns: (transfer none): The default GPluginManager instance.
 *
 * Since: 0.33.0
 */
GPluginManager *
gplugin_manager_get_default(void)
{
	return default_manager;
}
