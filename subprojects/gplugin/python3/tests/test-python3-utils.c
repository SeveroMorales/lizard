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

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <glib.h>

#include "gplugin-python3-utils.h"

/******************************************************************************
 * filename to module tests
 *****************************************************************************/
static void
test_filename_to_module_NULL_subprocess(void)
{
	gplugin_python3_filename_to_module(NULL);
}

static void
test_filename_to_module_NULL(void)
{
	g_test_trap_subprocess(
		"/loaders/python3/utils/filename_to_module/NULL/subprocess",
		0,
		0);

	g_test_trap_assert_failed();
}

static void
test_filename_to_module_empty(void)
{
	gchar *module = gplugin_python3_filename_to_module("");

	g_assert_cmpstr(module, ==, "");

	g_free(module);
}

static void
test_filename_to_module_no_extension(void)
{
	gchar *module = gplugin_python3_filename_to_module("foo");

	g_assert_cmpstr(module, ==, "foo");

	g_free(module);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv)
{
	g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

	/* tests */
	g_test_add_func(
		"/loaders/python3/utils/filename_to_module/NULL",
		test_filename_to_module_NULL);
	g_test_add_func(
		"/loaders/python3/utils/filename_to_module/NULL/subprocess",
		test_filename_to_module_NULL_subprocess);
	g_test_add_func(
		"/loaders/python3/utils/filename_to_module/empty",
		test_filename_to_module_empty);
	g_test_add_func(
		"/loaders/python3/utils/filename_to_module/no-extension",
		test_filename_to_module_no_extension);

	return g_test_run();
}
