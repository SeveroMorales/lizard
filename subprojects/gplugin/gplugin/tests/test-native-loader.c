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
test_broken_depend_plugin_load(void)
{
	GPluginManager *manager = gplugin_manager_get_default();
	GPluginPlugin *plugin = NULL;
	GPluginPluginState state;
	GError *error = NULL;

	/* add the test directory to the plugin manager's search paths */
	gplugin_manager_append_path(manager, TEST_DIR);

	/* refresh the plugin manager */
	gplugin_manager_refresh(manager);

	/* find the dependent plugin and make sure it isn't loaded */
	plugin = gplugin_manager_find_plugin(
		manager,
		"gplugin/broken-dependent-native-plugin");
	g_assert_nonnull(plugin);

	state = gplugin_plugin_get_state(plugin);
	g_assert_cmpint(state, !=, GPLUGIN_PLUGIN_STATE_LOADED);

	/* now attempt to load the dependent plugin, it's supposed to fail */
	g_assert_false(gplugin_manager_load_plugin(manager, plugin, &error));
	g_assert_error(error, GPLUGIN_DOMAIN, 0);
	g_error_free(error);
}

/******************************************************************************
 * Test bad plugins
 *****************************************************************************/
static void
test_query_error_subprocess(void)
{
	GPluginManager *manager = gplugin_manager_get_default();
	GPluginPlugin *plugin = NULL;

	/* add the test directory to the plugin manager's search paths */
	gplugin_manager_append_path(manager, TEST_BAD_DIR);

	/* refresh the plugin manager */
	gplugin_manager_refresh(manager);

	/* find the query-error plugin */
	plugin = gplugin_manager_find_plugin(manager, "gplugin/query-error");
	g_assert_null(plugin);
}

static void
test_query_error(void)
{
	g_test_trap_subprocess("/loaders/native/error/query/subprocess", 0, 0);

	g_test_trap_assert_stderr("*expected error*");
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv)
{

	g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);

	gplugin_loader_tests_main(NULL, TEST_DIR, "native");

	g_test_add_func(
		"/loaders/native/load/broken_dependent",
		test_broken_depend_plugin_load);

	/* bad plugin tests */
	g_test_add_func("/loaders/native/error/query", test_query_error);
	g_test_add_func(
		"/loaders/native/error/query/subprocess",
		test_query_error_subprocess);

	return g_test_run();
}
