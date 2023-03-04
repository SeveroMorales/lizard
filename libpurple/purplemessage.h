/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_MESSAGE_H
#define PURPLE_MESSAGE_H

#include <glib-object.h>

#include <libpurple/purpleattachment.h>

G_BEGIN_DECLS

#define PURPLE_TYPE_MESSAGE purple_message_get_type()

/**
 * PurpleMessageFlags:
 * @PURPLE_MESSAGE_SEND: Outgoing message.
 * @PURPLE_MESSAGE_RECV: Incoming message.
 * @PURPLE_MESSAGE_SYSTEM: System message.
 * @PURPLE_MESSAGE_AUTO_RESP: Auto response.
 * @PURPLE_MESSAGE_ACTIVE_ONLY: Hint to the UI that this message should not be
 *                              shown in conversations which are only open for
 *                              internal UI purposes (e.g. for contact-aware
 *                              conversations).
 * @PURPLE_MESSAGE_NICK: Contains your nick.
 * @PURPLE_MESSAGE_NO_LOG: Do not log.
 * @PURPLE_MESSAGE_ERROR: Error message.
 * @PURPLE_MESSAGE_DELAYED: Delayed message.
 * @PURPLE_MESSAGE_RAW: "Raw" message - don't apply formatting.
 * @PURPLE_MESSAGE_IMAGES: Message contains images.
 * @PURPLE_MESSAGE_NOTIFY: Message is a notification.
 * @PURPLE_MESSAGE_NO_LINKIFY: Message should not be auto-linkified.
 * @PURPLE_MESSAGE_INVISIBLE: Message should not be displayed.
 * @PURPLE_MESSAGE_REMOTE_SEND: Message sent from another location, not an echo
 *                              of a local one.
 *                              Since: 2.12.0
 * @PURPLE_MESSAGE_FORWARDED: The message has been forward to the recipient.
 *
 * Flags applicable to a message. Most will have send, recv or system.
 */
typedef enum /*< flags >*/
{
	PURPLE_MESSAGE_SEND         = 1 << 0,
	PURPLE_MESSAGE_RECV         = 1 << 1,
	PURPLE_MESSAGE_SYSTEM       = 1 << 2,
	PURPLE_MESSAGE_AUTO_RESP    = 1 << 3,
	PURPLE_MESSAGE_ACTIVE_ONLY  = 1 << 4,
	PURPLE_MESSAGE_NICK         = 1 << 5,
	PURPLE_MESSAGE_NO_LOG       = 1 << 6,
	PURPLE_MESSAGE_ERROR        = 1 << 7,
	PURPLE_MESSAGE_DELAYED      = 1 << 8,
	PURPLE_MESSAGE_RAW          = 1 << 9,
	PURPLE_MESSAGE_IMAGES       = 1 << 10,
	PURPLE_MESSAGE_NOTIFY       = 1 << 11,
	PURPLE_MESSAGE_NO_LINKIFY   = 1 << 12,
	PURPLE_MESSAGE_INVISIBLE    = 1 << 13,
	PURPLE_MESSAGE_REMOTE_SEND  = 1 << 14,
	PURPLE_MESSAGE_FORWARDED    = 1 << 15,
} PurpleMessageFlags;

/**
 * PurpleMessageContentType:
 * @PURPLE_MESSAGE_CONTENT_TYPE_PLAIN: The message has no formatting.
 * @PURPLE_MESSAGE_CONTENT_TYPE_HTML: The message is formatted in HTML.
 * @PURPLE_MESSAGE_CONTENT_TYPE_XHTML: The message is formatted in XHTML.
 * @PURPLE_MESSAGE_CONTENT_TYPE_MARKDOWN: The message is formatted in Markdown.
 *
 * The message formatting for the message.
 *
 * Since: 3.0.0
 */
typedef enum /*< prefix=PURPLE_MESSAGE_CONTENT_TYPE,underscore_name=PURPLE_MESSAGE_CONTENT_TYPE >*/
{
	PURPLE_MESSAGE_CONTENT_TYPE_PLAIN = 0,
	PURPLE_MESSAGE_CONTENT_TYPE_HTML,
	PURPLE_MESSAGE_CONTENT_TYPE_XHTML,
	PURPLE_MESSAGE_CONTENT_TYPE_MARKDOWN,
} PurpleMessageContentType;

/**
 * PurpleMessage:
 *
 * #PurpleMessage represents any message passed between users in libpurple.
 */

G_DECLARE_FINAL_TYPE(PurpleMessage, purple_message, PURPLE, MESSAGE, GObject)

#include "account.h"

/**
 * purple_message_new_outgoing:
 * @account: The account for this message.
 * @author: The author.
 * @recipient: The recipient.
 * @contents: The contents.
 * @flags: The #PurpleMessageFlags.
 *
 * Creates new outgoing message to @recipient.
 *
 * You don't need to set the #PURPLE_MESSAGE_SEND flag.  If the message is not
 * plain text be sure to call purple_message_set_content_type().
 *
 * Returns: (transfer full): The new #PurpleMessage instance.
 *
 * Since: 3.0.0
 */
PurpleMessage *purple_message_new_outgoing(PurpleAccount *account, const gchar *author, const gchar *recipient, const gchar *contents, PurpleMessageFlags flags);

/**
 * purple_message_new_incoming:
 * @account: The account for this message.
 * @who: Message's author.
 * @contents: The contents of a message.
 * @flags: The message flags.
 * @timestamp: The time of transmitting a message. May be 0 for a current time.
 *
 * Creates new incoming message (the user is the recipient).
 *
 * You don't need to set the #PURPLE_MESSAGE_RECV flag.
 *
 * Returns: the new #PurpleMessage.
 *
 * Since: 3.0.0
 */
PurpleMessage *purple_message_new_incoming(PurpleAccount *account, const gchar *who, const gchar *contents, PurpleMessageFlags flags, guint64 timestamp);

/**
 * purple_message_new_system:
 * @account: The account for this message.
 * @contents: The contents of a message.
 * @flags: The message flags.
 *
 * Creates new system message.
 *
 * You don't need to set the #PURPLE_MESSAGE_SYSTEM flag.
 *
 * Returns: the new #PurpleMessage.
 *
 * Since: 3.0.0
 */
PurpleMessage *purple_message_new_system(PurpleAccount *account, const gchar *contents, PurpleMessageFlags flags);

/**
 * purple_message_get_id:
 * @message: The message.
 *
 * Returns the unique identifier of the message. These identifiers are not
 * serialized - it's a per-session id.
 *
 * Returns: the global identifier of @message.
 *
 * Since: 3.0.0
 */
const gchar *purple_message_get_id(PurpleMessage *message);

/**
 * purple_message_get_author:
 * @message: The message.
 *
 * Returns the author of the message, not a local alias.
 *
 * Returns: the author of @message.
 *
 * Since: 3.0.0
 */
const gchar *purple_message_get_author(PurpleMessage *message);

/**
 * purple_message_set_author_name_color:
 * @message: The #PurpleMessage instance.
 * @color: The hex color code for the author of @message.
 *
 * Sets the author's name color of @message to @color. This is the color that
 * will be used to display the author's name in a user interface. The user
 * interface might not use this exact color, as it might need to adapt for
 * contrast or limits on the number of colors.
 *
 * Since: 3.0.0
 */
void purple_message_set_author_name_color(PurpleMessage *message, const gchar *color);

/**
 * purple_message_get_author_name_color:
 * @message: The #PurpleMessage instance.
 *
 * Gets the author's name color for @message.
 *
 * Returns: (transfer none): The hex color for the author of @message's name.
 *
 * Since: 3.0.0
 */
const gchar *purple_message_get_author_name_color(PurpleMessage *message);

/**
 * purple_message_set_recipient:
 * @message: The #PurpleMessage instance.
 * @recipient: The name of the recipient.
 *
 * Sets the recipient of @message to @recipient.
 *
 * Since: 3.0.0
 */
void purple_message_set_recipient(PurpleMessage *message, const gchar *recipient);

/**
 * purple_message_get_recipient:
 * @message: The message.
 *
 * Returns the recipient of the message, not a local alias.
 *
 * Returns: the recipient of @message.
 *
 * Since: 3.0.0
 */
const gchar *purple_message_get_recipient(PurpleMessage *message);

/**
 * purple_message_set_author_alias:
 * @message: The message.
 * @alias: The alias.
 *
 * Sets the alias of @message's author. You don't normally need to call this.
 *
 * Since: 3.0.0
 */
void purple_message_set_author_alias(PurpleMessage *message, const gchar *alias);

/**
 * purple_message_get_author_alias:
 * @message: The message.
 *
 * Returns the alias of @message author.
 *
 * Returns: the @message author's alias.
 *
 * Since: 3.0.0
 */
const gchar *purple_message_get_author_alias(PurpleMessage *message);

/**
 * purple_message_set_contents:
 * @message: The message.
 * @cont: The contents.
 *
 * Sets the contents of the @message. It might be HTML.
 *
 * Since: 3.0.0
 */
void purple_message_set_contents(PurpleMessage *message, const gchar *cont);

/**
 * purple_message_get_contents:
 * @message: The message.
 *
 * Returns the contents of the message.
 *
 * Returns: the contents of @message.
 *
 * Since: 3.0.0
 */
const gchar *purple_message_get_contents(PurpleMessage *message);

/**
 * purple_message_set_content_type:
 * @message: The #PurpleMessage instance.
 * @content_type: The #PurpleMessageContentType value.
 *
 * Sets the content-type of @message to @content_type.
 *
 * Since: 3.0.0
 */
void purple_message_set_content_type(PurpleMessage *message, PurpleMessageContentType content_type);

/**
 * purple_message_get_content_type:
 * @message: The #PurpleMessage instance.
 *
 * Gets the content-type of @message.
 *
 * Returns: The #PurpleMessageContentType of @message.
 *
 * Since: 3.0.0
 */
PurpleMessageContentType purple_message_get_content_type(PurpleMessage *message);

/**
 * purple_message_is_empty:
 * @message: The message.
 *
 * Checks, if the message's body is empty.
 *
 * Returns: %TRUE, if @message is empty.
 *
 * Since: 3.0.0
 */
gboolean purple_message_is_empty(PurpleMessage *message);

/**
 * purple_message_set_timestamp:
 * @message: The #PurpleMessage instance.
 * @timestamp: (nullable): The #GDateTime of the message.
 *
 * Sets the timestamp of @message.
 *
 * Since: 3.0.0
 */
void purple_message_set_timestamp(PurpleMessage *message, GDateTime *timestamp);

/**
 * purple_message_get_timestamp:
 * @message: The message.
 *
 * Returns a @message's timestamp.  If @message does not currently have a
 * timestamp, the current time will be set as the time stamp and returned.
 *
 * Returns: (transfer none): The #GDateTime timestamp from @message.
 *
 * Since: 3.0.0
 */
GDateTime *purple_message_get_timestamp(PurpleMessage *message);

/**
 * purple_message_format_timestamp:
 * @message: The #PurpleMessage instance.
 * @format: The format to output the time stamp as.
 *
 * Formats the timestamp of @message and returns it.
 *
 * Returns: The formatted timestamp.
 */
gchar *purple_message_format_timestamp(PurpleMessage *message, const gchar *format);

/**
 * purple_message_set_flags:
 * @message: The message.
 * @flags: The message flags.
 *
 * Sets flags for @message. It shouldn't be in a conflict with a message type,
 * so use it carefully.
 *
 * Since: 3.0.0
 */
void purple_message_set_flags(PurpleMessage *message, PurpleMessageFlags flags);

/**
 * purple_message_get_flags:
 * @message: The message.
 *
 * Returns the flags of a @message.
 *
 * Returns: the flags of a @message.
 *
 * Since: 3.0.0
 */
PurpleMessageFlags purple_message_get_flags(PurpleMessage *message);

/**
 * purple_message_set_error:
 * @message: The instance.
 * @error: (nullable) (transfer full): The error to set.
 *
 * Sets the error of @message to at @error.
 *
 * Since: 3.0.0
 */
void purple_message_set_error(PurpleMessage *message, GError *error);

/**
 * purple_message_get_error:
 * @message: The instance.
 *
 * Gets the error from @message.
 *
 * Returns: (nullable) (transfer none): The error from @message or %NULL.
 *
 * Since: 3.0.0
 */
GError *purple_message_get_error(PurpleMessage *message);

/**
 * purple_message_add_attachment:
 * @message: The #PurpleMessage instance.
 * @attachment: The #PurpleAttachment instance.
 *
 * Adds @attachment to @message.
 *
 * Returns %TRUE if an attachment with the same ID did not already exist.
 *
 * Since: 3.0.0
 */
gboolean purple_message_add_attachment(PurpleMessage *message, PurpleAttachment *attachment);

/**
 * purple_message_remove_attachment:
 * @message: The #PurpleMessage instance.
 * @id: The id of the #PurpleAttachment
 *
 * Removes the #PurpleAttachment identified by @id if it exists.
 *
 * Returns: %TRUE if the #PurpleAttachment was found and removed, %FALSE
 *          otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_message_remove_attachment(PurpleMessage *message, guint64 id);

/**
 * purple_message_get_attachment:
 * @message: The #PurpleMessage instance.
 * @id: The id of the #PurpleAttachment to get.
 *
 * Retrieves the #PurpleAttachment identified by @id from @message.
 *
 * Returns: (transfer full): The #PurpleAttachment if it was found, otherwise
 *                           %NULL.
 *
 * Since: 3.0.0
 */
PurpleAttachment *purple_message_get_attachment(PurpleMessage *message, guint64 id);

/**
 * purple_message_foreach_attachment:
 * @message: The #PurpleMessage instance.
 * @func: (scope call): The #PurpleAttachmentForeachFunc to call.
 * @data: User data to pass to @func.
 *
 * Calls @func for each #PurpleAttachment that's attached to @message.
 *
 * Since: 3.0.0
 */
void purple_message_foreach_attachment(PurpleMessage *message, PurpleAttachmentForeachFunc func, gpointer data);

/**
 * purple_message_clear_attachments:
 * @message: The #PurpleMessage instance.
 *
 * Removes all attachments from @message.
 *
 * Since: 3.0.0
 */
void purple_message_clear_attachments(PurpleMessage *message);

G_END_DECLS

#endif /* PURPLE_MESSAGE_H */
