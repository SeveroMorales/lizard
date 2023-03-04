/*
 * talkatu
 * Copyright (C) 2017-2020 Gary Kramlich <grim@reaperworld.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <talkatu.h>

static void
talkatu_test_action_group_activate_simple(void) {
	GSimpleActionGroup *ag = NULL;
	GtkTextBuffer *buffer = NULL;
	gchar **activated = NULL;

	ag = talkatu_action_group_new(TALKATU_FORMAT_HTML);
	buffer = talkatu_buffer_new(ag);
	talkatu_action_group_set_buffer(TALKATU_ACTION_GROUP(ag), buffer);

	talkatu_action_group_activate_format(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_BOLD);

	activated = talkatu_action_group_get_activated_formats(TALKATU_ACTION_GROUP(ag));

	g_assert_cmpuint(g_strv_length(activated), ==, 1);
	g_assert_true(g_strv_contains((const gchar * const *)activated, TALKATU_ACTION_FORMAT_BOLD));

	g_strfreev(activated);

	g_object_unref(G_OBJECT(buffer));
}

static void
talkatu_test_action_group_activate_multiple(void) {
	GSimpleActionGroup *ag = NULL;
	GtkTextBuffer *buffer = NULL;
	gchar **activated = NULL;

	ag = talkatu_action_group_new(TALKATU_FORMAT_HTML);
	buffer = talkatu_buffer_new(ag);
	talkatu_action_group_set_buffer(TALKATU_ACTION_GROUP(ag), buffer);

	talkatu_action_group_activate_format(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_BOLD);
	talkatu_action_group_activate_format(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_ITALIC);

	activated = talkatu_action_group_get_activated_formats(TALKATU_ACTION_GROUP(ag));

	g_assert_cmpuint(g_strv_length(activated), ==, 2);
	g_assert_true(g_strv_contains((const gchar * const *)activated, TALKATU_ACTION_FORMAT_BOLD));
	g_assert_true(g_strv_contains((const gchar * const *)activated, TALKATU_ACTION_FORMAT_ITALIC));

	g_strfreev(activated);

	g_object_unref(G_OBJECT(buffer));
}

static void
talkatu_test_action_group_activate_complex(void) {
	GSimpleActionGroup *ag = NULL;
	GtkTextBuffer *buffer = NULL;
	gchar **activated = NULL;

	ag = talkatu_action_group_new(TALKATU_FORMAT_HTML);
	buffer = talkatu_buffer_new(ag);
	talkatu_action_group_set_buffer(TALKATU_ACTION_GROUP(ag), buffer);

	talkatu_action_group_activate_format(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_BOLD);
	talkatu_action_group_activate_format(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_ITALIC);
	talkatu_action_group_activate_format(TALKATU_ACTION_GROUP(ag), TALKATU_ACTION_FORMAT_BOLD);

	activated = talkatu_action_group_get_activated_formats(TALKATU_ACTION_GROUP(ag));

	g_assert_cmpuint(g_strv_length(activated), ==, 1);
	g_assert_true(g_strv_contains((const gchar * const *)activated, TALKATU_ACTION_FORMAT_ITALIC));

	g_strfreev(activated);

	g_object_unref(G_OBJECT(buffer));
}



gint
main(gint argc, gchar **argv) {
	gint ret = 0;

	g_test_init(&argc, &argv, NULL);

	gtk_init();

	talkatu_init();

	g_test_add_func(
		"/action-group/activate/simple",
		talkatu_test_action_group_activate_simple
	);
	g_test_add_func(
		"/action-group/activate/multiple",
		talkatu_test_action_group_activate_multiple
	);
	g_test_add_func(
		"/action-group/activate/complex",
		talkatu_test_action_group_activate_complex
	);

	ret = g_test_run();

	talkatu_uninit();

	return ret;
}
