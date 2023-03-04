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
test_purple_avatar_new_static(void) {
	PurpleAvatar *avatar = NULL;
	GdkPixbuf *pixbuf = NULL;
	GError *error = NULL;
	const char *resource = "/im/pidgin/libpurple/tests/avatar/static.png";

	avatar = purple_avatar_new_from_resource(resource, &error);
	g_assert_no_error(error);
	g_assert_true(PURPLE_IS_AVATAR(avatar));

	g_assert_null(purple_avatar_get_filename(avatar));
	g_assert_false(purple_avatar_get_animated(avatar));
	g_assert_null(purple_avatar_get_animation(avatar));

	pixbuf = purple_avatar_get_pixbuf(avatar);
	g_assert_true(GDK_IS_PIXBUF(pixbuf));

	g_clear_object(&avatar);
}

static void
test_purple_avatar_new_animated(void) {
	PurpleAvatar *avatar = NULL;
	GdkPixbuf *pixbuf = NULL;
	GdkPixbufAnimation *animation = NULL;
	GError *error = NULL;
	const char *resource = "/im/pidgin/libpurple/tests/avatar/animated.gif";

	avatar = purple_avatar_new_from_resource(resource, &error);
	g_assert_no_error(error);
	g_assert_true(PURPLE_IS_AVATAR(avatar));

	g_assert_null(purple_avatar_get_filename(avatar));
	g_assert_true(purple_avatar_get_animated(avatar));

	pixbuf = purple_avatar_get_pixbuf(avatar);
	g_assert_true(GDK_IS_PIXBUF(pixbuf));

	animation = purple_avatar_get_animation(avatar);
	g_assert_true(GDK_IS_PIXBUF_ANIMATION(animation));

	g_clear_object(&avatar);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/avatar/new/static",
	                test_purple_avatar_new_static);
	g_test_add_func("/avatar/new/animated",
	                test_purple_avatar_new_animated);

	return g_test_run();
}
