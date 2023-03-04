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
test_hasl_mechanism_external_helper(HaslContext *context,
                                    const char *expected)
{
	HaslMechanism *mechanism = NULL;
	HaslMechanismResult result = 0;
	GError *error = NULL;
	guint8 *client_out = NULL;
	gsize client_out_length = 0;
	gsize expected_length = 0;

	if(expected != NULL) {
		expected_length = strlen(expected);
	}

	mechanism = g_object_new(HASL_TYPE_MECHANISM_EXTERNAL, NULL);

	result = hasl_mechanism_step(mechanism, context, NULL, 0, &client_out,
	                             &client_out_length, &error);

	g_assert_no_error(error);
	g_assert_cmpint(result, ==, HASL_MECHANISM_RESULT_SUCCESS);

	g_assert_cmpmem(client_out, client_out_length, expected, expected_length);

	g_free(client_out);
	g_clear_object(&mechanism);
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_hasl_mechanism_external_new(void) {
	HaslMechanism *mechanism = NULL;

	mechanism = g_object_new(HASL_TYPE_MECHANISM_EXTERNAL, NULL);

	g_assert_true(HASL_IS_MECHANISM_EXTERNAL(mechanism));

	g_clear_object(&mechanism);
}

static void
test_hasl_mechanism_external_authzid_null(void) {
	HaslContext *context = hasl_context_new();

	test_hasl_mechanism_external_helper(context, NULL);

	g_clear_object(&context);
}

static void
test_hasl_mechanism_external_authzid_empty(void) {
	HaslContext *context = hasl_context_new();

	hasl_context_set_authzid(context, "");

	test_hasl_mechanism_external_helper(context, "");

	g_clear_object(&context);
}

static void
test_hasl_mechanism_external_authzid_value(void) {
	HaslContext *context = hasl_context_new();

	hasl_context_set_authzid(context, "pidgy");

	test_hasl_mechanism_external_helper(context, "pidgy");

	g_clear_object(&context);
}

/******************************************************************************
 * Main
 *****************************************************************************/
int
main(int argc, char *argv[]) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/hasl/mechanism-external/new",
	                test_hasl_mechanism_external_new);

	g_test_add_func("/hasl/mechanism-external/authzid/null",
	                test_hasl_mechanism_external_authzid_null);
	g_test_add_func("/hasl/mechanism-external/authzid/empty",
	                test_hasl_mechanism_external_authzid_empty);
	g_test_add_func("/hasl/mechanism-external/authzid/value",
	                test_hasl_mechanism_external_authzid_value);

	return g_test_run();
}
