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
super_dependent_query(G_GNUC_UNUSED GError **error)
{
	/* clang-format-11 formats this correctly, so this can be removed then. */
	/* clang-format off */
	const gchar *const dependencies[] = {
		"gplugin/test-no-version",
		"gplugin/test-exact1==1.0",
		"gplugin/test-exact2=1.0",
		"gplugin/test-greater>1.9",
		"gplugin/test-greater-equal>=2.0",
		"gplugin/test-less<1.0",
		"gplugin/test-less-equal<=1.0",
		"gplugin/foo|gplugin/bar",
		"gplugin/foo|gplugin/baz>=2.0",
		"gplugin/foo|gplugin/fez<1.9",
		NULL};
	/* clang-format on */

	/* clang-format off */
	return gplugin_plugin_info_new(
		"gplugin/super-dependent",
		0x04030201,
		"name", "super dependent",
		"dependencies", dependencies,
		NULL);
	/* clang-format on */
}

static gboolean
super_dependent_load(
	G_GNUC_UNUSED GPluginPlugin *plugin,
	G_GNUC_UNUSED GError **error)
{
	return TRUE;
}

static gboolean
super_dependent_unload(
	G_GNUC_UNUSED GPluginPlugin *plugin,
	G_GNUC_UNUSED gboolean shutdown,
	G_GNUC_UNUSED GError **error)
{
	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(super_dependent)
