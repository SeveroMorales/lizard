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

#include <gplugin.h>
#include <gplugin-native.h>

static GPluginPluginInfo *
basic_query(G_GNUC_UNUSED GError **error)
{
	const gchar *const authors[] = {"author1", NULL};

	/* clang-format off */
	return gplugin_plugin_info_new(
		"gplugin/native-basic-plugin",
		0x01020304,
		"name", "basic plugin (C)",
		"category", "test",
		"version", "version",
		"summary", "summary",
		"license-id", "license",
		"description", "description",
		"authors", authors,
		"website", "website",
		NULL);
	/* clang-format on */
}

static gboolean
basic_load(GPluginPlugin *plugin, GError **error)
{
	if(!GPLUGIN_IS_PLUGIN(plugin)) {
		g_set_error_literal(error, GPLUGIN_DOMAIN, 0, "plugin was not set");

		return FALSE;
	}

	return TRUE;
}

static gboolean
basic_unload(
	GPluginPlugin *plugin,
	G_GNUC_UNUSED gboolean shutdown,
	GError **error)
{
	if(!GPLUGIN_IS_PLUGIN(plugin)) {
		g_set_error_literal(error, GPLUGIN_DOMAIN, 0, "plugin was not set");

		return FALSE;
	}

	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(basic)
