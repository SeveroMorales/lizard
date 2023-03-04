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

#ifndef PURPLE_CHAT_USER_H
#define PURPLE_CHAT_USER_H

#include <glib.h>
#include <glib-object.h>

/**
 * PurpleChatUser:
 *
 * Structure representing a chat user instance.
 */

/**
 * PurpleChatUserFlags:
 * @PURPLE_CHAT_USER_NONE:    No flags
 * @PURPLE_CHAT_USER_VOICE:   Voiced user or "Participant"
 * @PURPLE_CHAT_USER_HALFOP:  Half-op
 * @PURPLE_CHAT_USER_OP:      Channel Op or Moderator
 * @PURPLE_CHAT_USER_FOUNDER: Channel Founder
 * @PURPLE_CHAT_USER_TYPING:  Currently typing
 * @PURPLE_CHAT_USER_AWAY:    Currently away.
 *
 * Flags applicable to users in Chats.
 */
typedef enum /*< flags >*/
{
	PURPLE_CHAT_USER_NONE     = 0x0000,
	PURPLE_CHAT_USER_VOICE    = 0x0001,
	PURPLE_CHAT_USER_HALFOP   = 0x0002,
	PURPLE_CHAT_USER_OP       = 0x0004,
	PURPLE_CHAT_USER_FOUNDER  = 0x0008,
	PURPLE_CHAT_USER_TYPING   = 0x0010,
	PURPLE_CHAT_USER_AWAY     = 0x0020
} PurpleChatUserFlags;

#define PURPLE_TYPE_CHAT_USER purple_chat_user_get_type()
G_DECLARE_FINAL_TYPE(PurpleChatUser, purple_chat_user, PURPLE, CHAT_USER,
                     GObject)

#include <libpurple/purplechatconversation.h>

G_BEGIN_DECLS

/**
 * purple_chat_user_new:
 * @chat: The chat that the buddy belongs to.
 * @name: The name.
 * @alias: The alias.
 * @flags: The flags.
 *
 * Creates a new chat user
 *
 * Returns: The new chat user
 */
PurpleChatUser *purple_chat_user_new(PurpleChatConversation *chat,
                                     const gchar *name,
                                     const gchar *alias,
                                     PurpleChatUserFlags flags);

/**
 * purple_chat_user_set_chat:
 * @chat_user: The chat user
 * @chat: The chat conversation that @chat_user belongs to.
 *
 * Set the chat conversation associated with this chat user.
 *
 * Since: 3.0.0
 */
void purple_chat_user_set_chat(PurpleChatUser *chat_user,
                               PurpleChatConversation *chat);

/**
 * purple_chat_user_get_chat:
 * @chat_user: The chat user.
 *
 * Get the chat conversation associated with this chat user.
 *
 * Returns: (transfer full): The chat conversation that the buddy belongs to.
 *
 * Since: 3.0.0
 */
PurpleChatConversation *purple_chat_user_get_chat(PurpleChatUser *chat_user);

/**
 * purple_chat_user_get_alias:
 * @chat_user: The chat user.
 *
 * Get the alias of a chat user.
 *
 * Returns: The alias of the chat user.
 *
 * Since: 3.0.0
 */
const gchar *purple_chat_user_get_alias(PurpleChatUser *chat_user);

/**
 * purple_chat_user_get_name:
 * @chat_user: The chat user.
 *
 * Get the name of a chat user.
 *
 * Returns: The name of the chat user.
 */
const gchar *purple_chat_user_get_name(PurpleChatUser *chat_user);

/**
 * purple_chat_user_set_flags:
 * @chat_user: The chat user.
 * @flags: The new flags.
 *
 * Set the flags of a chat user.
 */
void purple_chat_user_set_flags(PurpleChatUser *chat_user,
                                PurpleChatUserFlags flags);

/**
 * purple_chat_user_get_flags:
 * @chat_user: The chat user.
 *
 * Get the flags of a chat user.
 *
 * Returns: The flags of the chat user.
 *
 * Since: 3.0.0
 */
PurpleChatUserFlags purple_chat_user_get_flags(PurpleChatUser *chat_user);

/**
 * purple_chat_user_is_buddy:
 * @chat_user: The chat user.
 *
 * Indicates if this chat user is on the buddy list.
 *
 * Returns: %TRUE if the chat user is on the buddy list.
 *
 * Since: 3.0.0
 */
gboolean purple_chat_user_is_buddy(PurpleChatUser *chat_user);

/**
 * purple_chat_user_compare:
 * @a: The first #PurpleChatUser.
 * @b: The second #PurpleChatUser.
 *
 * Compares #PurpleChatUser's @a and @b and returns -1 if @a should be sorted
 * first, 0 if sorted equally, and 1 if @a should be sorted after @b.
 *
 * Returns: The sorting order of @a and @b.
 */
gint purple_chat_user_compare(PurpleChatUser *a, PurpleChatUser *b);

G_END_DECLS

#endif /* PURPLE_CHAT_USER_H */
