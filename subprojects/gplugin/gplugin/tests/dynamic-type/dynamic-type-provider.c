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

G_DEFINE_DYNAMIC_TYPE(DynamicTest, dynamic_test, G_TYPE_OBJECT)

static void
dynamic_test_init(G_GNUC_UNUSED DynamicTest *inst)
{
}

static void
dynamic_test_class_finalize(G_GNUC_UNUSED DynamicTestClass *klass)
{
}

static void
dynamic_test_class_init(G_GNUC_UNUSED DynamicTestClass *klass)
{
}

static GPluginPluginInfo *
dynamic_provider_query(G_GNUC_UNUSED GError **error)
{
	return gplugin_plugin_info_new(
		"gplugin/dynamic-type-provider",
		GPLUGIN_NATIVE_PLUGIN_ABI_VERSION,
		"bind-global",
		TRUE,
		NULL);
}

static gboolean
dynamic_provider_load(GPluginPlugin *plugin, G_GNUC_UNUSED GError **error)
{
	dynamic_test_register_type(G_TYPE_MODULE(plugin));

	return TRUE;
}

static gboolean
dynamic_provider_unload(
	G_GNUC_UNUSED GPluginPlugin *plugin,
	G_GNUC_UNUSED gboolean shutdown,
	G_GNUC_UNUSED GError **error)
{
	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(dynamic_provider)
