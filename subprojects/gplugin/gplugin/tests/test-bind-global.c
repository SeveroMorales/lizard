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

#include <stdlib.h>

#include <glib.h>

#include <gplugin.h>
#include <gplugin-native.h>

/******************************************************************************
 * Tests
 *****************************************************************************/

/* This test kind of sucks because there's no way for us to lookup whether or
 * not a module handle has had its symbols bound globally.  Therefore, right
 * now we have to settle to see if it was loaded correctly.
 */
static void
test_bind_global(void)
{
	GPluginManager *manager = gplugin_manager_get_default();
	GPluginPlugin *plugin = NULL;

	gplugin_manager_remove_paths(manager);
	gplugin_manager_append_path(manager, TEST_BIND_GLOBAL_DIR);
	gplugin_manager_refresh(manager);

	plugin = gplugin_manager_find_plugin(manager, "gplugin/bind-global");
	g_assert_nonnull(plugin);
	g_assert_true(GPLUGIN_IS_PLUGIN(plugin));
	g_assert_true(GPLUGIN_IS_NATIVE_PLUGIN(plugin));
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
	g_test_add_func("/loaders/native/bind-global", test_bind_global);

	return g_test_run();
}
