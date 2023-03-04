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
#include <gplugin-native.h>

#define test_string(var, value) \
	G_STMT_START \
		{ \
			g_assert_cmpstr((var), ==, (value)); \
			g_assert_cmpstr((var), ==, gplugin_plugin_info_get_##var(info)); \
			g_free((var)); \
		} \
	G_STMT_END

#define test_int(var, value) \
	G_STMT_START \
		{ \
			g_assert_cmpint((var), ==, (value)); \
			g_assert_cmpint((var), ==, gplugin_plugin_info_get_##var(info)); \
		} \
	G_STMT_END

#define test_uint(var, value) \
	G_STMT_START \
		{ \
			g_assert_cmpuint((var), ==, (value)); \
			g_assert_cmpuint((var), ==, gplugin_plugin_info_get_##var(info)); \
		} \
	G_STMT_END

#define test_true(var) \
	G_STMT_START \
		{ \
			g_assert_true((var)); \
			g_assert_true(gplugin_plugin_info_get_##var(info)); \
		} \
	G_STMT_END

#define test_false(var) \
	G_STMT_START \
		{ \
			g_assert_false((var)); \
			g_assert_false(gplugin_plugin_info_get_##var(info)); \
		} \
	G_STMT_END

typedef gchar **(*TestStringVFunc)(GPluginPluginInfo *info);

static void
test_stringv(
	gchar **got,
	const gchar *const *const expected,
	TestStringVFunc func,
	GPluginPluginInfo *info)
{
	gint i = 0;
	gchar **tmp = NULL;

	/* make sure our arrarys are the same length */
	g_assert_cmpuint(g_strv_length(got), ==, g_strv_length((gchar **)expected));

	/* now walk through until expected[i] is null comparing each entry */
	for(i = 0; expected[i]; i++)
		g_assert_cmpstr(got[i], ==, expected[i]);

	/* call the accessor on the GPluginPluginInfo object */
	tmp = func(info);

	/* verify that the accessor returned the proper value as well */
	g_assert_cmpuint(g_strv_length(tmp), ==, g_strv_length((gchar **)expected));

	for(i = 0; expected[i]; i++)
		g_assert_cmpstr(tmp[i], ==, expected[i]);

	g_strfreev(got);
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_gplugin_plugin_info_construction(void)
{
	GPluginPluginInfo *info = NULL;
	gchar *id = NULL, *name = NULL, *version = NULL, *icon_name = NULL;
	gchar *license_id = NULL, *license_text = NULL, *license_url = NULL;
	gchar *summary = NULL, *description = NULL, *category = NULL;
	gchar *website = NULL;
	gchar **provides = NULL, **authors = NULL, **dependencies = NULL;
	gint priority = 0;
	guint abi_version = 0;
	gboolean internal = FALSE, auto_load = FALSE;
	const gchar *const r_provides[] = {"foo", NULL};
	const gchar *const r_authors[] = {"author", NULL};
	const gchar *const r_dependencies[] = {"dependency", NULL};

	/* clang-format off */
	info = g_object_new(
		GPLUGIN_TYPE_PLUGIN_INFO,
		"id", "gplugin-test/plugin-info-test",
		"provides", r_provides,
		"priority", 1000,
		"abi_version", GPLUGIN_NATIVE_PLUGIN_ABI_VERSION,
		"internal", TRUE,
		"auto-load", TRUE,
		"name", "name",
		"version", "version",
		"license-id", "license-id",
		"license-text", "license-text",
		"license-url", "license-url",
		"icon-name", "icon-name",
		"summary", "summary",
		"description", "description",
		"category", "category",
		"authors", r_authors,
		"website", "website",
		"dependencies", r_dependencies,
		NULL);
	/* clang-format on */

	g_assert_true(GPLUGIN_IS_PLUGIN_INFO(info));

	/* clang-format off */
	g_object_get(
		G_OBJECT(info),
		"id", &id,
		"provides", &provides,
		"priority", &priority,
		"abi_version", &abi_version,
		"internal", &internal,
		"auto-load", &auto_load,
		"name", &name,
		"version", &version,
		"license-id", &license_id,
		"license-text", &license_text,
		"license-url", &license_url,
		"icon-name", &icon_name,
		"summary", &summary,
		"description", &description,
		"category", &category,
		"authors", &authors,
		"website", &website,
		"dependencies", &dependencies,
		NULL);
	/* clang-format on */

	test_string(id, "gplugin-test/plugin-info-test");
	test_stringv(
		provides,
		r_provides,
		(TestStringVFunc)gplugin_plugin_info_get_provides,
		info);
	test_int(priority, 1000);
	test_uint(abi_version, GPLUGIN_NATIVE_PLUGIN_ABI_VERSION);
	test_true(internal);
	test_true(auto_load);
	test_string(name, "name");
	test_string(version, "version");
	test_string(license_id, "license-id");
	test_string(license_text, "license-text");
	test_string(license_url, "license-url");
	test_string(icon_name, "icon-name");
	test_string(summary, "summary");
	test_string(description, "description");
	test_string(category, "category");
	test_stringv(
		authors,
		r_authors,
		(TestStringVFunc)gplugin_plugin_info_get_authors,
		info);
	test_string(website, "website");
	test_stringv(
		dependencies,
		r_dependencies,
		(TestStringVFunc)gplugin_plugin_info_get_dependencies,
		info);

	g_object_unref(G_OBJECT(info));
}

static void
test_gplugin_plugin_info_new_empty(void)
{
	GPluginPluginInfo *info = NULL;
	gchar *id = NULL;
	guint32 abi_version = 0;

	info = gplugin_plugin_info_new("empty", 1, NULL);

	g_assert_true(GPLUGIN_IS_PLUGIN_INFO(info));

	/* clang-format off */
	g_object_get(
		G_OBJECT(info),
		"id", &id,
		"abi-version", &abi_version,
		NULL);
	/* clang-format on */

	test_string(id, "empty");
	test_uint(abi_version, 1);

	g_object_unref(G_OBJECT(info));
}

static void
test_gplugin_plugin_info_new_full(void)
{
	GPluginPluginInfo *info = NULL;
	gchar *id = NULL, *name = NULL, *version = NULL, *icon_name = NULL;
	gchar *license_id = NULL, *license_text = NULL, *license_url = NULL;
	gchar *summary = NULL, *description = NULL, *category = NULL;
	gchar *website = NULL;
	gchar **authors = NULL, **dependencies = NULL;
	guint abi_version = 0;
	gboolean internal = FALSE, auto_load = FALSE;
	const gchar *const r_authors[] = {"author", NULL};
	const gchar *const r_dependencies[] = {"dependency", NULL};

	/* clang-format off */
	info = gplugin_plugin_info_new(
		"gplugin-test/plugin-info-test",
		GPLUGIN_NATIVE_PLUGIN_ABI_VERSION,
		"internal", TRUE,
		"auto-load", TRUE,
		"name", "name",
		"version", "version",
		"license-id", "license-id",
		"license-text", "license-text",
		"license-url", "license-url",
		"icon-name", "icon-name",
		"summary", "summary",
		"description", "description",
		"category", "category",
		"authors", r_authors,
		"website", "website",
		"dependencies", r_dependencies,
		NULL);
	/* clang-format on */

	g_assert_true(GPLUGIN_IS_PLUGIN_INFO(info));

	/* clang-format off */
	g_object_get(
		G_OBJECT(info),
		"id", &id,
		"abi_version", &abi_version,
		"internal", &internal,
		"auto-load", &auto_load,
		"name", &name,
		"version", &version,
		"license-id", &license_id,
		"license-text", &license_text,
		"license-url", &license_url,
		"icon-name", &icon_name,
		"summary", &summary,
		"description", &description,
		"category", &category,
		"authors", &authors,
		"website", &website,
		"dependencies", &dependencies,
		NULL);
	/* clang-format on */

	test_string(id, "gplugin-test/plugin-info-test");
	test_uint(abi_version, GPLUGIN_NATIVE_PLUGIN_ABI_VERSION);
	test_true(internal);
	test_true(auto_load);
	test_string(name, "name");
	test_string(version, "version");
	test_string(license_id, "license-id");
	test_string(license_text, "license-text");
	test_string(license_url, "license-url");
	test_string(icon_name, "icon-name");
	test_string(summary, "summary");
	test_string(description, "description");
	test_string(category, "category");
	test_stringv(
		authors,
		r_authors,
		(TestStringVFunc)gplugin_plugin_info_get_authors,
		info);
	test_string(website, "website");
	test_stringv(
		dependencies,
		r_dependencies,
		(TestStringVFunc)gplugin_plugin_info_get_dependencies,
		info);

	g_object_unref(G_OBJECT(info));
}

static void
test_gplugin_plugin_info_authors_single(void)
{
	GPluginPluginInfo *info = NULL;
	const gchar *const authors[] = {"author", NULL};
	const gchar *const *g_authors = NULL;
	gint i;

	/* clang-format off */
	info = gplugin_plugin_info_new(
		"test/single-author",
		GPLUGIN_NATIVE_PLUGIN_ABI_VERSION,
		"authors", authors,
		NULL);
	/* clang-format on */

	g_authors = gplugin_plugin_info_get_authors(info);

	for(i = 0; authors[i]; i++)
		g_assert_cmpstr(authors[i], ==, g_authors[i]);

	g_object_unref(G_OBJECT(info));
}

static void
test_gplugin_plugin_info_authors_multiple(void)
{
	GPluginPluginInfo *info = NULL;
	const gchar *const authors[] = {"author1", "author2", NULL};
	const gchar *const *g_authors = NULL;
	gint i;

	/* clang-format off */
	info = gplugin_plugin_info_new(
		"test/single-author",
		GPLUGIN_NATIVE_PLUGIN_ABI_VERSION,
		"authors", authors,
		NULL);
	/* clang-format on */

	g_authors = gplugin_plugin_info_get_authors(info);

	for(i = 0; authors[i]; i++)
		g_assert_cmpstr(authors[i], ==, g_authors[i]);

	g_object_unref(G_OBJECT(info));
}

static void
test_gplugin_plugin_info_dependencies_single(void)
{
	GPluginPluginInfo *info = NULL;
	gchar *dependencies[] = {"dependency1", NULL};
	const gchar *const *g_dependencies = NULL;
	gint i;

	/* clang-format off */
	info = gplugin_plugin_info_new(
		"test/single-dependency",
		GPLUGIN_NATIVE_PLUGIN_ABI_VERSION,
		"dependencies", dependencies,
		NULL);
	/* clang-format on */

	g_dependencies = gplugin_plugin_info_get_dependencies(info);

	for(i = 0; dependencies[i]; i++)
		g_assert_cmpstr(dependencies[i], ==, g_dependencies[i]);

	g_object_unref(G_OBJECT(info));
}

static void
test_gplugin_plugin_info_dependencies_multiple(void)
{
	GPluginPluginInfo *info = NULL;
	gchar *dependencies[] = {"dependencie1", "dependencie2", NULL};
	const gchar *const *g_dependencies = NULL;
	gint i;

	/* clang-format off */
	info = gplugin_plugin_info_new(
		"test/single-dependencie",
		GPLUGIN_NATIVE_PLUGIN_ABI_VERSION,
		"dependencies", dependencies,
		NULL);
	/* clang-format on */

	g_dependencies = gplugin_plugin_info_get_dependencies(info);

	for(i = 0; dependencies[i]; i++)
		g_assert_cmpstr(dependencies[i], ==, g_dependencies[i]);

	g_object_unref(G_OBJECT(info));
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
		"/plugin-info/construction",
		test_gplugin_plugin_info_construction);

	g_test_add_func(
		"/plugin-info/new/empty",
		test_gplugin_plugin_info_new_empty);
	g_test_add_func("/plugin-info/new/full", test_gplugin_plugin_info_new_full);

	g_test_add_func(
		"/plugin-info/authors/single",
		test_gplugin_plugin_info_authors_single);
	g_test_add_func(
		"/plugin-info/authors/multiple",
		test_gplugin_plugin_info_authors_multiple);

	g_test_add_func(
		"/plugin-info/dependencies/single",
		test_gplugin_plugin_info_dependencies_single);
	g_test_add_func(
		"/plugin-info/dependencies/multiple",
		test_gplugin_plugin_info_dependencies_multiple);

	return g_test_run();
}
