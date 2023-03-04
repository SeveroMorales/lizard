/*
 * Copyright (C) 2011-2016 Gary Kramlich <grim@reaperworld.com>
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
test_unresolved_symbol(void)
{
	GPluginManager *manager = gplugin_manager_get_default();

	g_test_expect_message(
		G_LOG_DOMAIN,
		G_LOG_LEVEL_WARNING,
		"*some_unresolved_symbol*");

	gplugin_manager_remove_paths(manager);
	gplugin_manager_append_path(manager, PLUGIN_DIR);
	gplugin_manager_refresh(manager);

	g_test_assert_expected_messages();
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
	g_test_add_func(
		"/loaders/native/unresolved-symbol",
		test_unresolved_symbol);

	return g_test_run();
}
