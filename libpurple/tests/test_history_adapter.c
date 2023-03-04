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
 * TestPurpleHistoryAdapter
 *****************************************************************************/
#define TEST_PURPLE_TYPE_HISTORY_ADAPTER \
	(test_purple_history_adapter_get_type())
G_DECLARE_FINAL_TYPE(TestPurpleHistoryAdapter,
                     test_purple_history_adapter,
                     TEST_PURPLE, HISTORY_ADAPTER,
                     PurpleHistoryAdapter)

struct _TestPurpleHistoryAdapter {
	PurpleHistoryAdapter parent;

	gboolean activate;
	gboolean deactivate;
	gboolean query;
	gboolean remove;
	gboolean write;
};

G_DEFINE_TYPE(TestPurpleHistoryAdapter,
              test_purple_history_adapter,
              PURPLE_TYPE_HISTORY_ADAPTER)

static gboolean
test_purple_history_adapter_activate(PurpleHistoryAdapter *a,
                                     G_GNUC_UNUSED GError **error)
{
	TestPurpleHistoryAdapter *adapter = TEST_PURPLE_HISTORY_ADAPTER(a);

	adapter->activate = TRUE;

	return TRUE;
}

static gboolean
test_purple_history_adapter_deactivate(PurpleHistoryAdapter *a,
                                       G_GNUC_UNUSED GError **error)
{
	TestPurpleHistoryAdapter *adapter = TEST_PURPLE_HISTORY_ADAPTER(a);

	adapter->deactivate = TRUE;

	return TRUE;
}

static GList *
test_purple_history_adapter_query(PurpleHistoryAdapter *a,
                                  G_GNUC_UNUSED const gchar *query,
                                  G_GNUC_UNUSED GError **error)
{
	TestPurpleHistoryAdapter *adapter = TEST_PURPLE_HISTORY_ADAPTER(a);
	GList *list = NULL;

	adapter->query = TRUE;

	list = g_list_append(list, GINT_TO_POINTER(1));

	return list;
}

static gboolean
test_purple_history_adapter_remove(PurpleHistoryAdapter *a,
                                   G_GNUC_UNUSED const gchar *query,
                                   G_GNUC_UNUSED GError **error)
{
	TestPurpleHistoryAdapter *adapter = TEST_PURPLE_HISTORY_ADAPTER(a);

	adapter->remove = TRUE;

	return TRUE;
}

static gboolean
test_purple_history_adapter_write(PurpleHistoryAdapter *a,
                                  G_GNUC_UNUSED PurpleConversation *conversation,
                                  G_GNUC_UNUSED PurpleMessage *message,
                                  G_GNUC_UNUSED GError **error)
{
	TestPurpleHistoryAdapter *adapter = TEST_PURPLE_HISTORY_ADAPTER(a);

	adapter->write = TRUE;

	return TRUE;
}

static void
test_purple_history_adapter_init(G_GNUC_UNUSED TestPurpleHistoryAdapter *adapter) {
}

static void
test_purple_history_adapter_class_init(TestPurpleHistoryAdapterClass *klass) {
	PurpleHistoryAdapterClass *adapter_class = NULL;

	adapter_class = PURPLE_HISTORY_ADAPTER_CLASS(klass);

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
test_purple_history_adapter_test_properties(void) {
	PurpleHistoryAdapter *adapter = test_purple_history_adapter_new();

	g_assert_cmpstr(purple_history_adapter_get_id(adapter),
	                ==,
	                "test-adapter");
	g_assert_cmpstr(purple_history_adapter_get_name(adapter),
	                ==,
	                "Test Adapter");

	g_clear_object(&adapter);
}

static void
test_purple_history_adapter_test_activate(void) {
	PurpleHistoryAdapter *adapter = test_purple_history_adapter_new();
	TestPurpleHistoryAdapter *ta = TEST_PURPLE_HISTORY_ADAPTER(adapter);
	GError *error = NULL;
	gboolean result = FALSE;

	result = purple_history_adapter_activate(adapter, &error);
	g_assert_no_error(error);
	g_assert_true(result);
	g_assert_true(ta->activate);
	g_assert_false(ta->deactivate);
	g_assert_false(ta->query);
	g_assert_false(ta->remove);
	g_assert_false(ta->write);

	g_clear_object(&adapter);
}

static void
test_purple_history_adapter_test_deactivate(void) {
	PurpleHistoryAdapter *adapter = test_purple_history_adapter_new();
	TestPurpleHistoryAdapter *ta = TEST_PURPLE_HISTORY_ADAPTER(adapter);
	GError *error = NULL;
	gboolean result = FALSE;

	result = purple_history_adapter_deactivate(adapter, &error);
	g_assert_no_error(error);
	g_assert_true(result);
	g_assert_false(ta->activate);
	g_assert_true(ta->deactivate);
	g_assert_false(ta->query);
	g_assert_false(ta->remove);
	g_assert_false(ta->write);

	g_clear_object(&adapter);
}

static void
test_purple_history_adapter_test_query(void) {
	PurpleHistoryAdapter *adapter = test_purple_history_adapter_new();
	TestPurpleHistoryAdapter *ta = TEST_PURPLE_HISTORY_ADAPTER(adapter);
	GError *error = NULL;
	GList *result = NULL;

	result = purple_history_adapter_query(adapter, "query", &error);
	g_assert_no_error(error);
	g_assert_nonnull(result);
	g_assert_false(ta->activate);
	g_assert_false(ta->deactivate);
	g_assert_true(ta->query);
	g_assert_false(ta->remove);
	g_assert_false(ta->write);
	g_list_free(result);
	g_clear_object(&adapter);
}

static void
test_purple_history_adapter_test_remove(void) {
	PurpleHistoryAdapter *adapter = test_purple_history_adapter_new();
	TestPurpleHistoryAdapter *ta = TEST_PURPLE_HISTORY_ADAPTER(adapter);
	GError *error = NULL;
	gboolean result = FALSE;

	result = purple_history_adapter_remove(adapter, "query", &error);
	g_assert_no_error(error);
	g_assert_true(result);
	g_assert_false(ta->activate);
	g_assert_false(ta->deactivate);
	g_assert_false(ta->query);
	g_assert_true(ta->remove);
	g_assert_false(ta->write);

	g_clear_object(&adapter);
}

static void
test_purple_history_adapter_test_write(void) {
	PurpleAccount *account = NULL;
	PurpleConversation *conversation = NULL;
	PurpleHistoryAdapter *adapter = test_purple_history_adapter_new();
	PurpleMessage *message = NULL;
	TestPurpleHistoryAdapter *ta = TEST_PURPLE_HISTORY_ADAPTER(adapter);
	GError *error = NULL;
	gboolean result = FALSE;

	message = g_object_new(PURPLE_TYPE_MESSAGE, NULL);
	account = purple_account_new("test", "test");
	conversation = g_object_new(PURPLE_TYPE_IM_CONVERSATION,
	                            "account", account,
	                            "name", "pidgy",
	                            NULL);
	result = purple_history_adapter_write(adapter, conversation, message,
	                                      &error);

	g_assert_no_error(error);
	g_assert_true(result);
	g_assert_false(ta->activate);
	g_assert_false(ta->deactivate);
	g_assert_false(ta->query);
	g_assert_false(ta->remove);
	g_assert_true(ta->write);

	g_clear_object(&adapter);
	g_clear_object(&message);

	/* TODO: something is freeing our ref. */
	/* g_clear_object(&account); */

	g_clear_object(&conversation);
}


/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	test_ui_purple_init();

	g_test_add_func("/history-adapter/properties",
	                test_purple_history_adapter_test_properties);
	g_test_add_func("/history-adapter/activate",
	                test_purple_history_adapter_test_activate);
	g_test_add_func("/history-adapter/deactivate",
	                test_purple_history_adapter_test_deactivate);
	g_test_add_func("/history-adapter/query",
	                test_purple_history_adapter_test_query);
	g_test_add_func("/history-adapter/remove",
	                test_purple_history_adapter_test_remove);
	g_test_add_func("/history-adapter/write",
	                test_purple_history_adapter_test_write);

	return g_test_run();
}
