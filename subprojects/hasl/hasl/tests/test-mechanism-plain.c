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
 * Helpers
 *****************************************************************************/
static void
test_hasl_mechanism_plain_error_test(HaslContext *context) {
	HaslMechanism *mechanism = NULL;
	HaslMechanismResult result = 0;
	GError *error = NULL;
	guint8 *client_out = NULL;
	gsize client_out_length = 0;

	mechanism = g_object_new(HASL_TYPE_MECHANISM_PLAIN, NULL);

	result = hasl_mechanism_step(mechanism, context, NULL, 0, &client_out,
	                             &client_out_length, &error);

	g_assert_error(error, HASL_DOMAIN, 0);
	g_clear_error(&error);

	g_assert_cmpint(result, ==, HASL_MECHANISM_RESULT_ERROR);

	g_assert_null(client_out);
	g_assert_cmpint(client_out_length, ==, 0);

	g_clear_object(&mechanism);
}

static void
test_hasl_mechanism_plain_possible(HaslContext *context, gboolean expected,
                                   gboolean should_error)
{
	HaslMechanism *mechanism = NULL;
	GError *error = NULL;
	gboolean ret = FALSE;

	mechanism = g_object_new(HASL_TYPE_MECHANISM_PLAIN, NULL);

	ret = hasl_mechanism_possible(mechanism, context, &error);

	if(should_error) {
		g_assert_error(error, HASL_MECHANISM_PLAIN_DOMAIN, 0);
		g_clear_error(&error);
	} else {
		g_assert_no_error(error);
	}

	g_assert_true(ret == expected);

	g_clear_object(&mechanism);
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_hasl_mechanism_plain_new(void) {
	HaslMechanism *mechanism = NULL;

	mechanism = g_object_new(HASL_TYPE_MECHANISM_PLAIN, NULL);

	g_assert_true(HASL_IS_MECHANISM_PLAIN(mechanism));

	g_clear_object(&mechanism);
}

static void
test_hasl_mechanism_plain_username_required_null(void) {
	HaslContext *context = NULL;

	context = hasl_context_new();
	hasl_context_set_password(context, "hunter2");

	test_hasl_mechanism_plain_error_test(context);

	g_clear_object(&context);
}

static void
test_hasl_mechanism_plain_username_required_empty(void) {
	HaslContext *context = NULL;

	context = hasl_context_new();
	hasl_context_set_username(context, "");
	hasl_context_set_password(context, "hunter2");

	test_hasl_mechanism_plain_error_test(context);

	g_clear_object(&context);
}

static void
test_hasl_mechanism_plain_password_required_null(void) {
	HaslContext *context = NULL;

	context = hasl_context_new();
	hasl_context_set_username(context, "alice");

	test_hasl_mechanism_plain_error_test(context);

	g_clear_object(&context);
}

static void
test_hasl_mechanism_plain_password_required_empty(void) {
	HaslContext *context = NULL;

	context = hasl_context_new();
	hasl_context_set_username(context, "alice");
	hasl_context_set_password(context, "");

	test_hasl_mechanism_plain_error_test(context);

	g_clear_object(&context);
}

static void
test_hasl_mechanism_plain_possible_empty_context(void) {
	HaslContext *context = hasl_context_new();

	test_hasl_mechanism_plain_possible(context, FALSE, TRUE);

	g_clear_object(&context);
}

static void
test_hasl_mechanism_plain_possible_username_only(void) {
	HaslContext *context = hasl_context_new();

	hasl_context_set_username(context, "alice");

	test_hasl_mechanism_plain_possible(context, FALSE, TRUE);

	g_clear_object(&context);
}

static void
test_hasl_mechanism_plain_possible_password_only(void) {
	HaslContext *context = hasl_context_new();

	hasl_context_set_password(context, "hunter2");

	test_hasl_mechanism_plain_possible(context, FALSE, TRUE);

	g_clear_object(&context);
}

static void
test_hasl_mechanism_plain_possible_username_password(void) {
	HaslContext *context = hasl_context_new();

	hasl_context_set_username(context, "alice");
	hasl_context_set_password(context, "hunter2");
	hasl_context_set_tls(context, TRUE);

	test_hasl_mechanism_plain_possible(context, TRUE, FALSE);

	g_clear_object(&context);
}

static void
test_hasl_mechanism_plain_possible_authzid_username_password(void) {
	HaslContext *context = hasl_context_new();

	hasl_context_set_authzid(context, "pointy haired boss");
	hasl_context_set_username(context, "alice");
	hasl_context_set_password(context, "hunter2");
	hasl_context_set_tls(context, TRUE);

	test_hasl_mechanism_plain_possible(context, TRUE, FALSE);

	g_clear_object(&context);
}

static void
test_hasl_mechanism_plain_possible_allow_plain_in_clear_without_tls(void) {
	HaslContext *context = hasl_context_new();

	hasl_context_set_username(context, "alice");
	hasl_context_set_password(context, "hunter2");
	hasl_context_set_allow_clear_text(context, TRUE);

	test_hasl_mechanism_plain_possible(context, TRUE, FALSE);

	g_clear_object(&context);
}

static void
test_hasl_mechanism_plain_possible_allow_plain_in_clear_with_tls(void) {
	HaslContext *context = hasl_context_new();

	hasl_context_set_username(context, "alice");
	hasl_context_set_password(context, "hunter2");
	hasl_context_set_allow_clear_text(context, TRUE);
	hasl_context_set_tls(context, TRUE);

	test_hasl_mechanism_plain_possible(context, TRUE, FALSE);

	g_clear_object(&context);
}

static void
test_hasl_mechanism_plain_possible_allow_plain_in_clear_required(void) {
	HaslContext *context = hasl_context_new();

	hasl_context_set_username(context, "alice");
	hasl_context_set_password(context, "hunter2");
	hasl_context_set_tls(context, FALSE);

	test_hasl_mechanism_plain_possible(context, FALSE, TRUE);

	g_clear_object(&context);
}

static void
test_hasl_mechanism_plain_step_rfc4616_example1(void) {
	HaslContext *context = NULL;
	HaslMechanism *mechanism = NULL;
	HaslMechanismResult result = 0;
	GError *error = NULL;
	guint8 *client_out = NULL;
	gsize client_out_length = 0;

	context = hasl_context_new();
	hasl_context_set_username(context, "tim");
	hasl_context_set_password(context, "tanstaaftanstaaf");

	mechanism = g_object_new(HASL_TYPE_MECHANISM_PLAIN, NULL);

	result = hasl_mechanism_step(mechanism, context, NULL, 0, &client_out,
	                             &client_out_length, &error);

	g_assert_no_error(error);
	g_assert_cmpint(result, ==, HASL_MECHANISM_RESULT_SUCCESS);

	g_assert_cmpmem(client_out, client_out_length,
	                "\0tim\0tanstaaftanstaaf", 21);

	g_free(client_out);
	g_clear_object(&context);
	g_clear_object(&mechanism);
}

static void
test_hasl_mechanism_plain_step_with_authzid(void) {
	HaslContext *context = NULL;
	HaslMechanism *mechanism = NULL;
	HaslMechanismResult result = 0;
	GError *error = NULL;
	guint8 *client_out = NULL;
	gsize client_out_length = 0;

	context = hasl_context_new();
	hasl_context_set_authzid(context, "Ursel");
	hasl_context_set_username(context, "Kurt");
	hasl_context_set_password(context, "xipj3plmq");

	mechanism = g_object_new(HASL_TYPE_MECHANISM_PLAIN, NULL);

	result = hasl_mechanism_step(mechanism, context, NULL, 0, &client_out,
	                             &client_out_length, &error);

	g_assert_no_error(error);
	g_assert_cmpint(result, ==, HASL_MECHANISM_RESULT_SUCCESS);

	g_assert_cmpmem(client_out, client_out_length,
	                "Ursel\0Kurt\0xipj3plmq", 20);

	g_free(client_out);
	g_clear_object(&context);
	g_clear_object(&mechanism);
}

/******************************************************************************
 * Main
 *****************************************************************************/
int
main(int argc, char *argv[]) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/hasl/mechanism-plain/new",
	                test_hasl_mechanism_plain_new);

	g_test_add_func("/hasl/mechanism-plain/username-required/null",
	                test_hasl_mechanism_plain_username_required_null);
	g_test_add_func("/hasl/mechanism-plain/username-required/empty",
	                test_hasl_mechanism_plain_username_required_empty);

	g_test_add_func("/hasl/mechanism-plain/password-required/null",
	                test_hasl_mechanism_plain_password_required_null);
	g_test_add_func("/hasl/mechanism-plain/password-required/empty",
	                test_hasl_mechanism_plain_password_required_empty);

	g_test_add_func("/hasl/mechanism-plain/possible/empty-context",
	                test_hasl_mechanism_plain_possible_empty_context);
	g_test_add_func("/hasl/mechanism-plain/possible/username-only",
	                test_hasl_mechanism_plain_possible_username_only);
	g_test_add_func("/hasl/mechanism-plain/possible/password-only",
	                test_hasl_mechanism_plain_possible_password_only);
	g_test_add_func("/hasl/mechanism-plain/possible/username-password",
	                test_hasl_mechanism_plain_possible_username_password);
	g_test_add_func("/hasl/mechanism-plain/possible/authzid-username-password",
	                test_hasl_mechanism_plain_possible_authzid_username_password);
	g_test_add_func("/hasl/mechanism-plain/possible/allow-plain-in-clear-without-tls",
	                test_hasl_mechanism_plain_possible_allow_plain_in_clear_without_tls);
	g_test_add_func("/hasl/mechanism-plain/possible/allow-plain-in-clear-with-tls",
	                test_hasl_mechanism_plain_possible_allow_plain_in_clear_with_tls);
	g_test_add_func("/hasl/mechanism-plain/possible/allow-plain-in-clear-required",
	                test_hasl_mechanism_plain_possible_allow_plain_in_clear_required);

	g_test_add_func("/hasl/mechanism-plain/step/rfc4616-example1",
	                test_hasl_mechanism_plain_step_rfc4616_example1);
	g_test_add_func("/hasl/mechanism-plain/step/with-authzid",
	                test_hasl_mechanism_plain_step_with_authzid);

	return g_test_run();
}
