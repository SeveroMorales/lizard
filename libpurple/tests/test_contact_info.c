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
test_purple_contact_info_new(void) {
	PurpleContactInfo *info = NULL;

	info = purple_contact_info_new("id");

	g_assert_cmpstr(purple_contact_info_get_id(info), ==, "id");

	g_clear_object(&info);
}

static void
test_purple_contact_info_properties(void) {
	PurpleContactInfo *info = NULL;
	PurpleContactInfoPermission permission;
	PurplePerson *person = NULL;
	PurplePerson *person1 = NULL;
	PurplePresence *presence1 = NULL;
	PurpleTags *tags = NULL;
	GdkPixbuf *avatar = NULL;
	GdkPixbuf *avatar1 = NULL;
	char *id = NULL;
	char *username = NULL;
	char *display_name = NULL;
	char *alias = NULL;
	char *color = NULL;
	char *name_for_display = NULL;

	avatar = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 1, 1);
	person = purple_person_new();

	/* Use g_object_new so we can test setting properties by name. All of them
	 * call the setter methods, so by doing it this way we exercise more of the
	 * code.
	 */
	info = g_object_new(
		PURPLE_TYPE_CONTACT_INFO,
		"id", "id1",
		"username", "username",
		"display-name", "display-name",
		"alias", "alias",
		"color", "#e9c636",
		"avatar", avatar,
		"person", person,
		"permission", PURPLE_CONTACT_INFO_PERMISSION_ALLOW,
		NULL);

	/* Now use g_object_get to read all of the properties. */
	g_object_get(info,
		"id", &id,
		"username", &username,
		"display-name", &display_name,
		"alias", &alias,
		"color", &color,
		"avatar", &avatar1,
		"presence", &presence1,
		"tags", &tags,
		"person", &person1,
		"permission", &permission,
		"name-for-display", &name_for_display,
		NULL);

	/* Compare all the things. */
	g_assert_cmpstr(id, ==, "id1");
	g_assert_cmpstr(username, ==, "username");
	g_assert_cmpstr(display_name, ==, "display-name");
	g_assert_cmpstr(alias, ==, "alias");
	g_assert_cmpstr(color, ==, "#e9c636");
	g_assert_cmpstr(name_for_display, ==, "alias");
	g_assert_true(avatar1 == avatar);
	g_assert_nonnull(presence1);
	g_assert_nonnull(tags);
	g_assert_true(person1 == person);
	g_assert_true(permission == PURPLE_CONTACT_INFO_PERMISSION_ALLOW);

	/* Free/unref all the things. */
	g_clear_pointer(&id, g_free);
	g_clear_pointer(&username, g_free);
	g_clear_pointer(&display_name, g_free);
	g_clear_pointer(&alias, g_free);
	g_clear_pointer(&color, g_free);
	g_clear_pointer(&name_for_display, g_free);
	g_clear_object(&avatar1);
	g_clear_object(&presence1);
	g_clear_object(&tags);
	g_clear_object(&person);
	g_clear_object(&person1);

	g_clear_object(&avatar);
	g_clear_object(&info);
}

/******************************************************************************
 * get_name_for_display tests
 *****************************************************************************/
static void
test_purple_contact_info_get_name_for_display_person_with_alias(void) {
	PurpleContactInfo *info = NULL;
	PurplePerson *person = NULL;
	const char *alias = NULL;

	person = purple_person_new();
	purple_person_set_alias(person, "person alias");

	info = purple_contact_info_new("id");
	/* we don't set the alias on the contact info, as that takes priority over
	 * the person's alias.
	 */
	purple_contact_info_set_username(info, "username");
	purple_contact_info_set_display_name(info, "display name");
	purple_contact_info_set_person(info, person);

	alias = purple_contact_info_get_name_for_display(info);
	g_assert_cmpstr(alias, ==, "person alias");

	g_clear_object(&info);
	g_clear_object(&person);
}

static void
test_purple_contact_info_get_name_for_display_contact_info_with_alias(void) {
	PurpleContactInfo *info = NULL;
	PurplePerson *person = NULL;
	const char *alias = NULL;

	person = purple_person_new();
	purple_person_set_alias(person, "person alias");

	info = purple_contact_info_new("id");
	purple_contact_info_set_person(info, person);
	purple_contact_info_set_alias(info, "contact alias");
	purple_contact_info_set_username(info, "username");
	purple_contact_info_set_display_name(info, "display name");


	alias = purple_contact_info_get_name_for_display(info);
	g_assert_cmpstr(alias, ==, "contact alias");

	g_clear_object(&info);
	g_clear_object(&person);
}

static void
test_purple_contact_info_get_name_for_display_contact_info_with_display_name(void) {
	PurpleContactInfo *info = NULL;
	PurplePerson *person = NULL;
	const char *alias = NULL;

	person = purple_person_new();

	info = purple_contact_info_new("id");
	purple_contact_info_set_person(info, person);

	purple_contact_info_set_display_name(info, "display name");
	purple_contact_info_set_username(info, "username");

	alias = purple_contact_info_get_name_for_display(info);
	g_assert_cmpstr(alias, ==, "display name");

	g_clear_object(&info);
	g_clear_object(&person);
}

static void
test_purple_contact_info_get_name_for_display_username_fallback(void) {
	PurpleContactInfo *info = NULL;
	PurplePerson *person = NULL;
	const char *alias = NULL;

	person = purple_person_new();

	info = purple_contact_info_new("id");
	purple_contact_info_set_username(info, "username");
	purple_contact_info_set_person(info, person);

	alias = purple_contact_info_get_name_for_display(info);
	g_assert_cmpstr(alias, ==, "username");

	g_clear_object(&info);
	g_clear_object(&person);
}

static void
test_purple_contact_info_get_name_for_display_id_fallback(void) {
	PurpleContactInfo *info = NULL;
	PurplePerson *person = NULL;
	const char *alias = NULL;

	person = purple_person_new();

	info = purple_contact_info_new("id");
	purple_contact_info_set_person(info, person);

	alias = purple_contact_info_get_name_for_display(info);
	g_assert_cmpstr(alias, ==, "id");

	g_clear_object(&info);
	g_clear_object(&person);
}

/******************************************************************************
 * purple_contact_info_compare tests
 *****************************************************************************/
static void
test_purple_contact_info_compare_not_null__null(void) {
	PurpleContactInfo *info = NULL;

	info = purple_contact_info_new(NULL);

	g_assert_cmpint(purple_contact_info_compare(info, NULL), <, 0);

	g_clear_object(&info);
}

static void
test_purple_contact_info_compare_null__not_null(void) {
	PurpleContactInfo *info = NULL;

	info = purple_contact_info_new(NULL);

	g_assert_cmpint(purple_contact_info_compare(NULL, info), >, 0);

	g_clear_object(&info);
}

static void
test_purple_contact_info_compare_null__null(void) {
	g_assert_cmpint(purple_contact_info_compare(NULL, NULL), ==, 0);
}

static void
test_purple_contact_info_compare_person__no_person(void) {
	PurpleContactInfo *info_a = NULL;
	PurpleContactInfo *info_b = NULL;
	PurplePerson *person = NULL;

	info_a = purple_contact_info_new(NULL);
	person = purple_person_new();
	purple_contact_info_set_person(info_a, person);

	info_b = purple_contact_info_new(NULL);

	g_assert_cmpint(purple_contact_info_compare(info_a, info_b), <, 0);

	g_clear_object(&info_a);
	g_clear_object(&info_b);
	g_clear_object(&person);
}

static void
test_purple_contact_info_compare_no_person__person(void) {
	PurpleContactInfo *info_a = NULL;
	PurpleContactInfo *info_b = NULL;
	PurplePerson *person = NULL;

	info_a = purple_contact_info_new(NULL);

	info_b = purple_contact_info_new(NULL);
	person = purple_person_new();
	purple_contact_info_set_person(info_b, person);

	g_assert_cmpint(purple_contact_info_compare(info_a, info_b), >, 0);

	g_clear_object(&info_a);
	g_clear_object(&info_b);
	g_clear_object(&person);
}

static void
test_purple_contact_info_compare_name__name(void) {
	PurpleContactInfo *info_a = NULL;
	PurpleContactInfo *info_b = NULL;

	info_a = purple_contact_info_new(NULL);
	purple_contact_info_set_username(info_a, "aaa");

	info_b = purple_contact_info_new(NULL);
	purple_contact_info_set_username(info_b, "zzz");

	g_assert_cmpint(purple_contact_info_compare(info_a, info_b), <, 0);
	g_assert_cmpint(purple_contact_info_compare(info_b, info_a), >, 0);

	purple_contact_info_set_username(info_b, "aaa");
	g_assert_cmpint(purple_contact_info_compare(info_b, info_a), ==, 0);

	g_clear_object(&info_a);
	g_clear_object(&info_b);
}

/******************************************************************************
 * Matches
 *****************************************************************************/
static void
test_purple_contact_info_matches_accepts_null(void) {
	PurpleContactInfo *info = purple_contact_info_new(NULL);

	g_assert_true(purple_contact_info_matches(info, NULL));

	g_clear_object(&info);
}

static void
test_purple_contact_info_matches_empty_string(void) {
	PurpleContactInfo *info = purple_contact_info_new(NULL);

	g_assert_true(purple_contact_info_matches(info, ""));

	g_clear_object(&info);
}

static void
test_purple_contact_info_matches_username(void) {
	PurpleContactInfo *info = purple_contact_info_new(NULL);

	purple_contact_info_set_username(info, "username");

	g_assert_true(purple_contact_info_matches(info, "name"));

	g_clear_object(&info);
}

static void
test_purple_contact_info_matches_alias(void) {
	PurpleContactInfo *info = purple_contact_info_new(NULL);

	purple_contact_info_set_alias(info, "alias");

	g_assert_true(purple_contact_info_matches(info, "lia"));

	g_clear_object(&info);
}

static void
test_purple_contact_info_matches_display_name(void) {
	PurpleContactInfo *info = purple_contact_info_new(NULL);

	purple_contact_info_set_display_name(info, "display name");

	g_assert_true(purple_contact_info_matches(info, "play"));

	g_clear_object(&info);
}

static void
test_purple_contact_info_matches_none(void) {
	PurpleContactInfo *info = purple_contact_info_new("id");

	purple_contact_info_set_username(info, "username");
	purple_contact_info_set_alias(info, "alias");
	purple_contact_info_set_display_name(info, "display name");

	g_assert_false(purple_contact_info_matches(info, "nothing"));

	g_clear_object(&info);
}

/******************************************************************************
 * presence-changed signal tests
 *****************************************************************************/
static void
test_purple_contact_info_presence_changed_callback(PurpleContactInfo *info,
                                                   PurplePresence *presence,
                                                   GParamSpec *pspec,
                                                   gpointer data)
{
	guint *counter = data;

	g_assert_true(PURPLE_IS_CONTACT_INFO(info));
	g_assert_true(PURPLE_IS_PRESENCE(presence));
	g_assert_true(G_IS_PARAM_SPEC(pspec));

	*counter = *counter + 1;
}

static void
test_purple_contact_info_presence_changed_signal(void) {
	PurpleContactInfo *info = NULL;
	PurplePresence *presence = NULL;
	guint counter = 0;

	/* Create the info and add our callbacks, one for everything and another
	 * for just idle to make sure detail works.
	 */
	info = purple_contact_info_new(NULL);
	g_signal_connect(info, "presence-changed",
	                 G_CALLBACK(test_purple_contact_info_presence_changed_callback),
	                 &counter);
	g_signal_connect(info, "presence-changed::idle",
	                 G_CALLBACK(test_purple_contact_info_presence_changed_callback),
	                 &counter);

	/* Grab the presence and start changing stuff. */
	presence = purple_contact_info_get_presence(info);
	g_assert_true(PURPLE_IS_PRESENCE(presence));

	/* Set the presence as idle with no time, which should call our callback
	 * three times, twice for the non-detailed callback, and once for the
	 * detailed callback.
	 */
	g_assert_cmpint(counter, ==, 0);
	purple_presence_set_idle(presence, TRUE, NULL);
	g_assert_cmpint(counter, ==, 3);

	/* Cleanup. */
	g_clear_object(&info);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	test_ui_purple_init();

	g_test_add_func("/contact-info/new",
	                test_purple_contact_info_new);
	g_test_add_func("/contact-info/properties",
	                test_purple_contact_info_properties);

	g_test_add_func("/contact-info/get_name_for_display/person_with_alias",
	                test_purple_contact_info_get_name_for_display_person_with_alias);
	g_test_add_func("/contact-info/get_name_for_display/contact_with_alias",
	                test_purple_contact_info_get_name_for_display_contact_info_with_alias);
	g_test_add_func("/contact-info/get_name_for_display/contact_with_display_name",
	                test_purple_contact_info_get_name_for_display_contact_info_with_display_name);
	g_test_add_func("/contact-info/get_name_for_display/username_fallback",
	                test_purple_contact_info_get_name_for_display_username_fallback);
	g_test_add_func("/contact-info/get_name_for_display/id_fallback",
	                test_purple_contact_info_get_name_for_display_id_fallback);

	g_test_add_func("/contact-info/compare/not_null__null",
	                test_purple_contact_info_compare_not_null__null);
	g_test_add_func("/contact-info/compare/null__not_null",
	                test_purple_contact_info_compare_null__not_null);
	g_test_add_func("/contact-info/compare/null__null",
	                test_purple_contact_info_compare_null__null);
	g_test_add_func("/contact-info/compare/person__no_person",
	                test_purple_contact_info_compare_person__no_person);
	g_test_add_func("/contact-info/compare/no_person__person",
	                test_purple_contact_info_compare_no_person__person);
	g_test_add_func("/contact-info/compare/name__name",
	                test_purple_contact_info_compare_name__name);

	g_test_add_func("/contact-info/matches/accepts_null",
	                test_purple_contact_info_matches_accepts_null);
	g_test_add_func("/contact-info/matches/emptry_string",
	                test_purple_contact_info_matches_empty_string);
	g_test_add_func("/contact-info/matches/username",
	                test_purple_contact_info_matches_username);
	g_test_add_func("/contact-info/matches/alias",
	                test_purple_contact_info_matches_alias);
	g_test_add_func("/contact-info/matches/display_name",
	                test_purple_contact_info_matches_display_name);
	g_test_add_func("/contact-info/matches/none",
	                test_purple_contact_info_matches_none);

	g_test_add_func("/contact-info/presence-changed-signal",
	                test_purple_contact_info_presence_changed_signal);

	return g_test_run();
}
