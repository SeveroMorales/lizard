/*
 * purple - Jabber Protocol Plugin
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
 *
 */
#include <glib/gi18n-lib.h>

#include <purple.h>

#include "chat.h"
#include "iq.h"
#include "message.h"
#include "presence.h"
#include "xdata.h"
#include "data.h"

GList *
jabber_chat_info(G_GNUC_UNUSED PurpleProtocolChat *protocol_chat,
                 G_GNUC_UNUSED PurpleConnection *connection)
{
	GList *m = NULL;
	PurpleProtocolChatEntry *pce;

	pce = g_new0(PurpleProtocolChatEntry, 1);
	pce->label = _("_Room");
	pce->identifier = "room";
	pce->required = TRUE;
	m = g_list_append(m, pce);

	pce = g_new0(PurpleProtocolChatEntry, 1);
	pce->label = _("_Server");
	pce->identifier = "server";
	pce->required = TRUE;
	m = g_list_append(m, pce);

	pce = g_new0(PurpleProtocolChatEntry, 1);
	pce->label = _("_Handle");
	pce->identifier = "handle";
	pce->required = TRUE;
	m = g_list_append(m, pce);

	pce = g_new0(PurpleProtocolChatEntry, 1);
	pce->label = _("_Password");
	pce->identifier = "password";
	pce->secret = TRUE;
	m = g_list_append(m, pce);

	return m;
}

GHashTable *jabber_chat_info_defaults(PurpleConnection *gc, const char *chat_name)
{
	GHashTable *defaults;
	JabberStream *js = purple_connection_get_protocol_data(gc);

	defaults = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

	g_hash_table_insert(defaults, "handle", g_strdup(js->user->node));

	if (js->chat_servers)
		g_hash_table_insert(defaults, "server", g_strdup(js->chat_servers->data));

	if (chat_name != NULL) {
		JabberID *jid = jabber_id_new(chat_name);
		if(jid) {
			g_hash_table_insert(defaults, "room", g_strdup(jid->node));
			if(jid->domain)
				g_hash_table_replace(defaults, "server", g_strdup(jid->domain));
			if(jid->resource)
				g_hash_table_replace(defaults, "handle", g_strdup(jid->resource));
			jabber_id_free(jid);
		}
	}

	return defaults;
}

JabberChat *jabber_chat_find(JabberStream *js, const char *room,
		const char *server)
{
	JabberChat *chat = NULL;

	g_return_val_if_fail(room != NULL, NULL);
	g_return_val_if_fail(server != NULL, NULL);

	if(NULL != js->chats)
	{
		char *room_jid = g_strdup_printf("%s@%s", room, server);

		chat = g_hash_table_lookup(js->chats, room_jid);
		g_free(room_jid);
	}

	return chat;
}

static gboolean
find_by_id_cb(G_GNUC_UNUSED gpointer key, gpointer value, gpointer user_data)
{
	JabberChat *chat = value;

	return chat->id == GPOINTER_TO_INT(user_data);
}

JabberChat *jabber_chat_find_by_id(JabberStream *js, int id)
{
	return g_hash_table_find(js->chats, find_by_id_cb, GINT_TO_POINTER(id));
}

JabberChat *jabber_chat_find_by_conv(PurpleChatConversation *conv)
{
	PurpleAccount *account = purple_conversation_get_account(PURPLE_CONVERSATION(conv));
	PurpleConnection *gc = purple_account_get_connection(account);
	JabberStream *js;
	int id;
	if (!gc)
		return NULL;
	js = purple_connection_get_protocol_data(gc);
	id = purple_chat_conversation_get_id(conv);
	return jabber_chat_find_by_id(js, id);
}

void jabber_chat_invite(PurpleConnection *gc, int id, const char *msg,
		const char *name)
{
	JabberStream *js = purple_connection_get_protocol_data(gc);
	JabberChat *chat;
	PurpleXmlNode *message, *body, *x, *invite;
	char *room_jid;

	chat = jabber_chat_find_by_id(js, id);
	if(!chat)
		return;

	message = purple_xmlnode_new("message");

	room_jid = g_strdup_printf("%s@%s", chat->room, chat->server);

	if(chat->muc) {
		purple_xmlnode_set_attrib(message, "to", room_jid);
		x = purple_xmlnode_new_child(message, "x");
		purple_xmlnode_set_namespace(x, "http://jabber.org/protocol/muc#user");
		invite = purple_xmlnode_new_child(x, "invite");
		purple_xmlnode_set_attrib(invite, "to", name);
		if (msg) {
			body = purple_xmlnode_new_child(invite, "reason");
			purple_xmlnode_insert_data(body, msg, -1);
		}
	} else {
		purple_xmlnode_set_attrib(message, "to", name);
		/*
		 * Putting the reason into the body was an 'undocumented protocol,
		 * ...not part of "groupchat 1.0"'.
		 * http://xmpp.org/extensions/attic/jep-0045-1.16.html#invite
		 *
		 * Left here for compatibility.
		 */
		if (msg) {
			body = purple_xmlnode_new_child(message, "body");
			purple_xmlnode_insert_data(body, msg, -1);
		}

		x = purple_xmlnode_new_child(message, "x");
		purple_xmlnode_set_attrib(x, "jid", room_jid);

		/* The better place for it! XEP-0249 style. */
		if (msg)
			purple_xmlnode_set_attrib(x, "reason", msg);
		purple_xmlnode_set_namespace(x, "jabber:x:conference");
	}

	jabber_send(js, message);
	purple_xmlnode_free(message);
	g_free(room_jid);
}

void jabber_chat_member_free(JabberChatMember *jcm);

gchar *
jabber_get_chat_name(G_GNUC_UNUSED PurpleProtocolChat *protocol_chat,
                     GHashTable *data)
{
	char *room, *server, *chat_name = NULL;

	room = g_hash_table_lookup(data, "room");
	server = g_hash_table_lookup(data, "server");

	if (room && server) {
		chat_name = g_strdup_printf("%s@%s", room, server);
	}
	return chat_name;
}

static void insert_in_hash_table(gpointer key, gpointer value, gpointer user_data)
{
	GHashTable *hash_table = (GHashTable *)user_data;
	g_hash_table_insert(hash_table, g_strdup(key), g_strdup(value));
}

static JabberChat *
jabber_chat_new(JabberStream *js, const char *room, const char *server,
                const char *handle, G_GNUC_UNUSED const char *password,
                GHashTable *data)
{
	JabberChat *chat;
	char *jid;

	if (jabber_chat_find(js, room, server) != NULL)
		return NULL;

	chat = g_new0(JabberChat, 1);
	chat->js = js;
	chat->joined = 0;

	chat->room = g_strdup(room);
	chat->server = g_strdup(server);
	chat->handle = g_strdup(handle);

	/* Copy the data hash table to chat->components */
	chat->components = g_hash_table_new_full(g_str_hash, g_str_equal,
			g_free, g_free);
	if (data == NULL) {
		g_hash_table_insert(chat->components, g_strdup("handle"), g_strdup(handle));
		g_hash_table_insert(chat->components, g_strdup("room"), g_strdup(room));
		g_hash_table_insert(chat->components, g_strdup("server"), g_strdup(server));
		/* g_hash_table_insert(chat->components, g_strdup("password"), g_strdup(server)); */
	} else {
		g_hash_table_foreach(data, insert_in_hash_table, chat->components);
	}

	chat->members = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
			(GDestroyNotify)jabber_chat_member_free);

	jid = g_strdup_printf("%s@%s", room, server);
	g_hash_table_insert(js->chats, jid, chat);

	return chat;
}

/*
 * jabber_join_chat:
 * @room: The room to join. This MUST be normalized already.
 * @server: The server the room is on. This MUST be normalized already.
 * @password: (nullable): The password (if required) to join the room.
 * @data: (nullable): The chat hash table. If NULL, it will be generated for
 *        current core<>protocol API interface.
 *
 * In-protocol function for joining a chat room. Doesn't require sticking goop
 * into a hash table.
 */
static JabberChat *
jabber_join_chat(JabberStream *js, const char *room, const char *server,
                 const char *handle, const char *password, GHashTable *data)
{
	JabberChat *chat;

	PurpleConnection *gc;
	PurpleAccount *account;
	PurpleStatus *status;

	PurpleXmlNode *presence, *x;
	JabberBuddyState state;
	char *msg;
	int priority;

	char *jid;

	chat = jabber_chat_new(js, room, server, handle, password, data);
	if (chat == NULL)
		return NULL;

	gc = js->gc;
	account = purple_connection_get_account(gc);
	status = purple_account_get_active_status(account);
	purple_status_to_jabber(status, &state, &msg, &priority);

	presence = jabber_presence_create_js(js, state, msg, priority);
	g_free(msg);

	jid = g_strdup_printf("%s@%s/%s", room, server, handle);
	purple_xmlnode_set_attrib(presence, "to", jid);
	g_free(jid);

	x = purple_xmlnode_new_child(presence, "x");
	purple_xmlnode_set_namespace(x, "http://jabber.org/protocol/muc");

	if (password && *password) {
		PurpleXmlNode *p = purple_xmlnode_new_child(x, "password");
		purple_xmlnode_insert_data(p, password, -1);
	}

	jabber_send(js, presence);
	purple_xmlnode_free(presence);

	return chat;
}

void jabber_chat_join(PurpleConnection *gc, GHashTable *data)
{
	char *room, *server, *handle, *passwd;
	JabberID *jid;
	JabberStream *js = purple_connection_get_protocol_data(gc);
	char *tmp;

	room = g_hash_table_lookup(data, "room");
	server = g_hash_table_lookup(data, "server");
	handle = g_hash_table_lookup(data, "handle");
	passwd = g_hash_table_lookup(data, "password");

	if(!room || !server)
		return;

	if(!handle)
		handle = js->user->node;

	if(!jabber_nodeprep_validate(room)) {
		char *buf = g_strdup_printf(_("%s is not a valid room name"), room);
		purple_notify_error(gc, _("Invalid Room Name"), _("Invalid Room Name"),
			buf, purple_request_cpar_from_connection(gc));
		purple_serv_got_join_chat_failed(gc, data);
		g_free(buf);
		return;
	} else if(!jabber_domain_validate(server)) {
		char *buf = g_strdup_printf(_("%s is not a valid server name"), server);
		purple_notify_error(gc, _("Invalid Server Name"),
			_("Invalid Server Name"), buf,
			purple_request_cpar_from_connection(gc));
		purple_serv_got_join_chat_failed(gc, data);
		g_free(buf);
		return;
	} else if(!jabber_resourceprep_validate(handle)) {
		char *buf = g_strdup_printf(_("%s is not a valid room handle"), handle);
		purple_notify_error(gc, _("Invalid Room Handle"),
			_("Invalid Room Handle"), buf,
			purple_request_cpar_from_connection(gc));
		purple_serv_got_join_chat_failed(gc, data);
		g_free(buf);
		return;
	}

	/* Normalize the room and server parameters */
	tmp = g_strdup_printf("%s@%s", room, server);
	jid = jabber_id_new(tmp);
	g_free(tmp);

	if (jid == NULL) {
		/* TODO: Error message */

		g_return_if_reached();
	}

	/*
	 * Now that we've done all that nice core-interface stuff, let's join
	 * this room!
	 */
	jabber_join_chat(js, jid->node, jid->domain, handle, passwd, data);
	jabber_id_free(jid);
}

void
jabber_chat_leave(G_GNUC_UNUSED PurpleProtocolChat *protocol_chat,
                  PurpleConnection *gc, gint id)
{
	JabberStream *js = purple_connection_get_protocol_data(gc);
	JabberChat *chat = jabber_chat_find_by_id(js, id);

	if(!chat)
		return;

	jabber_chat_part(chat, NULL);

	chat->left = TRUE;
}

void jabber_chat_destroy(JabberChat *chat)
{
	JabberStream *js = chat->js;
	char *room_jid = g_strdup_printf("%s@%s", chat->room, chat->server);

	g_hash_table_remove(js->chats, room_jid);
	g_free(room_jid);
}

void jabber_chat_free(JabberChat *chat)
{
	if(chat->config_dialog_handle)
		purple_request_close(chat->config_dialog_type, chat->config_dialog_handle);

	g_free(chat->room);
	g_free(chat->server);
	g_free(chat->handle);
	g_hash_table_destroy(chat->members);
	g_hash_table_destroy(chat->components);

	g_clear_pointer(&chat->joined, g_date_time_unref);

	g_free(chat);
}

gboolean jabber_chat_find_buddy(PurpleChatConversation *conv, const char *name)
{
	return purple_chat_conversation_has_user(conv, name);
}

gchar *
jabber_chat_user_real_name(G_GNUC_UNUSED PurpleProtocolChat *protocol_chat,
                           PurpleConnection *gc, gint id, const gchar *who)
{
	JabberStream *js = purple_connection_get_protocol_data(gc);
	JabberChat *chat;
	JabberChatMember *jcm;

	chat = jabber_chat_find_by_id(js, id);

	if(!chat)
		return NULL;

	jcm = g_hash_table_lookup(chat->members, who);
	if (jcm != NULL && jcm->jid)
		return g_strdup(jcm->jid);


	return g_strdup_printf("%s@%s/%s", chat->room, chat->server, who);
}

static void jabber_chat_room_configure_x_data_cb(JabberStream *js, PurpleXmlNode *result, gpointer data)
{
	JabberChat *chat = data;
	PurpleXmlNode *query;
	JabberIq *iq;
	char *to = g_strdup_printf("%s@%s", chat->room, chat->server);

	iq = jabber_iq_new_query(js, JABBER_IQ_SET, "http://jabber.org/protocol/muc#owner");
	purple_xmlnode_set_attrib(iq->node, "to", to);
	g_free(to);

	query = purple_xmlnode_get_child(iq->node, "query");

	purple_xmlnode_insert_child(query, result);

	jabber_iq_send(iq);
}

static void
jabber_chat_room_configure_cb(JabberStream *js, const char *from,
                              JabberIqType type, G_GNUC_UNUSED const char *id,
                              PurpleXmlNode *packet,
                              G_GNUC_UNUSED gpointer data)
{
	PurpleXmlNode *query, *x;
	char *msg;
	JabberChat *chat;
	JabberID *jid;

	if (!from)
		return;

	if (type == JABBER_IQ_RESULT) {
		jid = jabber_id_new(from);

		if(!jid)
			return;

		chat = jabber_chat_find(js, jid->node, jid->domain);
		jabber_id_free(jid);

		if(!chat)
			return;

		if(!(query = purple_xmlnode_get_child(packet, "query")))
			return;

		for(x = purple_xmlnode_get_child(query, "x"); x; x = purple_xmlnode_get_next_twin(x)) {
			const char *xmlns;
			if(!(xmlns = purple_xmlnode_get_namespace(x)))
				continue;

			if(purple_strequal(xmlns, "jabber:x:data")) {
				chat->config_dialog_type = PURPLE_REQUEST_FIELDS;
				chat->config_dialog_handle = jabber_x_data_request(js, x, jabber_chat_room_configure_x_data_cb, chat);
				return;
			}
		}
	} else if (type == JABBER_IQ_ERROR) {
		char *msg = jabber_parse_error(js, packet, NULL);

		purple_notify_error(js->gc, _("Configuration error"),
			_("Configuration error"), msg,
			purple_request_cpar_from_connection(js->gc));

		g_free(msg);
		return;
	}

	msg = g_strdup_printf("Unable to configure room %s", from);

	purple_notify_info(js->gc, _("Unable to configure"),
		_("Unable to configure"), msg,
		purple_request_cpar_from_connection(js->gc));
	g_free(msg);

}

void jabber_chat_request_room_configure(JabberChat *chat) {
	JabberIq *iq;
	char *room_jid;

	if(!chat)
		return;

	chat->config_dialog_handle = NULL;

	if(!chat->muc) {
		purple_notify_error(chat->js->gc, _("Room Configuration Error"),
			_("Room Configuration Error"),
			_("This room is not capable of being configured"),
			purple_request_cpar_from_connection(chat->js->gc));
		return;
	}

	iq = jabber_iq_new_query(chat->js, JABBER_IQ_GET,
			"http://jabber.org/protocol/muc#owner");
	room_jid = g_strdup_printf("%s@%s", chat->room, chat->server);

	purple_xmlnode_set_attrib(iq->node, "to", room_jid);

	jabber_iq_set_callback(iq, jabber_chat_room_configure_cb, NULL);

	jabber_iq_send(iq);

	g_free(room_jid);
}

void jabber_chat_create_instant_room(JabberChat *chat) {
	JabberIq *iq;
	PurpleXmlNode *query, *x;
	char *room_jid;

	if(!chat)
		return;

	chat->config_dialog_handle = NULL;

	iq = jabber_iq_new_query(chat->js, JABBER_IQ_SET,
			"http://jabber.org/protocol/muc#owner");
	query = purple_xmlnode_get_child(iq->node, "query");
	x = purple_xmlnode_new_child(query, "x");
	room_jid = g_strdup_printf("%s@%s", chat->room, chat->server);

	purple_xmlnode_set_attrib(iq->node, "to", room_jid);
	purple_xmlnode_set_namespace(x, "jabber:x:data");
	purple_xmlnode_set_attrib(x, "type", "submit");

	jabber_iq_send(iq);

	g_free(room_jid);
}

static void
jabber_chat_register_x_data_result_cb(JabberStream *js,
                                      G_GNUC_UNUSED const char *from,
                                      JabberIqType type,
                                      G_GNUC_UNUSED const char *id,
                                      PurpleXmlNode *packet,
                                      G_GNUC_UNUSED gpointer data)
{
	if (type == JABBER_IQ_ERROR) {
		char *msg = jabber_parse_error(js, packet, NULL);

		purple_notify_error(js->gc, _("Registration error"),
			_("Registration error"), msg,
			purple_request_cpar_from_connection(js->gc));

		g_free(msg);
		return;
	}
}

static void jabber_chat_register_x_data_cb(JabberStream *js, PurpleXmlNode *result, gpointer data)
{
	JabberChat *chat = data;
	PurpleXmlNode *query;
	JabberIq *iq;
	char *to = g_strdup_printf("%s@%s", chat->room, chat->server);

	iq = jabber_iq_new_query(js, JABBER_IQ_SET, "jabber:iq:register");
	purple_xmlnode_set_attrib(iq->node, "to", to);
	g_free(to);

	query = purple_xmlnode_get_child(iq->node, "query");

	purple_xmlnode_insert_child(query, result);

	jabber_iq_set_callback(iq, jabber_chat_register_x_data_result_cb, NULL);

	jabber_iq_send(iq);
}

static void
jabber_chat_register_cb(JabberStream *js, const char *from, JabberIqType type,
                        G_GNUC_UNUSED const char *id, PurpleXmlNode *packet,
                        G_GNUC_UNUSED gpointer data)
{
	PurpleXmlNode *query, *x;
	char *msg;
	JabberChat *chat;
	JabberID *jid;

	if (!from)
		return;

	if (type == JABBER_IQ_RESULT) {
		jid = jabber_id_new(from);

		if(!jid)
			return;

		chat = jabber_chat_find(js, jid->node, jid->domain);
		jabber_id_free(jid);

		if(!chat)
			return;

		if(!(query = purple_xmlnode_get_child(packet, "query")))
			return;

		for(x = purple_xmlnode_get_child(query, "x"); x; x = purple_xmlnode_get_next_twin(x)) {
			const char *xmlns;

			if(!(xmlns = purple_xmlnode_get_namespace(x)))
				continue;

			if(purple_strequal(xmlns, "jabber:x:data")) {
				jabber_x_data_request(js, x, jabber_chat_register_x_data_cb, chat);
				return;
			}
		}
	} else if (type == JABBER_IQ_ERROR) {
		char *msg = jabber_parse_error(js, packet, NULL);

		purple_notify_error(js->gc, _("Registration error"),
			_("Registration error"), msg,
			purple_request_cpar_from_connection(js->gc));

		g_free(msg);
		return;
	}

	msg = g_strdup_printf("Unable to configure room %s", from);

	purple_notify_info(js->gc, _("Unable to configure"), _("Unable to "
		"configure"), msg, purple_request_cpar_from_connection(js->gc));
	g_free(msg);

}

void jabber_chat_register(JabberChat *chat)
{
	JabberIq *iq;
	char *room_jid;

	if(!chat)
		return;

	room_jid = g_strdup_printf("%s@%s", chat->room, chat->server);

	iq = jabber_iq_new_query(chat->js, JABBER_IQ_GET, "jabber:iq:register");
	purple_xmlnode_set_attrib(iq->node, "to", room_jid);
	g_free(room_jid);

	jabber_iq_set_callback(iq, jabber_chat_register_cb, NULL);

	jabber_iq_send(iq);
}

/* merge this with the function below when we get everyone on the same page wrt /commands */
void jabber_chat_change_topic(JabberChat *chat, const char *topic)
{
	JabberMessage *jm;

	jm = g_new0(JabberMessage, 1);
	jm->js = chat->js;
	jm->type = JABBER_MESSAGE_GROUPCHAT;
	jm->to = g_strdup_printf("%s@%s", chat->room, chat->server);

	if (topic && *topic)
		jm->subject = g_strdup(topic);
	else
		jm->subject = g_strdup("");

	jabber_message_send(jm);
	jabber_message_free(jm);
}

void
jabber_chat_set_topic(G_GNUC_UNUSED PurpleProtocolChat *protocol_chat,
                      PurpleConnection *gc, gint id, const gchar *topic)
{
	JabberStream *js = purple_connection_get_protocol_data(gc);
	JabberChat *chat = jabber_chat_find_by_id(js, id);

	if(!chat)
		return;

	jabber_chat_change_topic(chat, topic);
}


gboolean jabber_chat_change_nick(JabberChat *chat, const char *nick)
{
	PurpleXmlNode *presence;
	char *full_jid;
	PurpleAccount *account;
	PurpleStatus *status;
	JabberBuddyState state;
	char *msg;
	int priority;

	if(!chat->muc) {
		purple_conversation_write_system_message(
			PURPLE_CONVERSATION(chat->conv),
			_("Nick changing not supported in non-MUC chatrooms"), 0);
		return FALSE;
	}

	account = purple_connection_get_account(chat->js->gc);
	status = purple_account_get_active_status(account);

	purple_status_to_jabber(status, &state, &msg, &priority);

	presence = jabber_presence_create_js(chat->js, state, msg, priority);
	full_jid = g_strdup_printf("%s@%s/%s", chat->room, chat->server, nick);
	purple_xmlnode_set_attrib(presence, "to", full_jid);
	g_free(full_jid);
	g_free(msg);

	jabber_send(chat->js, presence);
	purple_xmlnode_free(presence);

	return TRUE;
}

void jabber_chat_part(JabberChat *chat, const char *msg)
{
	char *room_jid;
	PurpleXmlNode *presence;

	room_jid = g_strdup_printf("%s@%s/%s", chat->room, chat->server,
			chat->handle);
	presence = purple_xmlnode_new("presence");
	purple_xmlnode_set_attrib(presence, "to", room_jid);
	purple_xmlnode_set_attrib(presence, "type", "unavailable");
	if(msg) {
		PurpleXmlNode *status = purple_xmlnode_new_child(presence, "status");
		purple_xmlnode_insert_data(status, msg, -1);
	}
	jabber_send(chat->js, presence);

	purple_xmlnode_free(presence);
	g_free(room_jid);
}

static void
roomlist_disco_result_cb(JabberStream *js, G_GNUC_UNUSED const char *from,
                         JabberIqType type, G_GNUC_UNUSED const char *id,
                         PurpleXmlNode *packet, G_GNUC_UNUSED gpointer data)
{
	PurpleXmlNode *query;
	PurpleXmlNode *item;

	if(!js->roomlist)
		return;

	if (type == JABBER_IQ_ERROR) {
		char *err = jabber_parse_error(js, packet, NULL);
		purple_notify_error(js->gc, _("Error"),
			_("Error retrieving room list"), err,
			purple_request_cpar_from_connection(js->gc));
		purple_roomlist_set_in_progress(js->roomlist, FALSE);
		g_object_unref(js->roomlist);
		js->roomlist = NULL;
		g_free(err);
		return;
	}

	if(!(query = purple_xmlnode_get_child(packet, "query"))) {
		char *err = jabber_parse_error(js, packet, NULL);
		purple_notify_error(js->gc, _("Error"),
			_("Error retrieving room list"), err,
			purple_request_cpar_from_connection(js->gc));
		purple_roomlist_set_in_progress(js->roomlist, FALSE);
		g_object_unref(js->roomlist);
		js->roomlist = NULL;
		g_free(err);
		return;
	}

	for(item = purple_xmlnode_get_child(query, "item"); item;
			item = purple_xmlnode_get_next_twin(item)) {
		const char *name;
		PurpleRoomlistRoom *room;
		JabberID *jid;

		if(!(jid = jabber_id_new(purple_xmlnode_get_attrib(item, "jid"))))
			continue;
		name = purple_xmlnode_get_attrib(item, "name");


		room = purple_roomlist_room_new(jid->node, name);
		purple_roomlist_room_add_field(room, "room", g_strdup(jid->node));
		purple_roomlist_room_add_field(room, "server", g_strdup(jid->domain));
		purple_roomlist_room_add(js->roomlist, room);
		g_object_unref(room);

		jabber_id_free(jid);
	}
	purple_roomlist_set_in_progress(js->roomlist, FALSE);
	g_object_unref(js->roomlist);
	js->roomlist = NULL;
}

static void
roomlist_cancel_cb(JabberStream *js, G_GNUC_UNUSED const char *server) {
	if(js->roomlist) {
		purple_roomlist_set_in_progress(js->roomlist, FALSE);
		g_object_unref(js->roomlist);
		js->roomlist = NULL;
	}
}

static void roomlist_ok_cb(JabberStream *js, const char *server)
{
	JabberIq *iq;

	if(!js->roomlist)
		return;

	if(!server || !*server) {
		purple_notify_error(js->gc, _("Invalid Server"),
			_("Invalid Server"), NULL,
			purple_request_cpar_from_connection(js->gc));
		purple_roomlist_set_in_progress(js->roomlist, FALSE);
		return;
	}

	purple_roomlist_set_in_progress(js->roomlist, TRUE);

	iq = jabber_iq_new_query(js, JABBER_IQ_GET, NS_DISCO_ITEMS);

	purple_xmlnode_set_attrib(iq->node, "to", server);

	jabber_iq_set_callback(iq, roomlist_disco_result_cb, NULL);

	jabber_iq_send(iq);
}

PurpleRoomlist *
jabber_roomlist_get_list(G_GNUC_UNUSED PurpleProtocolRoomlist *protocol_roomlist,
                         PurpleConnection *gc)
{
	JabberStream *js = purple_connection_get_protocol_data(gc);

	if(js->roomlist)
		g_object_unref(js->roomlist);

	js->roomlist = purple_roomlist_new(purple_connection_get_account(js->gc));

	purple_request_input(gc, _("Enter a Conference Server"), _("Enter a Conference Server"),
			_("Select a conference server to query"),
			js->chat_servers ? js->chat_servers->data : NULL,
			FALSE, FALSE, NULL,
			_("Find Rooms"), G_CALLBACK(roomlist_ok_cb),
			_("Cancel"), G_CALLBACK(roomlist_cancel_cb),
			purple_request_cpar_from_connection(gc),
			js);

	return js->roomlist;
}

void
jabber_roomlist_cancel(G_GNUC_UNUSED PurpleProtocolRoomlist *protocol_roomlist,
                       PurpleRoomlist *list)
{
	PurpleAccount *account;
	PurpleConnection *gc;
	JabberStream *js;

	account = purple_roomlist_get_account(list);
	gc = purple_account_get_connection(account);
	js = purple_connection_get_protocol_data(gc);

	purple_roomlist_set_in_progress(list, FALSE);

	if (js->roomlist == list) {
		js->roomlist = NULL;
		g_object_unref(list);
	}
}

char *
jabber_roomlist_room_serialize(G_GNUC_UNUSED PurpleProtocolRoomlist *protocol_roomlist,
                               PurpleRoomlistRoom *room)
{
	const gchar *room_name = NULL, *server = NULL;

	room_name = purple_roomlist_room_get_field(room, "room");
	server = purple_roomlist_room_get_field(room, "server");

	return g_strdup_printf("%s@%s", room_name, server);
}

void jabber_chat_member_free(JabberChatMember *jcm)
{
	g_free(jcm->handle);
	g_free(jcm->jid);
	g_free(jcm);
}

void
jabber_chat_track_handle(JabberChat *chat, const char *handle, const char *jid,
                         G_GNUC_UNUSED const char *affiliation,
                         G_GNUC_UNUSED const char *role)
{
	JabberChatMember *jcm = g_new0(JabberChatMember, 1);

	jcm->handle = g_strdup(handle);
	jcm->jid = g_strdup(jid);

	g_hash_table_replace(chat->members, jcm->handle, jcm);

	/* XXX: keep track of role and affiliation */
}

void jabber_chat_remove_handle(JabberChat *chat, const char *handle)
{
	g_hash_table_remove(chat->members, handle);
}

gboolean jabber_chat_ban_user(JabberChat *chat, const char *who, const char *why)
{
	JabberChatMember *jcm;
	const char *jid;
	char *to;
	JabberIq *iq;
	PurpleXmlNode *query, *item, *reason;

	jcm = g_hash_table_lookup(chat->members, who);
	if (jcm && jcm->jid)
		jid = jcm->jid;
	else if (strchr(who, '@') != NULL)
		jid = who;
	else
		return FALSE;

	iq = jabber_iq_new_query(chat->js, JABBER_IQ_SET,
			"http://jabber.org/protocol/muc#admin");

	to = g_strdup_printf("%s@%s", chat->room, chat->server);
	purple_xmlnode_set_attrib(iq->node, "to", to);
	g_free(to);

	query = purple_xmlnode_get_child(iq->node, "query");
	item = purple_xmlnode_new_child(query, "item");
	purple_xmlnode_set_attrib(item, "jid", jid);
	purple_xmlnode_set_attrib(item, "affiliation", "outcast");
	if(why) {
		reason = purple_xmlnode_new_child(item, "reason");
		purple_xmlnode_insert_data(reason, why, -1);
	}

	jabber_iq_send(iq);

	return TRUE;
}

gboolean jabber_chat_affiliate_user(JabberChat *chat, const char *who, const char *affiliation)
{
	JabberChatMember *jcm;
	const char *jid;
	char *to;
	JabberIq *iq;
	PurpleXmlNode *query, *item;

	jcm = g_hash_table_lookup(chat->members, who);
	if (jcm && jcm->jid)
		jid = jcm->jid;
	else if (strchr(who, '@') != NULL)
		jid = who;
	else
		return FALSE;

	iq = jabber_iq_new_query(chat->js, JABBER_IQ_SET,
			"http://jabber.org/protocol/muc#admin");

	to = g_strdup_printf("%s@%s", chat->room, chat->server);
	purple_xmlnode_set_attrib(iq->node, "to", to);
	g_free(to);

	query = purple_xmlnode_get_child(iq->node, "query");
	item = purple_xmlnode_new_child(query, "item");
	purple_xmlnode_set_attrib(item, "jid", jid);
	purple_xmlnode_set_attrib(item, "affiliation", affiliation);

	jabber_iq_send(iq);

	return TRUE;
}

static void
jabber_chat_affiliation_list_cb(JabberStream *js,
                                G_GNUC_UNUSED const char *from,
                                JabberIqType type,
                                G_GNUC_UNUSED const char *id,
                                PurpleXmlNode *packet, gpointer data)
{
	JabberChat *chat;
	PurpleXmlNode *query, *item;
	int chat_id = GPOINTER_TO_INT(data);
	GString *buf;

	if(!(chat = jabber_chat_find_by_id(js, chat_id)))
		return;

	if (type == JABBER_IQ_ERROR)
		return;

	if(!(query = purple_xmlnode_get_child(packet, "query")))
		return;

	buf = g_string_new(_("Affiliations:"));

	item = purple_xmlnode_get_child(query, "item");
	if (item) {
		for( ; item; item = purple_xmlnode_get_next_twin(item)) {
			const char *jid = purple_xmlnode_get_attrib(item, "jid");
			const char *affiliation = purple_xmlnode_get_attrib(item, "affiliation");
			if (jid && affiliation)
				g_string_append_printf(buf, "\n%s %s", jid, affiliation);
		}
    } else {
		buf = g_string_append_c(buf, '\n');
		buf = g_string_append_len(buf, _("No users found"), -1);
	}

	purple_conversation_write_system_message(PURPLE_CONVERSATION(chat->conv),
		buf->str, PURPLE_MESSAGE_NO_LOG);

	g_string_free(buf, TRUE);
}

gboolean jabber_chat_affiliation_list(JabberChat *chat, const char *affiliation)
{
	JabberIq *iq;
	char *room_jid;
	PurpleXmlNode *query, *item;

	iq = jabber_iq_new_query(chat->js, JABBER_IQ_GET,
			"http://jabber.org/protocol/muc#admin");

	room_jid = g_strdup_printf("%s@%s", chat->room, chat->server);
	purple_xmlnode_set_attrib(iq->node, "to", room_jid);

	query = purple_xmlnode_get_child(iq->node, "query");
	item = purple_xmlnode_new_child(query, "item");
	purple_xmlnode_set_attrib(item, "affiliation", affiliation);

	jabber_iq_set_callback(iq, jabber_chat_affiliation_list_cb, GINT_TO_POINTER(chat->id));
	jabber_iq_send(iq);

	return TRUE;
}

gboolean jabber_chat_role_user(JabberChat *chat, const char *who,
                               const char *role, const char *why)
{
	char *to;
	JabberIq *iq;
	PurpleXmlNode *query, *item;
	JabberChatMember *jcm;

	jcm = g_hash_table_lookup(chat->members, who);

	if (!jcm || !jcm->handle)
		return FALSE;

	iq = jabber_iq_new_query(chat->js, JABBER_IQ_SET,
	                         "http://jabber.org/protocol/muc#admin");

	to = g_strdup_printf("%s@%s", chat->room, chat->server);
	purple_xmlnode_set_attrib(iq->node, "to", to);
	g_free(to);

	query = purple_xmlnode_get_child(iq->node, "query");
	item = purple_xmlnode_new_child(query, "item");
	purple_xmlnode_set_attrib(item, "nick", jcm->handle);
	purple_xmlnode_set_attrib(item, "role", role);
	if (why) {
		PurpleXmlNode *reason = purple_xmlnode_new_child(item, "reason");
		purple_xmlnode_insert_data(reason, why, -1);
	}

	jabber_iq_send(iq);

	return TRUE;
}

static void
jabber_chat_role_list_cb(JabberStream *js, G_GNUC_UNUSED const char *from,
                         JabberIqType type, G_GNUC_UNUSED const char *id,
                         PurpleXmlNode *packet, gpointer data)
{
	JabberChat *chat;
	PurpleXmlNode *query, *item;
	int chat_id = GPOINTER_TO_INT(data);
	GString *buf;

	if(!(chat = jabber_chat_find_by_id(js, chat_id)))
		return;

	if (type == JABBER_IQ_ERROR)
		return;

	if(!(query = purple_xmlnode_get_child(packet, "query")))
		return;

	buf = g_string_new(_("Roles:"));

	item = purple_xmlnode_get_child(query, "item");
	if (item) {
		for( ; item; item = purple_xmlnode_get_next_twin(item)) {
			const char *jid  = purple_xmlnode_get_attrib(item, "jid");
			const char *role = purple_xmlnode_get_attrib(item, "role");
			if (jid && role)
				g_string_append_printf(buf, "\n%s %s", jid, role);
	    }
	} else {
		buf = g_string_append_c(buf, '\n');
		buf = g_string_append_len(buf, _("No users found"), -1);
	}

	purple_conversation_write_system_message(PURPLE_CONVERSATION(chat->conv),
		buf->str, PURPLE_MESSAGE_NO_LOG);

	g_string_free(buf, TRUE);
}

gboolean jabber_chat_role_list(JabberChat *chat, const char *role)
{
	JabberIq *iq;
	char *room_jid;
	PurpleXmlNode *query, *item;

	iq = jabber_iq_new_query(chat->js, JABBER_IQ_GET,
			"http://jabber.org/protocol/muc#admin");

	room_jid = g_strdup_printf("%s@%s", chat->room, chat->server);
	purple_xmlnode_set_attrib(iq->node, "to", room_jid);

	query = purple_xmlnode_get_child(iq->node, "query");
	item = purple_xmlnode_new_child(query, "item");
	purple_xmlnode_set_attrib(item, "role", role);

	jabber_iq_set_callback(iq, jabber_chat_role_list_cb, GINT_TO_POINTER(chat->id));
	jabber_iq_send(iq);

	return TRUE;
}

static void
jabber_chat_disco_traffic_cb(JabberStream *js, G_GNUC_UNUSED const char *from,
                             G_GNUC_UNUSED JabberIqType type,
                             G_GNUC_UNUSED const char *id,
                             G_GNUC_UNUSED PurpleXmlNode *packet,
                             gpointer data)
{
	JabberChat *chat;
	int chat_id = GPOINTER_TO_INT(data);

	if(!(chat = jabber_chat_find_by_id(js, chat_id)))
		return;

	/* defaults, in case the conference server doesn't
	 * support this request */
	chat->xhtml = TRUE;
}

void jabber_chat_disco_traffic(JabberChat *chat)
{
	JabberIq *iq;
	PurpleXmlNode *query;
	char *room_jid;

	room_jid = g_strdup_printf("%s@%s", chat->room, chat->server);

	iq = jabber_iq_new_query(chat->js, JABBER_IQ_GET, NS_DISCO_INFO);

	purple_xmlnode_set_attrib(iq->node, "to", room_jid);

	query = purple_xmlnode_get_child(iq->node, "query");

	purple_xmlnode_set_attrib(query, "node", "http://jabber.org/protocol/muc#traffic");

	jabber_iq_set_callback(iq, jabber_chat_disco_traffic_cb, GINT_TO_POINTER(chat->id));

	jabber_iq_send(iq);

	g_free(room_jid);
}
