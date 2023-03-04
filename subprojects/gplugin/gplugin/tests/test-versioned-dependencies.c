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

#include <glib.h>

#include <gplugin.h>

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
_test_plugin_loaded(G_GNUC_UNUSED GPluginPlugin *dependent, const gchar *id)
{
	GPluginManager *manager = gplugin_manager_get_default();
	GPluginPlugin *plugin = NULL;

	plugin = gplugin_manager_find_plugin(manager, id);
	g_assert_cmpint(
		gplugin_plugin_get_state(plugin),
		==,
		GPLUGIN_PLUGIN_STATE_LOADED);

	g_object_unref(G_OBJECT(plugin));
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_load_with_dependencies(void)
{
	GPluginManager *manager = NULL;
	GPluginPlugin *plugin = NULL;
	GError *error = NULL;
	gboolean ret = FALSE;

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);

	manager = gplugin_manager_get_default();

	gplugin_manager_append_path(manager, TEST_VERSIONED_DEPENDENCY_DIR);
	gplugin_manager_refresh(manager);

	plugin = gplugin_manager_find_plugin(manager, "gplugin/super-dependent");
	g_assert_nonnull(plugin);
	g_assert_true(GPLUGIN_IS_PLUGIN(plugin));

	ret = gplugin_manager_load_plugin(manager, plugin, &error);
	g_assert_no_error(error);
	g_assert_true(ret);

	g_assert_cmpint(
		gplugin_plugin_get_state(plugin),
		==,
		GPLUGIN_PLUGIN_STATE_LOADED);

	/* now make sure each dependent plugin that's available was loaded */
	_test_plugin_loaded(plugin, "gplugin/test-no-version");
	_test_plugin_loaded(plugin, "gplugin/test-exact1");
	_test_plugin_loaded(plugin, "gplugin/test-exact2");
	_test_plugin_loaded(plugin, "gplugin/test-greater");
	_test_plugin_loaded(plugin, "gplugin/test-greater-equal");
	_test_plugin_loaded(plugin, "gplugin/test-less");
	_test_plugin_loaded(plugin, "gplugin/test-less-equal");
	_test_plugin_loaded(plugin, "gplugin/bar");
	_test_plugin_loaded(plugin, "gplugin/baz");
	_test_plugin_loaded(plugin, "gplugin/fez");

	g_object_unref(G_OBJECT(plugin));

	gplugin_uninit();
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv)
{
	g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

	/* test the load on query flag */
	g_test_add_func(
		"/dependent-versions/super-dependent",
		test_load_with_dependencies);

	return g_test_run();
}
