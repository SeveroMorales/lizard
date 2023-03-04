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

#include <glib/gi18n-lib.h>

#include "internal.h"
#include "debug.h"
#include "purpleconversationmanager.h"
#include "purplechatconversation.h"
#include "purpleenums.h"
#include "purpleprivate.h"
#include "request.h"
#include "server.h"

typedef struct {
	GList *ignored;     /* Ignored users.                            */
	char  *who;         /* The person who set the topic.             */
	char  *topic;       /* The topic.                                */
	int    id;          /* The chat ID.                              */
	char *nick;         /* Your nick in this chat.                   */
	gboolean left;      /* We left the chat and kept the window open */
	GHashTable *users;  /* Hash table of the users in the room.      */
} PurpleChatConversationPrivate;

enum {
	PROP_0,
	PROP_TOPIC_WHO,
	PROP_TOPIC,
	PROP_CHAT_ID,
	PROP_NICK,
	PROP_LEFT,
	N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES] = { NULL, };

enum {
	SIG_USER_JOINED,
	SIG_USER_LEFT,
	N_SIGNALS
};
static guint signals[N_SIGNALS] = { 0, };

G_DEFINE_TYPE_WITH_PRIVATE(PurpleChatConversation, purple_chat_conversation,
                           PURPLE_TYPE_CONVERSATION);

/**************************************************************************
 * Helpers
 **************************************************************************/
static guint
purple_conversation_user_hash(gconstpointer data) {
	gchar *collated;
	guint hash;

	collated = g_utf8_collate_key((const gchar *)data, -1);
	hash = g_str_hash(collated);
	g_free(collated);

	return hash;
}

static gboolean
purple_conversation_user_equal(gconstpointer a, gconstpointer b) {
	return !g_utf8_collate(a, b);
}

static void
purple_chat_conversation_clear_users_helper(gpointer data, gpointer user_data)
{
	PurpleChatConversation *chat = PURPLE_CHAT_CONVERSATION(user_data);
	const gchar *name = (const gchar *)data;
	gpointer handle = purple_conversations_get_handle();

	purple_signal_emit(handle, "chat-user-leaving", chat, name, NULL);
	purple_signal_emit(handle, "chat-user-left", chat, name, NULL);
}

/******************************************************************************
 * PurpleConversation Implementation
 *****************************************************************************/
static void
chat_conversation_write_message(PurpleConversation *conv, PurpleMessage *msg) {
	PurpleChatConversation *chat_conv = PURPLE_CHAT_CONVERSATION(conv);
	PurpleChatConversationPrivate *priv = NULL;
	PurpleMessageFlags flags;
	const gchar *author = NULL;

	g_return_if_fail(msg != NULL);

	priv = purple_chat_conversation_get_instance_private(chat_conv);

	/* Don't display this if the person who wrote it is ignored. */
	author = purple_message_get_author(msg);
	if(purple_chat_conversation_is_ignored_user(chat_conv, author)) {
		return;
	}

	flags = purple_message_get_flags(msg);
	if(flags & PURPLE_MESSAGE_RECV) {
		if(purple_utf8_has_word(purple_message_get_contents(msg), priv->nick)) {
			flags |= PURPLE_MESSAGE_NICK;
			purple_message_set_flags(msg, flags);
		}
	}

	_purple_conversation_write_common(conv, msg);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_chat_conversation_set_property(GObject *obj, guint param_id,
                                      const GValue *value, GParamSpec *pspec)
{
	PurpleChatConversation *chat = PURPLE_CHAT_CONVERSATION(obj);

	switch(param_id) {
		case PROP_CHAT_ID:
			purple_chat_conversation_set_id(chat, g_value_get_int(value));
			break;
		case PROP_NICK:
			purple_chat_conversation_set_nick(chat, g_value_get_string(value));
			break;
		case PROP_LEFT:
			if(g_value_get_boolean(value)) {
				purple_chat_conversation_leave(chat);
			}
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_chat_conversation_get_property(GObject *obj, guint param_id,
                                      GValue *value, GParamSpec *pspec)
{
	PurpleChatConversation *chat = PURPLE_CHAT_CONVERSATION(obj);

	switch(param_id) {
		case PROP_TOPIC_WHO:
			g_value_set_string(value, purple_chat_conversation_get_topic_who(chat));
			break;
		case PROP_TOPIC:
			g_value_set_string(value, purple_chat_conversation_get_topic(chat));
			break;
		case PROP_CHAT_ID:
			g_value_set_int(value, purple_chat_conversation_get_id(chat));
			break;
		case PROP_NICK:
			g_value_set_string(value, purple_chat_conversation_get_nick(chat));
			break;
		case PROP_LEFT:
			g_value_set_boolean(value, purple_chat_conversation_has_left(chat));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_chat_conversation_init(PurpleChatConversation *chat) {
	PurpleChatConversationPrivate *priv = NULL;

	priv = purple_chat_conversation_get_instance_private(chat);

	priv->users = g_hash_table_new_full(purple_conversation_user_hash,
	                                    purple_conversation_user_equal,
	                                    g_free, g_object_unref);
}

static void
purple_chat_conversation_constructed(GObject *obj) {
	PurpleChatConversation *chat = PURPLE_CHAT_CONVERSATION(obj);
	PurpleAccount *account = NULL;
	PurpleConnection *connection = NULL;
	const gchar *display_name = NULL;

	G_OBJECT_CLASS(purple_chat_conversation_parent_class)->constructed(obj);

	g_object_get(obj, "account", &account, NULL);

	connection = purple_account_get_connection(account);
	display_name = purple_connection_get_display_name(connection);
	if(display_name != NULL) {
		purple_chat_conversation_set_nick(chat, display_name);
	} else {
		PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
		const gchar *username = purple_contact_info_get_username(info);

		purple_chat_conversation_set_nick(chat, username);
	}

	g_object_unref(account);
}

static void
purple_chat_conversation_dispose(GObject *obj) {
	PurpleChatConversation *chat = PURPLE_CHAT_CONVERSATION(obj);
	PurpleChatConversationPrivate *priv = NULL;

	priv = purple_chat_conversation_get_instance_private(chat);

	g_hash_table_remove_all(priv->users);

	G_OBJECT_CLASS(purple_chat_conversation_parent_class)->dispose(obj);
}

static void
purple_chat_conversation_finalize(GObject *obj) {
	PurpleChatConversation *chat = PURPLE_CHAT_CONVERSATION(obj);
	PurpleConversation *conv = PURPLE_CONVERSATION(chat);
	PurpleConnection *gc = purple_conversation_get_connection(conv);
	PurpleChatConversationPrivate *priv = NULL;

	priv = purple_chat_conversation_get_instance_private(chat);

	if(PURPLE_IS_CONNECTION(gc)) {
		/* Still connected */
		gint chat_id = purple_chat_conversation_get_id(chat);

		/*
		 * Close the window when the user tells us to, and let the protocol
		 * deal with the internals on its own time. Don't do this if the
		 * protocol already knows it left the chat.
		 */
		if(!purple_chat_conversation_has_left(chat)) {
			purple_serv_chat_leave(gc, chat_id);
		}

		/*
		 * If they didn't call purple_serv_got_chat_left by now, it's too late.
		 * So we better do it for them before we destroy the thing.
		 */
		if(!purple_chat_conversation_has_left(chat)) {
			purple_serv_got_chat_left(gc, chat_id);
		}
	}

	g_clear_pointer(&priv->users, g_hash_table_destroy);

	g_list_free_full(priv->ignored, g_free);
	priv->ignored = NULL;

	g_clear_pointer(&priv->who, g_free);
	g_clear_pointer(&priv->topic, g_free);
	g_clear_pointer(&priv->nick, g_free);

	G_OBJECT_CLASS(purple_chat_conversation_parent_class)->finalize(obj);
}

static void
purple_chat_conversation_class_init(PurpleChatConversationClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	PurpleConversationClass *conv_class = PURPLE_CONVERSATION_CLASS(klass);

	obj_class->constructed = purple_chat_conversation_constructed;
	obj_class->dispose = purple_chat_conversation_dispose;
	obj_class->finalize = purple_chat_conversation_finalize;
	obj_class->get_property = purple_chat_conversation_get_property;
	obj_class->set_property = purple_chat_conversation_set_property;

	conv_class->write_message = chat_conversation_write_message;

	/**
	 * PurpleChatConversation::topic-who:
	 *
	 * The username who changed the topic last.
	 */
	properties[PROP_TOPIC_WHO] = g_param_spec_string(
		"topic-who", "who set topic",
		"Who set the topic of the chat.",
		NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleChatConversation::topic:
	 *
	 * The text of the topic.
	 */
	properties[PROP_TOPIC] = g_param_spec_string(
		"topic", "topic",
		"The topic of the chat.",
		NULL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleChatConversation::chat-id:
	 *
	 * The identifier of the chat.
	 */
	properties[PROP_CHAT_ID] = g_param_spec_int(
		"chat-id", "chat id",
		"The identifier of the chat.",
		G_MININT, G_MAXINT, 0,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleChatConversation::nick:
	 *
	 * The nickname of the user in the chat.
	 */
	properties[PROP_NICK] = g_param_spec_string(
		"nick", "nick",
		"The nickname of the user in a chat.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleChatConversation::left:
	 *
	 * Whether the user has left the chat or not.
	 */
	properties[PROP_LEFT] = g_param_spec_boolean(
		"left", "left",
		"Whether the user has left the chat.",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	/* Signals */

	/**
	 * PurpleChatConversation::user-joined:
	 * @chat: The chat instance.
	 * @username: The user that joined the conversation.
	 * @flags: The [flags@ChatUserFlags] for user.
	 * @new_arrival: %TRUE if the user is new to the conversation.
	 *
	 * Emitted after a @username has joined the conversation.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_USER_JOINED] = g_signal_new_class_handler(
		"user-joined",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		3,
		G_TYPE_STRING,
		PURPLE_TYPE_CHAT_USER_FLAGS,
		G_TYPE_BOOLEAN);

	/**
	 * PurpleChatConversation::user-left:
	 * @chat: The chat instance.
	 * @username: The user that left the conversation
	 * @reason: The optional reason why the user left the chat.
	 *
	 * Emitted after a @username has left the conversation.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_USER_LEFT] = g_signal_new_class_handler(
		"user-left",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		2,
		G_TYPE_STRING,
		G_TYPE_STRING);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleConversation *
purple_chat_conversation_new(PurpleAccount *account, const gchar *name) {
	PurpleConversation *chat = NULL;
	PurpleConversationManager *manager = NULL;
	PurpleConnection *connection = NULL;

	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);
	g_return_val_if_fail(name != NULL, NULL);

	/* Check if this conversation already exists. */
	manager = purple_conversation_manager_get_default();
	chat = purple_conversation_manager_find_chat(manager, account, name);
	if(PURPLE_IS_CHAT_CONVERSATION(chat)) {
		if(!purple_chat_conversation_has_left(PURPLE_CHAT_CONVERSATION(chat))) {
			purple_debug_warning("chat-conversation", "A chat named %s "
			                     "already exists.", name);
			return NULL;
		}
	}

	connection = purple_account_get_connection(account);
	if(!PURPLE_IS_CONNECTION(connection)) {
		PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
		purple_debug_warning("chat-conversation", "Refusing to create chat "
		                     "for disconnected account %s",
		                     purple_contact_info_get_username(info));
		return NULL;
	}

	return g_object_new(
		PURPLE_TYPE_CHAT_CONVERSATION,
		"account", account,
		"name", name,
		"title", name,
		NULL);
}

GList *
purple_chat_conversation_get_users(PurpleChatConversation *chat) {
	PurpleChatConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat), NULL);

	priv = purple_chat_conversation_get_instance_private(chat);

	return g_hash_table_get_values(priv->users);
}

guint
purple_chat_conversation_get_users_count(PurpleChatConversation *chat) {
	PurpleChatConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat), 0);

	priv = purple_chat_conversation_get_instance_private(chat);

	return g_hash_table_size(priv->users);
}

void
purple_chat_conversation_ignore(PurpleChatConversation *chat,
                                const gchar *name)
{
	PurpleChatConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat));
	g_return_if_fail(name != NULL);

	priv = purple_chat_conversation_get_instance_private(chat);

	/* Make sure the user isn't already ignored. */
	if(purple_chat_conversation_is_ignored_user(chat, name)) {
		return;
	}

	priv->ignored = g_list_prepend(priv->ignored, g_strdup(name));
}

void
purple_chat_conversation_unignore(PurpleChatConversation *chat,
                                  const gchar *name)
{
	PurpleChatConversationPrivate *priv = NULL;
	GList *item;

	g_return_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat));
	g_return_if_fail(name != NULL);

	priv = purple_chat_conversation_get_instance_private(chat);

	/* Make sure the user is actually ignored. */
	if(!purple_chat_conversation_is_ignored_user(chat, name)) {
		return;
	}

	item = g_list_find(priv->ignored,
					   purple_chat_conversation_get_ignored_user(chat, name));
	g_free(item->data);

	priv->ignored = g_list_delete_link(priv->ignored, item);
}

GList *
purple_chat_conversation_set_ignored(PurpleChatConversation *chat,
                                     GList *ignored)
{
	PurpleChatConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat), NULL);

	priv = purple_chat_conversation_get_instance_private(chat);

	priv->ignored = ignored;

	return ignored;
}

GList *
purple_chat_conversation_get_ignored(PurpleChatConversation *chat) {
	PurpleChatConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat), NULL);

	priv = purple_chat_conversation_get_instance_private(chat);
	return priv->ignored;
}

const gchar *
purple_chat_conversation_get_ignored_user(PurpleChatConversation *chat,
                                          const gchar *user)
{
	GList *ignored;

	g_return_val_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat), NULL);
	g_return_val_if_fail(user != NULL, NULL);

	ignored = purple_chat_conversation_get_ignored(chat);
	for(; ignored != NULL; ignored = ignored->next) {
		const gchar *ign = (const gchar *)ignored->data;

		if(!purple_utf8_strcasecmp(user, ign) ||
			((*ign == '+' || *ign == '%') &&
			  !purple_utf8_strcasecmp(user, ign + 1)))
		{
			return ign;
		}

		if(*ign == '@') {
			ign++;

			if((*ign == '+' && !purple_utf8_strcasecmp(user, ign + 1)) ||
			   (*ign != '+' && !purple_utf8_strcasecmp(user, ign)))
			{
				return ign;
			}
		}
	}

	return NULL;
}

gboolean
purple_chat_conversation_is_ignored_user(PurpleChatConversation *chat,
                                         const gchar *user)
{
	g_return_val_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat), FALSE);
	g_return_val_if_fail(user != NULL, FALSE);

	return (purple_chat_conversation_get_ignored_user(chat, user) != NULL);
}

void
purple_chat_conversation_set_topic(PurpleChatConversation *chat,
                                   const gchar *who, const gchar *topic)
{
	PurpleChatConversationPrivate *priv = NULL;
	GObject *obj;

	g_return_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat));

	priv = purple_chat_conversation_get_instance_private(chat);

	g_clear_pointer(&priv->who, g_free);
	g_clear_pointer(&priv->topic, g_free);

	priv->who = g_strdup(who);
	priv->topic = g_strdup(topic);

	obj = G_OBJECT(chat);
	g_object_freeze_notify(obj);
	g_object_notify_by_pspec(obj, properties[PROP_TOPIC_WHO]);
	g_object_notify_by_pspec(obj, properties[PROP_TOPIC]);
	g_object_thaw_notify(obj);

	purple_conversation_update(PURPLE_CONVERSATION(chat),
	                           PURPLE_CONVERSATION_UPDATE_TOPIC);

	purple_signal_emit(purple_conversations_get_handle(), "chat-topic-changed",
	                   chat, priv->who, priv->topic);
}

const gchar *
purple_chat_conversation_get_topic(PurpleChatConversation *chat) {
	PurpleChatConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat), NULL);

	priv = purple_chat_conversation_get_instance_private(chat);

	return priv->topic;
}

const gchar *
purple_chat_conversation_get_topic_who(PurpleChatConversation *chat) {
	PurpleChatConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat), NULL);

	priv = purple_chat_conversation_get_instance_private(chat);

	return priv->who;
}

void
purple_chat_conversation_set_id(PurpleChatConversation *chat, gint id) {
	PurpleChatConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat));

	priv = purple_chat_conversation_get_instance_private(chat);
	priv->id = id;

	g_object_notify_by_pspec(G_OBJECT(chat), properties[PROP_CHAT_ID]);
}

gint
purple_chat_conversation_get_id(PurpleChatConversation *chat) {
	PurpleChatConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat), -1);

	priv = purple_chat_conversation_get_instance_private(chat);

	return priv->id;
}

void
purple_chat_conversation_add_user(PurpleChatConversation *chat,
                                  const gchar *user, const gchar *extra_msg,
                                  PurpleChatUserFlags flags,
                                  gboolean new_arrival)
{
	GList *users = g_list_append(NULL, (gchar *)user);
	GList *extra_msgs = g_list_append(NULL, (gchar *)extra_msg);
	GList *flags2 = g_list_append(NULL, GINT_TO_POINTER(flags));

	purple_chat_conversation_add_users(chat, users, extra_msgs, flags2,
	                                   new_arrival);

	g_list_free(users);
	g_list_free(extra_msgs);
	g_list_free(flags2);
}

void
purple_chat_conversation_add_users(PurpleChatConversation *chat, GList *users,
                                   GList *extra_msgs, GList *flags,
                                   gboolean new_arrivals)
{
	PurpleConversation *conv;
	PurpleConversationUiOps *ops;
	PurpleChatUser *chatuser;
	PurpleChatConversationPrivate *priv;
	PurpleAccount *account;
	PurpleConnection *gc;
	PurpleProtocol *protocol;
	GList *cbuddies = NULL;
	gpointer handle;

	g_return_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat));
	g_return_if_fail(users != NULL);

	priv = purple_chat_conversation_get_instance_private(chat);
	conv = PURPLE_CONVERSATION(chat);
	ops = purple_conversation_get_ui_ops(conv);

	account = purple_conversation_get_account(conv);
	gc = purple_conversation_get_connection(conv);
	g_return_if_fail(PURPLE_IS_CONNECTION(gc));

	protocol = purple_connection_get_protocol(gc);
	g_return_if_fail(PURPLE_IS_PROTOCOL(protocol));

	handle = purple_conversations_get_handle();

	while(users != NULL && flags != NULL) {
		const gchar *user = (const gchar *)users->data;
		const gchar *alias = user;
		gboolean quiet;
		PurpleChatUserFlags flag = GPOINTER_TO_INT(flags->data);
		const gchar *extra_msg = (extra_msgs ? extra_msgs->data : NULL);

		if(!(purple_protocol_get_options(protocol) & OPT_PROTO_UNIQUE_CHATNAME)) {
			if(purple_strequal(priv->nick, purple_normalize(account, user))) {
				PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
				const gchar *alias2 = purple_contact_info_get_alias(info);
				if(alias2 != NULL) {
					alias = alias2;
				} else {
					const gchar *display_name = purple_connection_get_display_name(gc);
					if(display_name != NULL) {
						alias = display_name;
					}
				}
			} else {
				PurpleBuddy *buddy;
				if((buddy = purple_blist_find_buddy(purple_connection_get_account(gc), user)) != NULL) {
					alias = purple_buddy_get_contact_alias(buddy);
				}
			}
		}

		quiet = GPOINTER_TO_INT(purple_signal_emit_return_1(handle,
		                        "chat-user-joining", chat, user, flag)) ||
				purple_chat_conversation_is_ignored_user(chat, user);

		chatuser = purple_chat_user_new(chat, user, alias, flag);

		g_hash_table_replace(priv->users,
			g_strdup(purple_chat_user_get_name(chatuser)),
			chatuser);

		cbuddies = g_list_prepend(cbuddies, chatuser);

		if(!quiet && new_arrivals) {
			gchar *alias_esc = g_markup_escape_text(alias, -1);
			gchar *tmp;

			if(extra_msg == NULL) {
				tmp = g_strdup_printf(_("%s entered the room."), alias_esc);
			} else {
				gchar *extra_msg_esc = g_markup_escape_text(extra_msg, -1);
				tmp = g_strdup_printf(_("%s [<I>%s</I>] entered the room."),
				                      alias_esc, extra_msg_esc);
				g_free(extra_msg_esc);
			}
			g_free(alias_esc);

			purple_conversation_write_system_message(conv, tmp,
			                                         PURPLE_MESSAGE_NO_LINKIFY);
			g_free(tmp);
		}

		purple_signal_emit(handle, "chat-user-joined", chat, user, flag,
		                   new_arrivals);

		g_signal_emit(chat, signals[SIG_USER_JOINED], 0, user, flag,
		              new_arrivals);

		users = users->next;
		flags = flags->next;
		if(extra_msgs != NULL) {
			extra_msgs = extra_msgs->next;
		}
	}

	cbuddies = g_list_sort(cbuddies, (GCompareFunc)purple_chat_user_compare);

	if(ops != NULL && ops->chat_add_users != NULL) {
		ops->chat_add_users(chat, cbuddies, new_arrivals);
	}

	g_list_free(cbuddies);
}

void
purple_chat_conversation_rename_user(PurpleChatConversation *chat,
                                     const gchar *old_user,
                                     const gchar *new_user)
{
	PurpleConversation *conv;
	PurpleConversationUiOps *ops;
	PurpleAccount *account;
	PurpleConnection *gc;
	PurpleProtocol *protocol;
	PurpleChatUser *cb;
	PurpleChatUserFlags flags;
	PurpleChatConversationPrivate *priv;
	const gchar *new_alias = new_user;
	gchar tmp[BUF_LONG];
	gboolean is_me = FALSE;

	g_return_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat));
	g_return_if_fail(old_user != NULL);
	g_return_if_fail(new_user != NULL);

	priv = purple_chat_conversation_get_instance_private(chat);
	conv = PURPLE_CONVERSATION(chat);
	ops = purple_conversation_get_ui_ops(conv);
	account = purple_conversation_get_account(conv);

	gc = purple_conversation_get_connection(conv);
	g_return_if_fail(PURPLE_IS_CONNECTION(gc));
	protocol = purple_connection_get_protocol(gc);
	g_return_if_fail(PURPLE_IS_PROTOCOL(protocol));

	if(purple_strequal(priv->nick, purple_normalize(account, old_user))) {
		const gchar *alias;

		/* Note this for later. */
		is_me = TRUE;

		if(!(purple_protocol_get_options(protocol) & OPT_PROTO_UNIQUE_CHATNAME)) {
			alias = purple_contact_info_get_alias(PURPLE_CONTACT_INFO(account));
			if(alias != NULL) {
				new_alias = alias;
			} else {
				const gchar *display_name = purple_connection_get_display_name(gc);
				if(display_name != NULL) {
					new_alias = display_name;
				}
			}
		}
	} else if(!(purple_protocol_get_options(protocol) & OPT_PROTO_UNIQUE_CHATNAME)) {
		PurpleBuddy *buddy;
		if((buddy = purple_blist_find_buddy(purple_connection_get_account(gc), new_user)) != NULL) {
			new_alias = purple_buddy_get_contact_alias(buddy);
		}
	}

	flags = purple_chat_user_get_flags(purple_chat_conversation_find_user(chat, old_user));
	cb = purple_chat_user_new(chat, new_user, new_alias, flags);

	g_hash_table_replace(priv->users,
		g_strdup(purple_chat_user_get_name(cb)), cb);

	if(ops != NULL && ops->chat_rename_user != NULL) {
		ops->chat_rename_user(chat, old_user, new_user, new_alias);
	}

	cb = purple_chat_conversation_find_user(chat, old_user);
	if(cb) {
		g_hash_table_remove(priv->users, purple_chat_user_get_name(cb));
	}

	if(purple_chat_conversation_is_ignored_user(chat, old_user)) {
		purple_chat_conversation_unignore(chat, old_user);
		purple_chat_conversation_ignore(chat, new_user);
	} else if(purple_chat_conversation_is_ignored_user(chat, new_user)) {
		purple_chat_conversation_unignore(chat, new_user);
	}

	if(is_me) {
		purple_chat_conversation_set_nick(chat, new_user);
	}

	if(purple_prefs_get_bool("/purple/conversations/chat/show_nick_change") &&
	   !purple_chat_conversation_is_ignored_user(chat, new_user))
	{
		if(is_me) {
			gchar *escaped = g_markup_escape_text(new_user, -1);
			g_snprintf(tmp, sizeof(tmp), _("You are now known as %s"),
			           escaped);
			g_free(escaped);
		} else {
			const gchar *old_alias = old_user;
			const gchar *new_alias = new_user;
			gchar *escaped;
			gchar *escaped2;

			if(!(purple_protocol_get_options(protocol) & OPT_PROTO_UNIQUE_CHATNAME)) {
				PurpleBuddy *buddy;

				if((buddy = purple_blist_find_buddy(account, old_user)) != NULL) {
					old_alias = purple_buddy_get_contact_alias(buddy);
				}

				if((buddy = purple_blist_find_buddy(account, new_user)) != NULL) {
					new_alias = purple_buddy_get_contact_alias(buddy);
				}
			}

			escaped = g_markup_escape_text(old_alias, -1);
			escaped2 = g_markup_escape_text(new_alias, -1);
			g_snprintf(tmp, sizeof(tmp), _("%s is now known as %s"), escaped,
			           escaped2);
			g_free(escaped);
			g_free(escaped2);
		}

		purple_conversation_write_system_message(conv, tmp,
		                                         PURPLE_MESSAGE_NO_LINKIFY);
	}
}

void
purple_chat_conversation_remove_user(PurpleChatConversation *chat,
                                     const gchar *user, const gchar *reason)
{
	GList *users = g_list_append(NULL, (gchar *)user);

	purple_chat_conversation_remove_users(chat, users, reason);

	g_list_free(users);
}

void
purple_chat_conversation_remove_users(PurpleChatConversation *chat,
                                      GList *users, const gchar *reason)
{
	PurpleConversation *conv;
	PurpleConnection *gc;
	PurpleProtocol *protocol;
	PurpleConversationUiOps *ops;
	PurpleChatUser *cb;
	PurpleChatConversationPrivate *priv;
	GList *l;
	gboolean quiet;
	gpointer handle;

	g_return_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat));
	g_return_if_fail(users != NULL);

	priv = purple_chat_conversation_get_instance_private(chat);
	conv = PURPLE_CONVERSATION(chat);

	gc = purple_conversation_get_connection(conv);
	g_return_if_fail(PURPLE_IS_CONNECTION(gc));

	protocol = purple_connection_get_protocol(gc);
	g_return_if_fail(PURPLE_IS_PROTOCOL(protocol));

	ops = purple_conversation_get_ui_ops(conv);
	handle = purple_conversations_get_handle();

	for(l = users; l != NULL; l = l->next) {
		const gchar *user = (const gchar *)l->data;
		quiet = GPOINTER_TO_INT(purple_signal_emit_return_1(handle,
		                        "chat-user-leaving", chat, user, reason)) |
				purple_chat_conversation_is_ignored_user(chat, user);

		cb = purple_chat_conversation_find_user(chat, user);

		if(cb) {
			g_hash_table_remove(priv->users, purple_chat_user_get_name(cb));
		}

		/* NOTE: Don't remove them from ignored in case they re-enter. */
		if(!quiet) {
			const gchar *alias = user;
			gchar *alias_esc;
			gchar *tmp;

			if(!(purple_protocol_get_options(protocol) & OPT_PROTO_UNIQUE_CHATNAME)) {
				PurpleBuddy *buddy;

				if((buddy = purple_blist_find_buddy(purple_connection_get_account(gc), user)) != NULL) {
					alias = purple_buddy_get_contact_alias(buddy);
				}
			}

			alias_esc = g_markup_escape_text(alias, -1);

			if(reason == NULL || !*reason) {
				tmp = g_strdup_printf(_("%s left the room."), alias_esc);
			} else {
				char *reason_esc = g_markup_escape_text(reason, -1);
				tmp = g_strdup_printf(_("%s left the room (%s)."),
				                      alias_esc, reason_esc);
				g_free(reason_esc);
			}
			g_free(alias_esc);

			purple_conversation_write_system_message(conv, tmp,
			                                         PURPLE_MESSAGE_NO_LINKIFY);
			g_free(tmp);
		}

		purple_signal_emit(handle, "chat-user-left", conv, user, reason);

		g_signal_emit(chat, signals[SIG_USER_LEFT], 0, user, reason);
	}

	if(ops != NULL && ops->chat_remove_users != NULL) {
		ops->chat_remove_users(chat, users);
	}
}

void
purple_chat_conversation_clear_users(PurpleChatConversation *chat) {
	PurpleChatConversationPrivate *priv = NULL;
	PurpleConversationUiOps *ops = NULL;
	GList *names = NULL;

	g_return_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat));

	priv = purple_chat_conversation_get_instance_private(chat);
	ops = purple_conversation_get_ui_ops(PURPLE_CONVERSATION(chat));
	names = g_hash_table_get_keys(priv->users);

	if(ops != NULL && ops->chat_remove_users != NULL) {
		ops->chat_remove_users(chat, names);
	}

	g_list_foreach(names, purple_chat_conversation_clear_users_helper, chat);

	g_list_free(names);
	g_hash_table_remove_all(priv->users);
}

void
purple_chat_conversation_set_nick(PurpleChatConversation *chat,
                                  const gchar *nick)
{
	PurpleChatConversationPrivate *priv = NULL;
	PurpleAccount *account = NULL;

	g_return_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat));

	priv = purple_chat_conversation_get_instance_private(chat);

	account = purple_conversation_get_account(PURPLE_CONVERSATION(chat));

	g_free(priv->nick);
	priv->nick = g_strdup(purple_normalize(account, nick));

	g_object_notify_by_pspec(G_OBJECT(chat), properties[PROP_NICK]);
}

const gchar *
purple_chat_conversation_get_nick(PurpleChatConversation *chat) {
	PurpleChatConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat), NULL);

	priv = purple_chat_conversation_get_instance_private(chat);

	return priv->nick;
}

static void
invite_user_to_chat(gpointer data, PurpleRequestFields *fields) {
	PurpleChatConversation *chat;
	PurpleChatConversationPrivate *priv;
	PurpleConnection *pc;
	const gchar *user, *message;

	chat = PURPLE_CHAT_CONVERSATION(data);
	priv = purple_chat_conversation_get_instance_private(chat);

	user = purple_request_fields_get_string(fields, "screenname");
	message = purple_request_fields_get_string(fields, "message");

	pc = purple_conversation_get_connection(PURPLE_CONVERSATION(chat));
	purple_serv_chat_invite(pc, priv->id, message, user);
}

void
purple_chat_conversation_invite_user(PurpleChatConversation *chat,
                                     const gchar *user, const gchar *message,
                                     gboolean confirm)
{
	PurpleAccount *account;
	PurpleRequestFields *fields;
	PurpleRequestFieldGroup *group;
	PurpleRequestField *field;

	g_return_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat));

	if(user == NULL || *user == '\0' || message == NULL || *message == '\0') {
		confirm = TRUE;
	}

	account = purple_conversation_get_account(PURPLE_CONVERSATION(chat));

	if(!confirm) {
		purple_serv_chat_invite(purple_account_get_connection(account),
		                        purple_chat_conversation_get_id(chat), message,
		                        user);
		return;
	}

	fields = purple_request_fields_new();
	group = purple_request_field_group_new(_("Invite to chat"));
	purple_request_fields_add_group(fields, group);

	field = purple_request_field_string_new("screenname", _("Buddy"), user,
	                                        FALSE);
	purple_request_field_group_add_field(group, field);
	purple_request_field_set_required(field, TRUE);
	purple_request_field_set_type_hint(field, "screenname");

	field = purple_request_field_string_new("message", _("Message"), message,
	                                        FALSE);
	purple_request_field_group_add_field(group, field);

	purple_request_fields(chat, _("Invite to chat"), NULL,
	                      _("Please enter the name of the user you wish to "
	                        "invite, along with an optional invite message."),
	                      fields,
	                      _("Invite"), G_CALLBACK(invite_user_to_chat),
	                      _("Cancel"), NULL,
	                      purple_request_cpar_from_conversation(PURPLE_CONVERSATION(chat)),
	                      chat);
}

gboolean
purple_chat_conversation_has_user(PurpleChatConversation *chat,
                                  const gchar *user)
{
	g_return_val_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat), FALSE);
	g_return_val_if_fail(user != NULL, FALSE);

	return (purple_chat_conversation_find_user(chat, user) != NULL);
}

void
purple_chat_conversation_leave(PurpleChatConversation *chat) {
	PurpleChatConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat));

	priv = purple_chat_conversation_get_instance_private(chat);
	priv->left = TRUE;

	if(!g_object_get_data(G_OBJECT(chat), "is-finalizing")) {
		g_object_notify_by_pspec(G_OBJECT(chat), properties[PROP_LEFT]);
	}

	purple_conversation_update(PURPLE_CONVERSATION(chat),
	                           PURPLE_CONVERSATION_UPDATE_CHATLEFT);
}

gboolean
purple_chat_conversation_has_left(PurpleChatConversation *chat) {
	PurpleChatConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat), TRUE);

	priv = purple_chat_conversation_get_instance_private(chat);

	return priv->left;
}

PurpleChatUser *
purple_chat_conversation_find_user(PurpleChatConversation *chat,
                                   const gchar *name)
{
	PurpleChatConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat), NULL);
	g_return_val_if_fail(name != NULL, NULL);

	priv = purple_chat_conversation_get_instance_private(chat);

	return g_hash_table_lookup(priv->users, name);
}
