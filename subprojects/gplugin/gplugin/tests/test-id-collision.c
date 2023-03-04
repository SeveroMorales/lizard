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
test_id_collision(void)
{
	GPluginManager *manager = NULL;
	GSList *plugins = NULL;

	manager = gplugin_manager_get_default();

	gplugin_manager_append_path(manager, TEST_ID_DIR);
	gplugin_manager_refresh(manager);

	plugins = gplugin_manager_find_plugins(manager, "gplugin/id-collision");
	g_assert_nonnull(plugins);

	g_assert_cmpuint(g_slist_length(plugins), ==, 2);

	g_slist_free_full(plugins, g_object_unref);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv)
{
	g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);

	g_test_add_func("/loaders/native/id-collision", test_id_collision);

	return g_test_run();
}
