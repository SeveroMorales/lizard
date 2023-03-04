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

#ifndef TALKATU_ATTACHMENT_H
#define TALKATU_ATTACHMENT_H

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define TALKATU_TYPE_ATTACHMENT (talkatu_attachment_get_type())
G_DECLARE_INTERFACE(TalkatuAttachment, talkatu_attachment, TALKATU, ATTACHMENT, GObject)

struct _TalkatuAttachmentInterface {
	/*< private >*/
	GTypeInterface parent;

	/*< public >*/
	guint64 *(*get_hash_key)(TalkatuAttachment *attachment);

	/*< private >*/
	gpointer reserved[4];
};

typedef void (*TalkatuAttachmentForeachFunc)(TalkatuAttachment *attachment, gpointer data);

guint64 talkatu_attachment_get_id(TalkatuAttachment *attachment);
guint64 *talkatu_attachment_get_hash_key(TalkatuAttachment *attachment);
void talkatu_attachment_set_id(TalkatuAttachment *attachment, guint64 id);

gchar *talkatu_attachment_get_content_type(TalkatuAttachment *attachment);

gchar *talkatu_attachment_get_local_uri(TalkatuAttachment *attachment);
void talkatu_attachment_set_local_uri(TalkatuAttachment *attachment, const gchar *local_uri);

gchar *talkatu_attachment_get_remote_uri(TalkatuAttachment *attachment);
void talkatu_attachment_set_remote_uri(TalkatuAttachment *attachment, const gchar *remote_uri);

guint64 talkatu_attachment_get_size(TalkatuAttachment *attachment);
void talkatu_attachment_set_size(TalkatuAttachment *attachment, guint64 size);

gchar *talkatu_attachment_get_filename(TalkatuAttachment *attachment);

GIcon *talkatu_attachment_get_preview(TalkatuAttachment *attachment);

G_END_DECLS

#endif /* TALKATU_ATTACHMENT_H */
