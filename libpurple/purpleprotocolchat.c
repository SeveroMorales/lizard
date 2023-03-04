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

#include "purpleprotocolchat.h"

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_INTERFACE(PurpleProtocolChat, purple_protocol_chat,
                   PURPLE_TYPE_PROTOCOL)

static void
purple_protocol_chat_default_init(G_GNUC_UNUSED PurpleProtocolChatInterface *iface)
{
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GList *
purple_protocol_chat_info(PurpleProtocolChat *protocol_chat,
                          PurpleConnection *connection)
{
	PurpleProtocolChatInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CHAT(protocol_chat), NULL);
	g_return_val_if_fail(PURPLE_IS_CONNECTION(connection), NULL);

	iface = PURPLE_PROTOCOL_CHAT_GET_IFACE(protocol_chat);
	if(iface != NULL && iface->info != NULL) {
		return iface->info(protocol_chat, connection);
	}

	return NULL;
}

GHashTable *
purple_protocol_chat_info_defaults(PurpleProtocolChat *protocol_chat,
                                   PurpleConnection *connection,
                                   const gchar *chat_name)
{
	PurpleProtocolChatInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CHAT(protocol_chat), NULL);
	g_return_val_if_fail(PURPLE_IS_CONNECTION(connection), NULL);

	iface = PURPLE_PROTOCOL_CHAT_GET_IFACE(protocol_chat);
	if(iface != NULL && iface->info_defaults != NULL) {
		return iface->info_defaults(protocol_chat, connection, chat_name);
	}

	return NULL;
}

void
purple_protocol_chat_join(PurpleProtocolChat *protocol_chat,
                          PurpleConnection *connection,
                          GHashTable *components)
{
	PurpleProtocolChatInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_CHAT(protocol_chat));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));
	g_return_if_fail(components != NULL);

	iface = PURPLE_PROTOCOL_CHAT_GET_IFACE(protocol_chat);
	if(iface != NULL && iface->join != NULL) {
		iface->join(protocol_chat, connection, components);
	}
}

void
purple_protocol_chat_reject(PurpleProtocolChat *protocol_chat,
                            PurpleConnection *connection,
                            GHashTable *components)
{
	PurpleProtocolChatInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_CHAT(protocol_chat));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));
	g_return_if_fail(components != NULL);

	iface = PURPLE_PROTOCOL_CHAT_GET_IFACE(protocol_chat);
	if(iface != NULL && iface->reject != NULL) {
		iface->reject(protocol_chat, connection, components);
	}
}

gchar *
purple_protocol_chat_get_name(PurpleProtocolChat *protocol_chat,
                              GHashTable *components)
{
	PurpleProtocolChatInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CHAT(protocol_chat), NULL);
	g_return_val_if_fail(components != NULL, NULL);

	iface = PURPLE_PROTOCOL_CHAT_GET_IFACE(protocol_chat);
	if(iface != NULL && iface->get_name != NULL) {
		return iface->get_name(protocol_chat, components);
	}

	return NULL;
}

void
purple_protocol_chat_invite(PurpleProtocolChat *protocol_chat,
                            PurpleConnection *connection, gint id,
                            const gchar *message, const gchar *who)
{
	PurpleProtocolChatInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_CHAT(protocol_chat));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));
	g_return_if_fail(who != NULL);

	iface = PURPLE_PROTOCOL_CHAT_GET_IFACE(protocol_chat);
	if(iface != NULL && iface->invite != NULL) {
		iface->invite(protocol_chat, connection, id, message, who);
	}
}

void
purple_protocol_chat_leave(PurpleProtocolChat *protocol_chat,
                           PurpleConnection *connection, gint id)
{
	PurpleProtocolChatInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_CHAT(protocol_chat));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));

	iface = PURPLE_PROTOCOL_CHAT_GET_IFACE(protocol_chat);
	if(iface != NULL && iface->leave != NULL) {
		iface->leave(protocol_chat, connection, id);
	}
}

gint
purple_protocol_chat_send(PurpleProtocolChat *protocol_chat,
                          PurpleConnection *connection, gint id,
                          PurpleMessage *message)
{
	PurpleProtocolChatInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CHAT(protocol_chat), -1);
	g_return_val_if_fail(PURPLE_IS_CONNECTION(connection), -1);
	g_return_val_if_fail(PURPLE_IS_MESSAGE(message), -1);

	iface = PURPLE_PROTOCOL_CHAT_GET_IFACE(protocol_chat);
	if(iface != NULL && iface->send != NULL) {
		return iface->send(protocol_chat, connection, id, message);
	}

	return -1;
}

gchar *
purple_protocol_chat_get_user_real_name(PurpleProtocolChat *protocol_chat,
                                        PurpleConnection *connection, gint id,
                                        const gchar *who)
{
	PurpleProtocolChatInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CHAT(protocol_chat), NULL);
	g_return_val_if_fail(PURPLE_IS_CONNECTION(connection), NULL);
	g_return_val_if_fail(who != NULL, NULL);

	iface = PURPLE_PROTOCOL_CHAT_GET_IFACE(protocol_chat);
	if(iface != NULL && iface->get_user_real_name != NULL) {
		return iface->get_user_real_name(protocol_chat, connection, id, who);
	}

	return NULL;
}

void
purple_protocol_chat_set_topic(PurpleProtocolChat *protocol_chat,
                               PurpleConnection *connection, gint id,
                               const gchar *topic)
{
	PurpleProtocolChatInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_CHAT(protocol_chat));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));

	iface = PURPLE_PROTOCOL_CHAT_GET_IFACE(protocol_chat);
	if(iface != NULL && iface->set_topic != NULL) {
		iface->set_topic(protocol_chat, connection, id, topic);
	}
}
