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

#include <gplugin.h>
#include <gplugin-native.h>

#include "gplugin-python3-loader.h"
#include "gplugin-python3-plugin.h"

static GPluginLoader *loader = NULL;

static GPluginPluginInfo *
gplugin_python3_query(G_GNUC_UNUSED GError **error)
{
	/* clang-format-11 formats this correctly, so this can be removed then. */
	/* clang-format off */
	const gchar *const authors[] = {
		"Gary Kramlich <grim@reaperworld.com>",
		NULL};
	/* clang-format on */

	/* clang-format off */
	return gplugin_plugin_info_new(
		"gplugin/python3-loader",
		GPLUGIN_NATIVE_PLUGIN_ABI_VERSION,
		"internal", TRUE,
		"auto-load", TRUE,
		"name", "Python3 Plugin Loader",
		"version", GPLUGIN_VERSION,
		"license-id", "LGPL-2.0-or-later",
		"summary", "A plugin that can load python plugins",
		"description", "This plugin allows the loading of plugins written in "
		               "the Python3 programming language.",
		"authors", authors,
		"website", GPLUGIN_WEBSITE,
		"category", "loaders",
		"bind-global", TRUE,
		"unloadable", FALSE,
		NULL);
	/* clang-format on */
}

static gboolean
gplugin_python3_load(GPluginPlugin *plugin, GError **error)
{
	GPluginManager *manager = NULL;

	manager = gplugin_manager_get_default();

	gplugin_python3_plugin_register(G_TYPE_MODULE(plugin));
	gplugin_python3_loader_register(G_TYPE_MODULE(plugin));

	loader = gplugin_python3_loader_new();

	if(!gplugin_manager_register_loader(manager, loader, error)) {
		g_clear_object(&loader);

		return FALSE;
	}

	return TRUE;
}

static gboolean
gplugin_python3_unload(
	G_GNUC_UNUSED GPluginPlugin *plugin,
	gboolean shutdown,
	GError **error)
{
	GPluginManager *manager = NULL;
	gboolean ret = FALSE;

	if(!shutdown) {
		g_set_error_literal(
			error,
			GPLUGIN_DOMAIN,
			0,
			_("The Python3 loader can not be unloaded"));

		return FALSE;
	}

	manager = gplugin_manager_get_default();
	ret = gplugin_manager_unregister_loader(manager, loader, error);
	g_clear_object(&loader);

	return ret;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(gplugin_python3)
