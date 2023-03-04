/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#include <sqlite3.h>

#include <purple.h>

/******************************************************************************
 * get schema version tests
 *****************************************************************************/
static void
test_sqlite3_get_schema_version_null(void) {
	if(g_test_subprocess()) {
		GError *error = NULL;
		int version = 0;

		version = purple_sqlite3_get_schema_version(NULL, &error);
		g_assert_error(error, PURPLE_SQLITE3_DOMAIN, 0);
		g_assert_cmpint(version, ==, -1);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_failed();
	g_test_trap_assert_stderr("*assertion*!= NULL*");
}

static void
test_sqlite3_get_schema_version_new(void) {
	GError *error = NULL;
	sqlite3 *db = NULL;
	int rc = 0;
	int version = 0;

	rc = sqlite3_open(":memory:", &db);
	g_assert_nonnull(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);

	version = purple_sqlite3_get_schema_version(db, &error);
	g_assert_no_error(error);
	g_assert_cmpint(version, ==, 0);

	rc = sqlite3_close(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);
}

/******************************************************************************
 * string migration tests
 *****************************************************************************/
static void
test_sqlite3_string_migrations_null(void) {
	if(g_test_subprocess()) {
		GError *error = NULL;
		sqlite3 *db = NULL;
		gboolean res = FALSE;
		int rc = 0;

		rc = sqlite3_open(":memory:", &db);
		g_assert_nonnull(db);
		g_assert_cmpint(rc, ==, SQLITE_OK);

		res = purple_sqlite3_run_migrations_from_strings(db, NULL, &error);
		g_assert_no_error(error);
		g_assert_false(res);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_failed();
	g_test_trap_assert_stderr("*migrations != NULL*");
}

static void
test_sqlite3_string_migrations_null_terminator(void) {
	GError *error = NULL;
	sqlite3 *db = NULL;
	gboolean res = FALSE;
	int rc = 0;
	int version = -1;
	const char *migrations[] = {NULL};

	rc = sqlite3_open(":memory:", &db);
	g_assert_nonnull(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);

	res = purple_sqlite3_run_migrations_from_strings(db, migrations, &error);
	g_assert_no_error(error);
	g_assert_true(res);

	version = purple_sqlite3_get_schema_version(db, &error);
	g_assert_no_error(error);
	g_assert_cmpint(version, ==, 0);

	rc = sqlite3_close(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);
}

static void
test_sqlite3_string_migrations_multiple(void) {
	GError *error = NULL;
	sqlite3 *db = NULL;
	gboolean res = FALSE;
	int rc = 0;
	int version = -1;
	const char *migrations[] = {
		"CREATE TABLE foo(a TEXT); CREATE TABLE bar(b TEXT);",
		"CREATE TABLE baz(c TEXT);",
		NULL
	};

	rc = sqlite3_open(":memory:", &db);
	g_assert_nonnull(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);

	res = purple_sqlite3_run_migrations_from_strings(db, migrations, &error);
	g_assert_no_error(error);
	g_assert_true(res);

	version = purple_sqlite3_get_schema_version(db, &error);
	g_assert_no_error(error);
	g_assert_cmpint(version, ==, 2);

	/* Run the migrations again and make sure we remain at schema version 2. */
	res = purple_sqlite3_run_migrations_from_strings(db, migrations, &error);
	g_assert_no_error(error);
	g_assert_true(res);

	version = purple_sqlite3_get_schema_version(db, &error);
	g_assert_no_error(error);
	g_assert_cmpint(version, ==, 2);

	rc = sqlite3_close(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);
}

static void
test_sqlite3_string_migrations_syntax_error(void) {
	GError *error = NULL;
	sqlite3 *db = NULL;
	gboolean res = FALSE;
	int rc = 0;
	int version = -1;
	const char *migrations[] = {
		"CREATE TABLE broke(a TEXT",
		NULL
	};

	rc = sqlite3_open(":memory:", &db);
	g_assert_nonnull(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);

	res = purple_sqlite3_run_migrations_from_strings(db, migrations, &error);
	g_assert_error(error, PURPLE_SQLITE3_DOMAIN, 0);
	g_clear_error(&error);
	g_assert_false(res);

	version = purple_sqlite3_get_schema_version(db, &error);
	g_assert_no_error(error);
	g_assert_cmpint(version, ==, 0);

	rc = sqlite3_close(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);
}

static void
test_sqlite3_string_migrations_older(void) {
	GError *error = NULL;
	sqlite3 *db = NULL;
	gboolean res = FALSE;
	int rc = 0;
	int version = -1;
	const char *migrations1[] = {
		"CREATE TABLE foo(a TEXT); CREATE TABLE bar(b TEXT);",
		"CREATE TABLE baz(c TEXT);",
		NULL
	};
	const char *migrations2[] = {
		"CREATE TABLE foo(a TEXT); CREATE TABLE bar(b TEXT);",
		NULL
	};

	rc = sqlite3_open(":memory:", &db);
	g_assert_nonnull(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);

	res = purple_sqlite3_run_migrations_from_strings(db, migrations1, &error);
	g_assert_no_error(error);
	g_assert_true(res);

	version = purple_sqlite3_get_schema_version(db, &error);
	g_assert_no_error(error);
	g_assert_cmpint(version, ==, 2);

	/* Run the older migrations now and verify we get a failure. */
	res = purple_sqlite3_run_migrations_from_strings(db, migrations2, &error);
	g_assert_error(error, PURPLE_SQLITE3_DOMAIN, 0);
	g_clear_error(&error);
	g_assert_false(res);

	version = purple_sqlite3_get_schema_version(db, &error);
	g_assert_no_error(error);
	g_assert_cmpint(version, ==, 2);

	rc = sqlite3_close(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);
}

/******************************************************************************
 * resource migration tests
 *****************************************************************************/
static void
test_sqlite3_resource_migrations_null_path(void) {
	if(g_test_subprocess()) {
		GError *error = NULL;
		sqlite3 *db = NULL;
		gboolean res = FALSE;
		int rc = 0;

		rc = sqlite3_open(":memory:", &db);
		g_assert_nonnull(db);
		g_assert_cmpint(rc, ==, SQLITE_OK);

		res = purple_sqlite3_run_migrations_from_resources(db, NULL, NULL,
		                                                   &error);
		g_assert_no_error(error);
		g_assert_false(res);

		rc = sqlite3_close(db);
		g_assert_cmpint(rc, ==, SQLITE_OK);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_failed();
	g_test_trap_assert_stderr("*path != NULL*");
}

static void
test_sqlite3_resource_migrations_null_migrations(void) {
	if(g_test_subprocess()) {
		GError *error = NULL;
		sqlite3 *db = NULL;
		gboolean res = FALSE;
		const char *path = "/im/libpidgin/purple/tests/sqlite3/";
		int rc = 0;

		rc = sqlite3_open(":memory:", &db);
		g_assert_nonnull(db);
		g_assert_cmpint(rc, ==, SQLITE_OK);

		res = purple_sqlite3_run_migrations_from_resources(db, path, NULL,
		                                                   &error);
		g_assert_no_error(error);
		g_assert_false(res);

		rc = sqlite3_close(db);
		g_assert_cmpint(rc, ==, SQLITE_OK);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_failed();
	g_test_trap_assert_stderr("*migrations != NULL*");
}

static void
test_sqlite3_resource_migrations_null_terminator(void) {
	GError *error = NULL;
	sqlite3 *db = NULL;
	gboolean res = FALSE;
	int rc = 0;
	int version = -1;
	const char *migrations[] = {NULL};
	const char *path = "/im/pidgin/libpurple/tests/sqlite3/";

	rc = sqlite3_open(":memory:", &db);
	g_assert_nonnull(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);

	res = purple_sqlite3_run_migrations_from_resources(db, path, migrations,
	                                                   &error);
	g_assert_no_error(error);
	g_assert_true(res);

	version = purple_sqlite3_get_schema_version(db, &error);
	g_assert_no_error(error);
	g_assert_cmpint(version, ==, 0);

	rc = sqlite3_close(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);
}

static void
test_sqlite3_resource_migrations_multiple(void) {
	GError *error = NULL;
	sqlite3 *db = NULL;
	gboolean res = FALSE;
	int rc = 0;
	int version = -1;
	const char *migrations[] = {"initial.sql", "secondary.sql", NULL};
	const char *path = "/im/pidgin/libpurple/tests/sqlite3/";

	rc = sqlite3_open(":memory:", &db);
	g_assert_nonnull(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);

	res = purple_sqlite3_run_migrations_from_resources(db, path, migrations,
	                                                   &error);
	g_assert_no_error(error);
	g_assert_true(res);

	version = purple_sqlite3_get_schema_version(db, &error);
	g_assert_no_error(error);
	g_assert_cmpint(version, ==, 2);

	/* Run the migrations again and make sure we remain at schema version 2. */
	res = purple_sqlite3_run_migrations_from_strings(db, migrations, &error);
	g_assert_no_error(error);
	g_assert_true(res);

	version = purple_sqlite3_get_schema_version(db, &error);
	g_assert_no_error(error);
	g_assert_cmpint(version, ==, 2);

	rc = sqlite3_close(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);
}

static void
test_sqlite3_resource_migrations_missing(void) {
	GError *error = NULL;
	sqlite3 *db = NULL;
	gboolean res = FALSE;
	int rc = 0;
	int version = -1;
	const char *migrations[] = {"initial.sql", "imaginary.sql", NULL};
	const char *path = "/im/pidgin/libpurple/tests/sqlite3/";

	rc = sqlite3_open(":memory:", &db);
	g_assert_nonnull(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);

	res = purple_sqlite3_run_migrations_from_resources(db, path, migrations,
	                                                   &error);
	g_assert_error(error, G_RESOURCE_ERROR, 0);
	g_clear_error(&error);
	g_assert_false(res);

	version = purple_sqlite3_get_schema_version(db, &error);
	g_assert_no_error(error);
	g_assert_cmpint(version, ==, 1);

	rc = sqlite3_close(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);
}

static void
test_sqlite3_resource_migrations_syntax_error(void) {
	GError *error = NULL;
	sqlite3 *db = NULL;
	gboolean res = FALSE;
	int rc = 0;
	int version = -1;
	const char *migrations[] = {"malformed.sql", NULL};
	const char *path = "/im/pidgin/libpurple/tests/sqlite3/";

	rc = sqlite3_open(":memory:", &db);
	g_assert_nonnull(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);

	res = purple_sqlite3_run_migrations_from_resources(db, path, migrations,
	                                                   &error);
	g_assert_error(error, PURPLE_SQLITE3_DOMAIN, 0);
	g_clear_error(&error);
	g_assert_false(res);

	version = purple_sqlite3_get_schema_version(db, &error);
	g_assert_no_error(error);
	g_assert_cmpint(version, ==, 0);

	rc = sqlite3_close(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);
}

static void
test_sqlite3_resource_migrations_older(void) {
	GError *error = NULL;
	sqlite3 *db = NULL;
	gboolean res = FALSE;
	int rc = 0;
	int version = -1;
	const char *migrations1[] = {"initial.sql", "secondary.sql", NULL};
	const char *migrations2[] = {"initial.sql", NULL};
	const char *path = "/im/pidgin/libpurple/tests/sqlite3/";

	rc = sqlite3_open(":memory:", &db);
	g_assert_nonnull(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);

	res = purple_sqlite3_run_migrations_from_resources(db, path, migrations1,
	                                                   &error);
	g_assert_no_error(error);
	g_assert_true(res);

	version = purple_sqlite3_get_schema_version(db, &error);
	g_assert_no_error(error);
	g_assert_cmpint(version, ==, 2);

	/* Run the older migrations now and verify we get a failure. */
	res = purple_sqlite3_run_migrations_from_resources(db, path, migrations2,
	                                                   &error);
	g_assert_error(error, PURPLE_SQLITE3_DOMAIN, 0);
	g_clear_error(&error);
	g_assert_false(res);

	version = purple_sqlite3_get_schema_version(db, &error);
	g_assert_no_error(error);
	g_assert_cmpint(version, ==, 2);

	rc = sqlite3_close(db);
	g_assert_cmpint(rc, ==, SQLITE_OK);
}

/******************************************************************************
 * Main
 *****************************************************************************/
int
main(int argc, char *argv[]) {
	g_test_init(&argc, &argv, G_TEST_OPTION_ISOLATE_DIRS, NULL);

	g_test_add_func("/sqlite3/schema_version/null",
	                test_sqlite3_get_schema_version_null);
	g_test_add_func("/sqlite3/schema_version/new",
	                test_sqlite3_get_schema_version_new);

	g_test_add_func("/sqlite3/string_migrations/null",
	                test_sqlite3_string_migrations_null);
	g_test_add_func("/sqlite3/string_migrations/null-terminator",
	                test_sqlite3_string_migrations_null_terminator);
	g_test_add_func("/sqlite3/string_migrations/multiple",
	                test_sqlite3_string_migrations_multiple);
	g_test_add_func("/sqlite3/string_migrations/syntax-error",
	                test_sqlite3_string_migrations_syntax_error);
	g_test_add_func("/sqlite3/string_migrations/older",
	                test_sqlite3_string_migrations_older);

	g_test_add_func("/sqlite3/resource_migrations/null-path",
	                test_sqlite3_resource_migrations_null_path);
	g_test_add_func("/sqlite3/resource_migrations/null-migrations",
	                test_sqlite3_resource_migrations_null_migrations);
	g_test_add_func("/sqlite3/resource_migrations/null-terminator",
	                test_sqlite3_resource_migrations_null_terminator);
	g_test_add_func("/sqlite3/resource_migrations/multiple",
	                test_sqlite3_resource_migrations_multiple);
	g_test_add_func("/sqlite3/resource_migrations/missing",
	                test_sqlite3_resource_migrations_missing);
	g_test_add_func("/sqlite3/resource_migrations/syntax-error",
	                test_sqlite3_resource_migrations_syntax_error);
	g_test_add_func("/sqlite3/resource_migrations/older",
	                test_sqlite3_resource_migrations_older);

	return g_test_run();
}
