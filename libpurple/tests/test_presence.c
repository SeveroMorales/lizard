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
 * Main
 *****************************************************************************/
static void
test_purple_presence_migrate_online_without_message(void) {
	PurplePresence *presence = NULL;
	PurpleStatus *status = NULL;
	PurpleStatusType *type = NULL;

	type = purple_status_type_new(PURPLE_STATUS_AVAILABLE,
	                              "online", "online", TRUE);
	presence = purple_presence_new();
	status = purple_status_new(type, presence);

	purple_status_set_active(status, TRUE);

	/* Now verify that the presence returns the correct values for the
	 * properties.
	 */
	g_assert_cmpint(purple_presence_get_primitive(presence), ==,
	                PURPLE_STATUS_AVAILABLE);
	g_assert_null(purple_presence_get_message(presence));

	/* Cleanup. */
	g_clear_object(&presence);
	g_clear_object(&status);
	g_clear_pointer(&type, purple_status_type_destroy);
}

static void
test_purple_presence_migrate_online_with_message(void) {
	PurplePresence *presence = NULL;
	PurpleStatus *status = NULL;
	PurpleStatusType *type = NULL;
	GHashTable *attrs =  NULL;

	type = purple_status_type_new_with_attrs(PURPLE_STATUS_AVAILABLE,
	                                         "online", "online",
	                                         FALSE, TRUE, TRUE,
	                                         "message", "message",
	                                         purple_value_new(G_TYPE_STRING),
	                                         NULL);
	presence = purple_presence_new();

	status = purple_status_new(type, presence);

	attrs = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_insert(attrs, "message", "Greetings Programs!");
	purple_status_set_active_with_attributes(status, TRUE, attrs);
	g_hash_table_destroy(attrs);

	/* Now verify that the presence returns the correct values for the
	 * properties.
	 */
	g_assert_cmpint(purple_presence_get_primitive(presence), ==,
	                PURPLE_STATUS_AVAILABLE);
	g_assert_cmpstr(purple_presence_get_message(presence), ==,
	                "Greetings Programs!");

	/* Cleanup. */
	g_clear_object(&presence);
	g_clear_object(&status);
	g_clear_pointer(&type, purple_status_type_destroy);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	/* This tests verify that PurplePresence is properly notifying of property
	 * changes with the 2.x.y and earlier status back end. When PurplePresence
	 * has replaced that back end, these tests should be removed as well.
	 */
	g_test_add_func("/presence/migrate/online-without-message",
	                test_purple_presence_migrate_online_without_message);
	g_test_add_func("/presence/migrate/online-with-message",
	                test_purple_presence_migrate_online_with_message);

	return g_test_run();
}
