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
 * TestPurpleHistoryAdapter Implementation
 *****************************************************************************/
#define TEST_PURPLE_TYPE_HISTORY_ADAPTER \
	(test_purple_history_adapter_get_type())
G_DECLARE_FINAL_TYPE(TestPurpleHistoryAdapter,
                     test_purple_history_adapter,
                     TEST_PURPLE, HISTORY_ADAPTER,
                     PurpleHistoryAdapter)

struct _TestPurpleHistoryAdapter {
	PurpleHistoryAdapter parent;

	gboolean activate_called;
	gboolean deactivate_called;
	gboolean query_called;
	gboolean remove_called;
	gboolean write_called;
};

G_DEFINE_TYPE(TestPurpleHistoryAdapter,
              test_purple_history_adapter,
              PURPLE_TYPE_HISTORY_ADAPTER)

static gboolean
test_purple_history_adapter_activate(PurpleHistoryAdapter *a,
                                     G_GNUC_UNUSED GError **error)
{
	TestPurpleHistoryAdapter *ta = TEST_PURPLE_HISTORY_ADAPTER(a);

	ta->activate_called = TRUE;

	return TRUE;
}

static gboolean
test_purple_history_adapter_deactivate(PurpleHistoryAdapter *a,
                                       G_GNUC_UNUSED GError **error)
{
	TestPurpleHistoryAdapter *ta = TEST_PURPLE_HISTORY_ADAPTER(a);

	ta->deactivate_called = TRUE;

	return TRUE;
}

static GList *
test_purple_history_adapter_query(PurpleHistoryAdapter *a,
                                  G_GNUC_UNUSED const gchar *id,
                                  G_GNUC_UNUSED GError **error)
{
	TestPurpleHistoryAdapter *ta = TEST_PURPLE_HISTORY_ADAPTER(a);

	ta->query_called = TRUE;

	return NULL;
}

static gboolean
test_purple_history_adapter_remove(PurpleHistoryAdapter *a,
                                   G_GNUC_UNUSED const gchar *id,
                                   G_GNUC_UNUSED GError **error)
{
	TestPurpleHistoryAdapter *ta = TEST_PURPLE_HISTORY_ADAPTER(a);

	ta->remove_called = TRUE;

	return TRUE;
}

static gboolean
test_purple_history_adapter_write(PurpleHistoryAdapter *a,
                                  G_GNUC_UNUSED PurpleConversation *conversation,
                                  G_GNUC_UNUSED PurpleMessage *message,
                                  G_GNUC_UNUSED GError **error)
{
	TestPurpleHistoryAdapter *ta = TEST_PURPLE_HISTORY_ADAPTER(a);

	ta->write_called = TRUE;

	return TRUE;
}

static void
test_purple_history_adapter_init(G_GNUC_UNUSED TestPurpleHistoryAdapter *adapter)
{
}

static void
test_purple_history_adapter_class_init(TestPurpleHistoryAdapterClass *klass)
{
	PurpleHistoryAdapterClass *adapter_class = PURPLE_HISTORY_ADAPTER_CLASS(klass);

	adapter_class->activate = test_purple_history_adapter_activate;
	adapter_class->deactivate = test_purple_history_adapter_deactivate;
	adapter_class->query = test_purple_history_adapter_query;
	adapter_class->remove = test_purple_history_adapter_remove;
	adapter_class->write = test_purple_history_adapter_write;
}

static PurpleHistoryAdapter *
test_purple_history_adapter_new(void) {
	return g_object_new(
		TEST_PURPLE_TYPE_HISTORY_ADAPTER,
		"id", "test-adapter",
		"name", "Test Adapter",
		NULL);
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_purple_history_manager_get_default(void) {
	PurpleHistoryManager *manager1 = NULL, *manager2 = NULL;

	manager1 = purple_history_manager_get_default();
	g_assert_true(PURPLE_IS_HISTORY_MANAGER(manager1));

	manager2 = purple_history_manager_get_default();
	g_assert_true(PURPLE_IS_HISTORY_MANAGER(manager2));

	g_assert_true(manager1 == manager2);
}

/******************************************************************************
 * Registration Tests
 *****************************************************************************/
static void
test_purple_history_manager_registration(void) {
	PurpleHistoryManager *manager = NULL;
	PurpleHistoryAdapter *adapter = NULL;
	GError *error = NULL;
	gboolean r = FALSE;

	manager = g_object_new(PURPLE_TYPE_HISTORY_MANAGER, NULL);
	g_assert_true(PURPLE_IS_HISTORY_MANAGER(manager));

	adapter = test_purple_history_adapter_new();

	/* Register the first time cleanly. */
	r = purple_history_manager_register(manager, adapter, &error);
	g_assert_no_error(error);
	g_assert_true(r);

	/* Register again and verify the error. */
	r = purple_history_manager_register(manager, adapter, &error);
	g_assert_false(r);
	g_assert_error(error, PURPLE_HISTORY_MANAGER_DOMAIN, 0);
	g_clear_error(&error);

	/* Unregister the adapter. */
	r = purple_history_manager_unregister(manager, adapter, &error);
	g_assert_no_error(error);
	g_assert_true(r);

	/* Unregister the adapter again and verify the error. */
	r = purple_history_manager_unregister(manager, adapter, &error);
	g_assert_false(r);
	g_assert_error(error, PURPLE_HISTORY_MANAGER_DOMAIN, 0);
	g_clear_error(&error);

	/* Final clean ups. */
	g_clear_object(&adapter);
	g_clear_object(&manager);
}

/******************************************************************************
 * Set Active Tests
 *****************************************************************************/
static void
test_purple_history_manager_set_active_null(void) {
	PurpleHistoryManager *manager = NULL;
	GError *error = NULL;
	gboolean ret = FALSE;

	manager = g_object_new(PURPLE_TYPE_HISTORY_MANAGER, NULL);
	ret = purple_history_manager_set_active(manager, NULL, &error);

	g_assert_no_error(error);
	g_assert_true(ret);

	g_clear_object(&manager);
}

static void
test_purple_history_manager_set_active_non_existent(void) {
	PurpleHistoryManager *manager = NULL;
	GError *error = NULL;
	gboolean ret = FALSE;

	manager = g_object_new(PURPLE_TYPE_HISTORY_MANAGER, NULL);
	ret = purple_history_manager_set_active(manager, "foo", &error);

	g_assert_error(error, PURPLE_HISTORY_MANAGER_DOMAIN, 0);
	g_assert_false(ret);
	g_clear_error(&error);

	g_clear_object(&manager);
}

static void
test_purple_history_manager_set_active_normal(void) {
	PurpleHistoryManager *manager = NULL;
	PurpleHistoryAdapter *adapter = NULL;
	GError *error = NULL;
	gboolean r = FALSE;
	TestPurpleHistoryAdapter *ta = NULL;

	manager = g_object_new(PURPLE_TYPE_HISTORY_MANAGER, NULL);

	/* Create the adapter and register it in the manager. */
	adapter = test_purple_history_adapter_new();
	ta = TEST_PURPLE_HISTORY_ADAPTER(adapter);
	r = purple_history_manager_register(manager, adapter, &error);
	g_assert_no_error(error);
	g_assert_true(r);

	/* Set the adapter as active and verify it was successful. */
	r = purple_history_manager_set_active(manager, "test-adapter",
	                                      &error);
	g_assert_no_error(error);
	g_assert_true(r);
	g_assert_true(ta->activate_called);

	/* Verify that unregistering the active adapter fails */
	r = purple_history_manager_unregister(manager, adapter,
	                                      &error);
	g_assert_error(error, PURPLE_HISTORY_MANAGER_DOMAIN, 0);
	g_assert_false(r);
	g_clear_error(&error);

	/* Now unset the active adapter. */
	r = purple_history_manager_set_active(manager, NULL, &error);
	g_assert_no_error(error);
	g_assert_true(r);
	g_assert_true(ta->deactivate_called);

	/* Finally unregister the adapter now that it's no longer active. */
	r = purple_history_manager_unregister(manager, adapter, &error);
	g_assert_no_error(error);
	g_assert_true(r);

	/* And our final cleanup. */
	g_clear_object(&adapter);
	g_clear_object(&manager);
}

/******************************************************************************
 * No Adapter Tests
 *****************************************************************************/
static void
test_purple_history_manager_no_adapter_query(void) {
	PurpleHistoryManager *manager = NULL;
	GList *list = NULL;
	GError *error = NULL;

	manager = g_object_new(PURPLE_TYPE_HISTORY_MANAGER, NULL);
	list = purple_history_manager_query(manager, "", &error);

	g_assert_error(error, PURPLE_HISTORY_MANAGER_DOMAIN, 0);
	g_clear_error(&error);

	g_assert_null(list);

	g_clear_object(&manager);
}

static void
test_purple_history_manager_no_adapter_remove(void) {
	PurpleHistoryManager *manager = NULL;
	GError *error = NULL;
	gboolean result = FALSE;

	manager = g_object_new(PURPLE_TYPE_HISTORY_MANAGER, NULL);
	result = purple_history_manager_remove(manager, "", &error);

	g_assert_error(error, PURPLE_HISTORY_MANAGER_DOMAIN, 0);
	g_clear_error(&error);

	g_assert_false(result);

	g_clear_object(&manager);
}

static void
test_purple_history_manager_no_adapter_write(void) {
	PurpleAccount *account = NULL;
	PurpleConversation *conversation = NULL;
	PurpleHistoryManager *manager = NULL;
	PurpleMessage *message = NULL;
	GError *error = NULL;
	gboolean result = FALSE;

	manager = g_object_new(PURPLE_TYPE_HISTORY_MANAGER, NULL);

	message = g_object_new(PURPLE_TYPE_MESSAGE, NULL);
	account = purple_account_new("test", "test");
	conversation = g_object_new(PURPLE_TYPE_IM_CONVERSATION,
	                            "account", account,
	                            "name", "pidgy",
	                            NULL);

	result = purple_history_manager_write(manager, conversation, message,
	                                      &error);

	g_assert_error(error, PURPLE_HISTORY_MANAGER_DOMAIN, 0);
	g_clear_error(&error);

	g_assert_false(result);

	/* TODO: someone is freeing our ref. */
	/* g_clear_object(&account); */

	g_clear_object(&message);
	g_clear_object(&conversation);
	g_clear_object(&manager);
}

/******************************************************************************
 * Manager Tests
 *****************************************************************************/
static void
test_purple_history_manager_adapter_query(void) {
	PurpleHistoryManager *manager = NULL;
	PurpleHistoryAdapter *adapter = NULL;
	TestPurpleHistoryAdapter *ta = NULL;
	GList *list = NULL;
	GError *error = NULL;
	gboolean result = FALSE;

	manager = g_object_new(PURPLE_TYPE_HISTORY_MANAGER, NULL);
	adapter = test_purple_history_adapter_new();
	ta = TEST_PURPLE_HISTORY_ADAPTER(adapter);

	result = purple_history_manager_register(manager, adapter, &error);
	g_assert_no_error(error);
	g_assert_true(result);

	result = purple_history_manager_set_active(manager, "test-adapter",
	                                           &error);
	g_assert_no_error(error);
	g_assert_true(result);

	list = purple_history_manager_query(manager, "", &error);
	g_assert_no_error(error);
	g_assert_null(list);
	g_assert_true(ta->query_called);

	result = purple_history_manager_set_active(manager, NULL, &error);
	g_assert_no_error(error);
	g_assert_true(result);

	result = purple_history_manager_unregister(manager, adapter, &error);
	g_assert_no_error(error);
	g_assert_true(result);

	g_clear_object(&adapter);
	g_clear_object(&manager);
}

static void
test_purple_history_manager_adapter_remove(void) {
	PurpleHistoryManager *manager = NULL;
	PurpleHistoryAdapter *adapter = NULL;
	TestPurpleHistoryAdapter *ta = NULL;
	GError *error = NULL;
	gboolean result = FALSE;

	manager = g_object_new(PURPLE_TYPE_HISTORY_MANAGER, NULL);
	adapter = test_purple_history_adapter_new();
	ta = TEST_PURPLE_HISTORY_ADAPTER(adapter);

	result = purple_history_manager_register(manager, adapter, &error);
	g_assert_no_error(error);
	g_assert_true(result);

	result = purple_history_manager_set_active(manager, "test-adapter",
	                                           &error);
	g_assert_no_error(error);
	g_assert_true(result);

	result = purple_history_manager_remove(manager, "query", &error);
	g_assert_no_error(error);
	g_assert_true(result);
	g_assert_true(ta->remove_called);

	result = purple_history_manager_set_active(manager, NULL, &error);
	g_assert_no_error(error);
	g_assert_true(result);

	result = purple_history_manager_unregister(manager, adapter, &error);
	g_assert_no_error(error);
	g_assert_true(result);

	g_clear_object(&adapter);
	g_clear_object(&manager);
}

static void
test_purple_history_manager_adapter_write(void) {
	PurpleAccount *account = NULL;
	PurpleConversation *conversation = NULL;
	PurpleHistoryManager *manager = NULL;
	PurpleHistoryAdapter *adapter = NULL;
	PurpleMessage *message = NULL;
	TestPurpleHistoryAdapter *ta = NULL;
	GError *error = NULL;
	gboolean result = FALSE;

	manager = g_object_new(PURPLE_TYPE_HISTORY_MANAGER, NULL);
	adapter = test_purple_history_adapter_new();
	ta = TEST_PURPLE_HISTORY_ADAPTER(adapter);

	result = purple_history_manager_register(manager, adapter, &error);
	g_assert_no_error(error);
	g_assert_true(result);

	result = purple_history_manager_set_active(manager, "test-adapter", &error);
	g_assert_no_error(error);
	g_assert_true(result);

	message = g_object_new(PURPLE_TYPE_MESSAGE, NULL);
	account = purple_account_new("test", "test");
	conversation = g_object_new(PURPLE_TYPE_IM_CONVERSATION,
	                            "account", account,
	                            "name", "pidgy",
	                            NULL);
	result = purple_history_manager_write(manager, conversation, message,
	                                      &error);
	g_assert_no_error(error);
	g_assert_true(result);
	g_assert_true(ta->write_called);

	result = purple_history_manager_set_active(manager, NULL, &error);
	g_assert_no_error(error);
	g_assert_true(result);

	result = purple_history_manager_unregister(manager, adapter, &error);
	g_assert_no_error(error);
	g_assert_true(result);

	g_clear_object(&adapter);
	g_clear_object(&message);

	/* TODO: something is freeing our ref. */
	/* g_clear_object(&account); */

	g_clear_object(&conversation);
	g_clear_object(&manager);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	gint ret = 0;

	g_test_init(&argc, &argv, NULL);

	test_ui_purple_init();

	g_test_add_func("/history-manager/get-default",
	                test_purple_history_manager_get_default);
	g_test_add_func("/history-manager/registration",
	                test_purple_history_manager_registration);
	g_test_add_func("/history-manager/set-active/null",
	                test_purple_history_manager_set_active_null);
	g_test_add_func("/history-manager/set-active/non-existent",
	                test_purple_history_manager_set_active_non_existent);
	g_test_add_func("/history-manager/set-active/normal",
	                test_purple_history_manager_set_active_normal);

	/* Tests for manager with an adapter */
	g_test_add_func("/history-manager/adapter/query",
	                test_purple_history_manager_adapter_query);
	g_test_add_func("/history-manager/adapter/remove",
	                test_purple_history_manager_adapter_remove);
	g_test_add_func("/history-manager/adapter/write",
	                test_purple_history_manager_adapter_write);

	/* Tests for manager with no adapter */
	g_test_add_func("/history-manager/no-adapter/query",
	                test_purple_history_manager_no_adapter_query);
	g_test_add_func("/history-manager/no-adapter/remove",
	                test_purple_history_manager_no_adapter_remove);
	g_test_add_func("/history-manager/no-adapter/write",
	                test_purple_history_manager_no_adapter_write);
	ret = g_test_run();

	return ret;
}
