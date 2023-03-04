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

#ifndef TALKATU_MARKUP_H
#define TALKATU_MARKUP_H

#include <glib.h>
#include <glib-object.h>

#include <talkatu/talkatubuffer.h>

G_BEGIN_DECLS

gboolean talkatu_markup_deserialize_html(GtkTextBuffer *register_buffer, GtkTextBuffer *content_buffer, GtkTextIter *iter, const guint8 *data, gsize length, gboolean create_tags, gpointer user_data, GError **error);
void talkatu_markup_append_html(TalkatuBuffer *buffer, const gchar *text, gint len);
void talkatu_markup_insert_html(TalkatuBuffer *buffer, GtkTextIter *iter, const gchar *text, gint len);
void talkatu_markup_set_html(TalkatuBuffer *buffer, const gchar *text, gint len);

guint8 *talkatu_markup_serialize_html(GtkTextBuffer *register_buffer, GtkTextBuffer *content_buffer, const GtkTextIter *start, const GtkTextIter *end, gsize *length, gpointer user_data);
gchar *talkatu_markup_get_html(GtkTextBuffer *buffer, gsize *len);
gchar *talkatu_markup_get_html_range(GtkTextBuffer *buffer, const GtkTextIter *start, const GtkTextIter *end, gsize *len);

G_END_DECLS

#endif /* TALKATU_MARKUP_H */
