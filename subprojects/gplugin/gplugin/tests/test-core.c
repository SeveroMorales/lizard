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
test_gplugin_count_ids_and_plugins(gint *n_ids, gint *n_plugins)
{
	GPluginManager *manager = gplugin_manager_get_default();
	GList *l_ids = NULL, *l_id = NULL;
	gint ids = 0, plugins = 0;

	l_ids = gplugin_manager_list_plugins(manager);
	for(l_id = l_ids; l_id; l_id = l_id->next) {
		GSList *l_plugins = NULL, *l_plugin = NULL;
		const gchar *id = (const gchar *)l_id->data;

		ids += 1;

		l_plugins = gplugin_manager_find_plugins(manager, id);
		for(l_plugin = l_plugins; l_plugin; l_plugin = l_plugin->next) {
			plugins += 1;
		}
		g_slist_free_full(l_plugins, g_object_unref);
	}
	g_list_free(l_ids);

	if(n_ids) {
		*n_ids = ids;
	}

	if(n_plugins) {
		*n_plugins = plugins;
	}
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_gplugin_init_uninit(void)
{
	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);
	gplugin_uninit();
}

static void
test_gplugin_init_init_uninit(void)
{
	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);
	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);
	gplugin_uninit();
}

static void
test_gplugin_init_uninit_with_refresh(void)
{
	GPluginManager *manager = NULL;

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);

	manager = gplugin_manager_get_default();
	gplugin_manager_refresh(manager);

	gplugin_uninit();
}

static void
test_gplugin_init_uninit_with_refresh_plugins(void)
{
	GPluginManager *manager = NULL;

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);

	manager = gplugin_manager_get_default();
	gplugin_manager_append_path(manager, TEST_DIR);
	gplugin_manager_refresh(manager);

	gplugin_uninit();
}

static void
test_gplugin_init_uninit_with_double_refresh_plugins(void)
{
	GPluginManager *manager = NULL;
	gint f_ids = 0, s_ids = 0, f_plugins = 0, s_plugins = 0;

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);

	manager = gplugin_manager_get_default();

	gplugin_manager_append_path(manager, TEST_DIR);
	gplugin_manager_append_path(manager, TEST_ID_DIR);

	/* run the first refresh and count everything */
	gplugin_manager_refresh(manager);
	test_gplugin_count_ids_and_plugins(&f_ids, &f_plugins);

	/* now run the second refresh and make sure the counts match */
	gplugin_manager_refresh(manager);
	test_gplugin_count_ids_and_plugins(&s_ids, &s_plugins);

	g_assert_cmpint(f_ids, >, 0);
	g_assert_cmpint(f_plugins, >, 0);

	g_assert_cmpint(f_ids, ==, s_ids);
	g_assert_cmpint(f_plugins, ==, s_plugins);

	gplugin_uninit();
}

static void
test_gplugin_init_no_native_loader(void)
{
	GPluginManager *manager = NULL;

	gplugin_init(GPLUGIN_CORE_FLAGS_DISABLE_NATIVE_LOADER);

	manager = gplugin_manager_get_default();

	g_assert_null(gplugin_manager_get_loaders(manager));

	gplugin_uninit();
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv)
{
	g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

	g_test_add_func("/core/init_uninit", test_gplugin_init_uninit);
	g_test_add_func("/core/init_init_uninit", test_gplugin_init_init_uninit);
	g_test_add_func(
		"/core/init_uninit_with_refresh",
		test_gplugin_init_uninit_with_refresh);
	g_test_add_func(
		"/core/init_uninit_with_refresh_plugins",
		test_gplugin_init_uninit_with_refresh_plugins);
	g_test_add_func(
		"/core/init_uninit_with_double_refresh_plugins",
		test_gplugin_init_uninit_with_double_refresh_plugins);

	g_test_add_func(
		"/core/init/no-native-loader",
		test_gplugin_init_no_native_loader);

	return g_test_run();
}
