/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2021 Gary Kramlich <grim@reaperworld.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include "talkatu/talkatuhtmlpangorenderer.h"

typedef void (*TalkatuHtmlPangoRendererStartFunc)(TalkatuHtmlPangoRenderer *renderer,
                                                  const gchar *name,
                                                  const gchar **attribute_names,
                                                  const gchar **attribute_values);
typedef void (*TalkatuHtmlPangoRendererFinishFunc)(TalkatuHtmlPangoRenderer *renderer,
                                                   const gchar *name);

typedef struct {
	TalkatuHtmlPangoRendererStartFunc start;
	TalkatuHtmlPangoRendererFinishFunc finish;
	const gchar *alias;
} TalkatuHtmlPangoRendererElementFuncs;

struct _TalkatuHtmlPangoRenderer {
	TalkatuHtmlRenderer parent;

	GString *str;

	GHashTable *lookup;
};

G_DEFINE_TYPE(TalkatuHtmlPangoRenderer, talkatu_html_pango_renderer,
              TALKATU_TYPE_HTML_RENDERER)

/******************************************************************************
 * TalkatuHtmlPangoRenderer Element Functions
 *****************************************************************************/
static void
talkatu_html_pango_renderer_passthrough_start(TalkatuHtmlPangoRenderer *renderer,
                                              const gchar *name,
                                              G_GNUC_UNUSED const gchar **attribute_names,
                                              G_GNUC_UNUSED const gchar **attribute_values)
{
	g_string_append_printf(renderer->str, "<%s>", name);
}

static void
talkatu_html_pango_renderer_passthrough_finish(TalkatuHtmlPangoRenderer *renderer,
                                               const gchar *name)
{
	g_string_append_printf(renderer->str, "</%s>", name);
}

static void
talkatu_html_pango_renderer_code_start(TalkatuHtmlPangoRenderer *renderer,
                                       G_GNUC_UNUSED const gchar *name,
                                       G_GNUC_UNUSED const gchar **attribute_names,
                                       G_GNUC_UNUSED const gchar **attribute_values)
{
	g_string_append(renderer->str, "<span font=\"monospace\">");
}

static void
talkatu_html_pango_renderer_link_start(TalkatuHtmlPangoRenderer *renderer,
                                       G_GNUC_UNUSED const gchar *name,
                                       const gchar **attribute_names,
                                       const gchar **attribute_values)
{
	gint i = 0;

	g_string_append(renderer->str, "<a");

	if(attribute_names != NULL) {
		for(i = 0; attribute_names[i] != NULL; i++) {
			if(g_ascii_strcasecmp(attribute_names[i], "href") == 0) {
				g_string_append_printf(renderer->str,
				                       " href=\"%s\"", attribute_values[i]);
			}
		}
	}

	g_string_append(renderer->str, ">");
}

static void
talkatu_html_pango_renderer_font_start(TalkatuHtmlPangoRenderer *renderer,
                                       G_GNUC_UNUSED const gchar *name,
                                       const gchar **attribute_names,
                                       const gchar **attribute_values)
{
	gint i = 0;

	g_string_append(renderer->str, "<span");

	if(attribute_names != NULL) {
		for(i = 0; attribute_names[i] != NULL; i++) {
			if(g_ascii_strcasecmp(attribute_names[i], "size") == 0) {
				g_string_append_printf(renderer->str,
				                       " font=\"%s\"", attribute_values[i]);
			} else if(g_ascii_strcasecmp(attribute_names[i], "color") == 0) {
				g_string_append_printf(renderer->str,
				                       " foreground=\"%s\"", attribute_values[i]);
			}
		}
	}

	g_string_append(renderer->str, ">");
}

/******************************************************************************
 * TalkatuHtmlRendererElementFuncs API
 *****************************************************************************/
TalkatuHtmlPangoRendererElementFuncs *
talkatu_html_pango_renderer_element_funcs_new(TalkatuHtmlPangoRendererStartFunc start,
                                              TalkatuHtmlPangoRendererFinishFunc finish,
                                              const gchar *alias)
{
	TalkatuHtmlPangoRendererElementFuncs *funcs = NULL;

	funcs = g_new(TalkatuHtmlPangoRendererElementFuncs, 1);
	funcs->start = start;
	funcs->finish = finish;
	funcs->alias = alias;

	return funcs;
}

TalkatuHtmlPangoRendererElementFuncs *
talkatu_html_pango_renderer_element_funcs_new_passthrough(const gchar *alias) {
	return talkatu_html_pango_renderer_element_funcs_new(
		talkatu_html_pango_renderer_passthrough_start,
		talkatu_html_pango_renderer_passthrough_finish,
		alias
	);
}

/******************************************************************************
 * TalkatuHtmlRenderer Implementation
 *****************************************************************************/
static void
talkatu_html_pango_renderer_reset(TalkatuHtmlRenderer *renderer) {
	TalkatuHtmlPangoRenderer *pr = TALKATU_HTML_PANGO_RENDERER(renderer);

	g_string_free(pr->str, TRUE);
	pr->str = g_string_new("");
}

static void
talkatu_html_pango_renderer_element_start(TalkatuHtmlRenderer *renderer,
                                          const gchar *name,
                                          const gchar **attribute_names,
                                          const gchar **attribute_values)
{
	TalkatuHtmlPangoRenderer *pr = TALKATU_HTML_PANGO_RENDERER(renderer);
	TalkatuHtmlPangoRendererElementFuncs *funcs = NULL;

	funcs = g_hash_table_lookup(pr->lookup, name);
	if(funcs != NULL && funcs->start != NULL) {
		const gchar *real_name = (funcs->alias != NULL) ? funcs->alias : name;

		funcs->start(pr, real_name, attribute_names, attribute_values);
	}
}

static void
talkatu_html_pango_renderer_element_finish(TalkatuHtmlRenderer *renderer,
                                           const gchar *name)
{
	TalkatuHtmlPangoRenderer *pr = TALKATU_HTML_PANGO_RENDERER(renderer);
	TalkatuHtmlPangoRendererElementFuncs *funcs = NULL;

	funcs = g_hash_table_lookup(pr->lookup, name);
	if(funcs != NULL && funcs->finish != NULL) {
		const gchar *real_name = (funcs->alias != NULL) ? funcs->alias : name;

		funcs->finish(pr, real_name);
	}
}

static void
talkatu_html_pango_renderer_text(TalkatuHtmlRenderer *renderer,
                                 const gchar *text)
{
	TalkatuHtmlPangoRenderer *pr = TALKATU_HTML_PANGO_RENDERER(renderer);

	g_string_append(pr->str, text);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
talkatu_html_pango_renderer_init(TalkatuHtmlPangoRenderer *renderer) {
	TalkatuHtmlPangoRendererElementFuncs *funcs = NULL;

	renderer->str = g_string_new("");

	renderer->lookup = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
	                                         g_free);

	funcs = talkatu_html_pango_renderer_element_funcs_new(
		talkatu_html_pango_renderer_link_start,
		talkatu_html_pango_renderer_passthrough_finish,
		NULL
	);
	g_hash_table_insert(renderer->lookup, "a", funcs);

	funcs = talkatu_html_pango_renderer_element_funcs_new_passthrough(NULL);
	g_hash_table_insert(renderer->lookup, "b", funcs);

	funcs = talkatu_html_pango_renderer_element_funcs_new(
		talkatu_html_pango_renderer_code_start,
		talkatu_html_pango_renderer_passthrough_finish,
		"span"
	);
	g_hash_table_insert(renderer->lookup, "code", funcs);

	funcs = talkatu_html_pango_renderer_element_funcs_new_passthrough(NULL);
	g_hash_table_insert(renderer->lookup, "i", funcs);

	funcs = talkatu_html_pango_renderer_element_funcs_new(
		talkatu_html_pango_renderer_font_start,
		talkatu_html_pango_renderer_passthrough_finish,
		"span"
	);
	g_hash_table_insert(renderer->lookup, "font", funcs);

	funcs = talkatu_html_pango_renderer_element_funcs_new_passthrough(NULL);
	g_hash_table_insert(renderer->lookup, "s", funcs);

	funcs = talkatu_html_pango_renderer_element_funcs_new_passthrough("s");
	g_hash_table_insert(renderer->lookup, "strike", funcs);

	funcs = talkatu_html_pango_renderer_element_funcs_new_passthrough(NULL);
	g_hash_table_insert(renderer->lookup, "sub", funcs);

	funcs = talkatu_html_pango_renderer_element_funcs_new_passthrough(NULL);
	g_hash_table_insert(renderer->lookup, "sup", funcs);

	funcs = talkatu_html_pango_renderer_element_funcs_new_passthrough(NULL);
	g_hash_table_insert(renderer->lookup, "tt", funcs);

	funcs = talkatu_html_pango_renderer_element_funcs_new_passthrough(NULL);
	g_hash_table_insert(renderer->lookup, "u", funcs);
}

static void
talkatu_html_pango_renderer_finalize(GObject *obj) {
	TalkatuHtmlPangoRenderer *renderer = TALKATU_HTML_PANGO_RENDERER(obj);

	g_string_free(renderer->str, TRUE);
	g_hash_table_destroy(renderer->lookup);

	G_OBJECT_CLASS(talkatu_html_pango_renderer_parent_class)->finalize(obj);
}

static void
talkatu_html_pango_renderer_class_init(TalkatuHtmlPangoRendererClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	TalkatuHtmlRendererClass *renderer_class = TALKATU_HTML_RENDERER_CLASS(klass);

	obj_class->finalize = talkatu_html_pango_renderer_finalize;

	renderer_class->reset = talkatu_html_pango_renderer_reset;
	renderer_class->element_start = talkatu_html_pango_renderer_element_start;
	renderer_class->element_finish = talkatu_html_pango_renderer_element_finish;
	renderer_class->text = talkatu_html_pango_renderer_text;
}

/******************************************************************************
 * Public API
 *****************************************************************************/
TalkatuHtmlRenderer *
talkatu_html_pango_renderer_new(void) {
	return TALKATU_HTML_RENDERER(g_object_new(TALKATU_TYPE_HTML_PANGO_RENDERER,
	                                          NULL));
}

const gchar *
talkatu_html_pango_renderer_get_string(TalkatuHtmlPangoRenderer *renderer) {
	g_return_val_if_fail(TALKATU_IS_HTML_PANGO_RENDERER(renderer), NULL);

	return renderer->str->str;
}
