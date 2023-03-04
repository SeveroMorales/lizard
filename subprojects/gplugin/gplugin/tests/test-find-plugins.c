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

#include <glib.h>

#include <gplugin.h>

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
test_gplugin_manager_foreach_load_plugins(
	G_GNUC_UNUSED const gchar *id,
	GSList *plugins,
	G_GNUC_UNUSED gpointer data)
{
	GPluginManager *manager = gplugin_manager_get_default();
	GSList *l = NULL;

	for(l = plugins; l != NULL; l = l->next) {
		gplugin_manager_load_plugin(manager, GPLUGIN_PLUGIN(l->data), NULL);
	}
}

static void
test_gplugin_manager_foreach_unload_plugins(
	G_GNUC_UNUSED const gchar *id,
	GSList *plugins,
	G_GNUC_UNUSED gpointer data)
{
	GPluginManager *manager = gplugin_manager_get_default();
	GSList *l = NULL;

	for(l = plugins; l != NULL; l = l->next) {
		gplugin_manager_unload_plugin(manager, GPLUGIN_PLUGIN(l->data), NULL);
	}
}

/******************************************************************************
 * Tests
 *****************************************************************************/
/*< private >
 * test_gplugin_manager_find_plugins_with_state:
 *
 * Runs through the normal plugins in the plugins directory, trying to progress
 * their state each iteration while checking that
 * gplugin_manager_find_plugins_with_state() returns the correct values.
 *
 * If any changes are made to the plugins directory, this test will need to be
 * updated.
 */
static void
test_gplugin_manager_find_plugins_with_state(void)
{
	GPluginManager *manager = NULL;
	GSList *plugins = NULL;

	/* this is the list of the current plugins and the furthest state they can
	 * make it in the plugin lifecycle:
	 *
	 * basic-plugin.c: unloaded
	 * broken-dependent-plugin.c: queried
	 * dependent-plugin.c: queried
	 * load-exception: load failed
	 * load-failed: load failed
	 * unload-failed: unload failed
	 */
	gint QUERIED = 7;
	gint LOADED = 3;
	gint LOAD_FAILED = 2;
	gint UNLOADED = 3; /* unloaded plugins go back to the queried state */
	gint UNLOAD_FAILED = 2;

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);

	manager = gplugin_manager_get_default();

	gplugin_manager_append_path(manager, TEST_DIR);
	gplugin_manager_refresh(manager);

	/* make sure that all the plugins are queried */
	plugins = gplugin_manager_find_plugins_with_state(
		manager,
		GPLUGIN_PLUGIN_STATE_QUERIED);
	g_assert_cmpint(g_slist_length(plugins), ==, QUERIED);
	g_slist_free_full(plugins, g_object_unref);

	/* now load all of the plugins */
	gplugin_manager_foreach(
		manager,
		test_gplugin_manager_foreach_load_plugins,
		NULL);

	/* make sure we have the proper number loaded */
	plugins = gplugin_manager_find_plugins_with_state(
		manager,
		GPLUGIN_PLUGIN_STATE_LOADED);
	g_assert_cmpint(g_slist_length(plugins), ==, LOADED);
	g_slist_free_full(plugins, g_object_unref);

	/* make sure we have the proper number of load failed */
	plugins = gplugin_manager_find_plugins_with_state(
		manager,
		GPLUGIN_PLUGIN_STATE_LOAD_FAILED);
	g_assert_cmpint(g_slist_length(plugins), ==, LOAD_FAILED);
	g_slist_free_full(plugins, g_object_unref);

	/* unload all of the plugins */
	gplugin_manager_foreach(
		manager,
		test_gplugin_manager_foreach_unload_plugins,
		NULL);

	/* make sure we have the proper number unloaded */
	plugins = gplugin_manager_find_plugins_with_state(
		manager,
		GPLUGIN_PLUGIN_STATE_QUERIED);
	g_assert_cmpint(g_slist_length(plugins), ==, UNLOADED);
	g_slist_free_full(plugins, g_object_unref);

	/* make sure we have the proper number of unload failed */
	plugins = gplugin_manager_find_plugins_with_state(
		manager,
		GPLUGIN_PLUGIN_STATE_UNLOAD_FAILED);
	g_assert_cmpint(g_slist_length(plugins), ==, UNLOAD_FAILED);
	g_slist_free_full(plugins, g_object_unref);

	gplugin_uninit();
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv)
{
	g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

	g_test_add_func(
		"/manager/find_plugins/with_state",
		test_gplugin_manager_find_plugins_with_state);

	return g_test_run();
}
