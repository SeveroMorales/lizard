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

#ifndef TALKATU_BUFFER_H
#define TALKATU_BUFFER_H

#include <gtk/gtk.h>

typedef enum /*< prefix=TALKATU_BUFFER_STYLE,underscore_name=TALKATU_BUFFER_STYLE >*/
{
	TALKATU_BUFFER_STYLE_RICH,
	TALKATU_BUFFER_STYLE_WHOLE,
} TalkatuBufferStyle;

#define TALKATU_BUFFER_LINK_TARGET_ATTRIBUTE "talkatu_link_target"

G_BEGIN_DECLS

#define TALKATU_TYPE_BUFFER            (talkatu_buffer_get_type())

G_DECLARE_DERIVABLE_TYPE(TalkatuBuffer, talkatu_buffer, TALKATU, BUFFER, GtkTextBuffer)

struct _TalkatuBufferClass {
	/*< private >*/
	GtkTextBufferClass parent;

	/*< public >*/
	void (*insert_markup)(TalkatuBuffer *buffer, GtkTextIter *pos, const gchar *new_text, gint new_text_length);

	/*< private >*/
	gpointer reserved[4];
};

GtkTextBuffer *talkatu_buffer_new(GSimpleActionGroup *action_group);

TalkatuBufferStyle talkatu_buffer_get_style(TalkatuBuffer *buffer);

GSimpleActionGroup *talkatu_buffer_get_action_group(TalkatuBuffer *buffer);
void talkatu_buffer_set_action_group(TalkatuBuffer *buffer, GSimpleActionGroup *group);

void talkatu_buffer_insert_markup(TalkatuBuffer *buffer, GtkTextIter *pos, const gchar *new_text, gint new_text_length);
void talkatu_buffer_insert_markup_with_tags_by_name(TalkatuBuffer *buffer, GtkTextIter *pos, const gchar *new_text, gint new_text_length, const gchar *first_tag_name, ...);
void talkatu_buffer_insert_link(TalkatuBuffer *buffer, GtkTextIter *pos, const gchar *display_text, const gchar *url);

void talkatu_buffer_clear(TalkatuBuffer *buffer);

gchar *talkatu_buffer_get_plain_text(TalkatuBuffer *buffer);
gboolean talkatu_buffer_get_is_empty(TalkatuBuffer *buffer);

G_END_DECLS

#endif /* TALKATU_BUFFER_H */
