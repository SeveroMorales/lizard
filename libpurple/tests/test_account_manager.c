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

#define PURPLE_GLOBAL_HEADER_INSIDE
#include "../purpleprivate.h"
#undef PURPLE_GLOBAL_HEADER_INSIDE

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_purple_account_manager_get_default(void) {
	PurpleAccountManager *manager1 = NULL, *manager2 = NULL;

	manager1 = purple_account_manager_get_default();
	g_assert_true(PURPLE_IS_ACCOUNT_MANAGER(manager1));

	manager2 = purple_account_manager_get_default();
	g_assert_true(PURPLE_IS_ACCOUNT_MANAGER(manager2));

	g_assert_true(manager1 == manager2);
}

/******************************************************************************
 * Add/Remove Test
 *****************************************************************************/
static void
test_purple_account_manager_signal_called(G_GNUC_UNUSED PurpleAccountManager *manager,
                                          G_GNUC_UNUSED PurpleAccount *account,
                                          gpointer data)
{
	gboolean *called = (data);

	*called = TRUE;
}

static void
test_purple_account_manager_add_remove(void) {
	PurpleAccount *account = NULL;
	PurpleAccountManager *manager = NULL;
	gboolean signal_called = FALSE;

	account = purple_account_new("test", "test");
	manager = g_object_new(PURPLE_TYPE_ACCOUNT_MANAGER, NULL);

	g_signal_connect(manager, "added",
	                 G_CALLBACK(test_purple_account_manager_signal_called),
	                 &signal_called);
	g_signal_connect(manager, "removed",
	                 G_CALLBACK(test_purple_account_manager_signal_called),
	                 &signal_called);

	g_assert_cmpuint(g_list_model_get_n_items(G_LIST_MODEL(manager)), ==, 0);

	/* Add the account and verify that it was added and that the signal was
	 * emitted.
	 */
	purple_account_manager_add(manager, account);
	g_assert_cmpuint(g_list_model_get_n_items(G_LIST_MODEL(manager)), ==, 1);
	g_assert_true(signal_called);

	signal_called = FALSE;

	/* Remove the account and verify that it was removed and that the signal
	 * was emitted.
	 */
	purple_account_manager_remove(manager, account);
	g_assert_cmpuint(g_list_model_get_n_items(G_LIST_MODEL(manager)), ==, 0);
	g_assert_true(signal_called);

	/* Cleanup */
	g_clear_object(&account);
	g_clear_object(&manager);
}

/******************************************************************************
 * Find Tests
 *****************************************************************************/
static gboolean
test_purple_account_manager_find_func(gconstpointer a, gconstpointer b) {
	PurpleAccount *account = PURPLE_ACCOUNT((gpointer)a);
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
	const gchar *desired_username = b;
	const gchar *protocol_id = NULL;
	const gchar *username = NULL;

	/* Check if the protocol id matches expected. */
	protocol_id = purple_account_get_protocol_id(account);
	if(!purple_strequal(protocol_id, "test")) {
		return FALSE;
	}

	/* Finally verify the username. */
	username = purple_contact_info_get_username(info);
	if(!purple_strequal(username, desired_username)) {
		return FALSE;
	}

	return TRUE;
}

static void
test_purple_account_manager_find(void) {
	PurpleAccount *account = NULL, *found = NULL;
	PurpleAccountManager *manager = NULL;

	manager = g_object_new(PURPLE_TYPE_ACCOUNT_MANAGER, NULL);

	/* Try to find an account that doesn't exist. */
	found = purple_account_manager_find(manager, "test", "test");
	g_assert_null(found);

	/* Create the account that will be used in the rest of the test. */
	account = purple_account_new("test", "test");

	/* Now add an account and verify that we can find it. */
	purple_account_manager_add(manager, account);
	found = purple_account_manager_find(manager, "test", "test");
	g_assert_nonnull(found);

	/* Verify account can be found using a custom function. */
	found = purple_account_manager_find_custom(manager,
	                                           test_purple_account_manager_find_func,
	                                           "test");
	g_assert_nonnull(found);

	/* Finally remove the account and verify it can't be found. */
	purple_account_manager_remove(manager, account);
	found = purple_account_manager_find(manager, "test", "test");
	g_assert_null(found);

	/* Cleanup */
	g_clear_object(&account);
	g_clear_object(&manager);
}

/******************************************************************************
 * Foreach Tests
 *****************************************************************************/
static void
test_purple_account_manager_foreach_func(G_GNUC_UNUSED PurpleAccount *account,
                                         gpointer data)
{
	guint *count = (guint *)data;

	/* We have to use (*count)++ because *count++ doesn't work as the ++
	 * happens after the statement which is no longer the dereferenced pointer.
	 */
	(*count)++;
}

static void
test_purple_account_manager_foreach(void) {
	PurpleAccount *accounts[3];
	PurpleAccountManager *manager = NULL;
	guint count = 0;

	manager = g_object_new(PURPLE_TYPE_ACCOUNT_MANAGER, NULL);

	accounts[0] = purple_account_new("test0", "test");
	accounts[1] = purple_account_new("test1", "test");
	accounts[2] = purple_account_new("test2", "test");

	purple_account_manager_add(manager, accounts[0]);
	purple_account_manager_add(manager, accounts[1]);
	purple_account_manager_add(manager, accounts[2]);

	purple_account_manager_foreach(manager,
	                               test_purple_account_manager_foreach_func,
	                               &count);

	g_assert_cmpuint(count, ==, 3);

	/* Now remove everything and verify that the foreach callback wasn't
	 * called.
	 */
	purple_account_manager_remove(manager, accounts[0]);
	purple_account_manager_remove(manager, accounts[1]);
	purple_account_manager_remove(manager, accounts[2]);

	count = 0;
	purple_account_manager_foreach(manager,
	                               test_purple_account_manager_foreach_func,
	                               &count);

	g_assert_cmpuint(count, ==, 0);

	/* Cleanup */
	g_clear_object(&accounts[0]);
	g_clear_object(&accounts[1]);
	g_clear_object(&accounts[2]);
	g_clear_object(&manager);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	test_ui_purple_init();

	g_test_add_func("/account-manager/get-default",
	                test_purple_account_manager_get_default);
	g_test_add_func("/account-manager/add-remove",
	                test_purple_account_manager_add_remove);
	g_test_add_func("/account-manager/find",
	                test_purple_account_manager_find);
	g_test_add_func("/account-manager/foreach",
	                test_purple_account_manager_foreach);

	return g_test_run();
}
