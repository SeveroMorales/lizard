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

#include <glib/gi18n-lib.h>

#include <gplugin/gplugin-core.h>
#include <gplugin/gplugin-loader.h>
#include <gplugin/gplugin-manager.h>
#include <gplugin/gplugin-native-plugin.h>

/**
 * GPluginNativePlugin:
 *
 * An instance of a loaded native plugin.  A native plugin is a plugin that was
 * compiled to machine native code, typically these are written in C/C++.
 */

/**
 * GPluginNativePluginQueryFunc:
 * @error: A return address for a #GError.
 *
 * Specifies the function signature for the query function of a plugin.
 *
 * Returns: (transfer full): A #GPluginPluginInfo instance on success or %NULL
 *          with @error set on error.
 *
 * Since: 0.31.0
 */

/**
 * GPluginNativePluginLoadFunc:
 * @plugin: The #GPluginPlugin instance.
 * @error: A return address for a #GError.
 *
 * Specifies the function signature for the load function of a plugin.
 *
 * Returns: %TRUE if @plugin was successfully loaded, or %FALSE with @error
 *          set on failure.
 *
 * Since: 0.31.0
 */

/**
 * GPluginNativePluginUnloadFunc:
 * @plugin: The #GPluginPlugin instance.
 * @error: A return address for a #GError.
 *
 * Specifies the function signature for the unload function of a plugin.
 *
 * Returns: %TRUE if @plugin was successfully unloaded, or %FALSE with @error
 *          set on failure.
 *
 * Since: 0.31.0
 */

/* Apparently clang-format also tries to format code in comments.. But in this
 * case it makes it really difficult to understand so we disable it.
 */

/* clang-format off */

/**
 * GPLUGIN_NATIVE_PLUGIN_DECLARE:
 * @name: The prefix of the user defined function names.
 *
 * This macro expands to the proper functions that #GPluginNativeLoader looks
 * for when querying a plugin. They will call user defined functions named
 * <function>${name}_query</function>, <function>${name}_load</function>,
 * and <function>${name}_unload</function> which need to match the signatures
 * of #GPluginNativePluginQueryFunc, #GPluginNativePluginLoadFunc, and
 * #GPluginNativePluginUnloadFunc.
 *
 * This macro should be used over manually exporting the individual functions
 * to help with updates as well as future features like static plugin loading.
 *
 * <informalexample><programlisting>
 * GPLUGIN_NATIVE_PLUGIN_DECLARE(my_awesome_plugin)
 * </programlisting></informalexample>
 *
 * will expand to
 *
 * <informalexample><programlisting>
 * G_MODULE_EXPORT GPluginPluginInfo *gplugin_query(GError **error);
 * G_MODULE_EXPORT GPluginPluginInfo *gplugin_query(GError **error) {
 *         return my_awesome_plugin_query(error);
 * }
 *
 * G_MODULE_EXPORT gboolean gplugin_load(GPluginPlugin *plugin, GError **error);
 * G_MODULE_EXPORT gboolean gplugin_load(GPluginPlugin *plugin, GError **error) {
 *         return my_awesome_plugin_load(error);
 * }
 *
 * G_MODULE_EXPORT gboolean gplugin_unload(GPluginPlugin *plugin, GError **error);
 * G_MODULE_EXPORT gboolean gplugin_unload(GPluginPlugin *plugin, GError **error) {
 *         return my_awesome_plugin_unload(plugin, error);
 * }
 * </programlisting></informalexample>
 *
 * Since: 0.31.0
 */

/* clang-format on */

/******************************************************************************
 * Structs
 *****************************************************************************/
struct _GPluginNativePlugin {
	GTypeModule parent;

	GModule *module;

	gpointer load_func;
	gpointer unload_func;

	gchar *filename;
	GPluginLoader *loader;
	GPluginPluginInfo *info;
	GPluginPluginState state;
	GPluginPluginState desired_state;
	GError *error;
};

/******************************************************************************
 * Enums
 *****************************************************************************/
enum {
	PROP_ZERO,
	PROP_MODULE,
	PROP_LOAD_FUNC,
	PROP_UNLOAD_FUNC,
	N_PROPERTIES,
	/* overrides */
	PROP_FILENAME = N_PROPERTIES,
	PROP_LOADER,
	PROP_INFO,
	PROP_STATE,
	PROP_DESIRED_STATE,
	PROP_ERROR,
};
static GParamSpec *properties[N_PROPERTIES] = {
	NULL,
};

/******************************************************************************
 * GPluginPlugin Implementation
 *****************************************************************************/
static void
gplugin_native_plugin_iface_init(G_GNUC_UNUSED GPluginPluginInterface *iface)
{
	/* we just override properites from GPluginPlugin */
}

/******************************************************************************
 * GTypeModule Implementation
 *****************************************************************************/
static gboolean
gplugin_native_plugin_load(G_GNUC_UNUSED GTypeModule *module)
{
	return TRUE;
}

static void
gplugin_native_plugin_unload(G_GNUC_UNUSED GTypeModule *module)
{
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE_WITH_CODE(
	GPluginNativePlugin,
	gplugin_native_plugin,
	G_TYPE_TYPE_MODULE,
	G_IMPLEMENT_INTERFACE(
		GPLUGIN_TYPE_PLUGIN,
		gplugin_native_plugin_iface_init))

static void
gplugin_native_plugin_get_property(
	GObject *obj,
	guint param_id,
	GValue *value,
	GParamSpec *pspec)
{
	GPluginNativePlugin *plugin = GPLUGIN_NATIVE_PLUGIN(obj);

	switch(param_id) {
		case PROP_MODULE:
			g_value_set_pointer(
				value,
				gplugin_native_plugin_get_module(plugin));
			break;
		case PROP_LOAD_FUNC:
			g_value_set_pointer(value, plugin->load_func);
			break;
		case PROP_UNLOAD_FUNC:
			g_value_set_pointer(value, plugin->unload_func);
			break;

		/* overrides */
		case PROP_FILENAME:
			g_value_set_string(value, plugin->filename);
			break;
		case PROP_LOADER:
			g_value_set_object(value, plugin->loader);
			break;
		case PROP_INFO:
			g_value_set_object(value, plugin->info);
			break;
		case PROP_STATE:
			g_value_set_enum(value, plugin->state);
			break;
		case PROP_DESIRED_STATE:
			g_value_set_enum(value, plugin->desired_state);
			break;
		case PROP_ERROR:
			g_value_set_boxed(value, plugin->error);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
gplugin_native_plugin_set_property(
	GObject *obj,
	guint param_id,
	const GValue *value,
	GParamSpec *pspec)
{
	GPluginNativePlugin *plugin = GPLUGIN_NATIVE_PLUGIN(obj);

	switch(param_id) {
		case PROP_MODULE:
			plugin->module = g_value_get_pointer(value);
			break;
		case PROP_LOAD_FUNC:
			plugin->load_func = g_value_get_pointer(value);
			break;
		case PROP_UNLOAD_FUNC:
			plugin->unload_func = g_value_get_pointer(value);
			break;

		/* overrides */
		case PROP_FILENAME:
			plugin->filename = g_value_dup_string(value);
			break;
		case PROP_LOADER:
			plugin->loader = g_value_dup_object(value);
			break;
		case PROP_INFO:
			plugin->info = g_value_dup_object(value);
			break;
		case PROP_STATE:
			plugin->state = g_value_get_enum(value);
			break;
		case PROP_DESIRED_STATE:
			plugin->desired_state = g_value_get_enum(value);
			break;
		case PROP_ERROR:
			g_clear_error(&plugin->error);
			plugin->error = g_value_dup_boxed(value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
gplugin_native_plugin_finalize(GObject *obj)
{
	GPluginNativePlugin *plugin = GPLUGIN_NATIVE_PLUGIN(obj);

	g_clear_pointer(&plugin->filename, g_free);
	g_clear_object(&plugin->loader);
	g_clear_object(&plugin->info);
	g_clear_error(&plugin->error);

	g_module_close(plugin->module);

	G_OBJECT_CLASS(gplugin_native_plugin_parent_class)->finalize(obj);
}

static void
gplugin_native_plugin_init(G_GNUC_UNUSED GPluginNativePlugin *plugin)
{
}

static void
gplugin_native_plugin_class_init(GPluginNativePluginClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GTypeModuleClass *module_class = G_TYPE_MODULE_CLASS(klass);

	obj_class->finalize = gplugin_native_plugin_finalize;
	obj_class->get_property = gplugin_native_plugin_get_property;
	obj_class->set_property = gplugin_native_plugin_set_property;

	module_class->load = gplugin_native_plugin_load;
	module_class->unload = gplugin_native_plugin_unload;

	/**
	 * GPluginNativePlugin:module:
	 *
	 * The GModule instance for this plugin.
	 */
	properties[PROP_MODULE] = g_param_spec_pointer(
		"module",
		"module handle",
		"The GModule instance of the plugin",
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * GPluginNativePlugin:load-func:
	 *
	 * A function pointer to the load method of the plugin.
	 */
	properties[PROP_LOAD_FUNC] = g_param_spec_pointer(
		"load-func",
		"load function pointer",
		"address pointer to load function",
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * GPluginNativePlugin:unload-func:
	 *
	 * A function pointer to the unload method of the plugin.
	 */
	properties[PROP_UNLOAD_FUNC] = g_param_spec_pointer(
		"unload-func",
		"unload function pointer",
		"address pointer to the unload function",
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	/* add our overrides */
	g_object_class_override_property(obj_class, PROP_FILENAME, "filename");
	g_object_class_override_property(obj_class, PROP_LOADER, "loader");
	g_object_class_override_property(obj_class, PROP_INFO, "info");
	g_object_class_override_property(obj_class, PROP_STATE, "state");
	g_object_class_override_property(
		obj_class,
		PROP_DESIRED_STATE,
		"desired-state");
	g_object_class_override_property(obj_class, PROP_ERROR, "error");
}

/******************************************************************************
 * API
 *****************************************************************************/

/**
 * gplugin_native_plugin_get_module: (skip)
 * @plugin: #GPluginNativePlugin instance
 *
 * Returns the %GModule associated with this plugin.  This should really only
 * be used if you need to make your plugin resident.
 *
 * Returns: The %GModule associated with this plugin.
 */
GModule *
gplugin_native_plugin_get_module(GPluginNativePlugin *plugin)
{
	g_return_val_if_fail(GPLUGIN_IS_NATIVE_PLUGIN(plugin), NULL);

	return plugin->module;
}
