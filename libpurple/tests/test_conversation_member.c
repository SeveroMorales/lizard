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

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_purple_conversation_member_new(void) {
	PurpleContactInfo *info = NULL;
	PurpleConversationMember *member = NULL;

	info = purple_contact_info_new(NULL);
	g_assert_true(PURPLE_IS_CONTACT_INFO(info));

	member = purple_conversation_member_new(info);
	g_assert_true(PURPLE_IS_CONVERSATION_MEMBER(member));

	g_clear_object(&info);
	g_clear_object(&member);
}

static void
test_purple_conversation_member_properties(void) {
	PurpleContactInfo *info = NULL;
	PurpleContactInfo *info1 = NULL;
	PurpleConversationMember *member = NULL;
	PurpleTags *tags = NULL;
	PurpleTypingState typing_state = PURPLE_TYPING_STATE_NONE;

	info = purple_contact_info_new("abc123");

	/* Use g_object_new so we can test setting properties by name. All of them
	 * call the setter methods, so by doing it this way we exercise more of the
	 * code.
	 */
	member = g_object_new(
		PURPLE_TYPE_CONVERSATION_MEMBER,
		"contact-info", info,
		"typing-state", PURPLE_TYPING_STATE_TYPING,
		NULL);

	/* Now use g_object_get to read all of the properties. */
	g_object_get(member,
		"contact-info", &info1,
		"tags", &tags,
		"typing-state", &typing_state,
		NULL);

	/* Compare all the things. */
	g_assert_true(info1 == info);
	g_assert_true(PURPLE_IS_TAGS(tags));
	g_assert_cmpint(typing_state, ==, PURPLE_TYPING_STATE_TYPING);

	/* Free/unref all the things. */
	g_clear_object(&info1);
	g_clear_object(&tags);

	g_clear_object(&info);
	g_clear_object(&member);
}

/******************************************************************************
 * Typing State Timeout
 *****************************************************************************/
static void
test_purple_conversation_manager_timeout_notify(G_GNUC_UNUSED GObject *obj,
                                                G_GNUC_UNUSED GParamSpec *pspec,
                                                gpointer data)
{
	GMainLoop *loop = data;
	static guint count = 0;

	/* Increment count each time we're called. We're expecting to be called
	 * twice, so after that quit the main loop.
	 */
	count++;
	if(count >= 2) {
		g_main_loop_quit(loop);
	}
}

static gboolean
test_purple_conversation_manager_timeout_fail_safe(gpointer data) {
	GMainLoop *loop = data;

	g_warning("fail safe triggered");

	g_main_loop_quit(loop);

	return G_SOURCE_REMOVE;
}

static void
test_purple_conversation_member_typing_state_timeout(void) {
	PurpleContactInfo *info = NULL;
	PurpleConversationMember *member = NULL;
	PurpleTypingState typing_state = PURPLE_TYPING_STATE_TYPING;
	GMainLoop *loop = NULL;

	/* Create the main loop as we'll need it to let the timeout fire. */
	loop = g_main_loop_new(NULL, FALSE);

	/* Create the member and add a notify callback on the typing-state property
	 * so we can check it and exit the main loop.
	 */
	info = purple_contact_info_new(NULL);
	member = purple_conversation_member_new(info);
	g_signal_connect(member, "notify::typing-state",
	                 G_CALLBACK(test_purple_conversation_manager_timeout_notify),
	                 loop);

	/* Set the state to typing with a timeout of 1 second. */
	purple_conversation_member_set_typing_state(member,
	                                            PURPLE_TYPING_STATE_TYPING, 1);

	/* Add a fail safe timeout at 2 seconds to make sure the test won't get
	 * stuck waiting forever.
	 */
	g_timeout_add_seconds(2,
	                      test_purple_conversation_manager_timeout_fail_safe,
	                      loop);

	/* Run the main loop and let the timeouts fire. */
	g_main_loop_run(loop);

	/* Verify that our state got reset back to PURPLE_TYPING_STATE_NONE. */
	typing_state = purple_conversation_member_get_typing_state(member);
	g_assert_cmpint(typing_state, ==, PURPLE_TYPING_STATE_NONE);

	/* Clean everything up. */
	g_clear_object(&info);
	g_clear_object(&member);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/conversation-member/new",
	                test_purple_conversation_member_new);
	g_test_add_func("/conversation-member/properties",
	                test_purple_conversation_member_properties);

	g_test_add_func("/conversation-member/typing-state/timeout",
	                test_purple_conversation_member_typing_state_timeout);

	return g_test_run();
}
