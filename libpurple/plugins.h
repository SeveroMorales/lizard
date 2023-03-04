/* purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_PLUGINS_H
#define PURPLE_PLUGINS_H

#include <gplugin.h>
#include <gplugin-native.h>

#include "version.h"

#define PURPLE_PLUGINS_DOMAIN          (g_quark_from_static_string("plugins"))

#define PURPLE_TYPE_PLUGIN             GPLUGIN_TYPE_PLUGIN
#define PURPLE_PLUGIN(obj)             GPLUGIN_PLUGIN(obj)
#define PURPLE_IS_PLUGIN(obj)          GPLUGIN_IS_PLUGIN(obj)
#define PURPLE_PLUGIN_GET_IFACE(obj)   GPLUGIN_PLUGIN_GET_IFACE(obj)

/**
 * PurplePlugin:
 *
 * Represents a plugin handle.
 * This type is an alias for GPluginPlugin.
 */
typedef GPluginPlugin PurplePlugin;

typedef GPluginPluginInterface PurplePluginInterface;

#include "purpleplugininfo.h"

G_BEGIN_DECLS

/**************************************************************************/
/* Plugin API                                                             */
/**************************************************************************/

/**
 * purple_plugin_load:
 * @plugin: The plugin to load.
 * @error:  Return location for a #GError or %NULL. If provided, this
 *               will be set to the reason if the load fails.
 *
 * Attempts to load a plugin.
 *
 * Also see purple_plugin_unload().
 *
 * Returns: %TRUE if successful or already loaded, %FALSE otherwise.
 */
gboolean purple_plugin_load(PurplePlugin *plugin, GError **error);

/**
 * purple_plugin_unload:
 * @plugin: The plugin handle.
 * @error:  Return location for a #GError or %NULL. If provided, this
 *               will be set to the reason if the unload fails.
 *
 * Unloads the specified plugin.
 *
 * Also see purple_plugin_load().
 *
 * Returns: %TRUE if successful or not loaded, %FALSE otherwise.
 */
gboolean purple_plugin_unload(PurplePlugin *plugin, GError **error);

/**
 * purple_plugin_is_loaded:
 * @plugin: The plugin.
 *
 * Returns whether or not a plugin is currently loaded.
 *
 * Returns: %TRUE if loaded, or %FALSE otherwise.
 */
gboolean purple_plugin_is_loaded(PurplePlugin *plugin);

/**
 * purple_plugin_get_info:
 * @plugin: The plugin.
 *
 * Returns a plugin's #PurplePluginInfo instance.
 *
 * Returns: (transfer none): The plugin's #PurplePluginInfo instance.
 * GPlugin refs the plugin info object before returning it. This workaround
 * is to avoid managing the reference counts everywhere in our codebase
 * where we use the plugin info. The plugin info instance is guaranteed to
 * exist as long as the plugin exists.
 */
PurplePluginInfo *purple_plugin_get_info(PurplePlugin *plugin);

/**
 * purple_plugin_disable:
 *
 * Disable a plugin.
 *
 * This function adds the plugin to a list of plugins to "disable at the next
 * startup" by excluding said plugins from the list of plugins to save.  The
 * UI needs to call purple_plugins_save_loaded() after calling this for it
 * to have any effect.
 */
void purple_plugin_disable(PurplePlugin *plugin);

/**
 * purple_plugin_is_internal:
 * @plugin: The plugin.
 *
 * Returns whether a plugin is an internal plugin. Internal plugins provide
 * required additional functionality to the libpurple core. These plugins must
 * not be shown in plugin lists. Examples of such plugins are in-tree protocol
 * plugins, loaders etc.
 *
 * Returns: %TRUE if the plugin is an internal plugin, %FALSE otherwise.
 */
gboolean purple_plugin_is_internal(PurplePlugin *plugin);

/**
 * purple_plugin_get_dependent_plugins:
 * @plugin: The plugin whose dependent plugins are returned.
 *
 * Returns a list of plugins that depend on a particular plugin.
 *
 * Returns: (element-type PurplePlugin) (transfer none): The list of a plugins that depend on the specified
 *                           plugin.
 */
GSList *purple_plugin_get_dependent_plugins(PurplePlugin *plugin);

/**************************************************************************/
/* Plugins API                                                            */
/**************************************************************************/

/**
 * purple_plugins_find_all:
 *
 * Returns a list of all plugins, whether loaded or not.
 *
 * Returns: (element-type PurplePlugin) (transfer full): A list of all plugins.
 * 	       The list is owned by the caller, and must be
 *         g_list_free()d to avoid leaking the nodes.
 */
GList *purple_plugins_find_all(void);

/**
 * purple_plugins_get_loaded:
 *
 * Returns a list of all loaded plugins.
 *
 * Returns: (element-type PurplePlugin) (transfer none): A list of all loaded plugins.
 */
GList *purple_plugins_get_loaded(void);

/**
 * purple_plugins_add_search_path:
 * @path: The new search path.
 *
 * Add a new directory to search for plugins
 */
void purple_plugins_add_search_path(const gchar *path);

/**
 * purple_plugins_refresh:
 *
 * Forces a refresh of all plugins found in the search paths, and loads plugins
 * that are to be auto-loaded.
 *
 * See purple_plugins_add_search_path().
 */
void purple_plugins_refresh(void);

/**
 * purple_plugins_find_plugin:
 * @id: The plugin ID.
 *
 * Finds a plugin with the specified plugin ID.
 *
 * Returns: (transfer none): The plugin if found, or %NULL if not found.
 */
PurplePlugin *purple_plugins_find_plugin(const gchar *id);

/**
 * purple_plugins_find_by_filename:
 * @filename: The plugin filename.
 *
 * Finds a plugin with the specified filename (filename with a path).
 *
 * Returns: (transfer none): The plugin if found, or %NULL if not found.
 */
PurplePlugin *purple_plugins_find_by_filename(const char *filename);

/**
 * purple_plugins_save_loaded:
 * @key: The preference key to save the list of plugins to.
 *
 * Saves the list of loaded plugins to the specified preference key.
 * Plugins that are set to auto-load are not saved.
 */
void purple_plugins_save_loaded(const char *key);

/**
 * purple_plugins_load_saved:
 * @key: The preference key containing the list of plugins.
 *
 * Attempts to load all the plugins in the specified preference key
 * that were loaded when purple last quit.
 */
void purple_plugins_load_saved(const char *key);

/**************************************************************************/
/* Plugins Subsystem API                                                  */
/**************************************************************************/

/**
 * purple_plugins_init:
 *
 * Initializes the plugin subsystem
 */
void purple_plugins_init(void);

/**
 * purple_plugins_uninit:
 *
 * Uninitializes the plugin subsystem
 */
void purple_plugins_uninit(void);

G_END_DECLS

#endif /* PURPLE_PLUGINS_H */
