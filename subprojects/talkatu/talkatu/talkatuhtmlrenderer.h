/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2020 Gary Kramlich <grim@reaperworld.com>
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

#if !defined(TALKATU_GLOBAL_HEADER_INSIDE) && !defined(TALKATU_COMPILATION)
#error "only <talkatu.h> may be included directly"
#endif

#ifndef TALKATU_HTML_RENDERER_H
#define TALKATU_HTML_RENDERER_H

#include <glib.h>
#include <glib-object.h>

#define TALKATU_TYPE_HTML_RENDERER (talkatu_html_renderer_get_type())
G_DECLARE_DERIVABLE_TYPE(TalkatuHtmlRenderer, talkatu_html_renderer, TALKATU,
                         HTML_RENDERER, GObject)

struct _TalkatuHtmlRendererClass {
	/*< private >*/
	GObjectClass parent;

	/*< public >*/
	void (*reset)(TalkatuHtmlRenderer *renderer);

	void (*element_start)(TalkatuHtmlRenderer *renderer, const gchar *name, const gchar **attribute_names, const gchar **attribute_values);
	void (*element_finish)(TalkatuHtmlRenderer *renderer, const gchar *name);
	void (*text)(TalkatuHtmlRenderer *renderer, const gchar *text);
	void (*comment)(TalkatuHtmlRenderer *renderer, const gchar *comment);

	/*< private >*/
	gpointer reserved[4];
};

G_BEGIN_DECLS

void talkatu_html_renderer_render(TalkatuHtmlRenderer *renderer, const gchar *html);

void talkatu_html_renderer_reset(TalkatuHtmlRenderer *renderer);

G_END_DECLS

#endif /* TALKATU_HTML_RENDERER_H */

