/*
 * Copyright (C) 2011-2021 Gary Kramlich <grim@reaperworld.com>
 * Copyright (C) 2013 Ankit Vani <a@nevitus.org>
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

#include "dynamic-test.h"

static DynamicTest *test_object = NULL;

static GPluginPluginInfo *
dynamic_user_query(G_GNUC_UNUSED GError **error)
{
	const gchar *const dependencies[] = {"gplugin/dynamic-type-provider", NULL};

	/* clang-format off */
	return gplugin_plugin_info_new(
		"gplugin/dynamic-type-user",
		GPLUGIN_NATIVE_PLUGIN_ABI_VERSION,
		"dependencies", dependencies,
		NULL);
	/* clang-format on */
}

static gboolean
dynamic_user_load(G_GNUC_UNUSED GPluginPlugin *plugin, GError **error)
{
	test_object = g_object_new(DYNAMIC_TYPE_TEST, NULL);

	if(test_object == NULL) {
		g_set_error(
			error,
			GPLUGIN_DOMAIN,
			0,
			"could not create an instance of DynamicTest");
		return FALSE;
	}

	return TRUE;
}

static gboolean
dynamic_user_unload(
	G_GNUC_UNUSED GPluginPlugin *plugin,
	G_GNUC_UNUSED gboolean shutdown,
	GError **error)
{
	gpointer weak_test_object = test_object;
	g_object_add_weak_pointer(G_OBJECT(test_object), &weak_test_object);

	g_object_unref(test_object);

	if(DYNAMIC_IS_TEST(weak_test_object)) {
		g_set_error(error, GPLUGIN_DOMAIN, 0, "test_object is still valid");
		return FALSE;
	}

	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(dynamic_user)
