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
no_version_query(G_GNUC_UNUSED GError **error)
{
	return gplugin_plugin_info_new("gplugin/test-no-version", 0x04030201, NULL);
}

static gboolean
no_version_load(
	G_GNUC_UNUSED GPluginPlugin *plugin,
	G_GNUC_UNUSED GError **error)
{
	return TRUE;
}

static gboolean
no_version_unload(
	G_GNUC_UNUSED GPluginPlugin *plugin,
	G_GNUC_UNUSED gboolean shutdown,
	G_GNUC_UNUSED GError **error)
{
	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(no_version)
