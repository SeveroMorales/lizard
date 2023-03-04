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

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_auto_load_pass(void)
{
	GPluginManager *manager = gplugin_manager_get_default();
	GPluginPlugin *plugin = NULL;

	gplugin_manager_remove_paths(manager);
	gplugin_manager_append_path(manager, TEST_AUTO_LOAD_PASS_DIR);
	gplugin_manager_refresh(manager);

	plugin = gplugin_manager_find_plugin(manager, "gplugin/auto-load-pass");
	g_assert_nonnull(plugin);
	g_assert_true(GPLUGIN_IS_PLUGIN(plugin));

	g_assert_cmpint(
		gplugin_plugin_get_state(plugin),
		==,
		GPLUGIN_PLUGIN_STATE_LOADED);
}

static void
test_auto_load_fail(void)
{
	if(g_test_subprocess()) {
		GPluginManager *manager = gplugin_manager_get_default();

		/* this test is very simple since we can't get the exact error
		 * condition that we want.
		 *
		 * There's an error condition where a plugin will be stored twice, but
		 * we can't test for it since a g_warning gets output that kills our
		 * fork, so we lose the internal state of the plugin manager and thus
		 * can't see the plugin stored twice.  This has been fixed in the code,
		 * but it has to be looked for manually.
		 */
		gplugin_manager_remove_paths(manager);
		gplugin_manager_append_path(manager, TEST_DIR);
		gplugin_manager_append_path(manager, TEST_AUTO_LOAD_FAIL_DIR);
		gplugin_manager_refresh(manager);
	}

	g_test_trap_subprocess(NULL, 0, 0);

	g_test_trap_assert_stderr("*failed to load*during query*");
}

static void
test_loq(void)
{
	GPluginPluginInfo *info = NULL;

	/* Make sure the load on query option is still honored and sets the
	 * auto-load property.
	 */
	info = gplugin_plugin_info_new(
		"test-loq",
		0x01020304,
		"load-on-query",
		TRUE,
		NULL);

	G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	g_assert_true(gplugin_plugin_info_get_load_on_query(info));
	G_GNUC_END_IGNORE_DEPRECATIONS

	g_assert_true(gplugin_plugin_info_get_auto_load(info));

	g_clear_object(&info);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv)
{
	g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);

	/* test the load on query flag */
	g_test_add_func("/loaders/native/auto-load/pass", test_auto_load_pass);
	g_test_add_func("/loaders/native/auto-load/fail", test_auto_load_fail);
	g_test_add_func("/loaders/native/loq", test_loq);

	return g_test_run();
}
