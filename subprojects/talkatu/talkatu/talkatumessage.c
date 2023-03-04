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

#include <gtk/gtk.h>

#include "talkatu/talkatumessage.h"
#include "talkatu/talkatuenums.h"

/**
 * TalkatuMessage:
 *
 * #TalkatuMessage is an opaque data structure and can only be accessed using
 * the following functions.
 */

/**
 * TalkatuMessageInterface:
 * @add_attachment: The add_attachment vfunc is called to add an attachment to
 *                  the message.
 * @remove_attachment: The remove_attachment vfunc is called to remove an
 *                     attachment from the message.
 * @get_attachment: The get_attachment vfunc gets an attachment from the
 *                  message.
 * @foreach_attachment: The foreach_attachment vfunc is called to iterate over
 *                      each attachment in the message and call the provided
 *                      TalkatuForeachAttachmentFunc on them.
 * @clear_attachments: The clear_attachments vfunc is called to clear all
 *                     attachments from the message.
 *
 * #TalkatuMessage is an interface to be implemented that standardizes the way
 * messages are handled.  All of its properties should be overridden with a
 * sensible value returned for them.
 *
 * It also needs to implement a storage mechanism for attachments which are
 * identified by a #guint64.
 */

/**
 * TalkatuContentType:
 * @TALKATU_CONTENT_TYPE_PLAIN: Plain text content.
 * @TALKATU_CONTENT_TYPE_PANGO: Pango Markup content.
 * @TALKATU_CONTENT_TYPE_HTML: HTML content.
 * @TALKATU_CONTENT_TYPE_MARKDOWN: Markdown content.
 *
 * An enum representing a given markup type.
 */

G_DEFINE_INTERFACE(TalkatuMessage, talkatu_message, G_TYPE_INVALID)

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
talkatu_message_default_init(TalkatuMessageInterface *iface) {
	GParamSpec *pspec = NULL;

	/**
	 * TalkatuMessage::id:
	 *
	 * The identifier of the message.
	 */
	pspec = g_param_spec_string(
		"id", "id", "The identifier of the message",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
	);
	g_object_interface_install_property(iface, pspec);

	/**
	 * TalkatuMessage::timestamp:
	 *
	 * The timestamp for when this message was created.
	 */
	pspec = g_param_spec_boxed(
		"timestamp", "timestamp", "The timestamp of the message",
		G_TYPE_DATE_TIME,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS
	);
	g_object_interface_install_property(iface, pspec);

	/**
	 * TalkatuMessage::content-type:
	 *
	 * The #TalkatuContentType of the message.
	 */
	pspec = g_param_spec_enum(
		"content-type", "content-type", "The content-type of the message",
		TALKATU_TYPE_CONTENT_TYPE,
		TALKATU_CONTENT_TYPE_PLAIN,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS
	);
	g_object_interface_install_property(iface, pspec);

	/**
	 * TalkatuMessage::author:
	 *
	 * The author of the message.
	 */
	pspec = g_param_spec_string(
		"author", "author", "The author of the message",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
	);
	g_object_interface_install_property(iface, pspec);

	/**
	 * TalkatuMessage::author-name-color:
	 *
	 * The color for the author's name.
	 */
	pspec = g_param_spec_boxed(
		"author-name-color", "author-name-color",
		"The color to use when rendering the author's name",
		GDK_TYPE_RGBA,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
	);
	g_object_interface_install_property(iface, pspec);


	/**
	 * TalkatuMessage::contents:
	 *
	 * The contents of the message.
	 */
	pspec = g_param_spec_string(
		"contents", "contents", "The contents of the message",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
	);
	g_object_interface_install_property(iface, pspec);

	/**
	 * TalkatuMessage::edited:
	 *
	 * Sets whether the message has been edited by its author or not.
	 */
	pspec = g_param_spec_boolean(
		"edited", "edited",
		"Whether this message has been edited by its author",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
	);
	g_object_interface_install_property(iface, pspec);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_message_get_id:
 * @message: The #TalkatuMessage instance.
 *
 * Gets the identifier of the @message.
 *
 * Returns: (transfer full): The identifier of @message.
 */
gchar *
talkatu_message_get_id(TalkatuMessage *message) {
	gchar *ret = NULL;

	g_return_val_if_fail(TALKATU_IS_MESSAGE(message), NULL);

	g_object_get(G_OBJECT(message), "id", &ret, NULL);

	return ret;
}

/**
 * talkatu_message_set_id:
 * @message: The #TalkatuMessage instance.
 * @id: The new identifier of @message.
 *
 * Sets the identifier of @message.
 */
void
talkatu_message_set_id(TalkatuMessage *message, const gchar *id) {
	g_return_if_fail(TALKATU_IS_MESSAGE(message));

	g_object_set(G_OBJECT(message), "id", id, NULL);
}

/**
 * talkatu_message_get_timestamp:
 * @message: The #TalkatuMessage instance.
 *
 * Gets the timestamp for when this message was created.
 *
 * Returns: (transfer full) (nullable): The timestamp of @message.
 */
GDateTime *
talkatu_message_get_timestamp(TalkatuMessage *message) {
	GDateTime *timestamp;

	g_return_val_if_fail(TALKATU_IS_MESSAGE(message), NULL);

	g_object_get(G_OBJECT(message), "timestamp", &timestamp, NULL);

	if(timestamp != NULL) {
		return g_date_time_ref(timestamp);
	}

	return NULL;
}

/**
 * talkatu_message_set_timestamp:
 * @message: The #TalkatuMessage instance.
 * @timestamp: (nullable): The new timestamp for @message.
 *
 * Sets the creation timestamp for @message to @timestamp.
 */
void
talkatu_message_set_timestamp(TalkatuMessage *message, GDateTime *timestamp) {
	g_return_if_fail(TALKATU_IS_MESSAGE(message));

	g_object_set(G_OBJECT(message), "timestamp", timestamp, NULL);
}

/**
 * talkatu_message_get_content_type:
 * @message: The #TalkatuMessage instance.
 *
 * Gets the content-type of @message.
 *
 * Returns: The content type of @message.
 */
TalkatuContentType
talkatu_message_get_content_type(TalkatuMessage *message) {
	TalkatuContentType content_type;

	g_return_val_if_fail(TALKATU_IS_MESSAGE(message), TALKATU_CONTENT_TYPE_PLAIN);

	g_object_get(G_OBJECT(message), "content-type", &content_type, NULL);

	return content_type;
}

/**
 * talkatu_message_set_content_type:
 * @message: The #TalkatuMessage instance.
 * @content_type: The new #TalkatuContentType to set.
 *
 * Sets the content type of @message to @content_type.
 */
void
talkatu_message_set_content_type(TalkatuMessage *message,
                                 TalkatuContentType content_type)
{
	g_return_if_fail(TALKATU_IS_MESSAGE(message));

	g_object_set(G_OBJECT(message), "content-type", content_type, NULL);
}

/**
 * talkatu_message_get_author:
 * @message: The #TalkatuMessage instance.
 *
 * Gets the author of @message.
 *
 * Returns: (transfer full): The author of @message.
 */
gchar *
talkatu_message_get_author(TalkatuMessage *message) {
	gchar *author = NULL;

	g_return_val_if_fail(TALKATU_IS_MESSAGE(message), NULL);

	g_object_get(G_OBJECT(message), "author", &author, NULL);

	return author;
}

/**
 * talkatu_message_set_author:
 * @message: The #TalkatuMessage instance.
 * @author: The new author of @message.
 *
 * Sets the author of @message to @author.
 */
void
talkatu_message_set_author(TalkatuMessage *message, const gchar *author) {
	g_return_if_fail(TALKATU_IS_MESSAGE(message));

	g_object_set(G_OBJECT(message), "author", author, NULL);
}

/**
 * talkatu_message_get_author_name_color:
 * @message: The #TalkatuMessage instance.
 *
 * Gets a #GdkRGBA that the author's name will be rendered with or %NULL if no
 * color is set.
 *
 * Returns: (transfer full): The #GdkRGBA to renderer the author's name with if
 *          set, otherwise %NULL.
 */
GdkRGBA *
talkatu_message_get_author_name_color(TalkatuMessage *message) {
	GdkRGBA *color = NULL;

	g_return_val_if_fail(TALKATU_IS_MESSAGE(message), NULL);

	g_object_get(G_OBJECT(message), "author-name-color", &color, NULL);

	return color;
}

/**
 * talkatu_message_set_author_name_color:
 * @message: The #TalkatuMessage instance.
 * @color: The new color for the author's name.
 *
 * Sets the color for the name of the author of @message to @color. Calling
 * this with @color set to %NULL will unset a previously set color.
 */
void
talkatu_message_set_author_name_color(TalkatuMessage *message, GdkRGBA *color) {
	g_return_if_fail(TALKATU_IS_MESSAGE(message));

	g_object_set(G_OBJECT(message), "author-name-color", color, NULL);
}

/**
 * talkatu_message_get_contents:
 * @message: The #TalkatuMessage instance.
 *
 * Gets the contents of @message.
 *
 * Returns: (transfer full): The contents of @message.
 */
gchar *
talkatu_message_get_contents(TalkatuMessage *message) {
	gchar *contents = NULL;

	g_return_val_if_fail(TALKATU_IS_MESSAGE(message), NULL);

	g_object_get(G_OBJECT(message), "contents", &contents, NULL);

	return contents;
}

/**
 * talkatu_message_set_contents:
 * @message: The #TalkatuMessage instance.
 * @contents: The new contents of @message.
 *
 * Sets the contents of @message to @contents.
 */
void
talkatu_message_set_contents(TalkatuMessage *message, const gchar *contents) {
	g_return_if_fail(TALKATU_IS_MESSAGE(message));

	g_object_set(G_OBJECT(message), "contents", contents, NULL);
}

/**
 * talkatu_message_get_edited:
 * @message: The #TalkatuMessage instance.
 *
 * Gets whether @message has been edited by its author.
 *
 * Returns: %TRUE if @message was edited by its author, or %FALSE if not.
 */
gboolean
talkatu_message_get_edited(TalkatuMessage *message) {
	gboolean edited = FALSE;

	g_return_val_if_fail(TALKATU_IS_MESSAGE(message), FALSE);

	g_object_get(G_OBJECT(message), "edited", &edited, NULL);

	return edited;
}

/**
 * talkatu_message_set_edited:
 * @message: The #TalkatuMessage instance.
 * @edited: Whether the message has been edited or not.
 *
 * Sets whether @message has been edited or not.
 */
void
talkatu_message_set_edited(TalkatuMessage *message, gboolean edited) {
	g_return_if_fail(TALKATU_IS_MESSAGE(message));

	g_object_set(G_OBJECT(message), "edited", edited, NULL);
}

/**
 * talkatu_message_add_attachment:
 * @message: The #TalkatuMessage instance.
 * @attachment: The #TalkatuAttachment instance.
 *
 * Adds @attachment to @message.
 *
 * Returns %TRUE if an attachment with the same ID did not already exist.
 */
gboolean
talkatu_message_add_attachment(TalkatuMessage *message,
                               TalkatuAttachment *attachment)
{
	TalkatuMessageInterface *iface = NULL;

	g_return_val_if_fail(TALKATU_IS_MESSAGE(message), FALSE);
	g_return_val_if_fail(TALKATU_IS_ATTACHMENT(attachment), FALSE);

	iface = TALKATU_MESSAGE_GET_IFACE(message);
	if(iface && iface->add_attachment) {
		return iface->add_attachment(message, attachment);
	}

	return FALSE;
}

/**
 * talkatu_message_remove_attachment:
 * @message: The #TalkatuMessage instance.
 * @id: The id of the #TalkatuAttachment
 *
 * Removes the #TalkatuAttachment identified by @id if it exists.
 *
 * Returns: %TRUE if the #TalkatuAttachment was found and removed, %FALSE
 *          otherwise.
 */
gboolean
talkatu_message_remove_attachment(TalkatuMessage *message, guint64 id) {
	TalkatuMessageInterface *iface = NULL;

	g_return_val_if_fail(TALKATU_IS_MESSAGE(message), FALSE);

	iface = TALKATU_MESSAGE_GET_IFACE(message);
	if(iface && iface->remove_attachment) {
		return iface->remove_attachment(message, id);
	}

	return FALSE;
}

/**
 * talkatu_message_get_attachment:
 * @message: The #TalkatuMessage instance.
 * @id: The id of the #TalkatuAttachment to get.
 *
 * Retrieves the #TalkatuAttachment identified by @id from @message.
 *
 * Returns: (transfer full): The #TalkatuAttachment if it was found, otherwise
 *                           %NULL.
 */
TalkatuAttachment *
talkatu_message_get_attachment(TalkatuMessage *message, guint64 id) {
	TalkatuMessageInterface *iface = NULL;

	g_return_val_if_fail(TALKATU_IS_MESSAGE(message), NULL);

	iface = TALKATU_MESSAGE_GET_IFACE(message);
	if(iface && iface->get_attachment) {
		return iface->get_attachment(message, id);
	}

	return NULL;
}

/**
 * talkatu_message_foreach_attachment:
 * @message: The #TalkatuMessage instance.
 * @func: (scope call): The #TalkatuAttachmentForeachFunc to call.
 * @data: User data to pass to @func.
 *
 * Calls @func for each #TalkatuAttachment that's attached to @message.
 */
void
talkatu_message_foreach_attachment(TalkatuMessage *message,
                                   TalkatuAttachmentForeachFunc func,
                                   gpointer data)
{
	TalkatuMessageInterface *iface = NULL;

	g_return_if_fail(TALKATU_IS_MESSAGE(message));
	g_return_if_fail(func != NULL);

	iface = TALKATU_MESSAGE_GET_IFACE(message);
	if(iface && iface->foreach_attachment) {
		iface->foreach_attachment(message, func, data);
	}
}

/**
 * talkatu_message_clear_attachments:
 * @message: The #TalkatuMessage instance.
 *
 * Removes all attachments from @message.
 */
void
talkatu_message_clear_attachments(TalkatuMessage *message) {
	TalkatuMessageInterface *iface = NULL;

	g_return_if_fail(TALKATU_IS_MESSAGE(message));

	iface = TALKATU_MESSAGE_GET_IFACE(message);
	if(iface && iface->clear_attachments) {
		iface->clear_attachments(message);
	}
}
