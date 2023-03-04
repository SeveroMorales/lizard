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

#include <gmodule.h>
#include <glib/gi18n-lib.h>

#include <gplugin/gplugin-core.h>
#include <gplugin/gplugin-native-loader.h>
#include <gplugin/gplugin-native-plugin.h>
#include <gplugin/gplugin-private.h>

#define GPLUGIN_QUERY_SYMBOL "gplugin_query"
#define GPLUGIN_LOAD_SYMBOL "gplugin_load"
#define GPLUGIN_UNLOAD_SYMBOL "gplugin_unload"

/**
 * GPluginNativeLoader:
 *
 * A #GPluginLoader subclass that is able to load native plugins.
 */

/**
 * GPLUGIN_NATIVE_PLUGIN_ABI_VERSION:
 *
 * The ABI version of the #GPluginNativeLoader.  Your plugin should use this
 * as the value for #abi_version when call #gplugin_plugin_info_new.
 */

struct _GPluginNativeLoader {
	GPluginLoader parent;
};

G_DEFINE_TYPE(GPluginNativeLoader, gplugin_native_loader, GPLUGIN_TYPE_LOADER)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static gpointer
gplugin_native_loader_lookup_symbol(
	GModule *module,
	const gchar *name,
	GError **error)
{
	gpointer symbol = NULL;

	g_return_val_if_fail(module != NULL, NULL);

	if(!g_module_symbol(module, name, &symbol)) {
		g_set_error(
			error,
			GPLUGIN_DOMAIN,
			0,
			_("symbol %s was not found"),
			name);

		return NULL;
	}

	return symbol;
}

static GModule *
gplugin_native_loader_open(
	const gchar *filename,
	GModuleFlags flags,
	GError **error)
{
	GModule *module = NULL;

	module = g_module_open(filename, flags);
	if(module)
		return module;

	if(error) {
		const gchar *msg = g_module_error();

		*error = g_error_new(
			GPLUGIN_DOMAIN,
			0,
			_("Failed to open plugin '%s': %s"),
			filename,
			(msg) ? msg : _("Unknown error"));
	}

	return NULL;
}

/******************************************************************************
 * GPluginLoaderInterface API
 *****************************************************************************/
static GSList *
gplugin_native_loader_supported_extensions(G_GNUC_UNUSED GPluginLoader *l)
{
	GSList *exts = g_slist_append(NULL, G_MODULE_SUFFIX);

#if defined(__APPLE__) || defined(__MACH__)
	/* G_MODULE_SUFFIX only requests `.so` on not windows, and both .so and
	 * .dylib are used on macos, so add dylib if we're on macos.
	 * See: https://gitlab.gnome.org/GNOME/glib/issues/1413
	 */
	exts = g_slist_append(exts, "dylib");
#endif

	return exts;
}

static GPluginPluginInfo *
gplugin_native_loader_open_and_query(
	const gchar *filename,
	GModule **module,
	GModuleFlags flags,
	GPluginNativePluginQueryFunc *query,
	GError **error)
{
	GPluginPluginInfo *info = NULL;

	*module = gplugin_native_loader_open(filename, flags, error);
	if(*module == NULL) {
		return NULL;
	} else if(error && *error) {
		g_module_close(*module);
		*module = NULL;
		return NULL;
	}

	*query = gplugin_native_loader_lookup_symbol(
		*module,
		GPLUGIN_QUERY_SYMBOL,
		error);

	if((*query == NULL) || (error && *error)) {
		g_module_close(*module);
		*module = NULL;
		return NULL;
	}

	info = ((GPluginNativePluginQueryFunc)(*query))(error);
	if(error && *error) {
		g_module_close(*module);
		*module = NULL;
		return NULL;
	}

	return info;
}

static GPluginPlugin *
gplugin_native_loader_query(
	GPluginLoader *loader,
	const gchar *filename,
	GError **error)
{
	GPluginPlugin *plugin = NULL;
	GPluginPluginInfo *info = NULL;
	GPluginNativePluginLoadFunc load = NULL;
	GPluginNativePluginQueryFunc query = NULL;
	GPluginNativePluginUnloadFunc unload = NULL;
	GModule *module = NULL;

	info = gplugin_native_loader_open_and_query(
		filename,
		&module,
		G_MODULE_BIND_LOCAL,
		&query,
		error);

	/* If the query returned an error, clear any info it may have set and return
	 * NULL.
	 */
	if(error != NULL && *error != NULL) {
		g_clear_object(&info);

		return NULL;
	}

	/* If we didn't get an info back, create a generic error. */
	if(!GPLUGIN_IS_PLUGIN_INFO(info)) {
		/* Error was already checked earlier if it was non-null. */
		g_set_error_literal(
			error,
			GPLUGIN_DOMAIN,
			0,
			_("the query function did not return a "
			  "GPluginPluginInfo instance"));

		g_clear_object(&info);

		return NULL;
	}

	if(gplugin_plugin_info_get_bind_global(info)) {
		g_module_close(module);
		g_object_unref(G_OBJECT(info));

		info = gplugin_native_loader_open_and_query(
			filename,
			&module,
			0,
			&query,
			error);
		if(!GPLUGIN_IS_PLUGIN_INFO(info)) {
			if(error && *error == NULL) {
				g_set_error_literal(
					error,
					GPLUGIN_DOMAIN,
					0,
					_("the query function did not return a "
					  "GPluginPluginInfo instance"));
			}

			return NULL;
		}
	}

	/* now look for the load symbol */
	load =
		gplugin_native_loader_lookup_symbol(module, GPLUGIN_LOAD_SYMBOL, error);
	if(error && *error) {
		g_module_close(module);
		g_object_unref(G_OBJECT(info));
		return NULL;
	}

	/* now look for the unload symbol */
	unload = gplugin_native_loader_lookup_symbol(
		module,
		GPLUGIN_UNLOAD_SYMBOL,
		error);
	if(error && *error) {
		g_module_close(module);
		g_object_unref(G_OBJECT(info));
		return NULL;
	}

	/* now create the actual plugin instance */
	/* clang-format off */
	plugin = g_object_new(
		GPLUGIN_TYPE_NATIVE_PLUGIN,
		"module", module,
		"info", info,
		"load-func", load,
		"unload-func", unload,
		"loader", loader,
		"filename", filename,
		NULL);
	/* clang-format on */

	/* now that the plugin instance owns the info, remove our ref */
	g_object_unref(G_OBJECT(info));

	if(!GPLUGIN_IS_NATIVE_PLUGIN(plugin)) {
		g_module_close(module);
		g_set_error_literal(
			error,
			GPLUGIN_DOMAIN,
			0,
			_("failed to create plugin instance"));
		return NULL;
	}

	return plugin;
}

static gboolean
gplugin_native_loader_load(
	G_GNUC_UNUSED GPluginLoader *loader,
	GPluginPlugin *plugin,
	GError **error)
{
	GPluginNativePluginLoadFunc func;

	g_return_val_if_fail(plugin != NULL, FALSE);
	g_return_val_if_fail(GPLUGIN_IS_NATIVE_PLUGIN(plugin), FALSE);

	/* get and call the function */
	g_object_get(G_OBJECT(plugin), "load-func", &func, NULL);
	if(!func(plugin, error)) {
		if(error && *error == NULL)
			g_set_error_literal(error, GPLUGIN_DOMAIN, 0, _("unknown failure"));

		return FALSE;
	}

	return TRUE;
}

static gboolean
gplugin_native_loader_unload(
	G_GNUC_UNUSED GPluginLoader *loader,
	GPluginPlugin *plugin,
	gboolean shutdown,
	GError **error)
{
	GPluginNativePluginUnloadFunc func;

	g_return_val_if_fail(plugin != NULL, FALSE);
	g_return_val_if_fail(GPLUGIN_IS_NATIVE_PLUGIN(plugin), FALSE);

	/* get the function */
	g_object_get(G_OBJECT(plugin), "unload-func", &func, NULL);

	/* validate the function */
	if(func == NULL) {
		gchar *filename = gplugin_plugin_get_filename(plugin);

		g_warning(_("unload function for %s is NULL"), filename);

		g_free(filename);

		return FALSE;
	}

	/* now call the function */
	if(!func(plugin, shutdown, error)) {
		if(error && *error == NULL)
			g_set_error_literal(error, GPLUGIN_DOMAIN, 0, _("unknown failure"));

		return FALSE;
	}

	return TRUE;
}

static void
gplugin_native_loader_init(G_GNUC_UNUSED GPluginNativeLoader *loader)
{
}

static void
gplugin_native_loader_class_init(GPluginNativeLoaderClass *klass)
{
	GPluginLoaderClass *loader_class = GPLUGIN_LOADER_CLASS(klass);

	loader_class->supported_extensions =
		gplugin_native_loader_supported_extensions;
	loader_class->query = gplugin_native_loader_query;
	loader_class->load = gplugin_native_loader_load;
	loader_class->unload = gplugin_native_loader_unload;
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * gplugin_native_loader_new:
 *
 * Create a new instance of the native plugin loader.
 *
 * Typically you won't need to call this directly as GPlugin will create it
 * unless #GPLUGIN_CORE_FLAGS_DISABLE_NATIVE_LOADER is passed to
 * gplugin_init().
 *
 * Returns: (transfer full): The new instance.
 *
 * Since: 0.34.0
 */
GPluginLoader *
gplugin_native_loader_new(void)
{
	/* clang-format off */
	return g_object_new(
		GPLUGIN_TYPE_NATIVE_LOADER,
		"id", "gplugin-native",
		NULL);
	/* clang-format on */
}
