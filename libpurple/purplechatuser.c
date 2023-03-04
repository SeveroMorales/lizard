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

#include "purplechatuser.h"

#include "conversations.h"
#include "purpleenums.h"

struct _PurpleChatUser {
	GObject parent;

	PurpleChatConversation *chat;  /* The chat                              */
	gchar *name;                   /* The chat participant's name in the
	                                  chat.                                 */
	gchar *alias;                  /* The chat participant's alias, if known;
	                                  NULL otherwise.                       */
	gboolean buddy;                /* TRUE if this chat participant is on
	                                  the buddy list; FALSE otherwise.      */
	PurpleChatUserFlags flags;     /* A bitwise OR of flags for this
	                                  participant, such as whether they
	                                  are a channel operator.               */

	gboolean constructed;
};

enum {
	PROP_0,
	PROP_CHAT,
	PROP_NAME,
	PROP_ALIAS,
	PROP_FLAGS,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/**************************************************************************
 * Private Setters
 **************************************************************************/
static void
purple_chat_user_set_name(PurpleChatUser *chat_user, const gchar *name) {
	g_return_if_fail(PURPLE_IS_CHAT_USER(chat_user));

	g_free(chat_user->name);
	chat_user->name = g_strdup(name);

	g_object_notify_by_pspec(G_OBJECT(chat_user), properties[PROP_NAME]);
}

static void
purple_chat_user_set_alias(PurpleChatUser *chat_user, const gchar *alias) {
	g_return_if_fail(PURPLE_IS_CHAT_USER(chat_user));

	g_free(chat_user->alias);
	chat_user->alias = g_strdup(alias);

	g_object_notify_by_pspec(G_OBJECT(chat_user), properties[PROP_ALIAS]);
}

/**************************************************************************
 * GObject Implementation
 **************************************************************************/
G_DEFINE_TYPE(PurpleChatUser, purple_chat_user, G_TYPE_OBJECT);

static void
purple_chat_user_set_property(GObject *obj, guint param_id, const GValue *value,
                              GParamSpec *pspec)
{
	PurpleChatUser *chat_user = PURPLE_CHAT_USER(obj);

	switch (param_id) {
		case PROP_CHAT:
			purple_chat_user_set_chat(chat_user, g_value_get_object(value));
			break;
		case PROP_NAME:
			purple_chat_user_set_name(chat_user, g_value_get_string(value));
			break;
		case PROP_ALIAS:
			purple_chat_user_set_alias(chat_user, g_value_get_string(value));
			break;
		case PROP_FLAGS:
			purple_chat_user_set_flags(chat_user, g_value_get_flags(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_chat_user_get_property(GObject *obj, guint param_id, GValue *value,
                              GParamSpec *pspec)
{
	PurpleChatUser *chat_user = PURPLE_CHAT_USER(obj);

	switch (param_id) {
		case PROP_CHAT:
			g_value_set_object(value, purple_chat_user_get_chat(chat_user));
			break;
		case PROP_NAME:
			g_value_set_string(value, purple_chat_user_get_name(chat_user));
			break;
		case PROP_ALIAS:
			g_value_set_string(value, purple_chat_user_get_alias(chat_user));
			break;
		case PROP_FLAGS:
			g_value_set_flags(value, purple_chat_user_get_flags(chat_user));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_chat_user_init(PurpleChatUser *user) {
	user->constructed = FALSE;
}

static void
purple_chat_user_constructed(GObject *object) {
	PurpleChatUser *chat_user = NULL;
	PurpleAccount *account = NULL;

	G_OBJECT_CLASS(purple_chat_user_parent_class)->constructed(object);

	chat_user = PURPLE_CHAT_USER(object);
	account = purple_conversation_get_account(PURPLE_CONVERSATION(chat_user->chat));

	if(purple_blist_find_buddy(account, chat_user->name) != NULL) {
		chat_user->buddy = TRUE;
	}

	chat_user->constructed = TRUE;
}

static void
purple_chat_user_finalize(GObject *object) {
	PurpleChatUser *chat_user = PURPLE_CHAT_USER(object);

	g_free(chat_user->alias);
	g_free(chat_user->name);

	G_OBJECT_CLASS(purple_chat_user_parent_class)->finalize(object);
}

static void
purple_chat_user_class_init(PurpleChatUserClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->constructed = purple_chat_user_constructed;
	obj_class->get_property = purple_chat_user_get_property;
	obj_class->set_property = purple_chat_user_set_property;
	obj_class->finalize = purple_chat_user_finalize;

	properties[PROP_CHAT] = g_param_spec_object(
		"chat", "Chat",
		"The chat the buddy belongs to.",
		PURPLE_TYPE_CHAT_CONVERSATION,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	properties[PROP_NAME] = g_param_spec_string(
		"name", "Name",
		"Name of the chat user.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	properties[PROP_ALIAS] = g_param_spec_string(
		"alias", "Alias",
		"Alias of the chat user.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	properties[PROP_FLAGS] = g_param_spec_flags(
		"flags", "Buddy flags",
		"The flags for the chat user.",
		PURPLE_TYPE_CHAT_USER_FLAGS,
		PURPLE_CHAT_USER_NONE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleChatUser *
purple_chat_user_new(PurpleChatConversation *chat, const gchar *name,
                     const gchar *alias, PurpleChatUserFlags flags)
{
	PurpleChatUser *chat_user = NULL;

	g_return_val_if_fail(PURPLE_IS_CHAT_CONVERSATION(chat), NULL);
	g_return_val_if_fail(name != NULL, NULL);

	chat_user = g_object_new(
		PURPLE_TYPE_CHAT_USER,
		"chat", chat,
		"name", name,
		"alias", alias,
		"flags", flags,
		NULL);

	return chat_user;
}

const gchar *
purple_chat_user_get_alias(PurpleChatUser *chat_user) {
	g_return_val_if_fail(PURPLE_IS_CHAT_USER(chat_user), NULL);

	return chat_user->alias;
}

const gchar *
purple_chat_user_get_name(PurpleChatUser *chat_user) {
	g_return_val_if_fail(PURPLE_IS_CHAT_USER(chat_user), NULL);

	return chat_user->name;
}

void
purple_chat_user_set_flags(PurpleChatUser *chat_user,
                           PurpleChatUserFlags flags)
{
	PurpleConversationUiOps *ops;
	PurpleChatUserFlags oldflags;

	g_return_if_fail(PURPLE_IS_CHAT_USER(chat_user));

	if(flags == chat_user->flags) {
		return;
	}

	oldflags = chat_user->flags;
	chat_user->flags = flags;

	g_object_notify_by_pspec(G_OBJECT(chat_user), properties[PROP_FLAGS]);

	/* Only update the UI once the object is fully constructed.  This avoids an
	 * issue where at least with XMPP, user names will be duplicated in the
	 * chat user list.
	 */
	if(chat_user->constructed) {
		ops = purple_conversation_get_ui_ops(PURPLE_CONVERSATION(chat_user->chat));
		if(ops != NULL && ops->chat_update_user != NULL) {
			ops->chat_update_user(chat_user);
		}
	}

	purple_signal_emit(purple_conversations_get_handle(),
	                   "chat-user-flags", chat_user, oldflags, flags);
}

PurpleChatUserFlags
purple_chat_user_get_flags(PurpleChatUser *chat_user) {
	g_return_val_if_fail(PURPLE_IS_CHAT_USER(chat_user), PURPLE_CHAT_USER_NONE);

	return chat_user->flags;
}

void
purple_chat_user_set_chat(PurpleChatUser *chat_user,
                          PurpleChatConversation *chat)
{
	g_return_if_fail(PURPLE_IS_CHAT_USER(chat_user));

	if(g_set_object(&chat_user->chat, chat)) {
		g_object_notify_by_pspec(G_OBJECT(chat_user), properties[PROP_CHAT]);
	}
}

PurpleChatConversation *
purple_chat_user_get_chat(PurpleChatUser *chat_user) {
	g_return_val_if_fail(PURPLE_IS_CHAT_USER(chat_user), NULL);

	return chat_user->chat;
}

gboolean
purple_chat_user_is_buddy(PurpleChatUser *chat_user) {
	g_return_val_if_fail(PURPLE_IS_CHAT_USER(chat_user), FALSE);

	return chat_user->buddy;
}

gint
purple_chat_user_compare(PurpleChatUser *a, PurpleChatUser *b) {
	gchar *namea = NULL, *nameb = NULL;

	/* NULL is equal to NULL */
	if(a == NULL && b == NULL) {
		return 0;
	}

	/* non-NULL sorts before NULL */
	if(a == NULL && b != NULL) {
		return 1;
	} else if(a != NULL && b == NULL) {
		return -1;
	}

	/* higher valued flags sort before lower values */
	if(a->flags > b->flags) {
		return -1;
	} else if(a->flags < b->flags) {
		return 1;
	}

	/* buddies sort before non-buddies */
	if(a->buddy != b->buddy) {
		return a->buddy ? -1 : 1;
	}

	/* figure out what name we need to check for user a */
	if(a->alias) {
		namea = a->alias;
	} else if (a->name) {
		namea = a->name;
	}

	/* figure out what name we need to check for user b */
	if(b->alias) {
		nameb = b->alias;
	} else if(b->name) {
		nameb = b->name;
	}

	/* finally we're just sorting names */
	return purple_utf8_strcasecmp(namea, nameb);
}
