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
 * Callbacks
 *****************************************************************************/
static void
test_purple_authorization_request_accepted_counter_cb(G_GNUC_UNUSED PurpleAuthorizationRequest *request,
                                                      gpointer data)
{
	gint *counter = data;

	*counter = *counter + 1;
}

static void
test_purple_authorization_request_denied_counter_cb(G_GNUC_UNUSED PurpleAuthorizationRequest *request,
                                                    G_GNUC_UNUSED const gchar *message,
                                                    gpointer data)
{
	gint *counter = data;

	*counter = *counter + 1;
}

static void
test_purple_authorization_request_denied_message_cb(G_GNUC_UNUSED PurpleAuthorizationRequest *request,
                                                    const gchar *message,
                                                    gpointer data)
{
	gchar *expected = data;

	g_assert_cmpstr(message, ==, expected);
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_purple_authorization_request_new(void) {
	PurpleAccount *account1 = NULL, *account2 = NULL;
	PurpleAuthorizationRequest *request = NULL;
	const gchar *username = NULL;

	account1 = purple_account_new("test", "test");

	request = purple_authorization_request_new(account1, "remote-username");

	/* Make sure we got a valid authorization request. */
	g_assert_true(PURPLE_IS_AUTHORIZATION_REQUEST(request));

	/* Verify the account is set properly. */
	account2 = purple_authorization_request_get_account(request);
	g_assert_nonnull(account2);
	g_assert_true(account1 == account2);

	/* Verify the username set properly. */
	username = purple_authorization_request_get_username(request);
	g_assert_cmpstr(username, ==, "remote-username");

	/* Unref it to destroy it. */
	g_clear_object(&request);

	/* Clean up the account. */
	g_clear_object(&account1);
}

static void
test_purple_authorization_request_properties(void) {
	PurpleAccount *account = NULL;
	PurpleAuthorizationRequest *request = NULL;

	account = purple_account_new("test", "test");
	request = purple_authorization_request_new(account, "username");

	g_assert_true(PURPLE_IS_AUTHORIZATION_REQUEST(request));

	/* Verify the alias property works and is nullable. */
	purple_authorization_request_set_alias(request, "alias");
	g_assert_cmpstr(purple_authorization_request_get_alias(request), ==,
	                "alias");
	purple_authorization_request_set_alias(request, NULL);
	g_assert_null(purple_authorization_request_get_alias(request));

	/* Verify the message property works and is nullable. */
	purple_authorization_request_set_message(request, "message");
	g_assert_cmpstr(purple_authorization_request_get_message(request), ==,
	                "message");
	purple_authorization_request_set_message(request, NULL);
	g_assert_null(purple_authorization_request_get_message(request));

	/* Cleanup. */
	g_clear_object(&request);
	g_clear_object(&account);
}

static void
test_purple_authorization_request_accept(void) {
	if(g_test_subprocess()) {
		PurpleAccount *account = NULL;
		PurpleAuthorizationRequest *request = NULL;
		gint counter = 0;

		account = purple_account_new("test", "test");
		request = purple_authorization_request_new(account, "username");

		g_assert_true(PURPLE_IS_AUTHORIZATION_REQUEST(request));

		g_signal_connect(request, "accepted",
		                 G_CALLBACK(test_purple_authorization_request_accepted_counter_cb),
		                 &counter);

		/* Accept the request and verify that the callback was called. */
		purple_authorization_request_accept(request);
		g_assert_cmpint(counter, ==, 1);

		/* Accept the request again to trigger the critical. */
		purple_authorization_request_accept(request);
		g_assert_cmpint(counter, ==, 1);

		/* Cleanup. */
		g_clear_object(&account);
		g_clear_object(&request);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_stderr("*Purple-CRITICAL*request->handled*failed*");
}

static void
test_purple_authorization_request_accept_deny(void) {
	if(g_test_subprocess()) {
		PurpleAccount *account = NULL;
		PurpleAuthorizationRequest *request = NULL;
		gint counter = 0;

		account = purple_account_new("test", "test");
		request = purple_authorization_request_new(account, "username");

		g_assert_true(PURPLE_IS_AUTHORIZATION_REQUEST(request));

		g_signal_connect(request, "accepted",
		                 G_CALLBACK(test_purple_authorization_request_accepted_counter_cb),
		                 &counter);
		g_signal_connect(request, "denied",
		                 G_CALLBACK(test_purple_authorization_request_denied_counter_cb),
		                 &counter);

		/* Accept the request and verify that the callback was called. */
		purple_authorization_request_accept(request);
		g_assert_cmpint(counter, ==, 1);

		/* Deny the request to trigger the critical. */
		purple_authorization_request_deny(request, NULL);
		g_assert_cmpint(counter, ==, 1);

		/* Cleanup. */
		g_clear_object(&account);
		g_clear_object(&request);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_stderr("*Purple-CRITICAL*request->handled*failed*");
}

static void
test_purple_authorization_request_deny(void) {
	if(g_test_subprocess()) {
		PurpleAccount *account = NULL;
		PurpleAuthorizationRequest *request = NULL;
		gint counter = 0;

		account = purple_account_new("test", "test");
		request = purple_authorization_request_new(account, "username");

		g_assert_true(PURPLE_IS_AUTHORIZATION_REQUEST(request));

		g_signal_connect(request, "denied",
		                 G_CALLBACK(test_purple_authorization_request_denied_counter_cb),
		                 &counter);

		/* Deny the request and verify that the callback was called. */
		purple_authorization_request_deny(request, NULL);
		g_assert_cmpint(counter, ==, 1);

		/* Deny the request again to trigger the critical. */
		purple_authorization_request_deny(request, NULL);
		g_assert_cmpint(counter, ==, 1);

		/* Cleanup. */
		g_clear_object(&account);
		g_clear_object(&request);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_stderr("*Purple-CRITICAL*request->handled*failed*");
}

static void
test_purple_authorization_request_deny_accept(void) {
	if(g_test_subprocess()) {
		PurpleAccount *account = NULL;
		PurpleAuthorizationRequest *request = NULL;
		gint counter = 0;

		account = purple_account_new("test", "test");
		request = purple_authorization_request_new(account, "username");

		g_assert_true(PURPLE_IS_AUTHORIZATION_REQUEST(request));

		g_signal_connect(request, "denied",
		                 G_CALLBACK(test_purple_authorization_request_denied_counter_cb),
		                 &counter);
		g_signal_connect(request, "accepted",
		                 G_CALLBACK(test_purple_authorization_request_accepted_counter_cb),
		                 &counter);

		/* Deny the request and verify that the callback was called. */
		purple_authorization_request_deny(request, NULL);
		g_assert_cmpint(counter, ==, 1);

		/* Deny the request again to trigger the critical. */
		purple_authorization_request_accept(request);
		g_assert_cmpint(counter, ==, 1);

		/* Cleanup. */
		g_clear_object(&account);
		g_clear_object(&request);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_stderr("*Purple-CRITICAL*request->handled*failed*");
}

static void
test_purple_authorization_request_deny_message_null(void) {
	PurpleAccount *account = NULL;
	PurpleAuthorizationRequest *request = NULL;

	account = purple_account_new("test", "test");
	request = purple_authorization_request_new(account, "username");

	g_assert_true(PURPLE_IS_AUTHORIZATION_REQUEST(request));

	g_signal_connect(request, "denied",
	                 G_CALLBACK(test_purple_authorization_request_denied_message_cb),
	                 NULL);

	/* Deny the request the signal handler is expecting a message of NULL. */
	purple_authorization_request_deny(request, NULL);

	/* Cleanup. */
	g_clear_object(&account);
	g_clear_object(&request);
}

static void
test_purple_authorization_request_deny_message_non_null(void) {
	PurpleAccount *account = NULL;
	PurpleAuthorizationRequest *request = NULL;

	account = purple_account_new("test", "test");
	request = purple_authorization_request_new(account, "username");

	g_assert_true(PURPLE_IS_AUTHORIZATION_REQUEST(request));

	g_signal_connect(request, "denied",
	                 G_CALLBACK(test_purple_authorization_request_denied_message_cb),
	                 "this is a message");

	/* Deny the request the signal handler is expecting the above message. */
	purple_authorization_request_deny(request, "this is a message");

	/* Cleanup. */
	g_clear_object(&account);
	g_clear_object(&request);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	test_ui_purple_init();

	g_test_add_func("/request-authorization/new",
	                test_purple_authorization_request_new);
	g_test_add_func("/request-authorization/properties",
	                test_purple_authorization_request_properties);

	g_test_add_func("/request-authorization/accept",
	                test_purple_authorization_request_accept);
	g_test_add_func("/request-authorization/accept-deny",
	                test_purple_authorization_request_accept_deny);
	g_test_add_func("/request-authorization/deny",
	                test_purple_authorization_request_deny);
	g_test_add_func("/request-authorization/deny-accept",
	                test_purple_authorization_request_deny_accept);

	g_test_add_func("/request-authorization/deny-message/null",
	                test_purple_authorization_request_deny_message_null);
	g_test_add_func("/request-authorization/deny-message/non-null",
	                test_purple_authorization_request_deny_message_non_null);

	return g_test_run();
}
