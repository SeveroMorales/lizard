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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include "conversations.h"
#include "purpleprivate.h"

#include "purpleconversationmanager.h"

static PurpleConversationUiOps *default_ops = NULL;

void
purple_conversations_set_ui_ops(PurpleConversationUiOps *ops)
{
	default_ops = ops;
}

PurpleConversationUiOps *
purple_conversations_get_ui_ops(void)
{
	return default_ops;
}

void *
purple_conversations_get_handle(void)
{
	static int handle;

	return &handle;
}

void
purple_conversations_init(void)
{
	void *handle = purple_conversations_get_handle();

	/**********************************************************************
	 * Register preferences
	 **********************************************************************/

	/* Conversations */
	purple_prefs_add_none("/purple/conversations");

	/* Conversations -> Chat */
	purple_prefs_add_none("/purple/conversations/chat");
	purple_prefs_add_bool("/purple/conversations/chat/show_nick_change", TRUE);

	/* Conversations -> IM */
	purple_prefs_add_none("/purple/conversations/im");
	purple_prefs_add_bool("/purple/conversations/im/send_typing", TRUE);


	/**********************************************************************
	 * Register signals
	 **********************************************************************/
	purple_signal_register(handle, "writing-im-msg",
		purple_marshal_BOOLEAN__POINTER_POINTER, G_TYPE_BOOLEAN, 2,
		PURPLE_TYPE_IM_CONVERSATION, PURPLE_TYPE_MESSAGE);

	purple_signal_register(handle, "wrote-im-msg",
		purple_marshal_VOID__POINTER_POINTER, G_TYPE_NONE, 2,
		PURPLE_TYPE_IM_CONVERSATION, PURPLE_TYPE_MESSAGE);

	purple_signal_register(handle, "sending-im-msg",
		purple_marshal_VOID__POINTER_POINTER, G_TYPE_NONE,
		2, PURPLE_TYPE_ACCOUNT, PURPLE_TYPE_MESSAGE);

	purple_signal_register(handle, "sent-im-msg",
		purple_marshal_VOID__POINTER_POINTER, G_TYPE_NONE,
		2, PURPLE_TYPE_ACCOUNT, PURPLE_TYPE_MESSAGE);

	purple_signal_register(handle, "receiving-im-msg",
						 purple_marshal_BOOLEAN__POINTER_POINTER_POINTER_POINTER_POINTER,
						 G_TYPE_BOOLEAN, 5, PURPLE_TYPE_ACCOUNT,
						 G_TYPE_POINTER, /* pointer to a string */
						 G_TYPE_POINTER, /* pointer to a string */
						 PURPLE_TYPE_IM_CONVERSATION,
						 G_TYPE_POINTER); /* pointer to a string */

	purple_signal_register(handle, "received-im-msg",
						 purple_marshal_VOID__POINTER_POINTER_POINTER_POINTER_UINT,
						 G_TYPE_NONE, 5, PURPLE_TYPE_ACCOUNT, G_TYPE_STRING,
						 G_TYPE_STRING, PURPLE_TYPE_IM_CONVERSATION, G_TYPE_UINT);

	purple_signal_register(handle, "blocked-im-msg",
						 purple_marshal_VOID__POINTER_POINTER_POINTER_UINT_UINT,
						 G_TYPE_NONE, 5, PURPLE_TYPE_ACCOUNT, G_TYPE_STRING,
						 G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT);

	purple_signal_register(handle, "writing-chat-msg",
		purple_marshal_BOOLEAN__POINTER_POINTER, G_TYPE_BOOLEAN, 2,
		PURPLE_TYPE_IM_CONVERSATION, PURPLE_TYPE_MESSAGE);

	purple_signal_register(handle, "wrote-chat-msg",
		purple_marshal_VOID__POINTER_POINTER, G_TYPE_NONE, 2,
		PURPLE_TYPE_IM_CONVERSATION, PURPLE_TYPE_MESSAGE);

	purple_signal_register(handle, "sending-chat-msg",
		purple_marshal_VOID__POINTER_POINTER_UINT, G_TYPE_NONE,
		3, PURPLE_TYPE_ACCOUNT, PURPLE_TYPE_MESSAGE, G_TYPE_UINT);

	purple_signal_register(handle, "sent-chat-msg",
		purple_marshal_VOID__POINTER_POINTER_UINT, G_TYPE_NONE,
		3, PURPLE_TYPE_ACCOUNT, PURPLE_TYPE_MESSAGE, G_TYPE_UINT);

	purple_signal_register(handle, "receiving-chat-msg",
						 purple_marshal_BOOLEAN__POINTER_POINTER_POINTER_POINTER_POINTER,
						 G_TYPE_BOOLEAN, 5, PURPLE_TYPE_ACCOUNT,
						 G_TYPE_POINTER, /* pointer to a string */
						 G_TYPE_POINTER, /* pointer to a string */
						 PURPLE_TYPE_CHAT_CONVERSATION,
						 G_TYPE_POINTER); /* pointer to an unsigned int */

	purple_signal_register(handle, "received-chat-msg",
						 purple_marshal_VOID__POINTER_POINTER_POINTER_POINTER_UINT,
						 G_TYPE_NONE, 5, PURPLE_TYPE_ACCOUNT, G_TYPE_STRING,
						 G_TYPE_STRING, PURPLE_TYPE_CHAT_CONVERSATION, G_TYPE_UINT);

	purple_signal_register(handle, "conversation-created",
						 purple_marshal_VOID__POINTER, G_TYPE_NONE, 1,
						 PURPLE_TYPE_CONVERSATION);

	purple_signal_register(handle, "conversation-updated",
						 purple_marshal_VOID__POINTER_UINT, G_TYPE_NONE, 2,
						 PURPLE_TYPE_CONVERSATION, G_TYPE_UINT);

	purple_signal_register(handle, "deleting-conversation",
						 purple_marshal_VOID__POINTER, G_TYPE_NONE, 1,
						 PURPLE_TYPE_CONVERSATION);

	purple_signal_register(handle, "buddy-typing",
						 purple_marshal_VOID__POINTER_POINTER, G_TYPE_NONE, 2,
						 PURPLE_TYPE_ACCOUNT, G_TYPE_STRING);

	purple_signal_register(handle, "buddy-typed",
						 purple_marshal_VOID__POINTER_POINTER, G_TYPE_NONE, 2,
						 PURPLE_TYPE_ACCOUNT, G_TYPE_STRING);

	purple_signal_register(handle, "buddy-typing-stopped",
						 purple_marshal_VOID__POINTER_POINTER, G_TYPE_NONE, 2,
						 PURPLE_TYPE_ACCOUNT, G_TYPE_STRING);

	purple_signal_register(handle, "chat-user-joining",
						 purple_marshal_BOOLEAN__POINTER_POINTER_UINT,
						 G_TYPE_BOOLEAN, 3, PURPLE_TYPE_CHAT_CONVERSATION,
						 G_TYPE_STRING, G_TYPE_UINT);

	purple_signal_register(handle, "chat-user-joined",
						 purple_marshal_VOID__POINTER_POINTER_UINT_UINT,
						 G_TYPE_NONE, 4, PURPLE_TYPE_CHAT_CONVERSATION,
						 G_TYPE_STRING, G_TYPE_UINT, G_TYPE_BOOLEAN);

	purple_signal_register(handle, "chat-user-flags",
						 purple_marshal_VOID__POINTER_UINT_UINT, G_TYPE_NONE, 3,
						 PURPLE_TYPE_CHAT_USER, G_TYPE_UINT, G_TYPE_UINT);

	purple_signal_register(handle, "chat-user-leaving",
						 purple_marshal_BOOLEAN__POINTER_POINTER_POINTER,
						 G_TYPE_BOOLEAN, 3, PURPLE_TYPE_CHAT_CONVERSATION,
						 G_TYPE_STRING, G_TYPE_STRING);

	purple_signal_register(handle, "chat-user-left",
						 purple_marshal_VOID__POINTER_POINTER_POINTER,
						 G_TYPE_NONE, 3, PURPLE_TYPE_CHAT_CONVERSATION,
						 G_TYPE_STRING, G_TYPE_STRING);

	purple_signal_register(handle, "chat-inviting-user",
						 purple_marshal_VOID__POINTER_POINTER_POINTER,
						 G_TYPE_NONE, 3, PURPLE_TYPE_CHAT_CONVERSATION, 
						 G_TYPE_STRING,
						 G_TYPE_POINTER); /* pointer to a string */

	purple_signal_register(handle, "chat-invited-user",
						 purple_marshal_VOID__POINTER_POINTER_POINTER,
						 G_TYPE_NONE, 3, PURPLE_TYPE_CHAT_CONVERSATION,
						 G_TYPE_STRING, G_TYPE_STRING);

	purple_signal_register(handle, "chat-invited",
						 purple_marshal_INT__POINTER_POINTER_POINTER_POINTER_POINTER,
						 G_TYPE_INT, 5, PURPLE_TYPE_ACCOUNT, G_TYPE_STRING,
						 G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);

	purple_signal_register(handle, "chat-invite-blocked",
						 purple_marshal_VOID__POINTER_POINTER_POINTER_POINTER_POINTER,
						 G_TYPE_NONE, 5, PURPLE_TYPE_ACCOUNT, G_TYPE_STRING,
						 G_TYPE_STRING, G_TYPE_STRING,
						 G_TYPE_POINTER); /* (GHashTable *) */

	purple_signal_register(handle, "chat-joined",
						 purple_marshal_VOID__POINTER, G_TYPE_NONE, 1,
						 PURPLE_TYPE_CHAT_CONVERSATION);

	purple_signal_register(handle, "chat-join-failed",
						   purple_marshal_VOID__POINTER_POINTER, G_TYPE_NONE, 2,
						   PURPLE_TYPE_CONNECTION, G_TYPE_POINTER);

	purple_signal_register(handle, "chat-left",
						 purple_marshal_VOID__POINTER, G_TYPE_NONE, 1,
						 PURPLE_TYPE_CHAT_CONVERSATION);

	purple_signal_register(handle, "chat-topic-changed",
						 purple_marshal_VOID__POINTER_POINTER_POINTER,
						 G_TYPE_NONE, 3, PURPLE_TYPE_CHAT_CONVERSATION,
						 G_TYPE_STRING, G_TYPE_STRING);

	purple_signal_register(handle, "cleared-message-history",
	                       purple_marshal_VOID__POINTER, G_TYPE_NONE, 1,
	                       PURPLE_TYPE_CONVERSATION);

	purple_signal_register(handle, "conversation-extended-menu",
			     purple_marshal_VOID__POINTER_POINTER, G_TYPE_NONE, 2,
			     PURPLE_TYPE_CONVERSATION,
			     G_TYPE_POINTER); /* (GList **) */
}

void
purple_conversations_uninit(void)
{
	purple_signals_unregister_by_instance(purple_conversations_get_handle());
}
