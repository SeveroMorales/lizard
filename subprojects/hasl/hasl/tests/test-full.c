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

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_hasl_full_plain_only(void) {
	HaslContext *context = NULL;
	HaslMechanismResult result = 0;
	GError *error = NULL;
	guint8 *client_out = NULL;
	gsize client_out_length = 0;
	const char *mechanism = NULL;

	context = hasl_context_new();
	hasl_context_set_allowed_mechanisms(context, "PLAIN");
	hasl_context_set_username(context, "alice");
	hasl_context_set_password(context, "hunter2");
	hasl_context_set_allow_clear_text(context, TRUE);

	mechanism = hasl_context_next(context);
	g_assert_cmpstr(mechanism, ==, "PLAIN");

	result = hasl_context_step(context, NULL, 0, &client_out,
	                           &client_out_length, &error);
	g_assert_no_error(error);

	g_assert_cmpint(result, ==, HASL_MECHANISM_RESULT_SUCCESS);

	g_assert_cmpmem(client_out, client_out_length, "\0alice\0hunter2", 14);

	g_free(client_out);
	g_clear_object(&context);
}

static void
test_hasl_full_plain_external_plain_not_possible(void) {
	HaslContext *context = NULL;
	HaslMechanismResult result = 0;
	GError *error = NULL;
	guint8 *client_out = NULL;
	gsize client_out_length = 0;
	const char *mechanism = NULL;

	context = hasl_context_new();
	hasl_context_set_allowed_mechanisms(context, "PLAIN,EXTERNAL");
	hasl_context_set_authzid(context, "bob");
	hasl_context_set_allow_clear_text(context, TRUE);

	mechanism = hasl_context_next(context);
	g_assert_cmpstr(mechanism, ==, "EXTERNAL");

	result = hasl_context_step(context, NULL, 0, &client_out,
	                           &client_out_length, &error);
	g_assert_no_error(error);

	g_assert_cmpint(result, ==, HASL_MECHANISM_RESULT_SUCCESS);

	g_assert_cmpmem(client_out, client_out_length, "bob", 3);

	g_free(client_out);
	g_clear_object(&context);
}

static void
test_hasl_full_plain_external_plain_failed(void) {
	HaslContext *context = NULL;
	HaslMechanismResult result = 0;
	GError *error = NULL;
	guint8 *client_out = NULL;
	gsize client_out_length = 0;
	const char *mechanism = NULL;

	context = hasl_context_new();
	hasl_context_set_allowed_mechanisms(context, "PLAIN,EXTERNAL");
	hasl_context_set_authzid(context, "alice");
	hasl_context_set_username(context, "chad");
	hasl_context_set_password(context, "hunter2");
	hasl_context_set_allow_clear_text(context, TRUE);

	mechanism = hasl_context_next(context);
	g_assert_cmpstr(mechanism, ==, "PLAIN");

	result = hasl_context_step(context, NULL, 0, &client_out,
	                           &client_out_length, &error);
	g_assert_no_error(error);

	g_assert_cmpint(result, ==, HASL_MECHANISM_RESULT_SUCCESS);

	g_assert_cmpmem(client_out, client_out_length, "alice\0chad\0hunter2", 18);
	g_free(client_out);

	/* Pretend that the auth failed, so we try the next mechanism that should
	 * be EXTERNAL.
	 */
	mechanism = hasl_context_next(context);
	g_assert_cmpstr(mechanism, ==, "EXTERNAL");

	result = hasl_context_step(context, NULL, 0, &client_out,
	                           &client_out_length, &error);
	g_assert_no_error(error);

	g_assert_cmpint(result, ==, HASL_MECHANISM_RESULT_SUCCESS);

	g_assert_cmpmem(client_out, client_out_length, "alice", 5);
	g_free(client_out);

	g_clear_object(&context);
}

/******************************************************************************
 * Main
 *****************************************************************************/
int
main(int argc, char *argv[]) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/hasl/full/plain_only", test_hasl_full_plain_only);
	g_test_add_func("/hasl/full/plain_external/plain_not_possible",
	                test_hasl_full_plain_external_plain_not_possible);
	g_test_add_func("/hasl/full/plain_external/plain_failed",
	                test_hasl_full_plain_external_plain_failed);

	return g_test_run();
}
