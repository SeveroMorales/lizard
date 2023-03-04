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
 * Tests
 *****************************************************************************/
static void
test_purple_conversation_properties(void) {
	PurpleAccount *account = NULL;
	PurpleAccount *account1 = NULL;
	PurpleConnectionFlags features = 0;
	PurpleConversation *conversation = NULL;
	GListModel *members = NULL;
	gchar *name = NULL;
	gchar *title = NULL;

	account = purple_account_new("test", "test");

	/* Use g_object_new so we can test setting properties by name. All of them
	 * call the setter methods, so by doing it this way we exercise more of the
	 * code.
	 */
	conversation = g_object_new(
		PURPLE_TYPE_CONVERSATION,
		"account", account,
		"features", PURPLE_CONNECTION_FLAG_HTML,
		"name", "name1",
		"title", "title1",
		NULL);

	/* Now use g_object_get to read all of the properties. */
	g_object_get(conversation,
		"account", &account1,
		"features", &features,
		"members", &members,
		"name", &name,
		"title", &title,
		NULL);

	/* Compare all the things. */
	g_assert_true(account1 == account);
	g_assert_cmpint(features, ==, PURPLE_CONNECTION_FLAG_HTML);
	g_assert_true(G_IS_LIST_MODEL(members));
	g_assert_cmpstr(name, ==, "name1");

	/* We don't currently test title because purple_conversation_autoset_title
	 * makes it something we don't expect it to be.
	 */
#if 0
	g_assert_cmpstr(title, ==, "title1");
#endif

	/* Free/unref all the things. */
	g_clear_object(&account1);
	g_clear_object(&members);
	g_free(name);
	g_free(title);

	g_clear_object(&account);
	g_clear_object(&conversation);
}

/******************************************************************************
 * Membership tests and helpers
 *****************************************************************************/
static void
test_purple_conversation_membership_signal_cb(PurpleConversation *conversation,
                                              PurpleConversationMember *member,
                                              gboolean announce,
                                              const char *message,
                                              gpointer data)
{
	/* We use int's for called to make sure it was only called once. */
	gint *called = data;

	g_assert_true(PURPLE_IS_CONVERSATION(conversation));
	g_assert_true(PURPLE_IS_CONVERSATION_MEMBER(member));
	g_assert_true(announce);
	g_assert_cmpstr(message, ==, "announcement message");

	*called = *called + 1;
}

static void
test_purple_conversation_members_add_remove(void) {
	PurpleAccount *account = NULL;
	PurpleContactInfo *info = NULL;
	PurpleConversation *conversation = NULL;
	PurpleConversationMember *member = NULL;
	PurpleConversationMember *member1 = NULL;
	gboolean removed = FALSE;
	gint added_called = 0;
	gint removed_called = 0;

	/* Create our instances. */
	info = purple_contact_info_new(NULL);
	account = purple_account_new("test", "test");
	conversation = g_object_new(
		PURPLE_TYPE_CONVERSATION,
		"account", account,
		"name", "test-conversation",
		NULL);

	/* Connect our signals. */
	g_signal_connect(conversation, "member-added",
	                 G_CALLBACK(test_purple_conversation_membership_signal_cb),
	                 &added_called);
	g_signal_connect(conversation, "member-removed",
	                 G_CALLBACK(test_purple_conversation_membership_signal_cb),
	                 &removed_called);

	/* Add the member. */
	member = purple_conversation_add_member(conversation, info, TRUE,
	                                        "announcement message");
	g_assert_cmpint(added_called, ==, 1);
	g_assert_true(PURPLE_IS_CONVERSATION_MEMBER(member));

	/* Add our own reference to the returned member as we use it later to
	 * verify that double remove doesn't do anything.
	 */
	g_object_ref(member);

	/* Try to add the member again, which would just return the existing
	 * member and not emit the signal.
	 */
	member1 = purple_conversation_add_member(conversation, info, TRUE,
	                                         "announcement message");
	g_assert_cmpint(added_called, ==, 1);
	g_assert_true(PURPLE_IS_CONVERSATION_MEMBER(member1));
	g_assert_true(member1 == member);

	/* Now remove the member and verify the signal was called. */
	removed = purple_conversation_remove_member(conversation, member, TRUE,
	                                            "announcement message");
	g_assert_true(removed);
	g_assert_cmpint(removed_called, ==, 1);

	/* Try to remove the member again and verify that nothing was removed and
	 * that the signal wasn't emitted.
	 */
	removed = purple_conversation_remove_member(conversation, member, TRUE,
	                                            "announcement message");
	g_assert_false(removed);
	g_assert_cmpint(removed_called, ==, 1);

	/* Clean up everything. */
	g_clear_object(&info);
	g_clear_object(&member);
	g_clear_object(&account);
	g_clear_object(&conversation);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	test_ui_purple_init();

	g_test_add_func("/conversation/properties",
	                test_purple_conversation_properties);

	g_test_add_func("/conversation/members/add-remove",
	                test_purple_conversation_members_add_remove);

	return g_test_run();
}
