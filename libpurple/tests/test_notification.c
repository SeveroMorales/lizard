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
 * Helpers
 *****************************************************************************/
static void
test_purple_notification_destory_data_callback(gpointer data) {
	gboolean *called = data;

	*called = TRUE;
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_purple_notification_new(void) {
	PurpleAccount *account1 = NULL, *account2 = NULL;
	PurpleNotification *notification = NULL;
	PurpleNotificationType type = PURPLE_NOTIFICATION_TYPE_UNKNOWN;
	GDateTime *created_timestamp = NULL;
	const gchar *id = NULL;

	account1 = purple_account_new("test", "test");

	notification = purple_notification_new(PURPLE_NOTIFICATION_TYPE_GENERIC,
	                                       account1,
	                                       NULL,
	                                       NULL);

	/* Make sure we got a valid notification. */
	g_assert_true(PURPLE_IS_NOTIFICATION(notification));

	/* Check the type. */
	type = purple_notification_get_notification_type(notification);
	g_assert_cmpint(PURPLE_NOTIFICATION_TYPE_GENERIC, ==, type);

	/* Verify the account is set properly. */
	account2 = purple_notification_get_account(notification);
	g_assert_nonnull(account2);
	g_assert_true(account1 == account2);

	/* Make sure that the id was generated. */
	id = purple_notification_get_id(notification);
	g_assert_nonnull(id);

	/* Make sure that the created-timestamp was set. */
	created_timestamp = purple_notification_get_created_timestamp(notification);
	g_assert_nonnull(created_timestamp);

	/* Unref it to destory it. */
	g_clear_object(&notification);

	/* Clean up the account. */
	g_clear_object(&account1);
}

static void
test_purple_notification_destory_data_func(void) {
	PurpleNotification *notification = NULL;
	gboolean called = FALSE;

	/* Create the notification. */
	notification = purple_notification_new(PURPLE_NOTIFICATION_TYPE_GENERIC,
	                                       NULL,
	                                       &called,
	                                       test_purple_notification_destory_data_callback);

	/* Sanity check. */
	g_assert_true(PURPLE_IS_NOTIFICATION(notification));

	/* Unref it to force the destory callback to be called. */
	g_clear_object(&notification);

	/* Make sure the callback was called. */
	g_assert_true(called);
}

static void
test_purple_notification_properties(void) {
	PurpleNotification *notification = NULL;
	GDateTime *ts1 = NULL, *ts2 = NULL;

	notification = purple_notification_new(PURPLE_NOTIFICATION_TYPE_GENERIC,
	                                       NULL,
	                                       NULL,
	                                       NULL);

	g_assert_true(PURPLE_IS_NOTIFICATION(notification));

	/* Set the timestamp to current utc and verify it was set properly. */
	ts1 = g_date_time_new_now_utc();
	purple_notification_set_created_timestamp(notification, ts1);
	ts2 = purple_notification_get_created_timestamp(notification);
	g_assert_true(g_date_time_equal(ts1, ts2));
	g_date_time_unref(ts1);

	/* Set the title and verify it was set properly. */
	purple_notification_set_title(notification, "title");
	g_assert_true(purple_strequal(purple_notification_get_title(notification),
	                              "title"));

	/* Set the title and verify it was set properly. */
	purple_notification_set_icon_name(notification, "icon-name");
	g_assert_true(purple_strequal(purple_notification_get_icon_name(notification),
	                              "icon-name"));

	/* Set the read state and verify it. */
	purple_notification_set_read(notification, TRUE);
	g_assert_true(purple_notification_get_read(notification));
	purple_notification_set_read(notification, FALSE);
	g_assert_false(purple_notification_get_read(notification));

	/* Set the interactive state and verify it. */
	purple_notification_set_interactive(notification, TRUE);
	g_assert_true(purple_notification_get_interactive(notification));
	purple_notification_set_interactive(notification, FALSE);
	g_assert_false(purple_notification_get_interactive(notification));

	/* Cleanup. */
	g_clear_object(&notification);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	test_ui_purple_init();

	g_test_add_func("/notification/new",
	                test_purple_notification_new);
	g_test_add_func("/notification/destory-data-func",
	                test_purple_notification_destory_data_func);
	g_test_add_func("/notification/properties",
	                test_purple_notification_properties);

	return g_test_run();
}
