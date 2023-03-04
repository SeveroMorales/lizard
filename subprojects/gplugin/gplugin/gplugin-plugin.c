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

#include <gplugin/gplugin-plugin.h>

#include <gplugin/gplugin-core.h>
#include <gplugin/gplugin-enums.h>
#include <gplugin/gplugin-private.h>

/**
 * GPluginPluginState:
 * @GPLUGIN_PLUGIN_STATE_UNKNOWN: The state of the plugin is unknown.
 * @GPLUGIN_PLUGIN_STATE_ERROR: There was an error loading or unloading the
 *  plugin.
 * @GPLUGIN_PLUGIN_STATE_QUERIED: The plugin has been queried but not loaded.
 * @GPLUGIN_PLUGIN_STATE_REQUERY: The plugin should be re-queried.
 * @GPLUGIN_PLUGIN_STATE_LOADED: The plugin is loaded.
 * @GPLUGIN_PLUGIN_STATE_LOAD_FAILED: The plugin failed to load.
 * @GPLUGIN_PLUGIN_STATE_UNLOAD_FAILED: The plugin failed to unload.
 *
 * The known states of a plugin.
 */

/**
 * GPluginPlugin:
 *
 * #GPluginPlugin is an interface that represents what GPlugin expects for a
 * plugin.
 */

/**
 * GPluginPluginInterface:
 * @state_changed: The class closure for the #GPluginPlugin::state-changed
 *                 signal.
 *
 * The interface that defines the behavior of plugins, including properties and
 * signals.
 */

/******************************************************************************
 * Enums
 *****************************************************************************/
enum {
	SIG_STATE_CHANGED,
	SIG_LAST,
};
static guint signals[SIG_LAST] = {
	0,
};

G_DEFINE_INTERFACE(GPluginPlugin, gplugin_plugin, G_TYPE_INVALID)

/******************************************************************************
 * Object Stuff
 *****************************************************************************/
static void
gplugin_plugin_default_init(GPluginPluginInterface *iface)
{
	GParamSpec *pspec = NULL;

	/**
	 * GPluginPlugin:filename:
	 *
	 * The absolute path to the plugin on disk.
	 */
	pspec = g_param_spec_string(
		"filename",
		"filename",
		"The filename of the plugin",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_interface_install_property(iface, pspec);

	/**
	 * GPluginPlugin:loader:
	 *
	 * The loader that loaded this plugin.
	 */
	pspec = g_param_spec_object(
		"loader",
		"loader",
		"The loader for this plugin",
		GPLUGIN_TYPE_LOADER,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_interface_install_property(iface, pspec);

	/**
	 * GPluginPlugin:info:
	 *
	 * The plugin info from this plugin.
	 */
	pspec = g_param_spec_object(
		"info",
		"info",
		"The information for the plugin",
		GPLUGIN_TYPE_PLUGIN_INFO,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
	g_object_interface_install_property(iface, pspec);

	/**
	 * GPluginPlugin:state:
	 *
	 * The plugin state that this plugin is in.
	 */
	pspec = g_param_spec_enum(
		"state",
		"state",
		"The state of the plugin",
		GPLUGIN_TYPE_PLUGIN_STATE,
		GPLUGIN_PLUGIN_STATE_UNKNOWN,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT);
	g_object_interface_install_property(iface, pspec);

	/**
	 * GPluginPlugin:desired-state:
	 *
	 * The desired state of the plugin. Typically this just mirrors the state
	 * property, but if a state change failed this will remain set to the state
	 * that was attempted.
	 *
	 * See [method@GPlugin.Plugin.get_desired_state] for more information.
	 *
	 * Since: 0.38.0
	 */
	pspec = g_param_spec_enum(
		"desired-state",
		"desired-state",
		"The desired state of the plugin",
		GPLUGIN_TYPE_PLUGIN_STATE,
		GPLUGIN_PLUGIN_STATE_UNKNOWN,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);
	g_object_interface_install_property(iface, pspec);

	/**
	 * GPluginPlugin:error:
	 *
	 * An error that was returned if the plugin failed to load or unload.
	 */
	pspec = g_param_spec_boxed(
		"error",
		"error",
		"A GError returned from the load or unload function",
		G_TYPE_ERROR,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	g_object_interface_install_property(iface, pspec);

	/**
	 * GPluginPlugin::state-changed:
	 * @plugin: The plugin that changed states.
	 * @oldstate: The old state of @plugin.
	 * @newstate: The new state of @plugin.
	 *
	 * Emitted when @plugin changes state.
	 */
	signals[SIG_STATE_CHANGED] = g_signal_new(
		"state-changed",
		GPLUGIN_TYPE_PLUGIN,
		G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET(GPluginPluginInterface, state_changed),
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		2,
		GPLUGIN_TYPE_PLUGIN_STATE,
		GPLUGIN_TYPE_PLUGIN_STATE);
}

/******************************************************************************
 * GPluginPlugin API
 *****************************************************************************/

/**
 * gplugin_plugin_get_filename:
 * @plugin: The plugin instance.
 *
 * Returns the filename that @plugin was loaded from.
 *
 * Returns: (transfer full): The filename of @plugin.
 */
gchar *
gplugin_plugin_get_filename(GPluginPlugin *plugin)
{
	gchar *filename = NULL;

	g_return_val_if_fail(GPLUGIN_IS_PLUGIN(plugin), NULL);

	g_object_get(G_OBJECT(plugin), "filename", &filename, NULL);

	return filename;
}

/**
 * gplugin_plugin_get_loader:
 * @plugin: The plugin instance.
 *
 * Returns the loader that loaded @plugin.
 *
 * Returns: (transfer full): The #GPluginLoader that loaded @plugin.
 */
GPluginLoader *
gplugin_plugin_get_loader(GPluginPlugin *plugin)
{
	GPluginLoader *loader = NULL;

	g_return_val_if_fail(GPLUGIN_IS_PLUGIN(plugin), NULL);

	g_object_get(G_OBJECT(plugin), "loader", &loader, NULL);

	return loader;
}

/**
 * gplugin_plugin_get_info:
 * @plugin: The plugin instance.
 *
 * Returns the plugin info for @plugin.
 *
 * Returns: (transfer full): The plugin info instance for @plugin.
 */
GPluginPluginInfo *
gplugin_plugin_get_info(GPluginPlugin *plugin)
{
	GPluginPluginInfo *info = NULL;

	g_return_val_if_fail(GPLUGIN_IS_PLUGIN(plugin), NULL);

	g_object_get(G_OBJECT(plugin), "info", &info, NULL);

	return info;
}

/**
 * gplugin_plugin_get_state:
 * @plugin: The plugin instance.
 *
 * Gets the current state of @plugin.
 *
 * Returns: (transfer full): The current state of @plugin.
 */
GPluginPluginState
gplugin_plugin_get_state(GPluginPlugin *plugin)
{
	GPluginPluginState state = GPLUGIN_PLUGIN_STATE_UNKNOWN;

	g_return_val_if_fail(
		GPLUGIN_IS_PLUGIN(plugin),
		GPLUGIN_PLUGIN_STATE_UNKNOWN);

	g_object_get(G_OBJECT(plugin), "state", &state, NULL);

	return state;
}

/**
 * gplugin_plugin_set_state:
 * @plugin: The plugin instance.
 * @state: The new state for @plugin.
 *
 * Changes the state of @plugin to @state.  This function should only be called
 * by loaders.
 */
void
gplugin_plugin_set_state(GPluginPlugin *plugin, GPluginPluginState state)
{
	GPluginPluginState oldstate = GPLUGIN_PLUGIN_STATE_UNKNOWN;

	g_return_if_fail(GPLUGIN_IS_PLUGIN(plugin));

	oldstate = gplugin_plugin_get_state(plugin);

	g_object_set(G_OBJECT(plugin), "state", state, NULL);

	if(gplugin_get_flags() & GPLUGIN_CORE_FLAGS_LOG_PLUGIN_STATE_CHANGES) {
		GPluginPluginInfo *info = gplugin_plugin_get_info(plugin);

		g_message(
			"plugin %s state changed from %s to %s: filename=%s",
			gplugin_plugin_info_get_id(info),
			gplugin_plugin_state_to_string(oldstate),
			gplugin_plugin_state_to_string(state),
			gplugin_plugin_get_filename(plugin));

		g_clear_object(&info);
	}

	g_signal_emit(plugin, signals[SIG_STATE_CHANGED], 0, oldstate, state);
}

/**
 * gplugin_plugin_get_desired_state:
 *
 * Gets the desired state of the plugin. Typically this will hold the same
 * value of [property@GPlugin.Plugin:state], but if a state change failed this
 * will remain set to the state that was attempted.
 *
 * For example, say a user wants to unload a plugin but the plugin can't be
 * unloaded for some reason. The state will be set to loaded, but the
 * desired state will be set to unloaded.
 *
 * This behavior can be used to give the user the ability to disable a
 * plugin that normally isn't unloadabled from being loaded during the next
 * run of program.
 *
 * Returns: The desired state that the user has requested the plugin to be in.
 *
 * Since: 0.38.0
 */
GPluginPluginState
gplugin_plugin_get_desired_state(GPluginPlugin *plugin)
{
	GPluginPluginState desired_state = GPLUGIN_PLUGIN_STATE_UNKNOWN;

	g_return_val_if_fail(
		GPLUGIN_IS_PLUGIN(plugin),
		GPLUGIN_PLUGIN_STATE_UNKNOWN);

	g_object_get(G_OBJECT(plugin), "desired-state", &desired_state, NULL);

	return desired_state;
}

/**
 * gplugin_plugin_set_desired_state:
 * @plugin: The plugin instance.
 * @state: The desired state.
 *
 * Sets the desired state of the plugin. This shouldn't need to be called by
 * anyone except [class@GPlugin.Loader] which manages the state of plugins.
 *
 * Since: 0.38.0
 */
void
gplugin_plugin_set_desired_state(
	GPluginPlugin *plugin,
	GPluginPluginState state)
{
	g_return_if_fail(GPLUGIN_IS_PLUGIN(plugin));

	g_object_set(G_OBJECT(plugin), "desired-state", state, NULL);
}

/**
 * gplugin_plugin_get_error:
 * @plugin: The plugin instance.
 *
 * Gets the [struct@GLib.Error], if any, that the plugin returned during load or
 * unload.
 *
 * Returns: (transfer full): The error the plugin returned during load or
 *          unload, or %NULL if no error occurred.
 */
GError *
gplugin_plugin_get_error(GPluginPlugin *plugin)
{
	GError *error = NULL;

	g_return_val_if_fail(GPLUGIN_IS_PLUGIN(plugin), NULL);

	g_object_get(G_OBJECT(plugin), "error", &error, NULL);

	return error;
}

/**
 * gplugin_plugin_state_to_string:
 * @state: The #GPluginPluginState.
 *
 * Gets a string representation of @state.
 *
 * Returns: The string representation of @state.
 *
 * Since: 0.32.0
 */
const gchar *
gplugin_plugin_state_to_string(GPluginPluginState state)
{
	const gchar *state_str = NULL;

	switch(state) {
		case GPLUGIN_PLUGIN_STATE_QUERIED:
			state_str = "queried";
			break;
		case GPLUGIN_PLUGIN_STATE_REQUERY:
			state_str = "requery";
			break;
		case GPLUGIN_PLUGIN_STATE_LOADED:
			state_str = "loaded";
			break;
		case GPLUGIN_PLUGIN_STATE_LOAD_FAILED:
			state_str = "load-failed";
			break;
		case GPLUGIN_PLUGIN_STATE_UNLOAD_FAILED:
			state_str = "unload-failed";
			break;
		default:
			state_str = "unknown";
			break;
	}

	return state_str;
}
