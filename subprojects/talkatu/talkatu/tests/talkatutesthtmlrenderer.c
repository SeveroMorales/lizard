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

/******************************************************************************
 * TestTalkatuHtmlRenderer
 *****************************************************************************/
#define TEST_TALKATU_TYPE_HTML_PARSER (test_talkatu_html_renderer_get_type())
G_DECLARE_FINAL_TYPE(TestTalkatuHtmlRenderer, test_talkatu_html_renderer,
                     TEST_TALKATU, HTML_RENDERER, TalkatuHtmlRenderer)

struct _TestTalkatuHtmlRenderer {
	TalkatuHtmlRenderer parent;

	GString *str;
};

static void
test_talkatu_html_renderer_element_start(TalkatuHtmlRenderer *renderer,
                                         const gchar *name,
                                         const gchar **attr_names,
                                         const gchar **attr_values)
{
	TestTalkatuHtmlRenderer *tr = TEST_TALKATU_HTML_RENDERER(renderer);

	g_string_append_printf(tr->str, "<%s", name);

	if(attr_names != NULL) {
		gint i = 0;

		for(i = 0; attr_names[i] != NULL; i++) {
			g_string_append_printf(tr->str, " %s=\"", attr_names[i]);
			if(attr_values[i] != NULL) {
				g_string_append_printf(tr->str, "%s", attr_values[i]);
			}
			g_string_append_printf(tr->str, "\"");
		}
	}

	g_string_append_printf(tr->str, ">");
}

static void
test_talkatu_html_renderer_element_finish(TalkatuHtmlRenderer *renderer,
                                          const gchar *name)
{
	TestTalkatuHtmlRenderer *tr = TEST_TALKATU_HTML_RENDERER(renderer);

	g_string_append_printf(tr->str, "</%s>", name);
}

static void
test_talkatu_html_renderer_text(TalkatuHtmlRenderer *renderer,
                                const gchar *text)
{
	TestTalkatuHtmlRenderer *tr = TEST_TALKATU_HTML_RENDERER(renderer);

	g_string_append_printf(tr->str, "%s", text);
}

static void
test_talkatu_html_renderer_comment(TalkatuHtmlRenderer *renderer,
                                   const gchar *comment)
{
	TestTalkatuHtmlRenderer *tr = TEST_TALKATU_HTML_RENDERER(renderer);

	g_string_append_printf(tr->str, "<!--%s-->", comment);
}

G_DEFINE_TYPE(TestTalkatuHtmlRenderer, test_talkatu_html_renderer,
              TALKATU_TYPE_HTML_RENDERER);

static void
test_talkatu_html_renderer_init(TestTalkatuHtmlRenderer *renderer) {
	renderer->str = g_string_new("");
}

static void
test_talkatu_html_renderer_finalize(GObject *obj) {
	TestTalkatuHtmlRenderer *tr = TEST_TALKATU_HTML_RENDERER(obj);

	g_string_free(tr->str, TRUE);

	G_OBJECT_CLASS(test_talkatu_html_renderer_parent_class)->finalize(obj);
}

static void
test_talkatu_html_renderer_class_init(TestTalkatuHtmlRendererClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	TalkatuHtmlRendererClass *renderer_class = TALKATU_HTML_RENDERER_CLASS(klass);

	obj_class->finalize = test_talkatu_html_renderer_finalize;

	renderer_class->element_start = test_talkatu_html_renderer_element_start;
	renderer_class->element_finish = test_talkatu_html_renderer_element_finish;
	renderer_class->text = test_talkatu_html_renderer_text;
	renderer_class->comment = test_talkatu_html_renderer_comment;
}

TestTalkatuHtmlRenderer *
test_talkatu_html_renderer_new(void) {
	return TEST_TALKATU_HTML_RENDERER(g_object_new(TEST_TALKATU_TYPE_HTML_PARSER,
	                                               NULL));
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_talkatu_html_renderer_simple(void) {
	TestTalkatuHtmlRenderer *renderer = test_talkatu_html_renderer_new();
	const gchar *exp = "<html><head></head><body>plain text</body></html>";

	talkatu_html_renderer_render(TALKATU_HTML_RENDERER(renderer), "plain text");

	g_assert_cmpstr(renderer->str->str, ==, exp);

	g_object_unref(G_OBJECT(renderer));
}

static void
test_talkatu_html_renderer_mixed(void) {
	TestTalkatuHtmlRenderer *renderer = test_talkatu_html_renderer_new();
	const gchar *exp = \
		"<html><head></head><body>" \
		"<i><b>emphasis <u>underline</u></b> <strike>strike</strike> italic</i>" \
		"</body></html>";

	talkatu_html_renderer_render(TALKATU_HTML_RENDERER(renderer), exp);

	g_assert_cmpstr(renderer->str->str, ==, exp);

	g_object_unref(G_OBJECT(renderer));
}

static void
test_talkatu_html_renderer_with_comment(void) {
	TestTalkatuHtmlRenderer *renderer = test_talkatu_html_renderer_new();
	const gchar *exp = \
		"<html><head></head><body>" \
		"Hello, <!--Darkness, my old friend, here to conquer the--> World!" \
		"</body></html>";

	talkatu_html_renderer_render(TALKATU_HTML_RENDERER(renderer), exp);

	g_assert_cmpstr(renderer->str->str, ==, exp);

	g_object_unref(G_OBJECT(renderer));
}

static void
test_talkatu_html_renderer_with_attributes(void) {
	TestTalkatuHtmlRenderer *renderer = test_talkatu_html_renderer_new();
	const gchar *exp = \
		"<html><head></head><body>" \
		"<font size=\"2\" color=\"#007f00\">talkatu</font>" \
		"</body></html>";

	talkatu_html_renderer_render(TALKATU_HTML_RENDERER(renderer), exp);

	g_assert_cmpstr(renderer->str->str, ==, exp);

	g_object_unref(G_OBJECT(renderer));
}

static void
test_talkatu_html_renderer_with_nested_attributes(void) {
	TestTalkatuHtmlRenderer *renderer = test_talkatu_html_renderer_new();
	const gchar *exp = \
		"<html><head></head><body>" \
		"<font size=\"2\" color=\"#007f00\">" \
		"<a href=\"https://keep.imfreedom.org/talkatu/talkatu/\">talkatu</a>" \
		"</font>" \
		"</body></html>";

	talkatu_html_renderer_render(TALKATU_HTML_RENDERER(renderer), exp);

	g_assert_cmpstr(renderer->str->str, ==, exp);

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
		"/html-renderer/simple",
		test_talkatu_html_renderer_simple);

	g_test_add_func(
		"/html-renderer/mixed-children",
		test_talkatu_html_renderer_mixed);

	g_test_add_func(
		"/html-renderer/with-comment",
		test_talkatu_html_renderer_with_comment);

	g_test_add_func(
		"/html-renderer/with-attributes",
		test_talkatu_html_renderer_with_attributes);

	g_test_add_func(
		"/html-renderer/with-nested-attributes",
		test_talkatu_html_renderer_with_nested_attributes);

	ret = g_test_run();

	talkatu_uninit();

	return ret;
}

