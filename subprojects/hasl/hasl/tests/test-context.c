/*
 * Copyright (C) 2023 Hasl Developers
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

#include <hasl.h>

#if !GLIB_CHECK_VERSION(2, 74, 0)
const int G_TEST_SUBPROCESS_DEFAULT = 0;
#endif

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_hasl_context_new(void) {
	HaslContext *ctx = NULL;

	ctx = hasl_context_new();

	g_assert_true(HASL_IS_CONTEXT(ctx));

	g_clear_object(&ctx);
}

static void
test_hasl_context_properties(void) {
	HaslContext *ctx = NULL;
	char *allowed_mechanisms = NULL;
	char *authzid = NULL;
	char *username = NULL;
	char *password = NULL;
	gboolean tls = FALSE;
	gboolean allow_clear_text = FALSE;

	ctx = g_object_new(
		HASL_TYPE_CONTEXT,
		"allowed-mechanisms", "PLAIN",
		"authzid", "authzid",
		"username", "username",
		"password", "password",
		"tls", TRUE,
		"allow-clear-text", TRUE,
		NULL);

	g_assert_true(HASL_IS_CONTEXT(ctx));

	g_object_get(
		G_OBJECT(ctx),
		"allowed-mechanisms", &allowed_mechanisms,
		"authzid", &authzid,
		"username", &username,
		"password", &password,
		"tls", &tls,
		"allow-clear-text", &allow_clear_text,
		NULL);

	g_assert_cmpstr(allowed_mechanisms, ==, "PLAIN");
	g_assert_cmpstr(authzid, ==, "authzid");
	g_assert_cmpstr(username, ==, "username");
	g_assert_cmpstr(password, ==, "password");
	g_assert_true(tls);
	g_assert_true(allow_clear_text);

	g_clear_pointer(&allowed_mechanisms, g_free);
	g_clear_pointer(&authzid, g_free);
	g_clear_pointer(&username, g_free);
	g_clear_pointer(&password, g_free);

	g_clear_object(&ctx);
}

static void
test_hasl_context_add_mechanism_null(void) {
	if(g_test_subprocess()) {
		HaslContext *ctx = hasl_context_new();

		hasl_context_add_mechanism(ctx, NULL, HASL_TYPE_MECHANISM);

		g_clear_object(&ctx);
	}

	g_test_trap_subprocess(NULL, 0, G_TEST_SUBPROCESS_DEFAULT);
	g_test_trap_assert_failed();
	g_test_trap_assert_stderr("*assertion*name*");
}

static void
test_hasl_context_add_mechanism_empty(void) {
	if(g_test_subprocess()) {
		HaslContext *ctx = hasl_context_new();

		hasl_context_add_mechanism(ctx, "", HASL_TYPE_MECHANISM);

		g_clear_object(&ctx);
	}

	g_test_trap_subprocess(NULL, 0, G_TEST_SUBPROCESS_DEFAULT);
	g_test_trap_assert_failed();
	g_test_trap_assert_stderr("*assertion*name*");
}

static void
test_hasl_context_supported_mechanisms_default(void) {
	HaslContext *ctx = NULL;
	char *supported_mechanisms = NULL;

	ctx = hasl_context_new();
	supported_mechanisms = hasl_context_get_supported_mechanisms(ctx);

	g_assert_cmpstr(supported_mechanisms, ==, "EXTERNAL PLAIN");

	g_free(supported_mechanisms);
	g_clear_object(&ctx);
}

static void
test_hasl_context_supported_mechanisms_custom(void) {
	HaslContext *ctx = NULL;
	char *supported_mechanisms = NULL;
	gboolean r = FALSE;

	ctx = hasl_context_new();

	/* Add some custom mechanisms. */
	r = hasl_context_add_mechanism(ctx, "TEST", HASL_TYPE_MECHANISM);
	g_assert_true(r);

	r = hasl_context_add_mechanism(ctx, "DEBUG", HASL_TYPE_MECHANISM);
	g_assert_true(r);

	/* Now get the list and check it. */
	supported_mechanisms = hasl_context_get_supported_mechanisms(ctx);

	g_assert_cmpstr(supported_mechanisms, ==, "DEBUG EXTERNAL PLAIN TEST");

	g_free(supported_mechanisms);
	g_clear_object(&ctx);
}

static void
test_hasl_context_current_mechanism(void) {
	HaslContext *context = NULL;
	const char *current = NULL;

	context = hasl_context_new();
	hasl_context_set_username(context, "bob");
	hasl_context_set_password(context, "hunter2");
	hasl_context_set_tls(context, TRUE);

	/* Make sure NULL is returned if set_allowed_mechanisms hasn't been called
	 * yet.
	 */
	current = hasl_context_get_current_mechanism(context);
	g_assert_null(current);

	hasl_context_set_allowed_mechanisms(context, "PLAIN,EXTERNAL");

	/* Now make sure NULL is returned if next hasn't been called yet. */
	current = hasl_context_get_current_mechanism(context);
	g_assert_null(current);

	/* Call next until we exhausted it and make sure we get the values we're
	 * expecting.
	 */
	hasl_context_next(context);
	current = hasl_context_get_current_mechanism(context);
	g_assert_cmpstr(current, ==, "PLAIN");

	hasl_context_next(context);
	current = hasl_context_get_current_mechanism(context);
	g_assert_cmpstr(current, ==, "EXTERNAL");

	hasl_context_next(context);
	current = hasl_context_get_current_mechanism(context);
	g_assert_null(current);

	/* We do one extra to make sure we're not overflowing. */
	hasl_context_next(context);
	current = hasl_context_get_current_mechanism(context);
	g_assert_null(current);

	g_clear_object(&context);
}

static void
test_hasl_context_next_null_supported(void) {
	HaslContext *context = NULL;
	const char *next = NULL;

	context = hasl_context_new();

	next = hasl_context_next(context);
	g_assert_null(next);

	/* Make sure we don't move past the last item. */
	next = hasl_context_next(context);
	g_assert_null(next);

	g_clear_object(&context);
}

static void
test_hasl_context_next_plain_and_external(void) {
	HaslContext *context = NULL;
	const char *next = NULL;

	context = hasl_context_new();
	hasl_context_set_username(context, "alice");
	hasl_context_set_password(context, "hunter2");
	hasl_context_set_allowed_mechanisms(context, "PLAIN,EXTERNAL");
	hasl_context_set_allow_clear_text(context, TRUE);

	next = hasl_context_next(context);
	g_assert_cmpstr(next, ==, "PLAIN");

	next = hasl_context_next(context);
	g_assert_cmpstr(next, ==, "EXTERNAL");

	next = hasl_context_next(context);
	g_assert_null(next);

	/* Verify that we can't get past the end and get null on subsequent
	 * calls.
	 */
	next = hasl_context_next(context);
	g_assert_null(next);

	g_clear_object(&context);
}

static void
test_hasl_context_next_plain_not_possible(void) {
	HaslContext *context = NULL;
	const char *next = NULL;

	context = hasl_context_new();
	hasl_context_set_allowed_mechanisms(context, "PLAIN,EXTERNAL");

	next = hasl_context_next(context);
	g_assert_cmpstr(next, ==, "EXTERNAL");

	next = hasl_context_next(context);
	g_assert_null(next);

	/* Verify that we can't get past the end and get null on subsequent
	 * calls.
	 */
	next = hasl_context_next(context);
	g_assert_null(next);

	g_clear_object(&context);
}

static void
test_hasl_context_next_plain_not_possible_only_mechanism(void) {
	HaslContext *context = NULL;
	const char *next = NULL;

	context = hasl_context_new();
	hasl_context_set_allowed_mechanisms(context, "PLAIN");

	next = hasl_context_next(context);
	g_assert_null(next);

	/* Verify that we can't get past the end and get null on subsequent
	 * calls.
	 */
	next = hasl_context_next(context);
	g_assert_null(next);

	g_clear_object(&context);
}

/******************************************************************************
 * Main
 *****************************************************************************/
int
main(int argc, char *argv[]) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/hasl/context/new", test_hasl_context_new);
	g_test_add_func("/hasl/context/properties", test_hasl_context_properties);

	g_test_add_func("/hasl/context/add-mechanism/null",
	                test_hasl_context_add_mechanism_null);
	g_test_add_func("/hasl/context/add-mechanism/empty",
	                test_hasl_context_add_mechanism_empty);

	g_test_add_func("/hasl/context/supported-mechanisms/default",
	                test_hasl_context_supported_mechanisms_default);
	g_test_add_func("/hasl/context/supported-mechanisms/custom",
	                test_hasl_context_supported_mechanisms_custom);

	g_test_add_func("/hasl/context/current-mechanism",
	                test_hasl_context_current_mechanism);

	g_test_add_func("/hasl/context/next/null_supported",
	                test_hasl_context_next_null_supported);
	g_test_add_func("/hasl/context/next/plain_and_external",
	                test_hasl_context_next_plain_and_external);
	g_test_add_func("/hasl/context/next/plain_not_possible",
	                test_hasl_context_next_plain_not_possible);
	g_test_add_func("/hasl/context/next/plain_not_possible_only_mechanism",
	                test_hasl_context_next_plain_not_possible_only_mechanism);

	return g_test_run();
}
