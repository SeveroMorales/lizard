/*
 * purple
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

#ifndef PURPLE_CHAT_CONVERSATION_H
#define PURPLE_CHAT_CONVERSATION_H

#define PURPLE_TYPE_CHAT_CONVERSATION            (purple_chat_conversation_get_type())
#define PURPLE_CHAT_CONVERSATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), PURPLE_TYPE_CHAT_CONVERSATION, PurpleChatConversation))
#define PURPLE_CHAT_CONVERSATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PURPLE_TYPE_CHAT_CONVERSATION, PurpleChatConversationClass))
#define PURPLE_IS_CHAT_CONVERSATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), PURPLE_TYPE_CHAT_CONVERSATION))
#define PURPLE_IS_CHAT_CONVERSATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), PURPLE_TYPE_CHAT_CONVERSATION))
#define PURPLE_CHAT_CONVERSATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PURPLE_TYPE_CHAT_CONVERSATION, PurpleChatConversationClass))

typedef struct _PurpleChatConversation           PurpleChatConversation;
typedef struct _PurpleChatConversationClass      PurpleChatConversationClass;

#include "purplechatuser.h"
#include "purpleconversation.h"
#include "purpleimconversation.h"

G_BEGIN_DECLS

/**
 * PurpleChatConversation:
 *
 * Structure representing a chat conversation instance.
 */
struct _PurpleChatConversation {
	PurpleConversation parent_object;
};

/**
 * PurpleChatConversationClass:
 *
 * Base class for all #PurpleChatConversation's
 */
struct _PurpleChatConversationClass {
	PurpleConversationClass parent_class;

	/*< private >*/
	gpointer reserved[4];
};

/**
 * purple_chat_conversation_get_type:
 *
 * Returns: The #GType for the ChatConversation object.
 */
GType purple_chat_conversation_get_type(void);

/**
 * purple_chat_conversation_new:
 * @account: The account opening the conversation window on the purple user's
 *           end.
 * @name: The name of the conversation.
 *
 * Creates a new chat conversation.
 *
 * Returns: The new conversation.
 */
PurpleConversation *purple_chat_conversation_new(PurpleAccount *account, const gchar *name);

/**
 * purple_chat_conversation_get_users:
 * @chat: The chat.
 *
 * Returns a list of users in the chat room.  The members of the list
 * are PurpleChatUser objects.
 *
 * Returns: (element-type PurpleChatUser) (transfer container):
 *          The list of users. Use g_list_free() when done
 *          using the list.
 */
GList *purple_chat_conversation_get_users(PurpleChatConversation *chat);

/**
 * purple_chat_conversation_get_users_count:
 * @chat: The chat.
 *
 * Returns count of users in the chat room.
 *
 * Returns: The count of users in the chat room.
 */
guint purple_chat_conversation_get_users_count(PurpleChatConversation *chat);

/**
 * purple_chat_conversation_ignore:
 * @chat: The chat.
 * @name: The name of the user.
 *
 * Ignores a user in a chat room.
 */
void purple_chat_conversation_ignore(PurpleChatConversation *chat, const gchar *name);

/**
 * purple_chat_conversation_unignore:
 * @chat: The chat.
 * @name: The name of the user.
 *
 * Unignores a user in a chat room.
 */
void purple_chat_conversation_unignore(PurpleChatConversation *chat, const gchar *name);

/**
 * purple_chat_conversation_set_ignored:
 * @chat: The chat.
 * @ignored: (element-type utf8): The list of ignored users.
 *
 * Sets the list of ignored users in the chat room.
 *
 * Returns: (element-type utf8) (transfer none): The list passed.
 */
GList *purple_chat_conversation_set_ignored(PurpleChatConversation *chat, GList *ignored);

/**
 * purple_chat_conversation_get_ignored:
 * @chat: The chat.
 *
 * Returns the list of ignored users in the chat room.
 *
 * Returns: (element-type utf8) (transfer none): The list of ignored users.
 */
GList *purple_chat_conversation_get_ignored(PurpleChatConversation *chat);

/**
 * purple_chat_conversation_get_ignored_user:
 * @chat: The chat.
 * @user: The user to check in the ignore list.
 *
 * Returns the actual name of the specified ignored user, if it exists in
 * the ignore list.
 *
 * If the user found contains a prefix, such as '+' or '\@', this is also
 * returned. The username passed to the function does not have to have this
 * formatting.
 *
 * Returns: The ignored user if found, complete with prefixes, or %NULL
 *          if not found.
 */
const gchar *purple_chat_conversation_get_ignored_user(PurpleChatConversation *chat, const gchar *user);

/**
 * purple_chat_conversation_is_ignored_user:
 * @chat: The chat.
 * @user: The user.
 *
 * Returns %TRUE if the specified user is ignored.
 *
 * Returns: %TRUE if the user is in the ignore list; %FALSE otherwise.
 */
gboolean purple_chat_conversation_is_ignored_user(PurpleChatConversation *chat, const gchar *user);

/**
 * purple_chat_conversation_set_topic:
 * @chat: The chat.
 * @who: The user that set the topic.
 * @topic: The topic.
 *
 * Sets the chat room's topic.
 */
void purple_chat_conversation_set_topic(PurpleChatConversation *chat, const gchar *who, const gchar *topic);

/**
 * purple_chat_conversation_get_topic:
 * @chat: The chat.
 *
 * Returns the chat room's topic.
 *
 * Returns: The chat's topic.
 */
const gchar *purple_chat_conversation_get_topic(PurpleChatConversation *chat);

/**
 * purple_chat_conversation_get_topic_who:
 * @chat: The chat.
 *
 * Returns who set the chat room's topic.
 *
 * Returns: Who set the topic.
 */
const gchar *purple_chat_conversation_get_topic_who(PurpleChatConversation *chat);

/**
 * purple_chat_conversation_set_id:
 * @chat: The chat.
 * @id: The ID.
 *
 * Sets the chat room's ID.
 */
void purple_chat_conversation_set_id(PurpleChatConversation *chat, gint id);

/**
 * purple_chat_conversation_get_id:
 * @chat: The chat.
 *
 * Gets the chat room's ID.
 *
 * Returns: The ID.
 */
gint purple_chat_conversation_get_id(PurpleChatConversation *chat);

/**
 * purple_chat_conversation_add_user:
 * @chat: The chat.
 * @user: The user to add.
 * @extra_msg: An extra message to display with the join message.
 * @flags: The users flags
 * @new_arrival: Decides whether or not to show a join notice.
 *
 * Adds a user to a chat.
 */
void purple_chat_conversation_add_user(PurpleChatConversation *chat, const gchar *user, const gchar *extra_msg, PurpleChatUserFlags flags, gboolean new_arrival);

/**
 * purple_chat_conversation_add_users:
 * @chat: The chat.
 * @users: (element-type utf8): The list of users to add.
 * @extra_msgs: (element-type utf8) (nullable): An extra message to display
 *              with the join message for each user.  This list may be shorter
 *              than @users, in which case, the users after the end of
 *              extra_msgs will not have an extra message.  By extension, this
 *              means that extra_msgs can simply be %NULL and none of the users
 *              will have an extra message.
 * @flags: (element-type PurpleChatUserFlags): The list of flags for each user.
 *         This list data should be an int converted to pointer using
 *         GINT_TO_POINTER(flag)
 * @new_arrivals: Decides whether or not to show join notices.
 *
 * Adds a list of users to a chat.
 *
 * The data is copied from @users, @extra_msgs, and @flags, so it is up to
 * the caller to free this list after calling this function.
 */
void purple_chat_conversation_add_users(PurpleChatConversation *chat, GList *users, GList *extra_msgs, GList *flags, gboolean new_arrivals);

/**
 * purple_chat_conversation_rename_user:
 * @chat: The chat.
 * @old_user: The old username.
 * @new_user: The new username.
 *
 * Renames a user in a chat.
 */
void purple_chat_conversation_rename_user(PurpleChatConversation *chat, const gchar *old_user, const gchar *new_user);

/**
 * purple_chat_conversation_remove_user:
 * @chat: The chat.
 * @user: The user that is being removed.
 * @reason: The optional reason given for the removal. Can be %NULL.
 *
 * Removes a user from a chat, optionally with a reason.
 *
 * It is up to the developer to free this list after calling this function.
 */
void purple_chat_conversation_remove_user(PurpleChatConversation *chat, const gchar *user, const gchar *reason);

/**
 * purple_chat_conversation_remove_users:
 * @chat: The chat.
 * @users: (element-type utf8): The users that are being removed.
 * @reason: The optional reason given for the removal. Can be %NULL.
 *
 * Removes a list of users from a chat, optionally with a single reason.
 */
void purple_chat_conversation_remove_users(PurpleChatConversation *chat, GList *users, const gchar *reason);

/**
 * purple_chat_conversation_has_user:
 * @chat: The chat.
 * @user: The user to look for.
 *
 * Checks if a user is in a chat
 *
 * Returns: %TRUE if the user is in the chat, %FALSE if not
 */
gboolean purple_chat_conversation_has_user(PurpleChatConversation *chat, const gchar *user);

/**
 * purple_chat_conversation_clear_users:
 * @chat: The chat.
 *
 * Clears all users from a chat.
 */
void purple_chat_conversation_clear_users(PurpleChatConversation *chat);

/**
 * purple_chat_conversation_set_nick:
 * @chat: The chat.
 * @nick: The nick.
 *
 * Sets your nickname (used for highlighting) for a chat.
 */
void purple_chat_conversation_set_nick(PurpleChatConversation *chat, const gchar *nick);

/**
 * purple_chat_conversation_get_nick:
 * @chat: The chat.
 *
 * Gets your nickname (used for highlighting) for a chat.
 *
 * Returns: The nick.
 */
const gchar *purple_chat_conversation_get_nick(PurpleChatConversation *chat);

/**
 * purple_chat_conversation_leave:
 * @chat: The chat.
 *
 * Lets the core know we left a chat, without destroying it.
 * Called from purple_serv_got_chat_left().
 */
void purple_chat_conversation_leave(PurpleChatConversation *chat);

/**
 * purple_chat_conversation_find_user:
 * @chat: The chat.
 * @name: The name of the chat user to find.
 *
 * Find a chat user in a chat
 *
 * Returns: (transfer none): The #PurpleChatUser with the name referred by
 *          @name.
 */
PurpleChatUser *purple_chat_conversation_find_user(PurpleChatConversation *chat, const gchar *name);

/**
 * purple_chat_conversation_invite_user:
 * @chat: The chat.
 * @user: The user to invite to the chat.
 * @message: The message to send with the invitation.
 * @confirm: Prompt before sending the invitation. The user is always prompted
 *           if either @user or @message is %NULL.
 *
 * Invite a user to a chat.
 * The user will be prompted to enter the user's name or a message if one is
 * not given.
 */
void purple_chat_conversation_invite_user(PurpleChatConversation *chat, const gchar *user, const gchar *message, gboolean confirm);

/**
 * purple_chat_conversation_has_left:
 * @chat: The chat.
 *
 * Gets whether we're no longer in this chat, and just left the window open.
 *
 * Returns: %TRUE if we left the chat already, %FALSE if we're still there.
 */
gboolean purple_chat_conversation_has_left(PurpleChatConversation *chat);

G_END_DECLS

#endif /* PURPLE_CHAT_CONVERSATION_H */
