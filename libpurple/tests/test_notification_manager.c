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
test_purple_notification_manager_increment_cb(G_GNUC_UNUSED PurpleNotificationManager *manager,
                                              G_GNUC_UNUSED PurpleNotification *notification,
                                              gpointer data)
{
	gint *called = data;

	*called = *called + 1;
}

static void
test_purple_notification_manager_unread_count_cb(G_GNUC_UNUSED GObject *obj,
                                                 G_GNUC_UNUSED GParamSpec *pspec,
                                                 gpointer data)
{
	gint *called = data;

	*called = *called + 1;
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_purple_notification_manager_get_default(void) {
	PurpleNotificationManager *manager1 = NULL, *manager2 = NULL;

	manager1 = purple_notification_manager_get_default();
	g_assert_true(PURPLE_IS_NOTIFICATION_MANAGER(manager1));

	manager2 = purple_notification_manager_get_default();
	g_assert_true(PURPLE_IS_NOTIFICATION_MANAGER(manager2));

	g_assert_true(manager1 == manager2);
}

static void
test_purple_notification_manager_add_remove(void) {
	PurpleNotificationManager *manager = NULL;
	PurpleNotification *notification = NULL;
	gint added_called = 0, removed_called = 0;
	guint unread_count = 0;

	manager = g_object_new(PURPLE_TYPE_NOTIFICATION_MANAGER, NULL);

	g_assert_true(PURPLE_IS_NOTIFICATION_MANAGER(manager));

	/* Wire up our signals. */
	g_signal_connect(manager, "added",
	                 G_CALLBACK(test_purple_notification_manager_increment_cb),
	                 &added_called);
	g_signal_connect(manager, "removed",
	                 G_CALLBACK(test_purple_notification_manager_increment_cb),
	                 &removed_called);

	/* Create the notification and store it's id. */
	notification = purple_notification_new(PURPLE_NOTIFICATION_TYPE_GENERIC,
	                                       NULL, NULL, NULL);

	/* Add the notification to the manager. */
	purple_notification_manager_add(manager, notification);

	/* Make sure the added signal was called. */
	g_assert_cmpint(added_called, ==, 1);

	/* Verify that the unread count is 1. */
	unread_count = purple_notification_manager_get_unread_count(manager);
	g_assert_cmpint(unread_count, ==, 1);

	/* Remove the notification. */
	purple_notification_manager_remove(manager, notification);
	g_assert_cmpint(removed_called, ==, 1);

	/* Verify that the unread count is now 0. */
	unread_count = purple_notification_manager_get_unread_count(manager);
	g_assert_cmpint(unread_count, ==, 0);

	/* Clean up the manager. */
	g_clear_object(&manager);
}

static void
test_purple_notification_manager_double_add(void) {
	if(g_test_subprocess()) {
		PurpleNotificationManager *manager = NULL;
		PurpleNotification *notification = NULL;

		manager = g_object_new(PURPLE_TYPE_NOTIFICATION_MANAGER, NULL);

		notification = purple_notification_new(PURPLE_NOTIFICATION_TYPE_GENERIC,
		                                       NULL, NULL, NULL);

		purple_notification_manager_add(manager, notification);
		purple_notification_manager_add(manager, notification);

		/* This will never get called as the double add outputs a g_warning()
		 * that causes the test to fail. This is left to avoid a false postive
		 * in static analysis.
		 */
		g_clear_object(&manager);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_stderr("*Purple-WARNING*double add detected for notification*");
}

static void
test_purple_notification_manager_double_remove(void) {
	PurpleNotificationManager *manager = NULL;
	PurpleNotification *notification = NULL;
	gint removed_called = 0;

	manager = g_object_new(PURPLE_TYPE_NOTIFICATION_MANAGER, NULL);
	g_signal_connect(manager, "removed",
	                 G_CALLBACK(test_purple_notification_manager_increment_cb),
	                 &removed_called);

	notification = purple_notification_new(PURPLE_NOTIFICATION_TYPE_GENERIC,
	                                       NULL, NULL, NULL);
	/* Add an additional reference because the manager takes one and the id
	 * belongs to the notification. So without this, the first remove frees
	 * the id which would cause an invalid read.
	 */
	g_object_ref(notification);

	purple_notification_manager_add(manager, notification);

	purple_notification_manager_remove(manager, notification);
	purple_notification_manager_remove(manager, notification);

	g_assert_cmpint(removed_called, ==, 1);

	g_clear_object(&notification);
	g_clear_object(&manager);
}

static void
test_purple_notification_manager_remove_with_account_simple(void) {
	PurpleNotificationManager *manager = NULL;
	PurpleNotification *notification = NULL;
	PurpleAccount *account = NULL;
	GListModel *model = NULL;

	manager = g_object_new(PURPLE_TYPE_NOTIFICATION_MANAGER, NULL);
	model = G_LIST_MODEL(manager);
	account = purple_account_new("test", "test");

	/* Make sure that nothing happens on an empty list. */
	purple_notification_manager_remove_with_account(manager, account, TRUE);
	g_assert_cmpuint(0, ==, g_list_model_get_n_items(model));

	/* Add a single notification without the account */
	notification = purple_notification_new(PURPLE_NOTIFICATION_TYPE_GENERIC,
	                                       NULL, NULL, NULL);
	purple_notification_manager_add(manager, notification);
	purple_notification_manager_remove_with_account(manager, account, TRUE);
	g_assert_cmpuint(1, ==, g_list_model_get_n_items(model));
	purple_notification_manager_clear(manager);
	g_clear_object(&notification);

	/* Add a single notification with the account */
	notification = purple_notification_new(PURPLE_NOTIFICATION_TYPE_GENERIC,
	                                       account, NULL, NULL);
	purple_notification_manager_add(manager, notification);
	purple_notification_manager_remove_with_account(manager, account, TRUE);
	g_assert_cmpuint(0, ==, g_list_model_get_n_items(model));
	purple_notification_manager_clear(manager);
	g_clear_object(&notification);

	g_clear_object(&manager);
	g_clear_object(&account);
}

static void
test_purple_notification_manager_remove_with_account_mixed(void) {
	PurpleNotificationManager *manager = NULL;
	PurpleNotification *notification = NULL;
	PurpleAccount *accounts[3];
	GListModel *model = NULL;
	gint pattern[] = {0, 0, 1, 0, 2, 1, 0, 0, 1, 2, 0, 1, 0, 0, -1};
	gint i = 0;

	manager = g_object_new(PURPLE_TYPE_NOTIFICATION_MANAGER, NULL);
	model = G_LIST_MODEL(manager);

	accounts[0] = purple_account_new("account1", "account1");
	accounts[1] = purple_account_new("account2", "account2");
	accounts[2] = purple_account_new("account3", "account3");

	/* Add our notifications. */
	for(i = 0; pattern[i] >= 0; i++) {
		notification = purple_notification_new(PURPLE_NOTIFICATION_TYPE_GENERIC,
		                                       accounts[pattern[i]], NULL,
		                                       NULL);
		purple_notification_manager_add(manager, notification);
		g_clear_object(&notification);
	}

	g_assert_cmpuint(14, ==, g_list_model_get_n_items(model));

	/* Remove notifications for accounts[0]. */
	purple_notification_manager_remove_with_account(manager, accounts[0], TRUE);
	g_assert_cmpuint(6, ==, g_list_model_get_n_items(model));

	/* Remove notifications for accounts[1]. */
	purple_notification_manager_remove_with_account(manager, accounts[1], TRUE);
	g_assert_cmpuint(2, ==, g_list_model_get_n_items(model));

	/* Remove notifications for accounts[2]. */
	purple_notification_manager_remove_with_account(manager, accounts[2], TRUE);
	g_assert_cmpuint(0, ==, g_list_model_get_n_items(model));

	g_clear_object(&manager);
	g_clear_object(&accounts[0]);
	g_clear_object(&accounts[1]);
	g_clear_object(&accounts[2]);
}

static void
test_purple_notification_manager_remove_with_account_all(void) {
	PurpleNotificationManager *manager = NULL;
	PurpleNotification *notification = NULL;
	PurpleAccount *account = NULL;
	GListModel *model = NULL;

	/* This test will add 3 notifications to the notification manager for the
	 * same account. In order, they will be of types generic, connection error,
	 * and generic. We will also add a generic notification with no account, to
	 * make sure notifications don't accidentally get removed.
	 *
	 * Since the notification manager prepends items and leaves sorting up to
	 * the user interface, the order of the notifications will be the opposite
	 * of the order they were added.
	 *
	 * This ordering is specifically done because we batch remove items from
	 * the list. So by calling purple_notification_manager_remove_with_account
	 * with `all` set to FALSE, we should remove the second and fourth items,
	 * but leave the first and third items.
	 *
	 * We then call purple_notification_manager_remove_with_account with `all`
	 * set to TRUE, which will remove the connection error notification which
	 * is now the second item.
	 *
	 * Finally, we empty the manager and verify that its count is at 0.
	 */

	manager = g_object_new(PURPLE_TYPE_NOTIFICATION_MANAGER, NULL);
	model = G_LIST_MODEL(manager);
	account = purple_account_new("test", "test");

	/* Make sure that nothing happens on an empty list. */
	purple_notification_manager_remove_with_account(manager, account, TRUE);
	g_assert_cmpuint(0, ==, g_list_model_get_n_items(model));

	/* Add a generic notification with an account. */
	notification = purple_notification_new(PURPLE_NOTIFICATION_TYPE_GENERIC,
	                                       account, NULL, NULL);
	purple_notification_manager_add(manager, notification);
	g_clear_object(&notification);

	/* Add a connection error notification with the account. */
	notification = purple_notification_new(PURPLE_NOTIFICATION_TYPE_CONNECTION_ERROR,
	                                       account, NULL, NULL);
	purple_notification_manager_add(manager, notification);
	g_clear_object(&notification);

	/* Add a generic notification with the account. */
	notification = purple_notification_new(PURPLE_NOTIFICATION_TYPE_GENERIC,
	                                       account, NULL, NULL);
	purple_notification_manager_add(manager, notification);
	g_clear_object(&notification);

	/* Add a generic notification without an account. */
	notification = purple_notification_new(PURPLE_NOTIFICATION_TYPE_GENERIC,
	                                       NULL, NULL, NULL);
	purple_notification_manager_add(manager, notification);
	g_clear_object(&notification);

	/* Verify that we have all of the notifications in the manager. */
	g_assert_cmpuint(4, ==, g_list_model_get_n_items(model));

	/* Remove the transient notifications for the account. */
	purple_notification_manager_remove_with_account(manager, account, FALSE);
	g_assert_cmpuint(2, ==, g_list_model_get_n_items(model));

	/* Make sure that the second item is the connection error. */
	notification = g_list_model_get_item(G_LIST_MODEL(model), 1);
	g_assert_cmpint(purple_notification_get_notification_type(notification),
	                ==, PURPLE_NOTIFICATION_TYPE_CONNECTION_ERROR);
	g_clear_object(&notification);

	/* Remove the non-transient notifications for the account. */
	purple_notification_manager_remove_with_account(manager, account, TRUE);
	g_assert_cmpuint(1, ==, g_list_model_get_n_items(model));

	/* Remove the generic notification that's not tied to an account. */
	purple_notification_manager_clear(manager);
	g_assert_cmpuint(0, ==, g_list_model_get_n_items(model));

	g_clear_object(&manager);
	g_clear_object(&account);
}

static void
test_purple_notification_manager_read_propagation(void) {
	PurpleNotificationManager *manager = NULL;
	PurpleNotification *notification = NULL;
	gint read_called = 0, unread_called = 0, unread_count_called = 0;

	/* Create the manager. */
	manager = g_object_new(PURPLE_TYPE_NOTIFICATION_MANAGER, NULL);

	g_signal_connect(manager, "read",
	                 G_CALLBACK(test_purple_notification_manager_increment_cb),
	                 &read_called);
	g_signal_connect(manager, "unread",
	                 G_CALLBACK(test_purple_notification_manager_increment_cb),
	                 &unread_called);
	g_signal_connect(manager, "notify::unread-count",
	                 G_CALLBACK(test_purple_notification_manager_unread_count_cb),
	                 &unread_count_called);

	/* Create the notification and add a reference to it before we give our
	 * original refernce to the manager.
	 */
	notification = purple_notification_new(PURPLE_NOTIFICATION_TYPE_GENERIC,
	                                       NULL,
	                                       NULL,
	                                       NULL);

	g_object_ref(notification);

	purple_notification_manager_add(manager, notification);

	/* Verify that the read and unread signals were not yet emitted. */
	g_assert_cmpint(read_called, ==, 0);
	g_assert_cmpint(unread_called, ==, 0);

	/* Verify that the unread_count property changed. */
	g_assert_cmpint(unread_count_called, ==, 1);

	/* Now mark the notification as read. */
	purple_notification_set_read(notification, TRUE);

	g_assert_cmpint(read_called, ==, 1);
	g_assert_cmpint(unread_called, ==, 0);

	g_assert_cmpint(unread_count_called, ==, 2);

	/* Now mark the notification as unread. */
	purple_notification_set_read(notification, FALSE);

	g_assert_cmpint(read_called, ==, 1);
	g_assert_cmpint(unread_called, ==, 1);

	g_assert_cmpint(unread_count_called, ==, 3);

	/* Cleanup. */
	g_clear_object(&notification);
	g_clear_object(&manager);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	GMainLoop *loop = NULL;
	gint ret = 0;

	g_test_init(&argc, &argv, NULL);

	test_ui_purple_init();

	loop = g_main_loop_new(NULL, FALSE);

	g_test_add_func("/notification-manager/get-default",
	                test_purple_notification_manager_get_default);
	g_test_add_func("/notification-manager/add-remove",
	                test_purple_notification_manager_add_remove);
	g_test_add_func("/notification-manager/double-add",
	                test_purple_notification_manager_double_add);
	g_test_add_func("/notification-manager/double-remove",
	                test_purple_notification_manager_double_remove);

	g_test_add_func("/notification-manager/remove-with-account/simple",
	                test_purple_notification_manager_remove_with_account_simple);
	g_test_add_func("/notification-manager/remove-with-account/mixed",
	                test_purple_notification_manager_remove_with_account_mixed);
	g_test_add_func("/notification-manager/remove-with-account/all",
	                test_purple_notification_manager_remove_with_account_all);

	g_test_add_func("/notification-manager/read-propagation",
	                test_purple_notification_manager_read_propagation);

	ret = g_test_run();

	g_main_loop_unref(loop);

	return ret;
}
