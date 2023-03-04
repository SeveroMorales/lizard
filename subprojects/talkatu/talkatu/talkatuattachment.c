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

#include "talkatu/talkatuattachment.h"

#include <glib/gi18n-lib.h>

/**
 * TalkatuAttachment:
 *
 * TalkatuAttachment represents an attached file.  The files can be any type of
 * regular file but only images will be previewed based on their actual
 * contents.
 */

/**
 * TalkatuAttachmentInterface:
 * @get_hash_key: A function that returns the key to hash the attachment with.
 *                This is typically just a pointer to the internal id variable,
 *                which is necessary due to the differences in 64-bit integers
 *                across platforms.
 *
 * #TalkatuAttachmentInterface defines the methods and behaviors that make up a
 * #TalkatuAttachment.  This was made an interface so clients can represent
 * their data however they want but still provide a common interface for Talkatu
 * to work with.
 */

/**
 * TalkatuAttachmentForeachFunc:
 * @attachment: A #TalkatuAttachment instance.
 * @data: Caller supplied data.
 *
 * #TalkatuAttachmentForeachFunc is a callback function called against each
 * #TalkatuAttachment in a collection.
 */

G_DEFINE_INTERFACE(TalkatuAttachment, talkatu_attachment, G_TYPE_INVALID);

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
talkatu_attachment_default_init(TalkatuAttachmentInterface *iface) {
	GParamSpec *pspec = NULL;

	/**
	 * TalkatuAttachment::id:
	 *
	 * The identifier of the attachment.
	 */
	pspec = g_param_spec_uint64(
		"id", "id", "The identifier of the attachment",
		0, G_MAXUINT64, 0,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
	);
	g_object_interface_install_property(iface, pspec);

	/**
	 * TalkatuAttachment::content-type:
	 *
	 * The content type of the attachment.
	 */
	pspec = g_param_spec_string(
		"content-type", "content-type", "The content type of the attachment",
		"application/octet-stream",
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS
	);
	g_object_interface_install_property(iface, pspec);

	/**
	 * TalkatuAttachment::local-uri:
	 *
	 * The local URI of the attachment.
	 */
	pspec = g_param_spec_string(
		"local-uri", "local-uri", "The local URI of the attachment",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
	);
	g_object_interface_install_property(iface, pspec);

	/**
	 * TalkatuAttachment::remote-uri:
	 *
	 * The remote URI of the attachment.
	 */
	pspec = g_param_spec_string(
		"remote-uri", "remote-uri", "The remote URI of the attachment",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
	);
	g_object_interface_install_property(iface, pspec);

	/**
	 * TalkatuAttachment::size:
	 *
	 * The size of the attachment.
	 */
	pspec = g_param_spec_uint64(
		"size", "size", "The file size of the attachment in bytes",
		0, G_MAXUINT64, 0,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
	);
	g_object_interface_install_property(iface, pspec);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_attachment_new:
 * @id: The identifier of the attachment.
 * @content_type: The content type of the attachment.
 *
 * Creates a new attachment with @content_type.
 *
 * Returns: (transfer full): The new #TalkatuAttachment.
 */
TalkatuAttachment *
talkatu_attachment_new(guint64 id, const gchar *content_type) {
	g_return_val_if_fail(content_type != NULL, NULL);

	return TALKATU_ATTACHMENT(g_object_new(
		TALKATU_TYPE_ATTACHMENT,
		"id", id,
		"content-type", content_type,
		NULL
	));
}

/**
 * talkatu_attachment_get_id:
 * @attachment: The #TalkatuAttachment instance.
 *
 * Gets the ID associated with @attachment.
 *
 * Returns: The ID of @attachment.
 */
guint64
talkatu_attachment_get_id(TalkatuAttachment *attachment) {
	guint64 ret = 0;

	g_return_val_if_fail(TALKATU_IS_ATTACHMENT(attachment), 0);

	g_object_get(G_OBJECT(attachment), "id", &ret, NULL);

	return ret;
}

/**
 * talkatu_attachment_get_hash_key:
 * @attachment: The #TalkatuAttachment instance.
 *
 * Gets the hash key of @attachment.  This should only be used when
 * trying to address a #TalkatuAttachment in a #GHashTable that is using
 * g_int64_hash() as the key function.
 *
 * Returns: (transfer none): The hash key of @attachment.
 */
guint64 *
talkatu_attachment_get_hash_key(TalkatuAttachment *attachment) {
	TalkatuAttachmentInterface *iface = NULL;

	g_return_val_if_fail(TALKATU_IS_ATTACHMENT(attachment), NULL);

	iface = TALKATU_ATTACHMENT_GET_IFACE(attachment);

	return iface->get_hash_key(attachment);
}

/**
 * talkatu_attachment_set_id:
 * @attachment: The #TalkatuAttachment instance.
 * @id: The new ID for @attachment.
 *
 * Sets the ID of @attachment to @id.
 */
void
talkatu_attachment_set_id(TalkatuAttachment *attachment, guint64 id) {
	g_return_if_fail(TALKATU_IS_ATTACHMENT(attachment));

	g_object_set(G_OBJECT(attachment), "id", id, NULL);
}

/**
 * talkatu_attachment_get_content_type:
 * @attachment: The #TalkatuAttachment instance.
 *
 * Returns the content type of the attachment.
 *
 * Returns: The content type of @attachment.
 */
gchar *
talkatu_attachment_get_content_type(TalkatuAttachment *attachment) {
	gchar *content_type = NULL;

	g_return_val_if_fail(TALKATU_IS_ATTACHMENT(attachment), NULL);

	g_object_get(G_OBJECT(attachment), "content-type", &content_type, NULL);

	return content_type;
}

/**
 * talkatu_attachment_get_local_uri:
 * @attachment: The #TalkatuAttachment instance.
 *
 * Gets the local URI if any for @attachment.
 *
 * Returns: The local URI for @attachment.
 */
gchar *
talkatu_attachment_get_local_uri(TalkatuAttachment *attachment) {
	gchar *local_uri = NULL;

	g_return_val_if_fail(TALKATU_IS_ATTACHMENT(attachment), NULL);

	g_object_get(G_OBJECT(attachment), "local-uri", &local_uri, NULL);

	return local_uri;
}

/**
 * talkatu_attachment_set_local_uri:
 * @attachment: The #TalkatuAttachment instance.
 * @local_uri: The new local URI.
 *
 * Sets the local URI of @attachment.
 */
void
talkatu_attachment_set_local_uri(TalkatuAttachment *attachment,
                                 const gchar *local_uri)
{
	g_return_if_fail(TALKATU_IS_ATTACHMENT(attachment));

	g_object_set(G_OBJECT(attachment), "local-uri", local_uri, NULL);
}

/**
 * talkatu_attachment_get_remote_uri:
 * @attachment: The #TalkatuAttachment instance.
 *
 * Gets the remote URI if any for @attachment.
 *
 * Returns: (nullable): The remote URI for @attachment.
 */
gchar *
talkatu_attachment_get_remote_uri(TalkatuAttachment *attachment) {
	gchar *remote_uri = NULL;

	g_return_val_if_fail(TALKATU_IS_ATTACHMENT(attachment), NULL);

	g_object_get(G_OBJECT(attachment), "remote-uri", &remote_uri, NULL);

	return remote_uri;
}

/**
 * talkatu_attachment_set_remote_uri:
 * @attachment: The #TalkatuAttachment instance.
 * @remote_uri: The new remote URI.
 *
 * Sets the remote URI of @attachment.
 */
void
talkatu_attachment_set_remote_uri(TalkatuAttachment *attachment,
                                  const gchar *remote_uri)
{
	g_return_if_fail(TALKATU_IS_ATTACHMENT(attachment));

	g_object_set(G_OBJECT(attachment), "remote-uri", remote_uri, NULL);
}

/**
 * talkatu_attachment_get_size:
 * @attachment: The #TalkatuAttachment instance.
 *
 * Gets the size of @attachment.
 *
 * Returns: The size of @attachment.
 */
guint64
talkatu_attachment_get_size(TalkatuAttachment *attachment) {
	guint64 size;

	g_return_val_if_fail(TALKATU_IS_ATTACHMENT(attachment), 0);

	g_object_get(G_OBJECT(attachment), "size", &size, NULL);

	return size;
}

/**
 * talkatu_attachment_set_size:
 * @attachment: The #TalkatuAttachment instance.
 * @size: The new size of @attachment.
 *
 * Sets the size of @attachment to @size.
 */
void
talkatu_attachment_set_size(TalkatuAttachment *attachment, guint64 size) {
	g_return_if_fail(TALKATU_IS_ATTACHMENT(attachment));

	g_object_set(G_OBJECT(attachment), "size", size, NULL);
}

/**
 * talkatu_attachment_get_filename:
 * @attachment: The #TalkatuAttachment instance.
 *
 * Gets the base filename for @attachment.  Remote URI will be checked before
 * local URI, but the basename of one of those is what will be returned.
 *
 * Returns: (transfer full): The filename for @attachment.
 */
gchar *
talkatu_attachment_get_filename(TalkatuAttachment *attachment) {
	gchar *remote_uri = NULL, *local_uri = NULL;

	g_return_val_if_fail(TALKATU_IS_ATTACHMENT(attachment), NULL);

	remote_uri = talkatu_attachment_get_remote_uri(attachment);
	if(remote_uri != NULL && remote_uri[0] != '\0') {
		gchar *ret = g_path_get_basename(remote_uri);

		g_free(remote_uri);

		return ret;
	}

	local_uri = talkatu_attachment_get_local_uri(attachment);
	if(local_uri != NULL && local_uri[0] != '\0') {
		gchar *ret = g_path_get_basename(local_uri);

		g_free(local_uri);

		return ret;
	}

	return g_strdup("unknown");
}

/**
 * talkatu_attachment_get_preview:
 * @attachment: The #TalkatuAttachment instance.
 *
 * Create a #GIcon as a preview for @attachment.
 *
 * Returns: (transfer full): A preview image of @attachment.
 */
GIcon *
talkatu_attachment_get_preview(TalkatuAttachment *attachment) {
	gchar *content_type = NULL;
	const gchar *name = "text-x-generic-template";

	g_return_val_if_fail(TALKATU_IS_ATTACHMENT(attachment), NULL);

	content_type = talkatu_attachment_get_content_type(attachment);
	if(content_type == NULL) {
		return g_themed_icon_new(name);
	}

	if(g_str_has_prefix(content_type, "image/")) {
		gchar *local_uri = talkatu_attachment_get_local_uri(attachment);
		if(local_uri != NULL) {
			GFile *file = g_file_new_for_uri(local_uri);
			GIcon *icon = g_file_icon_new(file);

			g_object_unref(G_OBJECT(file));
			g_free(local_uri);
			g_free(content_type);

			return icon;
		}

		name = "image-x-generic";
	} else if(g_str_has_prefix(content_type, "text/")) {
		name = "text-x-generic";
	} else if(g_str_has_prefix(content_type, "audio/")) {
		name = "audio-x-generic";
	}

	g_free(content_type);

	return g_themed_icon_new(name);
}
