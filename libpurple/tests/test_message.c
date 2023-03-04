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
test_purple_message_properties(void) {
	PurpleMessage *message = NULL;
	PurpleMessageContentType content_type = 0;
	PurpleMessageFlags flags = 0;
	GDateTime *timestamp = NULL;
	GDateTime *timestamp1 = NULL;
	GError *error = NULL;
	GError *error1 = NULL;
	char *id = NULL;
	char *author = NULL;
	char *author_alias = NULL;
	char *author_name_color = NULL;
	char *recipient = NULL;
	char *contents = NULL;

	timestamp = g_date_time_new_from_unix_utc(911347200);
	error = g_error_new(g_quark_from_static_string("test-message"), 0,
	                    "delivery failed");

	message = g_object_new(
		PURPLE_TYPE_MESSAGE,
		"id", "id",
		"author", "author",
		"author-alias", "alias",
		"author-name-color", "purple",
		"recipient", "pidgy",
		"contents", "Now that is a big door",
		"content-type", PURPLE_MESSAGE_CONTENT_TYPE_MARKDOWN,
		"timestamp", timestamp,
		"flags", PURPLE_MESSAGE_SYSTEM,
		"error", error,
		NULL);

	g_object_get(
		message,
		"id", &id,
		"author", &author,
		"author-alias", &author_alias,
		"author-name-color", &author_name_color,
		"recipient", &recipient,
		"contents", &contents,
		"content-type", &content_type,
		"timestamp", &timestamp1,
		"flags", &flags,
		"error", &error1,
		NULL);

	g_assert_cmpstr(id, ==, "id");
	g_assert_cmpstr(author, ==, "author");
	g_assert_cmpstr(author_alias, ==, "alias");
	g_assert_cmpstr(author_name_color, ==, "purple");
	g_assert_cmpstr(recipient, ==, "pidgy");
	g_assert_cmpstr(contents, ==, "Now that is a big door");
	g_assert_cmpint(content_type, ==, PURPLE_MESSAGE_CONTENT_TYPE_MARKDOWN);
	g_assert_true(g_date_time_equal(timestamp1, timestamp));
	g_assert_cmpint(flags, ==, PURPLE_MESSAGE_SYSTEM);
	g_assert_error(error1, error->domain, error->code);

	g_clear_pointer(&id, g_free);
	g_clear_pointer(&author, g_free);
	g_clear_pointer(&author_alias, g_free);
	g_clear_pointer(&author_name_color, g_free);
	g_clear_pointer(&recipient, g_free);
	g_clear_pointer(&contents, g_free);
	g_clear_pointer(&timestamp, g_date_time_unref);
	g_clear_pointer(&timestamp1, g_date_time_unref);
	g_clear_error(&error1);
	g_clear_object(&message);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/message/properties",
	                test_purple_message_properties);

	return g_test_run();
}
