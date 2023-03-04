/*
 * @file jingle.c
 *
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

#include <purple.h>

#include "content.h"
#include "jingle.h"
#include "session.h"
#include "iceudp.h"
#include "rawudp.h"
#include "rtp.h"

#include <string.h>
#include <gst/gst.h>

GType
jingle_get_type(const gchar *type)
{
	if (type == NULL)
		return G_TYPE_NONE;

	if (purple_strequal(type, JINGLE_TRANSPORT_RAWUDP))
		return JINGLE_TYPE_RAWUDP;
	else if (purple_strequal(type, JINGLE_TRANSPORT_ICEUDP))
		return JINGLE_TYPE_ICEUDP;
	else if (purple_strequal(type, JINGLE_APP_RTP))
		return JINGLE_TYPE_RTP;
	else
		return G_TYPE_NONE;
}

static void
jingle_handle_unknown_type(G_GNUC_UNUSED JingleSession *session,
                           G_GNUC_UNUSED PurpleXmlNode *jingle)
{
	/* Send error */
}

static void
jingle_handle_content_accept(JingleSession *session, PurpleXmlNode *jingle)
{
	PurpleXmlNode *content = purple_xmlnode_get_child(jingle, "content");
	jabber_iq_send(jingle_session_create_ack(session, jingle));

	for (; content; content = purple_xmlnode_get_next_twin(content)) {
		const gchar *name = purple_xmlnode_get_attrib(content, "name");
		const gchar *creator = purple_xmlnode_get_attrib(content, "creator");
		jingle_session_accept_content(session, name, creator);
		/* signal here */
	}
}

static void
jingle_handle_content_add(JingleSession *session, PurpleXmlNode *jingle)
{
	PurpleXmlNode *content = purple_xmlnode_get_child(jingle, "content");
	jabber_iq_send(jingle_session_create_ack(session, jingle));

	for (; content; content = purple_xmlnode_get_next_twin(content)) {
		JingleContent *pending_content =
				jingle_content_parse(content);
		if (pending_content == NULL) {
			purple_debug_error("jingle",
					"Error parsing \"content-add\" content.\n");
			jabber_iq_send(jingle_session_terminate_packet(session,
				"unsupported-applications"));
		} else {
			jingle_session_add_pending_content(session,
					pending_content);
		}
	}

	/* XXX: signal here */
}

static void
jingle_handle_content_modify(JingleSession *session, PurpleXmlNode *jingle)
{
	PurpleXmlNode *content = purple_xmlnode_get_child(jingle, "content");
	jabber_iq_send(jingle_session_create_ack(session, jingle));

	for (; content; content = purple_xmlnode_get_next_twin(content)) {
		const gchar *name = purple_xmlnode_get_attrib(content, "name");
		const gchar *creator = purple_xmlnode_get_attrib(content, "creator");
		JingleContent *local_content = jingle_session_find_content(session, name, creator);

		if (local_content != NULL) {
			const gchar *senders = purple_xmlnode_get_attrib(content, "senders");
			gchar *local_senders = jingle_content_get_senders(local_content);
			if (!purple_strequal(senders, local_senders))
				jingle_content_modify(local_content, senders);
			g_free(local_senders);
		} else {
			purple_debug_error("jingle", "content_modify: unknown content\n");
			jabber_iq_send(jingle_session_terminate_packet(session,
				"unknown-applications"));
		}
	}
}

static void
jingle_handle_content_reject(JingleSession *session, PurpleXmlNode *jingle)
{
	PurpleXmlNode *content = purple_xmlnode_get_child(jingle, "content");
	jabber_iq_send(jingle_session_create_ack(session, jingle));

	for (; content; content = purple_xmlnode_get_next_twin(content)) {
		const gchar *name = purple_xmlnode_get_attrib(content, "name");
		const gchar *creator = purple_xmlnode_get_attrib(content, "creator");
		jingle_session_remove_pending_content(session, name, creator);
		/* signal here */
	}
}

static void
jingle_handle_content_remove(JingleSession *session, PurpleXmlNode *jingle)
{
	PurpleXmlNode *content = purple_xmlnode_get_child(jingle, "content");

	jabber_iq_send(jingle_session_create_ack(session, jingle));

	for (; content; content = purple_xmlnode_get_next_twin(content)) {
		const gchar *name = purple_xmlnode_get_attrib(content, "name");
		const gchar *creator = purple_xmlnode_get_attrib(content, "creator");
		jingle_session_remove_content(session, name, creator);
	}
}

static void
jingle_handle_description_info(JingleSession *session, PurpleXmlNode *jingle)
{
	PurpleXmlNode *content = purple_xmlnode_get_child(jingle, "content");

	jabber_iq_send(jingle_session_create_ack(session, jingle));

	jingle_session_accept_session(session);

	for (; content; content = purple_xmlnode_get_next_twin(content)) {
		const gchar *name = purple_xmlnode_get_attrib(content, "name");
		const gchar *creator = purple_xmlnode_get_attrib(content, "creator");
		JingleContent *parsed_content =
				jingle_session_find_content(session, name, creator);
		if (parsed_content == NULL) {
			purple_debug_error("jingle", "Error parsing content\n");
			jabber_iq_send(jingle_session_terminate_packet(session,
				"unsupported-applications"));
		} else {
			jingle_content_handle_action(parsed_content, content,
					JINGLE_DESCRIPTION_INFO);
		}
	}
}

static void
jingle_handle_security_info(JingleSession *session, PurpleXmlNode *jingle)
{
	jabber_iq_send(jingle_session_create_ack(session, jingle));
}

static void
jingle_handle_session_accept(JingleSession *session, PurpleXmlNode *jingle)
{
	PurpleXmlNode *content = purple_xmlnode_get_child(jingle, "content");

	jabber_iq_send(jingle_session_create_ack(session, jingle));

	jingle_session_accept_session(session);

	for (; content; content = purple_xmlnode_get_next_twin(content)) {
		const gchar *name = purple_xmlnode_get_attrib(content, "name");
		const gchar *creator = purple_xmlnode_get_attrib(content, "creator");
		JingleContent *parsed_content =
				jingle_session_find_content(session, name, creator);
		if (parsed_content == NULL) {
			purple_debug_error("jingle", "Error parsing content\n");
			jabber_iq_send(jingle_session_terminate_packet(session,
				"unsupported-applications"));
		} else {
			jingle_content_handle_action(parsed_content, content,
					JINGLE_SESSION_ACCEPT);
		}
	}
}

static void
jingle_handle_session_info(JingleSession *session, PurpleXmlNode *jingle)
{
	jabber_iq_send(jingle_session_create_ack(session, jingle));
	/* XXX: call signal */
}

static void
jingle_handle_session_initiate(JingleSession *session, PurpleXmlNode *jingle)
{
	PurpleXmlNode *content = purple_xmlnode_get_child(jingle, "content");

	for (; content; content = purple_xmlnode_get_next_twin(content)) {
		JingleContent *parsed_content = jingle_content_parse(content);
		if (parsed_content == NULL) {
			purple_debug_error("jingle", "Error parsing content\n");
			jabber_iq_send(jingle_session_terminate_packet(session,
				"unsupported-applications"));
		} else {
			jingle_session_add_content(session, parsed_content);
			/* since we don't have the possibility to enable
			 * our local video after the video connection is
			 * established, we ignore senders of type
			 * 'initiator', and reply with senders of type
			 * 'both' instead.
			 */
			jingle_content_modify(parsed_content, "both");
			jingle_content_handle_action(parsed_content, content,
					JINGLE_SESSION_INITIATE);
		}
	}

	jabber_iq_send(jingle_session_create_ack(session, jingle));
}

static void
jingle_handle_session_terminate(JingleSession *session, PurpleXmlNode *jingle)
{
	jabber_iq_send(jingle_session_create_ack(session, jingle));

	jingle_session_handle_action(session, jingle,
			JINGLE_SESSION_TERMINATE);
	/* display reason? */
	g_object_unref(session);
}

static void
jingle_handle_transport_accept(JingleSession *session, PurpleXmlNode *jingle)
{
	PurpleXmlNode *content = purple_xmlnode_get_child(jingle, "content");

	jabber_iq_send(jingle_session_create_ack(session, jingle));

	for (; content; content = purple_xmlnode_get_next_twin(content)) {
		const gchar *name = purple_xmlnode_get_attrib(content, "name");
		const gchar *creator = purple_xmlnode_get_attrib(content, "creator");
		JingleContent *content = jingle_session_find_content(session, name, creator);
		jingle_content_accept_transport(content);
	}
}

static void
jingle_handle_transport_info(JingleSession *session, PurpleXmlNode *jingle)
{
	PurpleXmlNode *content = purple_xmlnode_get_child(jingle, "content");

	jabber_iq_send(jingle_session_create_ack(session, jingle));

	for (; content; content = purple_xmlnode_get_next_twin(content)) {
		const gchar *name = purple_xmlnode_get_attrib(content, "name");
		const gchar *creator = purple_xmlnode_get_attrib(content, "creator");
		JingleContent *parsed_content =
				jingle_session_find_content(session, name, creator);
		if (parsed_content == NULL) {
			purple_debug_error("jingle", "Error parsing content\n");
			jabber_iq_send(jingle_session_terminate_packet(session,
				"unsupported-applications"));
		} else {
			jingle_content_handle_action(parsed_content, content,
					JINGLE_TRANSPORT_INFO);
		}
	}
}

static void
jingle_handle_transport_reject(JingleSession *session, PurpleXmlNode *jingle)
{
	PurpleXmlNode *content = purple_xmlnode_get_child(jingle, "content");

	jabber_iq_send(jingle_session_create_ack(session, jingle));

	for (; content; content = purple_xmlnode_get_next_twin(content)) {
		const gchar *name = purple_xmlnode_get_attrib(content, "name");
		const gchar *creator = purple_xmlnode_get_attrib(content, "creator");
		JingleContent *content = jingle_session_find_content(session, name, creator);
		jingle_content_remove_pending_transport(content);
	}
}

static void
jingle_handle_transport_replace(JingleSession *session, PurpleXmlNode *jingle)
{
	PurpleXmlNode *content = purple_xmlnode_get_child(jingle, "content");

	jabber_iq_send(jingle_session_create_ack(session, jingle));

	for (; content; content = purple_xmlnode_get_next_twin(content)) {
		const gchar *name = purple_xmlnode_get_attrib(content, "name");
		const gchar *creator = purple_xmlnode_get_attrib(content, "creator");
		PurpleXmlNode *xmltransport = purple_xmlnode_get_child(content, "transport");
		JingleTransport *transport = jingle_transport_parse(xmltransport);
		JingleContent *content = jingle_session_find_content(session, name, creator);

		jingle_content_set_pending_transport(content, transport);
	}
}

typedef struct {
	const char *name;
	void (*handler)(JingleSession*, PurpleXmlNode*);
} JingleAction;

static const JingleAction jingle_actions[] = {
	{"unknown-type",	jingle_handle_unknown_type},
	{"content-accept",	jingle_handle_content_accept},
	{"content-add",		jingle_handle_content_add},
	{"content-modify",	jingle_handle_content_modify},
	{"content-reject",	jingle_handle_content_reject},
	{"content-remove",	jingle_handle_content_remove},
	{"description-info",	jingle_handle_description_info},
	{"security-info",	jingle_handle_security_info},
	{"session-accept",	jingle_handle_session_accept},
	{"session-info",	jingle_handle_session_info},
	{"session-initiate",	jingle_handle_session_initiate},
	{"session-terminate",	jingle_handle_session_terminate},
	{"transport-accept",	jingle_handle_transport_accept},
	{"transport-info",	jingle_handle_transport_info},
	{"transport-reject",	jingle_handle_transport_reject},
	{"transport-replace",	jingle_handle_transport_replace},
};

const gchar *
jingle_get_action_name(JingleActionType action)
{
	return jingle_actions[action].name;
}

JingleActionType
jingle_get_action_type(const gchar *action)
{
	static const int num_actions =
			sizeof(jingle_actions)/sizeof(JingleAction);
	/* Start at 1 to skip the unknown-action type */
	int i = 1;
	for (; i < num_actions; ++i) {
		if (purple_strequal(action, jingle_actions[i].name))
			return i;
	}
	return JINGLE_UNKNOWN_TYPE;
}

void
jingle_parse(JabberStream *js, const char *from, JabberIqType type,
             G_GNUC_UNUSED const char *id, PurpleXmlNode *jingle)
{
	const gchar *action;
	const gchar *sid;
	JingleActionType action_type;
	JingleSession *session;

	if (type != JABBER_IQ_SET) {
		/* TODO: send iq error here */
		return;
	}

	if (!(action = purple_xmlnode_get_attrib(jingle, "action"))) {
		/* TODO: send iq error here */
		return;
	}

	action_type = jingle_get_action_type(action);

	purple_debug_info("jabber", "got Jingle package action = %s\n",
			  action);

	if (!(sid = purple_xmlnode_get_attrib(jingle, "sid"))) {
		/* send iq error here */
		return;
	}

	if (!(session = jingle_session_find_by_sid(js, sid))
			&& !purple_strequal(action, "session-initiate")) {
		purple_debug_error("jingle", "jabber_jingle_session_parse couldn't find session\n");
		/* send iq error here */
		return;
	}

	if (action_type == JINGLE_SESSION_INITIATE) {
		if (session) {
			/* This should only happen if you start a session with yourself */
			purple_debug_error("jingle", "Jingle session with "
					"id={%s} already exists\n", sid);
			/* send iq error */
			return;
		} else {
			char *own_jid = g_strdup_printf("%s@%s/%s", js->user->node,
					js->user->domain, js->user->resource);
			session = jingle_session_create(js, sid, own_jid, from, FALSE);
			g_free(own_jid);
		}
	}

	jingle_actions[action_type].handler(session, jingle);
}

void
jingle_terminate_sessions(JabberStream *js)
{
	if (js->sessions) {
		GList *list = g_hash_table_get_values(js->sessions);
		g_list_free_full(list, g_object_unref);
		g_clear_pointer(&js->sessions, g_hash_table_unref);
	}
}

static void
jingle_create_relay_info(const gchar *ip, guint port, const gchar *username,
	const gchar *password, const gchar *relay_type, GPtrArray *relay_info)
{
	GValue value;
	GstStructure *turn_setup = gst_structure_new("relay-info",
		"ip", G_TYPE_STRING, ip,
		"port", G_TYPE_UINT, port,
		"username", G_TYPE_STRING, username,
		"password", G_TYPE_STRING, password,
		"relay-type", G_TYPE_STRING, relay_type,
		NULL);
	purple_debug_info("jabber", "created gst_structure %p\n", turn_setup);
	if (turn_setup) {
		memset(&value, 0, sizeof(GValue));
		g_value_init(&value, GST_TYPE_STRUCTURE);
		gst_value_set_structure(&value, turn_setup);
		g_ptr_array_add(relay_info, &value);
		gst_structure_free(turn_setup);
	}
}

static void
jingle_param_free(GValue *value)
{
	g_value_unset(value);
	g_free(value);
}

GHashTable *
jingle_get_params(G_GNUC_UNUSED JabberStream *js, const gchar *relay_ip,
                  guint relay_udp, guint relay_tcp, guint relay_ssltcp,
                  const gchar *relay_username, const gchar *relay_password)
{
	GHashTable *params = NULL;
	GValue *value = NULL;

	if(relay_ip) {
		GPtrArray *relay_info = g_ptr_array_new_full(1, (GDestroyNotify)gst_structure_free);

		params = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify)jingle_param_free);

		if (relay_udp) {
			jingle_create_relay_info(relay_ip, relay_udp, relay_username,
				relay_password, "udp", relay_info);
		}
		if (relay_tcp) {
			jingle_create_relay_info(relay_ip, relay_tcp, relay_username,
				relay_password, "tcp", relay_info);
		}
		if (relay_ssltcp) {
			jingle_create_relay_info(relay_ip, relay_ssltcp, relay_username,
				relay_password, "tls", relay_info);
		}
		value = g_new(GValue, 1);
		g_value_init(value, G_TYPE_PTR_ARRAY);
		g_value_take_boxed(value, relay_info);
		g_hash_table_insert(params, "relay-info", value);
	}

	return params;
}
