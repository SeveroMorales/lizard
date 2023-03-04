/* purple
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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_CONVERSATION_UI_OPS_H
#define PURPLE_CONVERSATION_UI_OPS_H

#include <glib.h>
#include <glib-object.h>

#define PURPLE_TYPE_CONVERSATION_UI_OPS (purple_conversation_ui_ops_get_type())
typedef struct _PurpleConversationUiOps PurpleConversationUiOps;

#include <purpleconversation.h>
#include <purplechatconversation.h>
#include <purpleimconversation.h>
#include <purplemessage.h>

/**
 * PurpleConversationUiOps:
 * @create_conversation: Called when @conv is created (but before the
 *   <link linkend="conversations-conversation-created"><literal>"conversation-created"</literal></link>
 *                       signal is emitted).
 * @destroy_conversation: Called just before @conv is freed.
 * @write_chat: Write a message to a chat. If this field is %NULL, libpurple
 *              will fall back to using @write_conv.
 *              See purple_conversation_write_message().
 * @write_im: Write a message to an IM conversation. If this field is %NULL,
 *            libpurple will fall back to using @write_conv.
 *            See purple_conversation_write_message().
 * @write_conv: Write a message to a conversation. This is used rather than the
 *              chat- or im-specific ops for errors, system messages (such as "x
 *              is now know as y"), and as the fallback if @write_im and
 *              @write_chat are not implemented. It should be implemented, or
 *              the UI will miss conversation error messages and your users will
 *              hate you. See purple_conversation_write_message().
 * @chat_add_users: Add @cbuddies to a chat.
 *                  <sbr/>@cbuddies:     A GList of #PurpleChatUser structs.
 *                  <sbr/>@new_arrivals: Whether join notices should be shown.
 *                                       (Join notices are actually written to
 *                                       the conversation by
 *                                       purple_chat_conversation_add_users())
 * @chat_rename_user: Rename the user in this chat named @old_name to @new_name.
 *                    (The rename message is written to the conversation by
 *                    libpurple.) See purple_chat_conversation_rename_user().
 *                    <sbr/>@new_alias: @new_name's new alias, if they have one.
 * @chat_remove_users: Remove @users from a chat @chat.
 *                     See purple_chat_conversation_remove_users().
 * @chat_update_user: Called when a user's flags are changed.
 *                    See purple_chat_user_set_flags().
 * @present: Present this conversation to the user; for example, by displaying
 *           the IM dialog.
 * @has_focus: If this UI has a concept of focus (as in a windowing system) and
 *             this conversation has the focus, return %TRUE; otherwise, return
 *             %FALSE.
 * @send_confirm: Prompt the user for confirmation to send @message. This
 *                function should arrange for the message to be sent if the user
 *                accepts. If this field is %NULL, libpurple will fall back to
 *                using purple_request_action().
 *
 * libpurple needs to tell the user interface when certain things happen in a
 * conversation and it uses this structure to do so.
 *
 * Any UI representing a conversation must assign a filled-out
 * #PurpleConversationUiOps structure to the #PurpleConversation.
 */
struct _PurpleConversationUiOps
{
	void (*create_conversation)(PurpleConversation *conv);
	void (*destroy_conversation)(PurpleConversation *conv);

	void (*write_chat)(PurpleChatConversation *chat, PurpleMessage *msg);
	void (*write_im)(PurpleIMConversation *im, PurpleMessage *msg);
	void (*write_conv)(PurpleConversation *conv, PurpleMessage *msg);

	void (*chat_add_users)(PurpleChatConversation *chat,
	                       GList *cbuddies,
	                       gboolean new_arrivals);

	void (*chat_rename_user)(PurpleChatConversation *chat, const char *old_name,
	                         const char *new_name, const char *new_alias);

	void (*chat_remove_users)(PurpleChatConversation *chat, GList *users);

	void (*chat_update_user)(PurpleChatUser *cb);

	void (*present)(PurpleConversation *conv);
	gboolean (*has_focus)(PurpleConversation *conv);

	void (*send_confirm)(PurpleConversation *conv, const char *message);

	/*< private >*/
	void (*_purple_reserved1)(void);
	void (*_purple_reserved2)(void);
	void (*_purple_reserved3)(void);
	void (*_purple_reserved4)(void);
};

G_BEGIN_DECLS

/**
 * purple_conversation_ui_ops_get_type:
 *
 * Returns: The #GType for the #PurpleConversationUiOps boxed structure.
 */
GType purple_conversation_ui_ops_get_type(void);

G_END_DECLS

#endif /* PURPLE_CONVERSATION_UI_OPS_H */
