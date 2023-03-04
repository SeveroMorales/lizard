/*
 * talkatu
 * Copyright (C) 2017-2021 Gary Kramlich <grim@reaperworld.com>
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

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_talkatu_html_pango_renderer_simple(void) {
	TalkatuHtmlRenderer *renderer = talkatu_html_pango_renderer_new();
	const gchar *inp = "plain text";
	const gchar *exp = "plain text";
	const gchar *act = NULL;

	talkatu_html_renderer_render(renderer, inp);
	act = talkatu_html_pango_renderer_get_string(TALKATU_HTML_PANGO_RENDERER(renderer));

	g_assert_cmpstr(act, ==, exp);

	g_object_unref(G_OBJECT(renderer));
}

static void
test_talkatu_html_pango_renderer_mixed(void) {
	TalkatuHtmlRenderer *renderer = talkatu_html_pango_renderer_new();
	const gchar *inp = \
		"<html><head></head><body>" \
		"<i><b>emphasis <u>underline</u></b> <strike>strike</strike> italic</i>" \
		"</body></html>";
	const gchar *exp = \
		"<i><b>emphasis <u>underline</u></b> <s>strike</s> italic</i>";
	const gchar *act = NULL;

	talkatu_html_renderer_render(renderer, inp);
	act = talkatu_html_pango_renderer_get_string(TALKATU_HTML_PANGO_RENDERER(renderer));

	g_assert_cmpstr(act, ==, exp);

	g_object_unref(G_OBJECT(renderer));
}

static void
test_talkatu_html_pango_renderer_with_comment(void) {
	TalkatuHtmlRenderer *renderer = talkatu_html_pango_renderer_new();
	const gchar *inp = \
		"Hello, <!--Darkness, my old friend, here to conquer the--> World!";
	const gchar *exp = "Hello,  World!";
	const gchar *act = NULL;

	talkatu_html_renderer_render(renderer, inp);
	act = talkatu_html_pango_renderer_get_string(TALKATU_HTML_PANGO_RENDERER(renderer));

	g_assert_cmpstr(act, ==, exp);

	g_object_unref(G_OBJECT(renderer));
}

static void
test_talkatu_html_pango_renderer_with_attributes(void) {
	TalkatuHtmlRenderer *renderer = talkatu_html_pango_renderer_new();
	const gchar *inp = \
		"<html><head></head><body>" \
		"<font size=\"2\" color=\"#007f00\">talkatu</font>" \
		"</body></html>";
	const gchar *exp = "<span font=\"2\" foreground=\"#007f00\">talkatu</span>";
	const gchar *act = NULL;

	talkatu_html_renderer_render(renderer, inp);
	act = talkatu_html_pango_renderer_get_string(TALKATU_HTML_PANGO_RENDERER(renderer));

	g_assert_cmpstr(act, ==, exp);

	g_object_unref(G_OBJECT(renderer));
}

static void
test_talkatu_html_pango_renderer_with_nested_attributes(void) {
	TalkatuHtmlRenderer *renderer = talkatu_html_pango_renderer_new();
	const gchar *inp = \
		"<html><head></head><body>" \
		"<font size=\"2\" color=\"#007f00\">" \
		"<a href=\"https://keep.imfreedom.org/talkatu/talkatu/\">talkatu</a>" \
		"</font>" \
		"</body></html>";
	const gchar *exp = \
		"<span font=\"2\" foreground=\"#007f00\">" \
		"<a href=\"https://keep.imfreedom.org/talkatu/talkatu/\">talkatu</a>" \
		"</span>";
	const gchar *act = NULL;

	talkatu_html_renderer_render(renderer, inp);
	act = talkatu_html_pango_renderer_get_string(TALKATU_HTML_PANGO_RENDERER(renderer));

	g_assert_cmpstr(act, ==, exp);

	g_object_unref(G_OBJECT(renderer));
}

static void
test_talkatu_html_pango_renderer_code(void) {
	TalkatuHtmlRenderer *renderer = talkatu_html_pango_renderer_new();
	const gchar *inp = \
		"<html><head></head><body>" \
		"<code>monkey</code>" \
		"</body></html>";
	const gchar *exp = \
		"<span font=\"monospace\">monkey</span>";
	const gchar *act = NULL;

	talkatu_html_renderer_render(renderer, inp);
	act = talkatu_html_pango_renderer_get_string(TALKATU_HTML_PANGO_RENDERER(renderer));

	g_assert_cmpstr(act, ==, exp);

	g_object_unref(G_OBJECT(renderer));
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv) {
	gint ret = 0;

	g_test_init(&argc, &argv, NULL);

	gtk_init();

	talkatu_init();

	g_test_add_func(
		"/html-pango-renderer/simple",
		test_talkatu_html_pango_renderer_simple);

	g_test_add_func(
		"/html-pango-renderer/mixed-children",
		test_talkatu_html_pango_renderer_mixed);

	g_test_add_func(
		"/html-pango-renderer/with-comment",
		test_talkatu_html_pango_renderer_with_comment);

	g_test_add_func(
		"/html-pango-renderer/with-attributes",
		test_talkatu_html_pango_renderer_with_attributes);

	g_test_add_func(
		"/html-pango-renderer/with-nested-attributes",
		test_talkatu_html_pango_renderer_with_nested_attributes);

	g_test_add_func(
		"/html-pango-renderer/code",
		test_talkatu_html_pango_renderer_code);

	ret = g_test_run();

	talkatu_uninit();

	return ret;
}

