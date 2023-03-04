/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include <glib.h>

#include <purple.h>

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_purple_tags_lookup_exists(void) {
	PurpleTags *tags = purple_tags_new();
	gboolean found = FALSE;
	const gchar *value = NULL;

	purple_tags_add(tags, "foo");
	value = purple_tags_lookup(tags, "foo", &found);
	g_assert_null(value);
	g_assert_true(found);

	purple_tags_add(tags, "bar:baz");
	value = purple_tags_lookup(tags, "bar", &found);
	g_assert_cmpstr(value, ==, "baz");
	g_assert_true(found);

	/* make sure that a name of pur doesn't match a tag of purple */
	purple_tags_add(tags, "purple");
	value = purple_tags_lookup(tags, "pur", &found);
	g_assert_null(value);
	g_assert_false(found);

	g_clear_object(&tags);
}

static void
test_purple_tags_lookup_non_existent(void) {
	PurpleTags *tags = purple_tags_new();
	gboolean found = FALSE;
	const gchar *value;

	value = purple_tags_lookup(tags, "foo", &found);
	g_assert_null(value);
	g_assert_false(found);

	g_clear_object(&tags);
}

static void
test_purple_tags_add_remove_bare(void) {
	PurpleTags *tags = purple_tags_new();

	purple_tags_add(tags, "tag1");
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	purple_tags_remove(tags, "tag1");
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 0);

	g_clear_object(&tags);
}

static void
test_purple_tags_add_duplicate_bare(void) {
	PurpleTags *tags = purple_tags_new();

	purple_tags_add(tags, "tag1");
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	purple_tags_add(tags, "tag1");
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	g_clear_object(&tags);
}

static void
test_purple_tags_remove_non_existent_bare(void) {
	PurpleTags *tags = purple_tags_new();

	purple_tags_remove(tags, "tag1");
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 0);

	g_clear_object(&tags);
}

static void
test_purple_tags_add_remove_with_value(void) {
	PurpleTags *tags = purple_tags_new();

	purple_tags_add(tags, "tag1:purple");
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	purple_tags_remove(tags, "tag1:purple");
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 0);

	g_clear_object(&tags);
}

static void
test_purple_tags_add_duplicate_with_value(void) {
	PurpleTags *tags = purple_tags_new();

	purple_tags_add(tags, "tag1:purple");
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	purple_tags_add(tags, "tag1:purple");
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	g_clear_object(&tags);
}

static void
test_purple_tags_add_with_value(void) {
	PurpleTags *tags = purple_tags_new();
	const char *value = NULL;
	gboolean found = FALSE;

	purple_tags_add_with_value(tags, "tag1", "purple");
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	value = purple_tags_lookup(tags, "tag1", &found);
	g_assert_cmpstr(value, ==, "purple");
	g_assert_true(found);

	g_clear_object(&tags);
}

static void
test_purple_tags_add_with_value_null(void) {
	PurpleTags *tags = purple_tags_new();
	const char *value = NULL;
	gboolean found = FALSE;

	purple_tags_add_with_value(tags, "tag1", NULL);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	value = purple_tags_lookup(tags, "tag1", &found);
	g_assert_null(value);
	g_assert_true(found);

	g_clear_object(&tags);
}

static void
test_purple_tags_remove_non_existent_with_value(void) {
	PurpleTags *tags = purple_tags_new();

	purple_tags_remove(tags, "tag1:purple");
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 0);

	g_clear_object(&tags);
}

static void
test_purple_tags_get_single(void) {
	PurpleTags *tags = purple_tags_new();
	const gchar *value = NULL;

	purple_tags_add(tags, "tag1:purple");
	value = purple_tags_get(tags, "tag1");

	g_assert_cmpstr(value, ==, "purple");

	g_clear_object(&tags);
}

static void
test_purple_tags_get_multiple(void) {
	PurpleTags *tags = purple_tags_new();
	const gchar *value = NULL;

	purple_tags_add(tags, "tag1:purple");
	purple_tags_add(tags, "tag1:pink");

	value = purple_tags_get(tags, "tag1");

	g_assert_cmpstr(value, ==, "purple");

	g_clear_object(&tags);
}

static void
test_purple_tags_get_all(void) {
	PurpleTags *tags = purple_tags_new();
	GList *all_tags = NULL;
	const gchar *values[] = {"foo", "bar", "baz", "qux", "quux", NULL};
	gint i = 0;
	gint len = 0;

	for(i = 0; values[i] != NULL; i++) {
		purple_tags_add(tags, values[i]);
		len++;
	}

	all_tags = purple_tags_get_all(tags);
	i = 0;

	for(GList *l = all_tags; l != NULL; l = l->next) {
		const gchar *value = l->data;

		g_assert_cmpint(i, <, len);
		g_assert_cmpstr(value, ==, values[i]);

		i++;
	}

	g_assert_cmpint(i, ==, len);

	g_clear_object(&tags);
}

static void
test_purple_tags_to_string_single(void) {
	PurpleTags *tags = purple_tags_new();
	gchar *value = NULL;

	purple_tags_add(tags, "foo");
	value = purple_tags_to_string(tags, NULL);

	g_assert_cmpstr(value, ==, "foo");

	g_free(value);

	g_clear_object(&tags);
}

static void
test_purple_tags_to_string_multiple_with_separator(void) {
	PurpleTags *tags = purple_tags_new();
	gchar *value = NULL;

	purple_tags_add(tags, "foo");
	purple_tags_add(tags, "bar");
	purple_tags_add(tags, "baz");
	value = purple_tags_to_string(tags, ", ");

	g_assert_cmpstr(value, ==, "foo, bar, baz");

	g_free(value);

	g_clear_object(&tags);
}

static void
test_purple_tags_to_string_multiple_with_null_separator(void) {
	PurpleTags *tags = purple_tags_new();
	gchar *value = NULL;

	purple_tags_add(tags, "foo");
	purple_tags_add(tags, "bar");
	purple_tags_add(tags, "baz");
	value = purple_tags_to_string(tags, NULL);

	g_assert_cmpstr(value, ==, "foobarbaz");

	g_free(value);

	g_clear_object(&tags);
}

/******************************************************************************
 * purple_tag_parse Tests
 *****************************************************************************/
typedef struct {
	char *tag;
	char *name;
	char *value;
} TestPurpleTagParseData;

static void
test_purple_tag_parse(void) {
	TestPurpleTagParseData data[] = {
		{"", "", NULL},
		{"foo", "foo", NULL},
		{"🐦", "🐦", NULL},
		{":", "", ""},
		{"foo:bar", "foo", "bar"},
		{"🐦:", "🐦", ""},
		{":🐦", "", "🐦"},
		{NULL},
	};

	for(int i = 0; data[i].tag != NULL; i++) {
		char *name = NULL;
		char *value = NULL;

		purple_tag_parse(data[i].tag, &name, &value);

		g_assert_cmpstr(name, ==, data[i].name);
		g_assert_cmpstr(value, ==, data[i].value);

		g_free(name);
		g_free(value);
	}

	/* Finally make sure that we don't crash if neither optional argument was
	 * passed.
	 */
	purple_tag_parse("", NULL, NULL);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
gint
main(gint argc, gchar **argv) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/tags/lookup-exists", test_purple_tags_lookup_exists);
	g_test_add_func("/tags/lookup-non-existent",
	                test_purple_tags_lookup_non_existent);

	g_test_add_func("/tags/add-remove-bare",
	                test_purple_tags_add_remove_bare);
	g_test_add_func("/tags/add-duplicate-bare",
	                test_purple_tags_add_duplicate_bare);
	g_test_add_func("/tags/remove-non-existent-bare",
	                test_purple_tags_remove_non_existent_bare);

	g_test_add_func("/tags/add-with-value",
	                test_purple_tags_add_with_value);
	g_test_add_func("/tags/add-with-value-null",
	                test_purple_tags_add_with_value_null);

	g_test_add_func("/tags/add-remove-with-value",
	                test_purple_tags_add_remove_with_value);
	g_test_add_func("/tags/add-duplicate-with-value",
	                test_purple_tags_add_duplicate_with_value);
	g_test_add_func("/tags/remove-non-existent-with-value",
	                test_purple_tags_remove_non_existent_with_value);

	g_test_add_func("/tags/get-single", test_purple_tags_get_single);
	g_test_add_func("/tags/get-multiple", test_purple_tags_get_multiple);

	g_test_add_func("/tags/get-all", test_purple_tags_get_all);

	g_test_add_func("/tags/to-string-single",
	                test_purple_tags_to_string_single);
	g_test_add_func("/tags/to-string-multiple-with-separator",
	                test_purple_tags_to_string_multiple_with_separator);
	g_test_add_func("/tags/to-string-multiple-with-null-separator",
	                test_purple_tags_to_string_multiple_with_null_separator);

	g_test_add_func("/tag/parse", test_purple_tag_parse);

	return g_test_run();
}
