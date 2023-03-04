/*
 * Copyright (C) 2011-2014 Gary Kramlich <grim@reaperworld.com>
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

#include <stdlib.h>

#include <glib.h>

#include <gplugin.h>
#include <gplugin-native.h>
#include <gplugin/gplugin-loader-tests.h>

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_dynamic_type(void)
{
	GPluginManager *manager = gplugin_manager_get_default();
	GPluginPlugin *provider = NULL, *user = NULL;
	GPluginPluginState state;
	GError *error = NULL;
	gboolean ret = FALSE;

	gplugin_manager_append_path(manager, TEST_DYNAMIC_DIR);
	gplugin_manager_refresh(manager);

	provider =
		gplugin_manager_find_plugin(manager, "gplugin/dynamic-type-provider");

	g_assert_nonnull(provider);
	ret = gplugin_manager_load_plugin(manager, provider, &error);
	g_assert_no_error(error);
	g_assert_true(ret);

	state = gplugin_plugin_get_state(provider);
	g_assert_cmpint(state, ==, GPLUGIN_PLUGIN_STATE_LOADED);

	user = gplugin_manager_find_plugin(manager, "gplugin/dynamic-type-user");

	g_assert_nonnull(user);
	ret = gplugin_manager_load_plugin(manager, user, &error);
	g_assert_no_error(error);
	g_assert_true(ret);

	state = gplugin_plugin_get_state(user);
	g_assert_cmpint(state, ==, GPLUGIN_PLUGIN_STATE_LOADED);

	/* now unload the plugin */
	ret = gplugin_manager_unload_plugin(manager, user, &error);
	g_assert_no_error(error);
	g_assert_true(ret);

	state = gplugin_plugin_get_state(user);
	g_assert_cmpint(state, ==, GPLUGIN_PLUGIN_STATE_QUERIED);

	ret = gplugin_manager_unload_plugin(manager, provider, &error);
	g_assert_no_error(error);
	g_assert_true(ret);

	state = gplugin_plugin_get_state(provider);
	g_assert_cmpint(state, ==, GPLUGIN_PLUGIN_STATE_QUERIED);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv)
{

	g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);

	/* test dynamic types */
	g_test_add_func("/loaders/native/dynamic-type", test_dynamic_type);

	return g_test_run();
}
