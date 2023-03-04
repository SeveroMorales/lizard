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

#define TEST_HASL_DOMAIN (g_quark_from_static_string("test-hasl"))

/******************************************************************************
 * Mechanism Implementation
 *****************************************************************************/
G_DECLARE_FINAL_TYPE(TestHaslMechanism, test_hasl_mechanism, TEST_HASL,
                     MECHANISM, HaslMechanism)

struct _TestHaslMechanism {
	HaslMechanism parent;

	gboolean possible;
	GError *possible_error;
};

G_DEFINE_TYPE(TestHaslMechanism, test_hasl_mechanism, HASL_TYPE_MECHANISM)

static gboolean
test_hasl_mechanism_possible(HaslMechanism *mechanism,
                             G_GNUC_UNUSED HaslContext *ctx,
                             GError **error)
{
	TestHaslMechanism *test_mechanism = (TestHaslMechanism *)mechanism;

	if(test_mechanism->possible_error != NULL) {
		g_propagate_error(error, test_mechanism->possible_error);
	}

	return test_mechanism->possible;
}

static HaslMechanismResult
test_hasl_mechanism_step(HaslMechanism *mechanism, HaslContext *ctx,
                         const guint8 *server_in, gsize server_in_length,
                         guint8 **client_out, gsize *client_out_length,
                         GError **error)
{
	g_assert_true(TEST_HASL_IS_MECHANISM(mechanism));
	g_assert_true(HASL_IS_CONTEXT(ctx));

	g_assert_cmpstr((const char *)server_in, ==, "server-in");
	g_assert_cmpint(server_in_length, ==, strlen("server-in"));

	g_assert_nonnull(client_out);
	g_assert_nonnull(client_out_length);
	g_assert_nonnull(error);

	*client_out = (guint8 *)g_strdup("client-out");
	*client_out_length = strlen("client-out");

	g_set_error(error, TEST_HASL_DOMAIN, 0, "this is an error");

	return HASL_MECHANISM_RESULT_ERROR;
}

static void
test_hasl_mechanism_init(G_GNUC_UNUSED TestHaslMechanism *mechanism) {
}

static void
test_hasl_mechanism_class_init(TestHaslMechanismClass *klass) {
	HaslMechanismClass *mechanism_class = HASL_MECHANISM_CLASS(klass);

	mechanism_class->possible = test_hasl_mechanism_possible;
	mechanism_class->step = test_hasl_mechanism_step;
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_hasl_mechanism_implementation_possible_true(void) {
	TestHaslMechanism *test_mechanism = NULL;
	HaslMechanism *mechanism = NULL;
	HaslContext *ctx = NULL;
	GError *error = NULL;
	gboolean ret = FALSE;

	mechanism = g_object_new(test_hasl_mechanism_get_type(), NULL);
	test_mechanism = TEST_HASL_MECHANISM(mechanism);
	test_mechanism->possible = TRUE;

	ctx = hasl_context_new();

	ret = hasl_mechanism_possible(mechanism, ctx, &error);
	g_assert_no_error(error);
	g_assert_true(ret);

	g_clear_object(&mechanism);
	g_clear_object(&ctx);
}

static void
test_hasl_mechanism_implementation_possible_false(void) {
	TestHaslMechanism *test_mechanism = NULL;
	HaslMechanism *mechanism = NULL;
	HaslContext *ctx = NULL;
	GError *error = NULL;
	gboolean ret = FALSE;

	mechanism = g_object_new(test_hasl_mechanism_get_type(), NULL);

	test_mechanism = TEST_HASL_MECHANISM(mechanism);
	test_mechanism->possible = FALSE;
	test_mechanism->possible_error = g_error_new(TEST_HASL_DOMAIN, 0, "error");

	ctx = hasl_context_new();

	ret = hasl_mechanism_possible(mechanism, ctx, &error);

	g_assert_error(error, TEST_HASL_DOMAIN, 0);
	g_clear_error(&error);

	g_assert_false(ret);

	g_clear_object(&mechanism);
	g_clear_object(&ctx);
}

static void
test_hasl_mechanism_implementation_step(void) {
	HaslContext *ctx = NULL;
	HaslMechanism *mechanism = NULL;
	GError *error = NULL;
	guint8 *client_out = NULL;
	gsize client_out_length;
	gboolean ret = FALSE;

	ctx = hasl_context_new();
	g_assert_true(HASL_IS_CONTEXT(ctx));

	mechanism = g_object_new(test_hasl_mechanism_get_type(), NULL);
	g_assert_true(HASL_IS_MECHANISM(mechanism));

	ret = hasl_mechanism_step(mechanism, ctx,
	                          (guint8 *)"server-in", strlen("server-in"),
	                          &client_out, &client_out_length, &error);
	g_assert_error(error, g_quark_from_static_string("test-hasl"), 0);
	g_clear_error(&error);

	g_assert_cmpint(ret, ==, HASL_MECHANISM_RESULT_ERROR);

	g_assert_nonnull(client_out);
	g_assert_cmpstr((char *)client_out, ==, "client-out");
	g_free(client_out);

	g_assert_cmpint(client_out_length, ==, strlen("client-out"));

	g_clear_object(&ctx);
	g_clear_object(&mechanism);
}

static void
test_hasl_mechanism_no_implementation_possible(void) {
	HaslMechanism *mechanism = NULL;
	HaslContext *context = NULL;
	GError *error = NULL;
	gboolean ret = FALSE;

	mechanism = g_object_new(HASL_TYPE_MECHANISM, NULL);
	context = hasl_context_new();

	ret = hasl_mechanism_possible(mechanism, context, &error);

	g_assert_no_error(error);
	g_assert_true(ret);

	g_clear_object(&context);
	g_clear_object(&mechanism);
}

static void
test_hasl_mechanism_no_implementation_step(void) {
	HaslContext *ctx = hasl_context_new();
	HaslMechanism *mechanism = g_object_new(HASL_TYPE_MECHANISM, NULL);
	GError *error = NULL;
	guint8 *client_out = NULL;
	gboolean ret = FALSE;

	ret = hasl_mechanism_step(mechanism, ctx, NULL, -1, &client_out, NULL,
	                          &error);
	g_assert_no_error(error);
	g_assert_cmpint(ret, ==, HASL_MECHANISM_RESULT_ERROR);

	g_assert_null(client_out);

	g_clear_object(&ctx);
	g_clear_object(&mechanism);
}

/******************************************************************************
 * Main
 *****************************************************************************/
int
main(int argc, char *argv[]) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/hasl/mechanism/implementation/possible/true",
	                test_hasl_mechanism_implementation_possible_true);
	g_test_add_func("/hasl/mechanism/implementation/possible/false",
	                test_hasl_mechanism_implementation_possible_false);
	g_test_add_func("/hasl/mechanism/implementation/step",
	                test_hasl_mechanism_implementation_step);

	g_test_add_func("/hasl/mechanism/no-implementation/possible",
	                test_hasl_mechanism_no_implementation_possible);
	g_test_add_func("/hasl/mechanism/no-implementation/step",
	                test_hasl_mechanism_no_implementation_step);

	return g_test_run();
}
