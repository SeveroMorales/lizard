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
test_purple_person_items_changed_cb(G_GNUC_UNUSED GListModel *model,
                                    G_GNUC_UNUSED guint position,
                                    G_GNUC_UNUSED guint removed,
                                    G_GNUC_UNUSED guint added,
                                    gpointer data)
{
	gboolean *called = data;

	*called = TRUE;
}

static void
test_purple_person_notify_cb(G_GNUC_UNUSED GObject *obj,
                             G_GNUC_UNUSED GParamSpec *pspec,
                             gpointer data)
{
	gboolean *called = data;

	*called = TRUE;
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_purple_person_new(void) {
	PurplePerson *person = NULL;

	person = purple_person_new();

	g_assert_true(PURPLE_IS_PERSON(person));

	g_clear_object(&person);
}

static void
test_purple_person_properties(void) {
	PurpleContact *person = NULL;
	PurpleTags *tags = NULL;
	GdkPixbuf *avatar = NULL;
	GdkPixbuf *avatar1 = NULL;
	GdkPixbuf *avatar_for_display = NULL;
	char *id = NULL;
	char *alias = NULL;
	char *name_for_display = NULL;

	/* Create our avatar for testing. */
	avatar = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 1, 1);

	/* Use g_object_new so we can test setting properties by name. All of them
	 * call the setter methods, so by doing it this way we exercise more of the
	 * code.
	 */
	person = g_object_new(
		PURPLE_TYPE_PERSON,
		"alias", "alias",
		"avatar", avatar,
		NULL);

	/* Now use g_object_get to read all of the properties. */
	g_object_get(person,
		"id", &id,
		"alias", &alias,
		"avatar", &avatar1,
		"avatar-for-display", &avatar_for_display,
		"name-for-display", &name_for_display,
		"tags", &tags,
		NULL);

	/* Compare all the things. */
	g_assert_nonnull(id);
	g_assert_cmpstr(alias, ==, "alias");
	g_assert_true(avatar1 == avatar);
	g_assert_true(avatar1 == avatar_for_display);
	g_assert_cmpstr(name_for_display, ==, "alias");
	g_assert_nonnull(tags);

	/* Free/unref all the things. */
	g_clear_pointer(&id, g_free);
	g_clear_pointer(&alias, g_free);
	g_clear_object(&avatar1);
	g_clear_object(&avatar_for_display);
	g_clear_pointer(&name_for_display, g_free);
	g_clear_object(&tags);

	g_clear_object(&avatar);
	g_clear_object(&person);
}

static void
test_purple_person_avatar_for_display_person(void) {
	PurpleContactInfo *info = NULL;
	PurplePerson *person = NULL;
	GdkPixbuf *avatar = NULL;

	person = purple_person_new();
	avatar = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 1, 1);
	purple_person_set_avatar(person, avatar);

	info = purple_contact_info_new("id");
	purple_person_add_contact_info(person, info);

	/* Make sure the person's alias is overriding the contact info. */
	g_assert_true(purple_person_get_avatar_for_display(person) == avatar);

	g_clear_object(&info);
	g_clear_object(&person);
	g_clear_object(&avatar);
}

static void
test_purple_person_avatar_for_display_contact(void) {
	PurpleContactInfo *info = NULL;
	PurplePerson *person = NULL;
	GdkPixbuf *avatar = NULL;

	person = purple_person_new();

	info = purple_contact_info_new("id");
	avatar = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 1, 1);
	purple_contact_info_set_avatar(info, avatar);
	purple_person_add_contact_info(person, info);

	/* Make sure the person's alias is overriding the contact info. */
	g_assert_true(purple_person_get_avatar_for_display(person) == avatar);

	g_clear_object(&info);
	g_clear_object(&person);
	g_clear_object(&avatar);
}

static void
test_purple_person_name_for_display_person(void) {
	PurpleContactInfo *info = NULL;
	PurplePerson *person = NULL;

	person = purple_person_new();
	purple_person_set_alias(person, "person-alias");

	info = purple_contact_info_new("id");
	purple_person_add_contact_info(person, info);

	/* Make sure the person's alias is overriding the contact info. */
	g_assert_cmpstr(purple_person_get_name_for_display(person), ==,
	                "person-alias");

	g_clear_object(&info);
	g_clear_object(&person);
}

static void
test_purple_person_name_for_display_contact(void) {
	PurpleContactInfo *info = NULL;
	PurplePerson *person = NULL;

	person = purple_person_new();

	info = purple_contact_info_new("id");
	purple_person_add_contact_info(person, info);

	/* Make sure the contact info's name for display is called when the
	 * person's alias is unset.
	 */
	g_assert_cmpstr(purple_person_get_name_for_display(person), ==, "id");

	g_clear_object(&info);
	g_clear_object(&person);
}

static void
test_purple_person_contacts_single(void) {
	PurpleContactInfo *info = NULL;
	PurplePerson *person = NULL;
	PurplePerson *person1 = NULL;
	guint n_items = 0;
	gboolean removed = FALSE;
	gboolean changed = FALSE;

	info = purple_contact_info_new("id");
	person = purple_person_new();
	g_signal_connect(person, "items-changed",
	                 G_CALLBACK(test_purple_person_items_changed_cb), &changed);

	n_items = g_list_model_get_n_items(G_LIST_MODEL(person));
	g_assert_cmpuint(n_items, ==, 0);
	purple_person_add_contact_info(person, info);
	n_items = g_list_model_get_n_items(G_LIST_MODEL(person));
	g_assert_cmpuint(n_items, ==, 1);
	g_assert_true(changed);

	person1 = purple_contact_info_get_person(info);
	g_assert_true(person1 == person);

	changed = FALSE;

	removed = purple_person_remove_contact_info(person, info);
	g_assert_true(removed);
	n_items = g_list_model_get_n_items(G_LIST_MODEL(person));
	g_assert_cmpuint(n_items, ==, 0);
	g_assert_true(changed);

	person1 = purple_contact_info_get_person(info);
	g_assert_null(person1);

	g_clear_object(&person);
	g_clear_object(&info);
}

static void
test_purple_person_contacts_multiple(void) {
	PurplePerson *person = NULL;
	GPtrArray *infos = NULL;
	guint n_items = 0;
	const gint n_infos = 5;
	gboolean changed = FALSE;

	person = purple_person_new();
	g_signal_connect(person, "items-changed",
	                 G_CALLBACK(test_purple_person_items_changed_cb), &changed);

	infos = g_ptr_array_new_full(n_infos, g_object_unref);
	for(gint i = 0; i < n_infos; i++) {
		PurpleContactInfo *info = NULL;
		gchar *username = NULL;

		changed = FALSE;

		n_items = g_list_model_get_n_items(G_LIST_MODEL(person));
		g_assert_cmpuint(n_items, ==, i);

		username = g_strdup_printf("username%d", i);
		info = purple_contact_info_new(NULL);
		purple_contact_info_set_username(info, username);
		g_free(username);

		/* Add the contact info to the ptr array so we can remove it below. */
		g_ptr_array_add(infos, info);

		/* Add the contact info to the person and make sure that all the magic
		 * happened.
		 */
		purple_person_add_contact_info(person, info);
		n_items = g_list_model_get_n_items(G_LIST_MODEL(person));
		g_assert_cmpuint(n_items, ==, i + 1);
		g_assert_true(changed);
	}

	for(gint i = 0; i < n_infos; i++) {
		PurpleContactInfo *info = g_ptr_array_index(infos, i);
		gboolean removed = FALSE;

		changed = FALSE;

		n_items = g_list_model_get_n_items(G_LIST_MODEL(person));
		g_assert_cmpuint(n_items, ==, n_infos - i);

		removed = purple_person_remove_contact_info(person, info);
		g_assert_true(removed);

		n_items = g_list_model_get_n_items(G_LIST_MODEL(person));
		g_assert_cmpuint(n_items, ==, n_infos - (i + 1));

		g_assert_true(changed);
	}

	/* Final sanity check that the person has no more contacts. */
	n_items = g_list_model_get_n_items(G_LIST_MODEL(person));
	g_assert_cmpuint(n_items, ==, 0);

	g_ptr_array_free(infos, TRUE);

	g_clear_object(&person);
}

static void
test_purple_person_priority_single(void) {
	PurpleContactInfo *info = NULL;
	PurpleContactInfo *priority = NULL;
	PurplePerson *person = NULL;
	PurplePresence *presence = NULL;
	PurpleStatus *status = NULL;
	PurpleStatusType *status_type = NULL;
	gboolean called = FALSE;

	person = purple_person_new();
	g_signal_connect(person, "notify::priority-contact-info",
	                 G_CALLBACK(test_purple_person_notify_cb), &called);
	priority = purple_person_get_priority_contact_info(person);
	g_assert_null(priority);

	/* Now create a real contact. */
	info = purple_contact_info_new(NULL);
	purple_person_add_contact_info(person, info);

	/* Set the status of the contact. */
	presence = purple_contact_info_get_presence(info);
	status_type = purple_status_type_new(PURPLE_STATUS_AVAILABLE, "available",
	                                     "Available", FALSE);
	status = purple_status_new(status_type, presence);
	g_object_set(G_OBJECT(presence), "active-status", status, NULL);
	g_clear_object(&status);

	g_assert_true(called);

	priority = purple_person_get_priority_contact_info(person);
	g_assert_true(priority == info);

	purple_status_type_destroy(status_type);
	g_clear_object(&person);
	g_clear_object(&info);
	g_clear_object(&presence);
}

static void
test_purple_person_priority_multiple_with_change(void) {
	PurpleContactInfo *priority = NULL;
	PurpleContactInfo *first = NULL;
	PurpleContactInfo *sorted_contact = NULL;
	PurplePerson *person = NULL;
	PurplePresence *sorted_presence = NULL;
	PurpleStatus *status = NULL;
	PurpleStatusType *available = NULL;
	PurpleStatusType *offline = NULL;
	gboolean changed = FALSE;
	gint n_infos = 5;
	guint n_items = 0;

	/* This unit test is a bit complicated, but it adds 5 contact infos to a
	 * person all whose presences are set to offline. After adding all the
	 * contact infos, we verify that the first contact info we added is the
	 * priority contact info. Then we flip the active status of the n_infos - 2
	 * infos to available. This should make it the priority contact info which
	 * we then assert.
	 */

	/* Create our status types. */
	available = purple_status_type_new(PURPLE_STATUS_AVAILABLE, "available",
	                                   "Available", FALSE);
	offline = purple_status_type_new(PURPLE_STATUS_OFFLINE, "offline",
	                                 "Offline", FALSE);

	/* Create the person and connected to the notify signal for the
	 * priority-contact property.
	 */
	person = purple_person_new();
	g_signal_connect(person, "notify::priority-contact-info",
	                 G_CALLBACK(test_purple_person_notify_cb), &changed);
	priority = purple_person_get_priority_contact_info(person);
	g_assert_null(priority);

	/* Create and add all contact infos. */
	for(gint i = 0; i < n_infos; i++) {
		PurpleContactInfo *info = NULL;
		PurplePresence *presence = NULL;
		gchar *username = NULL;

		/* Set changed to false as it shouldn't be changed. */
		changed = FALSE;

		/* Now create a real contact. */
		username = g_strdup_printf("username%d", i + 1);
		info = purple_contact_info_new(NULL);
		purple_contact_info_set_username(info, username);
		g_free(username);

		/* Set the status for the contact. */
		presence = purple_contact_info_get_presence(info);
		status = purple_status_new(offline, presence);
		g_object_set(G_OBJECT(presence), "active-status", status, NULL);
		g_clear_object(&status);

		purple_person_add_contact_info(person, info);

		if(i == 0) {
			first = g_object_ref(info);
			g_assert_true(changed);
		} else {
			g_assert_false(changed);

			if(i == n_infos - 2) {
				sorted_contact = g_object_ref(info);
				sorted_presence = g_object_ref(presence);
			}
		}

		g_clear_object(&info);
	}

	n_items = g_list_model_get_n_items(G_LIST_MODEL(person));
	g_assert_cmpuint(n_items, ==, n_infos);

	priority = purple_person_get_priority_contact_info(person);
	g_assert_true(priority == first);
	g_clear_object(&first);

	/* Now set the second from the last contact info's status to available, and
	 * verify that that contact info is now the priority contact info.
	 */
	changed = FALSE;
	status = purple_status_new(available, sorted_presence);
	g_object_set(G_OBJECT(sorted_presence), "active-status", status, NULL);
	g_clear_object(&status);
	g_assert_true(changed);
	priority = purple_person_get_priority_contact_info(person);
	g_assert_true(priority == sorted_contact);

	/* Cleanup. */
	purple_status_type_destroy(offline);
	purple_status_type_destroy(available);

	g_clear_object(&sorted_contact);
	g_clear_object(&sorted_presence);

	g_clear_object(&person);
}

/******************************************************************************
 * Matches tests
 *****************************************************************************/
static void
test_purple_person_matches_accepts_null(void) {
	PurplePerson *person = purple_person_new();

	g_assert_true(purple_person_matches(person, NULL));

	g_clear_object(&person);
}

static void
test_purple_person_matches_empty_string(void) {
	PurplePerson *person = purple_person_new();

	g_assert_true(purple_person_matches(person, ""));

	g_clear_object(&person);
}

static void
test_purple_person_matches_alias(void) {
	PurplePerson *person = purple_person_new();

	purple_person_set_alias(person, "this is the alias");

	g_assert_true(purple_person_matches(person, "the"));
	g_assert_false(purple_person_matches(person, "what"));

	g_clear_object(&person);
}

static void
test_purple_person_matches_contact_info(void) {
	PurplePerson *person = purple_person_new();
	PurpleContactInfo *info = purple_contact_info_new(NULL);

	purple_contact_info_set_username(info, "user1");
	purple_person_add_contact_info(person, info);
	g_clear_object(&info);

	g_assert_true(purple_person_matches(person, "user1"));

	g_clear_object(&person);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	test_ui_purple_init();

	g_test_add_func("/person/new",
	                test_purple_person_new);
	g_test_add_func("/person/properties",
	                test_purple_person_properties);

	g_test_add_func("/person/avatar-for-display/person",
	                test_purple_person_avatar_for_display_person);
	g_test_add_func("/person/avatar-for-display/contact",
	                test_purple_person_avatar_for_display_contact);

	g_test_add_func("/person/name-for-display/person",
	                test_purple_person_name_for_display_person);
	g_test_add_func("/person/name-for-display/contact",
	                test_purple_person_name_for_display_contact);

	g_test_add_func("/person/contacts/single",
	                test_purple_person_contacts_single);
	g_test_add_func("/person/contacts/multiple",
	                test_purple_person_contacts_multiple);

	g_test_add_func("/person/priority/single",
	                test_purple_person_priority_single);
	g_test_add_func("/person/priority/multiple-with-change",
	                test_purple_person_priority_multiple_with_change);

	g_test_add_func("/person/matches/accepts_null",
	                test_purple_person_matches_accepts_null);
	g_test_add_func("/person/matches/empty_string",
	                test_purple_person_matches_empty_string);
	g_test_add_func("/person/matches/alias",
	                test_purple_person_matches_alias);
	g_test_add_func("/person/matches/contact_info",
	                test_purple_person_matches_contact_info);

	return g_test_run();
}
