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

typedef struct {
	const gchar *initial;
	const gchar *expected;
	gint start_offset;
	gint end_offset;
} TalkatuTestHtmlSerializeData;

static TalkatuTestHtmlSerializeData *
talkatu_test_html_serialization_data_new(const gchar *initial,
                                         const gchar *expected,
                                         gint start_offset,
                                         gint end_offset)
{
	TalkatuTestHtmlSerializeData *d = g_new(TalkatuTestHtmlSerializeData, 1);

	d->initial = initial;
	d->expected = expected;
	d->start_offset = start_offset;
	d->end_offset = end_offset;

	return d;
}

static void
talkatu_test_html_serialization(gconstpointer data) {
	GtkTextBuffer *buffer = NULL;
	GSimpleActionGroup *ag = NULL;
	const gchar *html = (const gchar *)data;
	gchar *actual = NULL;

	ag = talkatu_action_group_new(TALKATU_FORMAT_HTML);
	buffer = talkatu_buffer_new(ag);
	talkatu_action_group_set_buffer(TALKATU_ACTION_GROUP(ag), buffer);

	talkatu_markup_set_html(TALKATU_BUFFER(buffer), html, -1);
	actual = talkatu_markup_get_html(buffer, NULL);

	g_assert_cmpstr(actual, ==, html);

	g_free(actual);
}

static void
talkatu_test_html_serialization_range(gconstpointer data) {
	TalkatuTestHtmlSerializeData *d = (TalkatuTestHtmlSerializeData *)data;
	GtkTextBuffer *buffer = NULL;
	GtkTextIter start, end;
	GSimpleActionGroup *ag = NULL;
	gchar *actual = NULL;

	ag = talkatu_action_group_new(TALKATU_FORMAT_HTML);
	buffer = talkatu_buffer_new(ag);
	talkatu_action_group_set_buffer(TALKATU_ACTION_GROUP(ag), buffer);

	talkatu_markup_set_html(TALKATU_BUFFER(buffer), d->initial, -1);

	gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &start, d->start_offset);
	gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &end, d->end_offset);
	actual = talkatu_markup_get_html_range(buffer, &start, &end, NULL);

	g_assert_cmpstr(actual, ==, d->expected);

	g_free(actual);
}

gint
main(gint argc, gchar **argv) {
	TalkatuTestHtmlSerializeData *d;
	gint ret;

	g_test_init(&argc, &argv, NULL);

	gtk_init();

	talkatu_init();

	g_test_add_data_func(
		"/html/serialization/plain",
		"foo",
		talkatu_test_html_serialization
	);

	g_test_add_data_func(
		"/html/serialization/simple",
		"foo <b>bar</b> baz",
		talkatu_test_html_serialization
	);

	g_test_add_data_func(
		"/html/serialization/start",
		"<b>foo bar</b> baz",
		talkatu_test_html_serialization
	);

	g_test_add_data_func(
		"/html/serialization/end",
		"foo <b>bar baz</b>",
		talkatu_test_html_serialization
	);

	g_test_add_data_func(
		"/html/serialization/not-a-tag",
		"html ",
		talkatu_test_html_serialization
	);

	/* range tests */
	d = talkatu_test_html_serialization_data_new(
		"<b>bold</b>",
		"<b>old</b>",
		1,
		4
	);
	g_test_add_data_func_full(
		"/html/serialization/already-opened",
		d,
		talkatu_test_html_serialization_range,
		g_free
	);

	d = talkatu_test_html_serialization_data_new(
		"<b>bold</b>",
		"<b>bol</b>",
		0,
		3
	);
	g_test_add_data_func_full(
		"/html/serialization/not-closed",
		d,
		talkatu_test_html_serialization_range,
		g_free
	);

	g_test_add_data_func_full(
		"/html/serialization/open-late-does-not-close",
		talkatu_test_html_serialization_data_new(
			"<b>bold</b> <i>italic</i>",
			"<b>bold</b> <i>it</i>",
			0,
			7
		),
		talkatu_test_html_serialization_range,
		g_free
	);

	g_test_add_data_func_full(
		"/html/serialization/unknown-tags",
		talkatu_test_html_serialization_data_new(
			"<html><head></head><body><p><b>hiya</b></p></body></html>",
			"<b>hiya</b>",
			0,
			4
		),
		talkatu_test_html_serialization_range,
		g_free
	);

	ret = g_test_run();

	talkatu_uninit();

	return ret;
}

