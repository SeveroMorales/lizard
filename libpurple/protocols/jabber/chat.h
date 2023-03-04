/**
 * @file chat.h Chat stuff
 *
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

#ifndef PURPLE_JABBER_CHAT_H
#define PURPLE_JABBER_CHAT_H

#include <purple.h>

#include "jabber.h"

typedef struct {
	char *handle;
	char *jid;
} JabberChatMember;


typedef struct {
	JabberStream *js;
	char *room;
	char *server;
	char *handle;
	GHashTable *components;
	int id;
	PurpleChatConversation *conv;
	gboolean muc;
	gboolean xhtml;
	PurpleRequestType config_dialog_type;
	void *config_dialog_handle;
	GHashTable *members;
	gboolean left;
	GDateTime *joined;
} JabberChat;

GList *jabber_chat_info(PurpleProtocolChat *protocol_chat, PurpleConnection *connection);
GHashTable *jabber_chat_info_defaults(PurpleConnection *gc, const char *chat_name);
char *jabber_get_chat_name(PurpleProtocolChat *protocol_chat, GHashTable *data);

void jabber_chat_join(PurpleConnection *gc, GHashTable *data);
JabberChat *jabber_chat_find(JabberStream *js, const char *room,
		const char *server);
JabberChat *jabber_chat_find_by_id(JabberStream *js, int id);
JabberChat *jabber_chat_find_by_conv(PurpleChatConversation *conv);
void jabber_chat_destroy(JabberChat *chat);
void jabber_chat_free(JabberChat *chat);
gboolean jabber_chat_find_buddy(PurpleChatConversation *conv, const char *name);
void jabber_chat_invite(PurpleConnection *gc, int id, const char *message,
		const char *name);
void jabber_chat_leave(PurpleProtocolChat *protocol_chat, PurpleConnection *gc, int id);
char *jabber_chat_user_real_name(PurpleProtocolChat *protocol_chat, PurpleConnection *gc, int id, const char *who);
void jabber_chat_request_room_configure(JabberChat *chat);
void jabber_chat_create_instant_room(JabberChat *chat);
void jabber_chat_register(JabberChat *chat);
void jabber_chat_change_topic(JabberChat *chat, const char *topic);
void jabber_chat_set_topic(PurpleProtocolChat *protocol_chat, PurpleConnection *gc, int id, const char *topic);
gboolean jabber_chat_change_nick(JabberChat *chat, const char *nick);
void jabber_chat_part(JabberChat *chat, const char *msg);
void jabber_chat_track_handle(JabberChat *chat, const char *handle,
		const char *jid, const char *affiliation, const char *role);
void jabber_chat_remove_handle(JabberChat *chat, const char *handle);
gboolean jabber_chat_ban_user(JabberChat *chat, const char *who,
		const char *why);
gboolean jabber_chat_affiliate_user(JabberChat *chat, const char *who,
		const char *affiliation);
gboolean jabber_chat_affiliation_list(JabberChat *chat, const char *affiliation);
gboolean jabber_chat_role_user(JabberChat *chat, const char *who,
		const char *role, const char *why);
gboolean jabber_chat_role_list(JabberChat *chat, const char *role);

PurpleRoomlist *jabber_roomlist_get_list(PurpleProtocolRoomlist *protocol_roomlist, PurpleConnection *gc);
void jabber_roomlist_cancel(PurpleProtocolRoomlist *protocol_roomlist, PurpleRoomlist *list);
char *jabber_roomlist_room_serialize(PurpleProtocolRoomlist *protocol_roomlist, PurpleRoomlistRoom *room);

void jabber_chat_disco_traffic(JabberChat *chat);

#endif /* PURPLE_JABBER_CHAT_H */
