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

#include "purpleprotocolclient.h"

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_INTERFACE(PurpleProtocolClient, purple_protocol_client,
                   PURPLE_TYPE_PROTOCOL)

static void
purple_protocol_client_default_init(G_GNUC_UNUSED PurpleProtocolClientInterface *iface)
{
}

/******************************************************************************
 * Public API
 *****************************************************************************/
const gchar *
purple_protocol_client_list_emblem(PurpleProtocolClient *client,
                                   PurpleBuddy *buddy)
{
	PurpleProtocolClientInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CLIENT(client), NULL);
	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), NULL);

	iface = PURPLE_PROTOCOL_CLIENT_GET_IFACE(client);
	if(iface != NULL && iface->list_emblem != NULL) {
		return iface->list_emblem(client, buddy);
	}

	return NULL;
}

gchar *
purple_protocol_client_status_text(PurpleProtocolClient *client,
                                   PurpleBuddy *buddy)
{
	PurpleProtocolClientInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CLIENT(client), NULL);
	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), NULL);

	iface = PURPLE_PROTOCOL_CLIENT_GET_IFACE(client);
	if(iface != NULL && iface->status_text != NULL) {
		return iface->status_text(client, buddy);
	}

	return NULL;
}

void
purple_protocol_client_tooltip_text(PurpleProtocolClient *client,
                                    PurpleBuddy *buddy,
                                    PurpleNotifyUserInfo *user_info,
                                    gboolean full)
{
	PurpleProtocolClientInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_CLIENT(client));
	g_return_if_fail(PURPLE_IS_BUDDY(buddy));
	g_return_if_fail(user_info != NULL);

	iface = PURPLE_PROTOCOL_CLIENT_GET_IFACE(client);
	if(iface != NULL && iface->tooltip_text != NULL) {
		iface->tooltip_text(client, buddy, user_info, full);
	}
}

GList *
purple_protocol_client_blist_node_menu(PurpleProtocolClient *client,
                                       PurpleBlistNode *node)
{
	PurpleProtocolClientInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CLIENT(client), NULL);
	g_return_val_if_fail(PURPLE_IS_BLIST_NODE(node), NULL);

	iface = PURPLE_PROTOCOL_CLIENT_GET_IFACE(client);
	if(iface != NULL && iface->blist_node_menu != NULL) {
		return iface->blist_node_menu(client, node);
	}

	return NULL;
}

void
purple_protocol_client_buddy_free(PurpleProtocolClient *client,
                                  PurpleBuddy *buddy)
{
	PurpleProtocolClientInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_CLIENT(client));
	g_return_if_fail(PURPLE_IS_BUDDY(buddy));

	iface = PURPLE_PROTOCOL_CLIENT_GET_IFACE(client);
	if(iface != NULL && iface->buddy_free != NULL) {
		iface->buddy_free(client, buddy);
	}
}

void
purple_protocol_client_convo_closed(PurpleProtocolClient *client,
                                    PurpleConnection *connection,
                                    const gchar *who)
{
	PurpleProtocolClientInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_CLIENT(client));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));
	g_return_if_fail(who != NULL);

	iface = PURPLE_PROTOCOL_CLIENT_GET_IFACE(client);
	if(iface != NULL && iface->convo_closed != NULL) {
		iface->convo_closed(client, connection, who);
	}
}

const gchar *
purple_protocol_client_normalize(PurpleProtocolClient *client,
                                 PurpleAccount *account, const gchar *who)
{
	PurpleProtocolClientInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CLIENT(client), NULL);
	g_return_val_if_fail(who != NULL, NULL);

	iface = PURPLE_PROTOCOL_CLIENT_GET_IFACE(client);
	if(iface != NULL && iface->normalize != NULL) {
		return iface->normalize(client, account, who);
	}

	return NULL;
}

PurpleChat *
purple_protocol_client_find_blist_chat(PurpleProtocolClient *client,
                                       PurpleAccount *account,
                                       const gchar *name)
{
	PurpleProtocolClientInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CLIENT(client), NULL);
	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);
	g_return_val_if_fail(name != NULL, NULL);

	iface = PURPLE_PROTOCOL_CLIENT_GET_IFACE(client);
	if(iface != NULL && iface->find_blist_chat != NULL) {
		return iface->find_blist_chat(client, account, name);
	}

	return NULL;
}

gboolean
purple_protocol_client_offline_message(PurpleProtocolClient *client,
                                       PurpleBuddy *buddy)
{
	PurpleProtocolClientInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CLIENT(client), FALSE);
	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), FALSE);

	iface = PURPLE_PROTOCOL_CLIENT_GET_IFACE(client);
	if(iface != NULL && iface->offline_message != NULL) {
		return iface->offline_message(client, buddy);
	}

	return FALSE;
}

GHashTable *
purple_protocol_client_get_account_text_table(PurpleProtocolClient *client,
                                              PurpleAccount *account)
{
	PurpleProtocolClientInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CLIENT(client), NULL);

	iface = PURPLE_PROTOCOL_CLIENT_GET_IFACE(client);
	if(iface != NULL && iface->get_account_text_table != NULL) {
		return iface->get_account_text_table(client, account);
	}

	return NULL;
}

PurpleMood *
purple_protocol_client_get_moods(PurpleProtocolClient *client,
                                 PurpleAccount *account)
{
	PurpleProtocolClientInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CLIENT(client), NULL);
	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);

	iface = PURPLE_PROTOCOL_CLIENT_GET_IFACE(client);
	if(iface != NULL && iface->get_moods != NULL) {
		return iface->get_moods(client, account);
	}

	return NULL;
}

gssize
purple_protocol_client_get_max_message_size(PurpleProtocolClient *client,
                                            PurpleConversation *conv)
{
	PurpleProtocolClientInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CLIENT(client), 0);
	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conv), 0);

	iface = PURPLE_PROTOCOL_CLIENT_GET_IFACE(client);
	if(iface != NULL && iface->get_max_message_size != NULL) {
		return iface->get_max_message_size(client, conv);
	}

	return 0;
}
