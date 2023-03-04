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
test_newest_version(const gchar *id, const gchar *version)
{
	GPluginManager *manager = gplugin_manager_get_default();
	GPluginPlugin *plugin = NULL;
	GPluginPluginInfo *info = NULL;

	gplugin_manager_append_path(manager, TEST_DIR);
	gplugin_manager_refresh(manager);

	plugin = gplugin_manager_find_plugin_with_newest_version(manager, id);
	g_assert_nonnull(plugin);
	g_assert_true(GPLUGIN_IS_PLUGIN(plugin));

	info = gplugin_plugin_get_info(plugin);
	g_assert_nonnull(info);
	g_assert_true(GPLUGIN_IS_PLUGIN_INFO(info));

	g_assert_cmpstr(gplugin_plugin_info_get_version(info), ==, version);

	g_clear_object(&info);
	g_clear_object(&plugin);
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_newest_version_multiple_semantic(void)
{
	test_newest_version(
		"gplugin/test-newest-version/multiple-semantic",
		"1.1.0");
}

static void
test_newest_version_solo_non_semantic(void)
{
	test_newest_version(
		"gplugin/test-newest-version/solo-non-semantic",
		"abc123");
}

static void
test_newest_version_non_semantic_and_semantic(void)
{
	test_newest_version(
		"gplugin/test-newest-version/non-semantic-and-semantic",
		"1.0.0");
}

static void
test_newest_version_solo_no_version(void)
{
	test_newest_version("gplugin/test-newest-version/solo-no-version", NULL);
}

static void
test_newest_version_no_version_and_semantic(void)
{
	test_newest_version(
		"gplugin/test-newest-version/no-version-and-semantic",
		"1.0.0");
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv)
{

	g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);

	g_test_add_func(
		"/manager/find_plugin_newest_version/multiple-semantic",
		test_newest_version_multiple_semantic);
	g_test_add_func(
		"/manager/find_plugin_newest_version/no-version-and-semantic",
		test_newest_version_no_version_and_semantic);
	g_test_add_func(
		"/manager/find_plugin_newest_version/non-semantic-and-semantic",
		test_newest_version_non_semantic_and_semantic);
	g_test_add_func(
		"/manager/find_plugin_newest_version/solo-no-version",
		test_newest_version_solo_no_version);
	g_test_add_func(
		"/manager/find_plugin_newest_version/solo-non-semantic",
		test_newest_version_solo_non_semantic);

	return g_test_run();
}
