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

#ifndef TALKATU_MESSAGE_H
#define TALKATU_MESSAGE_H

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include <talkatu/talkatuattachment.h>

typedef enum _TalkatuContentType /*< prefix=TALKATU_CONTENT_TYPE,underscore_name=TALKATU_CONTENT_TYPE >*/
{
	TALKATU_CONTENT_TYPE_PLAIN =  0,
	TALKATU_CONTENT_TYPE_PANGO,
	TALKATU_CONTENT_TYPE_HTML,
	TALKATU_CONTENT_TYPE_MARKDOWN,
} TalkatuContentType;

G_BEGIN_DECLS

#define TALKATU_TYPE_MESSAGE (talkatu_message_get_type())
G_DECLARE_INTERFACE(TalkatuMessage, talkatu_message, TALKATU, MESSAGE, GObject)

struct _TalkatuMessageInterface {
	/*< private >*/
	GTypeInterface parent;

	/*< public >*/
	gboolean (*add_attachment)(TalkatuMessage *message, TalkatuAttachment *attachment);
	gboolean (*remove_attachment)(TalkatuMessage *message, guint64 id);
	TalkatuAttachment *(*get_attachment)(TalkatuMessage *message, guint64 id);
	void (*foreach_attachment)(TalkatuMessage *message, TalkatuAttachmentForeachFunc func, gpointer data);
	void (*clear_attachments)(TalkatuMessage *message);

	/*< private >*/
	gpointer reserved[4];
};

gchar *talkatu_message_get_id(TalkatuMessage *message);
void talkatu_message_set_id(TalkatuMessage *message, const gchar *id);

GDateTime *talkatu_message_get_timestamp(TalkatuMessage *message);
void talkatu_message_set_timestamp(TalkatuMessage *message, GDateTime *timestamp);

TalkatuContentType talkatu_message_get_content_type(TalkatuMessage *message);
void talkatu_message_set_content_type(TalkatuMessage *message, TalkatuContentType content_type);

gchar *talkatu_message_get_author(TalkatuMessage *message);
void talkatu_message_set_author(TalkatuMessage *message, const gchar *author);

GdkRGBA *talkatu_message_get_author_name_color(TalkatuMessage *message);
void talkatu_message_set_author_name_color(TalkatuMessage *message, GdkRGBA *color);

gchar *talkatu_message_get_contents(TalkatuMessage *message);
void talkatu_message_set_contents(TalkatuMessage *message, const gchar *contents);

gboolean talkatu_message_get_edited(TalkatuMessage *message);
void talkatu_message_set_edited(TalkatuMessage *message, gboolean edited);

gboolean talkatu_message_add_attachment(TalkatuMessage *message, TalkatuAttachment *attachment);
gboolean talkatu_message_remove_attachment(TalkatuMessage *message, guint64 id);
TalkatuAttachment *talkatu_message_get_attachment(TalkatuMessage *message, guint64 id);
void talkatu_message_foreach_attachment(TalkatuMessage *message, TalkatuAttachmentForeachFunc func, gpointer data);
void talkatu_message_clear_attachments(TalkatuMessage *message);

G_END_DECLS

#endif /* TALKATU_MESSAGE_H */
