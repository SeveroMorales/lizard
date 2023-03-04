/*
 * Copyright (C) 2011-2020 Gary Kramlich <grim@reaperworld.com>
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

#include <gplugin/gplugin-core.h>
#include <gplugin/gplugin-loader.h>

/**
 * GPluginLoader:
 *
 * An abstract class that should not be accessed directly.
 */

/**
 * GPluginLoaderClass:
 * @supported_extensions: The supported_extensions vfunc returns a #GList of
 *                        file extensions that this loader supports without the
 *                        leading dot. For example: 'so', 'dll', 'py', etc.
 * @query: The query vfunc is called when the plugin manager needs to query a
 *         plugin that has a file extension from @supported_extensions.
 * @load: The load vfunc is called when the plugin manager wants to load a
 *        plugin that was previously queried by this loader.
 * @unload: The unload vfunc is called when the plugin manager wants to unload
 *          a previously loaded plugin from this loader.
 *
 * #GPluginLoaderClass defines the behavior for loading plugins.
 */

typedef struct {
	gchar *id;
} GPluginLoaderPrivate;

enum {
	PROP_0,
	PROP_ID,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {
	NULL,
};

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(
	GPluginLoader,
	gplugin_loader,
	G_TYPE_OBJECT)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
gplugin_loader_set_id(GPluginLoader *loader, const gchar *id)
{
	GPluginLoaderPrivate *priv = gplugin_loader_get_instance_private(loader);

	g_free(priv->id);
	priv->id = g_strdup(id);

	g_object_notify_by_pspec(G_OBJECT(loader), properties[PROP_ID]);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
gplugin_loader_get_property(
	GObject *obj,
	guint param_id,
	GValue *value,
	GParamSpec *pspec)
{
	GPluginLoader *loader = GPLUGIN_LOADER(obj);

	switch(param_id) {
		case PROP_ID:
			g_value_set_string(value, gplugin_loader_get_id(loader));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
gplugin_loader_set_property(
	GObject *obj,
	guint param_id,
	const GValue *value,
	GParamSpec *pspec)
{
	GPluginLoader *loader = GPLUGIN_LOADER(obj);

	switch(param_id) {
		case PROP_ID:
			gplugin_loader_set_id(loader, g_value_get_string(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
gplugin_loader_finalize(GObject *obj)
{
	GPluginLoader *loader = NULL;
	GPluginLoaderPrivate *priv = NULL;

	loader = GPLUGIN_LOADER(obj);
	priv = gplugin_loader_get_instance_private(loader);

	g_clear_pointer(&priv->id, g_free);

	G_OBJECT_CLASS(gplugin_loader_parent_class)->finalize(obj);
}

static void
gplugin_loader_init(G_GNUC_UNUSED GPluginLoader *loader)
{
}

static void
gplugin_loader_class_init(GPluginLoaderClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = gplugin_loader_get_property;
	obj_class->set_property = gplugin_loader_set_property;
	obj_class->finalize = gplugin_loader_finalize;

	/**
	 * GPluginLoader::id:
	 *
	 * The identifier of the loader.
	 *
	 * Since: 0.34.0
	 */
	properties[PROP_ID] = g_param_spec_string(
		"id",
		"id",
		"The identifier of the loader",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * API
 *****************************************************************************/

/**
 * gplugin_loader_get_id:
 * @loader: The loader instance.
 *
 * Gets the identifier of @loader.
 *
 * Returns: The ID of @loader.
 *
 * Since: 0.34.0
 */
const gchar *
gplugin_loader_get_id(GPluginLoader *loader)
{
	GPluginLoaderPrivate *priv = NULL;

	g_return_val_if_fail(GPLUGIN_IS_LOADER(loader), NULL);

	priv = gplugin_loader_get_instance_private(loader);

	return priv->id;
}

/**
 * gplugin_loader_query_plugin:
 * @loader: The loader instance performing the query.
 * @filename: The filename to query.
 * @error: (nullable): The return location for a [struct@GLib.Error], or %NULL.
 *
 * This function is called by the plugin manager to ask @loader to query
 * @filename and determine if it's a usable plugin.
 *
 * Returns: (transfer full): A plugin instance or %NULL on failure.
 */
GPluginPlugin *
gplugin_loader_query_plugin(
	GPluginLoader *loader,
	const gchar *filename,
	GError **error)
{
	GPluginLoaderClass *klass = NULL;
	GPluginPlugin *plugin = NULL;
	GError *real_error = NULL;

	g_return_val_if_fail(loader != NULL, NULL);
	g_return_val_if_fail(GPLUGIN_IS_LOADER(loader), NULL);
	g_return_val_if_fail(filename, NULL);
	g_return_val_if_fail(error != NULL, NULL);

	/* We don't have a plugin instance yet, so we just set the desired-state at
	 * the same time we set the state because that's the first opportunity we
	 * have.
	 */

	klass = GPLUGIN_LOADER_GET_CLASS(loader);

	if(klass != NULL && klass->query != NULL) {
		plugin = klass->query(loader, filename, &real_error);
	}

	if(!GPLUGIN_IS_PLUGIN(plugin)) {
		if(real_error == NULL) {
			real_error = g_error_new_literal(
				GPLUGIN_DOMAIN,
				0,
				"Failed to query plugin : unknown");
		}

		g_propagate_error(error, real_error);
	} else {
		/* If the plugin successfully queried but returned an error, ignore the
		 * error.
		 */
		g_clear_error(&real_error);

		/* Set a few properties. */
		/* clang-format off */
		g_object_set(G_OBJECT(plugin),
			"error", NULL,
			"state", GPLUGIN_PLUGIN_STATE_QUERIED,
			"desired-state", GPLUGIN_PLUGIN_STATE_QUERIED,
			NULL);
		/* clang-format on */
	}

	return plugin;
}

/**
 * gplugin_loader_load_plugin:
 * @loader: The loader instance performing the load.
 * @plugin: The plugin instance to load.
 * @error: (nullable): The return location for a [struct@GLib.Error], or %NULL.
 *
 * This function is called by the plugin manager to ask @loader to load
 * @plugin.
 *
 * Returns: %TRUE if @plugin was loaded successfully, %FALSE otherwise.
 */
gboolean
gplugin_loader_load_plugin(
	GPluginLoader *loader,
	GPluginPlugin *plugin,
	GError **error)
{
	GPluginLoaderClass *klass = NULL;
	GError *real_error = NULL;
	gboolean ret = FALSE;

	g_return_val_if_fail(loader != NULL, FALSE);
	g_return_val_if_fail(GPLUGIN_IS_LOADER(loader), FALSE);
	g_return_val_if_fail(GPLUGIN_IS_PLUGIN(plugin), FALSE);

	/* if the plugin is already loaded there's nothing for us to do */
	if(gplugin_plugin_get_state(plugin) == GPLUGIN_PLUGIN_STATE_LOADED) {
		return TRUE;
	}

	gplugin_plugin_set_desired_state(plugin, GPLUGIN_PLUGIN_STATE_LOADED);

	klass = GPLUGIN_LOADER_GET_CLASS(loader);

	if(klass != NULL && klass->load != NULL) {
		ret = klass->load(loader, plugin, &real_error);
	}

	if(!ret) {
		if(real_error == NULL) {
			real_error = g_error_new_literal(
				GPLUGIN_DOMAIN,
				0,
				"Failed to load plugin : unknown");
		}

		/* Set the error on the plugin as well.  This  has to be before we
		 * propagate the error, because error is invalidate at that point.
		 */
		g_object_set(plugin, "error", real_error, NULL);

		/* Set the state after the error is set, because people might connect
		 * to the notify signal on the state property.
		 */
		gplugin_plugin_set_state(plugin, GPLUGIN_PLUGIN_STATE_LOAD_FAILED);

		g_propagate_error(error, real_error);
	} else {
		/* If the plugin successfully loaded but returned an error, ignore the
		 * error.
		 */
		g_clear_error(&real_error);

		/* Likewise, make sure the plugin's error is set to NULL. */
		g_object_set(G_OBJECT(plugin), "error", NULL, NULL);

		gplugin_plugin_set_state(plugin, GPLUGIN_PLUGIN_STATE_LOADED);
	}

	return ret;
}

/**
 * gplugin_loader_unload_plugin:
 * @loader: The loader instance performing the unload.
 * @plugin: The plugin instance to unload.
 * @shutdown: Whether or not GPlugin is shutting down.
 * @error: (nullable): The return location for a [struct@GLib.Error], or %NULL.
 *
 * This function is called by the plugin manager to ask @loader to unload
 * @plugin.
 *
 * Returns: %TRUE if @plugin was unloaded successfully, %FALSE otherwise.
 */
gboolean
gplugin_loader_unload_plugin(
	GPluginLoader *loader,
	GPluginPlugin *plugin,
	gboolean shutdown,
	GError **error)
{
	GPluginLoaderClass *klass = NULL;
	GError *real_error = NULL;
	gboolean ret = FALSE;

	g_return_val_if_fail(loader != NULL, FALSE);
	g_return_val_if_fail(GPLUGIN_IS_LOADER(loader), FALSE);
	g_return_val_if_fail(GPLUGIN_IS_PLUGIN(plugin), FALSE);

	if(gplugin_plugin_get_state(plugin) != GPLUGIN_PLUGIN_STATE_LOADED) {
		return TRUE;
	}

	gplugin_plugin_set_desired_state(plugin, GPLUGIN_PLUGIN_STATE_QUERIED);

	klass = GPLUGIN_LOADER_GET_CLASS(loader);

	if(klass != NULL && klass->unload != NULL) {
		ret = klass->unload(loader, plugin, shutdown, &real_error);
	}

	if(!ret) {
		if(real_error == NULL) {
			real_error = g_error_new_literal(
				GPLUGIN_DOMAIN,
				0,
				"Failed to unload plugin : unknown");
		}

		g_object_set(G_OBJECT(plugin), "error", real_error, NULL);

		/* Set the state after the error is set, because people might connect
		 * to the notify signal on the state property.
		 */
		gplugin_plugin_set_state(plugin, GPLUGIN_PLUGIN_STATE_UNLOAD_FAILED);

		g_propagate_error(error, real_error);
	} else {
		/* If the plugin successfully unloaded but returned an error, ignore the
		 * error.
		 */
		g_clear_error(error);

		gplugin_plugin_set_state(plugin, GPLUGIN_PLUGIN_STATE_QUERIED);
	}

	return ret;
}

/**
 * gplugin_loader_get_supported_extensions:
 * @loader: The loader instance.
 *
 * Returns a [struct@GLib.SList] of strings containing the extensions that the
 * loader supports.  Each extension should not include the dot.  For example:
 * so, dll, py, etc.
 *
 * Returns: (element-type utf8) (transfer container): A [struct@GLib.SList] of
 *          extensions that the loader supports.
 */
GSList *
gplugin_loader_get_supported_extensions(GPluginLoader *loader)
{
	GPluginLoaderClass *klass = NULL;

	g_return_val_if_fail(GPLUGIN_IS_LOADER(loader), NULL);

	klass = GPLUGIN_LOADER_GET_CLASS(loader);
	if(klass != NULL && klass->supported_extensions) {
		return klass->supported_extensions(loader);
	}

	return NULL;
}
