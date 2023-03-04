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

#include <gplugin.h>

/******************************************************************************
 * Tests
 *****************************************************************************/
/* bad versions */
static void
test_gplugin_version_null__null(void)
{
	g_assert_cmpint(gplugin_version_compare(NULL, NULL), ==, 0);
}

static void
test_gplugin_version_null__1_2_3(void)
{
	g_assert_cmpint(gplugin_version_compare(NULL, "1.2.3"), <, 0);
}

static void
test_gplugin_version_1_2_3__null(void)
{
	g_assert_cmpint(gplugin_version_compare("1.2.3", NULL), >, 0);
}

static void
test_gplugin_version_abc__1_2_3(void)
{
	g_assert_cmpint(gplugin_version_compare("abc", "1.2.3"), <, 0);
}

static void
test_gplugin_version_1_2_3__abc(void)
{
	g_assert_cmpint(gplugin_version_compare("1.2.3", "abc"), >, 0);
}

/* major version tests */
static void
test_gplugin_version_1_0_0__0_0_0(void)
{
	g_assert_cmpint(gplugin_version_compare("1.0.0", "0.0.0"), >, 0);
}

static void
test_gplugin_version_1_0_0__1_0_0(void)
{
	g_assert_cmpint(gplugin_version_compare("1.0.0", "1.0.0"), ==, 0);
}

static void
test_gplugin_version_0_0_0__1_0_0(void)
{
	g_assert_cmpint(gplugin_version_compare("0.0.0", "1.0.0"), <, 0);
}

/* minor version tests */
static void
test_gplugin_version_0_1_0__0_0_0(void)
{
	g_assert_cmpint(gplugin_version_compare("0.1.0", "0.0.0"), >, 0);
}

static void
test_gplugin_version_0_1_0__0_1_0(void)
{
	g_assert_cmpint(gplugin_version_compare("0.1.0", "0.1.0"), ==, 0);
}

static void
test_gplugin_version_0_0_0__0_1_0(void)
{
	g_assert_cmpint(gplugin_version_compare("0.0.0", "0.1.0"), <, 0);
}

/* micro version tests */
static void
test_gplugin_version_0_0_1__0_0_0(void)
{
	g_assert_cmpint(gplugin_version_compare("0.0.1", "0.0.0"), >, 0);
}

static void
test_gplugin_version_0_0_1__0_0_1(void)
{
	g_assert_cmpint(gplugin_version_compare("0.0.1", "0.0.1"), ==, 0);
}

static void
test_gplugin_version_0_0_0__0_0_1(void)
{
	g_assert_cmpint(gplugin_version_compare("0.0.0", "0.0.1"), <, 0);
}

/* major-minor tests */
static void
test_gplugin_version_1_0__0_1(void)
{
	g_assert_cmpint(gplugin_version_compare("1.0", "0.1"), >, 0);
}

static void
test_gplugin_version_1_0__1_0(void)
{
	g_assert_cmpint(gplugin_version_compare("1.0", "1.0"), ==, 0);
}

static void
test_gplugin_version_0_1__1_0(void)
{
	g_assert_cmpint(gplugin_version_compare("0.1", "1.0"), <, 0);
}

/* major tests */
static void
test_gplugin_version_1__0(void)
{
	g_assert_cmpint(gplugin_version_compare("1", "0"), >, 0);
}

static void
test_gplugin_version_1__1(void)
{
	g_assert_cmpint(gplugin_version_compare("1", "1"), ==, 0);
}

static void
test_gplugin_version_0__1(void)
{
	g_assert_cmpint(gplugin_version_compare("0", "1"), <, 0);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv)
{
	g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

	gplugin_init(GPLUGIN_CORE_FLAGS_NONE);

	/* bad versions */
	g_test_add_func(
		"/version-compare/null__null",
		test_gplugin_version_null__null);
	g_test_add_func(
		"/version-compare/null__1_2_3",
		test_gplugin_version_null__1_2_3);
	g_test_add_func(
		"/version-compare/1_2_3__null",
		test_gplugin_version_1_2_3__null);
	g_test_add_func(
		"/version-compare/abc__1_2_3",
		test_gplugin_version_abc__1_2_3);
	g_test_add_func(
		"/version-compare/1_2_3__abc",
		test_gplugin_version_1_2_3__abc);

	/* major version */
	g_test_add_func(
		"/version-compare/1_0_0__0_0_0",
		test_gplugin_version_1_0_0__0_0_0);
	g_test_add_func(
		"/version-compare/1_0_0__1_0_0",
		test_gplugin_version_1_0_0__1_0_0);
	g_test_add_func(
		"/version-compare/0_0_0__1_0_0",
		test_gplugin_version_0_0_0__1_0_0);

	/* minor version */
	g_test_add_func(
		"/version-compare/0_1_0__0_0_0",
		test_gplugin_version_0_1_0__0_0_0);
	g_test_add_func(
		"/version-compare/0_1_0__0_1_0",
		test_gplugin_version_0_1_0__0_1_0);
	g_test_add_func(
		"/version-compare/0_0_0__0_1_0",
		test_gplugin_version_0_0_0__0_1_0);

	/* micro version */
	g_test_add_func(
		"/version-compare/0_0_1__0_0_0",
		test_gplugin_version_0_0_1__0_0_0);
	g_test_add_func(
		"/version-compare/0_0_1__0_0_1",
		test_gplugin_version_0_0_1__0_0_1);
	g_test_add_func(
		"/version-compare/0_0_0__0_0_1",
		test_gplugin_version_0_0_0__0_0_1);

	/* major-minor */
	g_test_add_func("/version-compare/1_0__0_1", test_gplugin_version_1_0__0_1);
	g_test_add_func("/version-compare/1_0__1_0", test_gplugin_version_1_0__1_0);
	g_test_add_func("/version-compare/0_1__1_0", test_gplugin_version_0_1__1_0);

	/* major */
	g_test_add_func("/version-compare/1__0", test_gplugin_version_1__0);
	g_test_add_func("/version-compare/1__1", test_gplugin_version_1__1);
	g_test_add_func("/version-compare/0__1", test_gplugin_version_0__1);

	return g_test_run();
}
