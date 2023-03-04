/*
 * purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
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
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_ATTACHMENT_H
#define PURPLE_ATTACHMENT_H

#include <glib-object.h>

G_BEGIN_DECLS

#define PURPLE_TYPE_ATTACHMENT purple_attachment_get_type()

/**
 * PurpleAttachment:
 *
 * #PurpleAttachment represents a file attached to a #PurpleMessage.
 */

G_DECLARE_FINAL_TYPE(PurpleAttachment, purple_attachment, PURPLE, ATTACHMENT, GObject)

/**
 * PurpleAttachmentForeachFunc:
 * @attachment: The #PurpleAttachment instance.
 * @data: User supplied data.
 *
 * Called when iterating #PurpleAttachment's.
 *
 * Since: 3.0.0
 */
typedef void (*PurpleAttachmentForeachFunc)(PurpleAttachment *attachment, gpointer data);

/**
 * purple_attachment_new:
 * @id: The identifier of the attachment.
 * @content_type: The mime-type of the content.
 *
 * Creates a new #PurpleAttachment with the given @id and @content_type.
 *
 * Since: 3.0.0
 */
PurpleAttachment *purple_attachment_new(guint64 id, const gchar *content_type);

/**
 * purple_attachment_get_id:
 * @attachment: The #PurpleAttachment instance.
 *
 * Gets the ID from @attachment.
 *
 * Returns: The ID of @attachment.
 *
 * Since: 3.0.0
 */
guint64 purple_attachment_get_id(PurpleAttachment *attachment);

/**
 * purple_attachment_get_hash_key:
 * @attachment: The #PurpleAttachment instance.
 *
 * Gets the hash key of @attachment.  This should only be used when
 * trying to address a #PurpleAttachment in a #GHashTable that is using
 * g_int64_hash() as the key function.
 *
 * Returns: (transfer none): The hash key of @attachment.
 *
 * Since: 3.0.0
 */
guint64 *purple_attachment_get_hash_key(PurpleAttachment *attachment);

/**
 * purple_attachment_set_id:
 * @attachment: The #PurpleAttachment instance.
 * @id: The new ID for @attachment.
 *
 * Sets the ID of @attachment to @id.
 *
 * Since: 3.0.0
 */
void purple_attachment_set_id(PurpleAttachment *attachment, guint64 id);

/**
 * purple_attachment_get_content_type:
 * @attachment: The #PurpleAttachment instance.
 *
 * Gets the content-type of @attachment.
 *
 * Returns: The content-type of @attachment.
 *
 * Since: 3.0.0
 */
const gchar *purple_attachment_get_content_type(PurpleAttachment *attachment);

/**
 * purple_attachment_get_local_uri:
 * @attachment: The #PurpleAttachment instance.
 *
 * Gets the local URI if any for @attachment.
 *
 * Returns: (nullable): The local URI for @attachment.
 *
 * Since: 3.0.0
 */
const gchar *purple_attachment_get_local_uri(PurpleAttachment *attachment);

/**
 * purple_attachment_set_local_uri:
 * @attachment: The #PurpleAttachment instance.
 * @local_uri: The new local URI.
 *
 * Sets the local URI of @attachment.
 *
 * Since: 3.0.0
 */
void purple_attachment_set_local_uri(PurpleAttachment *attachment, const gchar *local_uri);

/**
 * purple_attachment_get_remote_uri:
 * @attachment: The #PurpleAttachment instance.
 *
 * Gets the remote URI if any for @attachment.
 *
 * Returns: (nullable): The remote URI for @attachment.
 *
 * Since: 3.0.0
 */
const gchar *purple_attachment_get_remote_uri(PurpleAttachment *attachment);

/**
 * purple_attachment_set_remote_uri:
 * @attachment: The #PurpleAttachment instance.
 * @remote_uri: The new remote URI.
 *
 * Sets the remote URI of @attachment.
 *
 * Since: 3.0.0
 */
void purple_attachment_set_remote_uri(PurpleAttachment *attachment, const gchar *remote_uri);

/**
 * purple_attachment_get_size:
 * @attachment: The #PurpleAttachment instance.
 *
 * Gets the size of @attachment.
 *
 * Returns: The size of @attachment.
 *
 * Since: 3.0.0
 */
guint64 purple_attachment_get_size(PurpleAttachment *attachment);

/**
 * purple_attachment_set_size:
 * @attachment: The #PurpleAttachment instance.
 * @size: The new size of @attachment.
 *
 * Sets the size of @attachment to @size.
 *
 * Since: 3.0.0
 */
void purple_attachment_set_size(PurpleAttachment *attachment, guint64 size);

/**
 * purple_attachment_get_filename:
 * @attachment: The #PurpleAttachment instance.
 *
 * Gets the base filename for @attachment.  Remote URI will be checked before
 * local URI, but the basename of one of those is what will be returned.
 *
 * Returns: (transfer full): The filename for @attachment.
 *
 * Since: 3.0.0
 */
gchar *purple_attachment_get_filename(PurpleAttachment *attachment);

G_END_DECLS

#endif /* PURPLE_ATTACHMENT_H */
