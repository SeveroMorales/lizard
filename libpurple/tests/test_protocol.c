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

#include <purple.h>

#include "test_ui.h"

/******************************************************************************
 * Globals
 *****************************************************************************/

/* Since we're using GTask to test asynchronous functions, we need to use a
 * main loop.
 */
static GMainLoop *loop = NULL;

#define TEST_PROTOCOL_DOMAIN (g_quark_from_static_string("test-protocol"))

/******************************************************************************
 * TestPurpleProtocol
 *****************************************************************************/
#define TEST_PURPLE_TYPE_PROTOCOL (test_purple_protocol_get_type())
G_DECLARE_FINAL_TYPE(TestPurpleProtocol,
                     test_purple_protocol,
                     TEST_PURPLE, PROTOCOL,
                     PurpleProtocol)

struct _TestPurpleProtocol {
	PurpleProtocol parent;

	gboolean result;
	GError *error;

	gboolean can_connect_async;
	gboolean can_connect_finish;
};

G_DEFINE_TYPE(TestPurpleProtocol, test_purple_protocol, PURPLE_TYPE_PROTOCOL)

static void
test_purple_protocol_can_connect_async(PurpleProtocol *protocol,
                                       G_GNUC_UNUSED PurpleAccount *account,
                                       GCancellable *cancellable,
                                       GAsyncReadyCallback callback,
                                       gpointer data)
{
	TestPurpleProtocol *test_protocol = TEST_PURPLE_PROTOCOL(protocol);
	GTask *task = NULL;

	task = g_task_new(protocol, cancellable, callback, data);
	if(test_protocol->error != NULL) {
		g_task_return_error(task, test_protocol->error);
	} else {
		g_task_return_boolean(task, test_protocol->result);
	}
	g_clear_object(&task);

	test_protocol->can_connect_async = TRUE;
}

static gboolean
test_purple_protocol_can_connect_finish(PurpleProtocol *protocol,
                                        GAsyncResult *result,
                                        GError **error)
{
	TestPurpleProtocol *test_protocol = TEST_PURPLE_PROTOCOL(protocol);

	test_protocol->can_connect_finish = TRUE;

	return g_task_propagate_boolean(G_TASK(result), error);
}

static void
test_purple_protocol_finalize(GObject *obj) {
	TestPurpleProtocol *protocol = TEST_PURPLE_PROTOCOL(obj);

	g_clear_error(&protocol->error);

	G_OBJECT_CLASS(test_purple_protocol_parent_class)->finalize(obj);
}

static void
test_purple_protocol_init(G_GNUC_UNUSED TestPurpleProtocol *provider) {
}

static void
test_purple_protocol_class_init(TestPurpleProtocolClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	PurpleProtocolClass *protocol_class = PURPLE_PROTOCOL_CLASS(klass);

	obj_class->finalize = test_purple_protocol_finalize;

	protocol_class->can_connect_async = test_purple_protocol_can_connect_async;
	protocol_class->can_connect_finish = test_purple_protocol_can_connect_finish;
}

static TestPurpleProtocol *
test_purple_protocol_new(gboolean result, GError *error) {
	TestPurpleProtocol *test_protocol = NULL;

	test_protocol = g_object_new(
		TEST_PURPLE_TYPE_PROTOCOL,
		"id", "test-provider",
		"name", "Test Provider",
		NULL);

	test_protocol->result = result;
	test_protocol->error = error;

	return test_protocol;
}

/******************************************************************************
 * TestPurpleProtocol->can_connect Tests
 *****************************************************************************/
static gboolean
test_purple_protocol_timeout_cb(gpointer data) {
	g_main_loop_quit((GMainLoop *)data);

	g_warning("timed out waiting for the callback function to be called");

	return FALSE;
}

static void
test_purple_protocol_can_connect_cb(GObject *obj, GAsyncResult *res,
                                    gpointer data)
{
	PurpleProtocol *protocol = PURPLE_PROTOCOL(obj);
	TestPurpleProtocol *test_protocol = TEST_PURPLE_PROTOCOL(protocol);
	GError *error = NULL;
	gboolean result = FALSE;

	result = purple_protocol_can_connect_finish(protocol, res, &error);

	g_main_loop_quit(loop);

	if(test_protocol->error != NULL) {
		g_assert_error(error, TEST_PROTOCOL_DOMAIN, 0);
	} else {
		g_assert_no_error(error);
	}

	g_assert_true(result == test_protocol->result);

	/* This is the account that we need to unref. */
	g_object_unref(data);
}

static gboolean
test_purple_protocol_can_connect_idle(gpointer data) {
	PurpleProtocol *p = PURPLE_PROTOCOL(data);
	PurpleAccount *account = NULL;

	account = purple_account_new("test", "test");

	purple_protocol_can_connect_async(p, account, NULL,
	                                  test_purple_protocol_can_connect_cb,
	                                  account);

	return G_SOURCE_REMOVE;
}

static void
test_purple_protocol_can_connect(TestPurpleProtocol *test_protocol) {
	g_idle_add(test_purple_protocol_can_connect_idle, test_protocol);
	g_timeout_add_seconds(100, test_purple_protocol_timeout_cb, loop);

	g_main_loop_run(loop);

	g_assert_true(test_protocol->can_connect_async);
	g_assert_true(test_protocol->can_connect_finish);

	g_object_unref(test_protocol);
}

static void
test_purple_protocol_can_connect_error(void) {
	TestPurpleProtocol *protocol = NULL;
	GError *error = NULL;

	error = g_error_new(TEST_PROTOCOL_DOMAIN, 0, "no network");
	protocol = test_purple_protocol_new(FALSE, error);

	test_purple_protocol_can_connect(protocol);
}

static void
test_purple_protocol_can_connect_false(void) {
	TestPurpleProtocol *test_protocol = test_purple_protocol_new(FALSE, NULL);

	test_purple_protocol_can_connect(test_protocol);
}

static void
test_purple_protocol_can_connect_true(void) {
	TestPurpleProtocol *test_protocol = test_purple_protocol_new(TRUE, NULL);

	test_purple_protocol_can_connect(test_protocol);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	gint ret = 0;

	g_test_init(&argc, &argv, NULL);

	test_ui_purple_init();

	loop = g_main_loop_new(NULL, FALSE);

	g_test_add_func("/protocol/can-connect/error",
	                test_purple_protocol_can_connect_error);
	g_test_add_func("/protocol/can-connect/false",
	                test_purple_protocol_can_connect_false);
	g_test_add_func("/protocol/can-connect/true",
	                test_purple_protocol_can_connect_true);

	ret = g_test_run();

	g_main_loop_unref(loop);

	return ret;
}
