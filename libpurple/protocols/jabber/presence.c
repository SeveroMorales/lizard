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

#include "buddy.h"
#include "chat.h"
#include "presence.h"
#include "iq.h"
#include "jutil.h"
#include "adhoccommands.h"

#include "usermood.h"
#include "usertune.h"

static GHashTable *presence_handlers = NULL;

static const struct {
	const char *name;
	JabberPresenceType type;
} jabber_presence_types[] = {
	{ "error", JABBER_PRESENCE_ERROR },
	{ "probe", JABBER_PRESENCE_PROBE },
	{ "unavailable", JABBER_PRESENCE_UNAVAILABLE },
	{ "subscribe", JABBER_PRESENCE_SUBSCRIBE },
	{ "subscribed", JABBER_PRESENCE_SUBSCRIBED },
	{ "unsubscribe", JABBER_PRESENCE_UNSUBSCRIBE },
	{ "unsubscribed", JABBER_PRESENCE_UNSUBSCRIBED }
	/* { NULL, JABBER_PRESENCE_AVAILABLE } the default */
};

typedef void (JabberPresenceHandler)(JabberStream *js,
                                     JabberPresence *presence,
                                     PurpleXmlNode *child);

static JabberPresenceType
str_to_presence_type(const char *type)
{
	gsize i;

	if (type == NULL)
		return JABBER_PRESENCE_AVAILABLE;

	for (i = 0; i < G_N_ELEMENTS(jabber_presence_types); ++i)
		if (purple_strequal(type, jabber_presence_types[i].name))
			return jabber_presence_types[i].type;

	purple_debug_warning("jabber", "Unknown presence type '%s'\n", type);
	return JABBER_PRESENCE_AVAILABLE;
}

static void
chats_send_presence_foreach(G_GNUC_UNUSED gpointer key, gpointer val,
                            gpointer user_data)
{
	JabberChat *chat = val;
	PurpleXmlNode *presence = user_data;
	char *chat_full_jid;

	if(!chat->conv || chat->left)
		return;

	chat_full_jid = g_strdup_printf("%s@%s/%s", chat->room, chat->server,
			chat->handle);

	purple_xmlnode_set_attrib(presence, "to", chat_full_jid);
	jabber_send(chat->js, presence);
	g_free(chat_full_jid);
}

void jabber_presence_fake_to_self(JabberStream *js, PurpleStatus *status)
{
	PurpleAccount *account;
	PurplePresence *presence;
	JabberBuddy *jb;
	JabberBuddyResource *jbr;
	const char *username;
	JabberBuddyState state;
	char *msg;
	int priority;

	g_return_if_fail(js->user != NULL);

	account = purple_connection_get_account(js->gc);
	username = purple_connection_get_display_name(js->gc);
	presence = purple_account_get_presence(account);
	if (status == NULL)
		status = purple_presence_get_active_status(presence);
	purple_status_to_jabber(status, &state, &msg, &priority);

	jb = js->user_jb;

	if (state == JABBER_BUDDY_STATE_UNAVAILABLE ||
			state == JABBER_BUDDY_STATE_UNKNOWN) {
		jabber_buddy_remove_resource(jb, js->user->resource);
	} else {
		jbr = jabber_buddy_track_resource(jb, js->user->resource, priority,
				state, msg);

		jbr->idle = 0;
		if(purple_presence_is_idle(presence)) {
			GDateTime *idle_since = purple_presence_get_idle_time(presence);

			if(idle_since != NULL) {
				jbr->idle = g_date_time_to_unix(idle_since);
			}
		}
	}

	/*
	 * While we need to track the status of this resource, the core
	 * only cares if we're on our own buddy list.
	 */
	if (purple_blist_find_buddy(account, username)) {
		jbr = jabber_buddy_find_resource(jb, NULL);
		if (jbr) {
			purple_protocol_got_user_status(account, username,
					jabber_buddy_state_get_status_id(jbr->state),
					"priority", jbr->priority,
					jbr->status ? "message" : NULL, jbr->status,
					NULL);
			purple_protocol_got_user_idle(account, username, jbr->idle, jbr->idle);
		} else {
			purple_protocol_got_user_status(account, username, "offline",
					msg ? "message" : NULL, msg,
					NULL);
		}
	}
	g_free(msg);
}

void
jabber_set_status(G_GNUC_UNUSED PurpleProtocolServer *protocol_server,
                  PurpleAccount *account, PurpleStatus *status)
{
	PurpleConnection *gc;
	JabberStream *js;

	if (!purple_account_is_connected(account))
		return;

	if (purple_status_is_exclusive(status) && !purple_status_is_active(status)) {
		/* An exclusive status can't be deactivated. You should just
		 * activate some other exclusive status. */
		return;
	}

	gc = purple_account_get_connection(account);
	js = purple_connection_get_protocol_data(gc);

	/* it's a mood update */
	if (purple_status_type_get_primitive(purple_status_get_status_type(status)) == PURPLE_STATUS_MOOD) {
		const char *mood =
			purple_status_get_attr_string(status, PURPLE_MOOD_NAME);
		const char *mood_text =
			purple_status_get_attr_string(status, PURPLE_MOOD_COMMENT);
		jabber_mood_set(js, mood, mood_text);
		return;
	}

	jabber_presence_send(js, FALSE);
}

void jabber_presence_send(JabberStream *js, gboolean force)
{
	PurpleAccount *account;
	PurpleXmlNode *presence, *x, *photo;
	char *stripped = NULL;
	JabberBuddyState state;
	int priority;
	const char *artist = NULL, *title = NULL, *source = NULL, *uri = NULL, *track = NULL;
	int length = -1;
	gboolean allowBuzz;
	PurplePresence *p;
	PurpleStatus *status, *tune;

	account = purple_connection_get_account(js->gc);
	p = purple_account_get_presence(account);
	status = purple_presence_get_active_status(p);

	/* we don't want to send presence before we've gotten our roster */
	if (js->state != JABBER_STREAM_CONNECTED) {
		purple_debug_misc("jabber", "attempt to send presence before roster retrieved\n");
		return;
	}

	purple_status_to_jabber(status, &state, &stripped, &priority);

	/* check for buzz support */
	allowBuzz = purple_status_get_attr_boolean(status,"buzz");
	/* changing the buzz state has to trigger a re-broadcasting of the presence for caps */

	/* check if there are any differences to the <presence> and send them in that case */
	if (force || allowBuzz != js->allowBuzz || js->old_state != state ||
		!purple_strequal(js->old_msg, stripped) || js->old_priority != priority ||
		!purple_strequal(js->old_avatarhash, js->avatar_hash) || js->old_idle != js->idle) {
		/* Need to update allowBuzz before creating the presence (with caps) */
		js->allowBuzz = allowBuzz;

		presence = jabber_presence_create_js(js, state, stripped, priority);

		/* Per XEP-0153 4.1, we must always send the <x> */
		x = purple_xmlnode_new_child(presence, "x");
		purple_xmlnode_set_namespace(x, "vcard-temp:x:update");
		/*
		 * FIXME: Per XEP-0153 4.3.2 bullet 2, we must not publish our
		 * image hash if another resource has logged in and updated the
		 * vcard avatar. Requires changes in jabber_presence_parse.
		 */
		if (js->vcard_fetched) {
			/* Always publish a <photo>; it's empty if we have no image. */
			photo = purple_xmlnode_new_child(x, "photo");
			if (js->avatar_hash)
				purple_xmlnode_insert_data(photo, js->avatar_hash, -1);
		}

		jabber_send(js, presence);

		g_hash_table_foreach(js->chats, chats_send_presence_foreach, presence);
		purple_xmlnode_free(presence);

		/* update old values */

		g_free(js->old_msg);
		g_free(js->old_avatarhash);
		js->old_msg = g_strdup(stripped);
		js->old_avatarhash = g_strdup(js->avatar_hash);
		js->old_state = state;
		js->old_priority = priority;
		js->old_idle = js->idle;
	}
	g_free(stripped);

	/* next, check if there are any changes to the tune values */
	tune = purple_presence_get_status(p, "tune");
	if (purple_status_is_active(tune)) {
		artist = purple_status_get_attr_string(tune, PURPLE_TUNE_ARTIST);
		title = purple_status_get_attr_string(tune, PURPLE_TUNE_TITLE);
		source = purple_status_get_attr_string(tune, PURPLE_TUNE_ALBUM);
		uri = purple_status_get_attr_string(tune, PURPLE_TUNE_URL);
		track = purple_status_get_attr_string(tune, PURPLE_TUNE_TRACK);
		length = (!purple_status_get_attr_value(tune, PURPLE_TUNE_TIME)) ? -1 :
				purple_status_get_attr_int(tune, PURPLE_TUNE_TIME);
	}

	if(!purple_strequal(artist, js->old_artist) || !purple_strequal(title, js->old_title) ||
			!purple_strequal(source, js->old_source) || !purple_strequal(uri, js->old_uri) ||
			!purple_strequal(track, js->old_track) || (length != js->old_length)) {
		PurpleJabberTuneInfo tuneinfo = {
			(char*)artist,
			(char*)title,
			(char*)source,
			(char*)track,
			length,
			(char*)uri
		};
		jabber_tune_set(js->gc, &tuneinfo);

		/* update old values */
		g_free(js->old_artist);
		g_free(js->old_title);
		g_free(js->old_source);
		g_free(js->old_uri);
		g_free(js->old_track);
		js->old_artist = g_strdup(artist);
		js->old_title = g_strdup(title);
		js->old_source = g_strdup(source);
		js->old_uri = g_strdup(uri);
		js->old_length = length;
		js->old_track = g_strdup(track);
	}

#undef CHANGED

	jabber_presence_fake_to_self(js, status);
}

PurpleXmlNode *jabber_presence_create_js(JabberStream *js, JabberBuddyState state, const char *msg, int priority)
{
	PurpleXmlNode *show, *status, *presence, *pri, *c;
	const char *show_string = NULL;

	g_return_val_if_fail(js !=NULL, NULL);

	presence = purple_xmlnode_new("presence");

	if(state == JABBER_BUDDY_STATE_UNAVAILABLE)
		purple_xmlnode_set_attrib(presence, "type", "unavailable");
	else if(state != JABBER_BUDDY_STATE_ONLINE &&
			state != JABBER_BUDDY_STATE_UNKNOWN &&
			state != JABBER_BUDDY_STATE_ERROR)
		show_string = jabber_buddy_state_get_show(state);

	if(show_string) {
		show = purple_xmlnode_new_child(presence, "show");
		purple_xmlnode_insert_data(show, show_string, -1);
	}

	if(msg) {
		status = purple_xmlnode_new_child(presence, "status");
		purple_xmlnode_insert_data(status, msg, -1);
	}

	if(priority) {
		char *pstr = g_strdup_printf("%d", priority);
		pri = purple_xmlnode_new_child(presence, "priority");
		purple_xmlnode_insert_data(pri, pstr, -1);
		g_free(pstr);
	}

	/* if we are idle and not offline, include idle */
	if (js->idle && state != JABBER_BUDDY_STATE_UNAVAILABLE) {
		PurpleXmlNode *query = purple_xmlnode_new_child(presence, "query");
		gchar seconds[10];
		g_snprintf(seconds, 10, "%d", (int) (time(NULL) - js->idle));

		purple_xmlnode_set_namespace(query, NS_LAST_ACTIVITY);
		purple_xmlnode_set_attrib(query, "seconds", seconds);
	}

	/* JEP-0115 */
	/* calculate hash */
	jabber_caps_calculate_own_hash(js);
	/* create xml */
	c = purple_xmlnode_new_child(presence, "c");
	purple_xmlnode_set_namespace(c, "http://jabber.org/protocol/caps");
	purple_xmlnode_set_attrib(c, "node", CAPS0115_NODE);
	purple_xmlnode_set_attrib(c, "hash", "sha-1");
	purple_xmlnode_set_attrib(c, "ver", jabber_caps_get_own_hash(js));

	return presence;
}

struct _jabber_add_permit {
	PurpleConnection *gc;
	JabberStream *js;
	char *who;
};

static void
authorize_add_cb(G_GNUC_UNUSED PurpleAuthorizationRequest *request,
                 gpointer data)
{
	struct _jabber_add_permit *jap = data;

	PURPLE_ASSERT_CONNECTION_IS_VALID(jap->gc);

	jabber_presence_subscription_set(purple_connection_get_protocol_data(jap->gc),
		jap->who, "subscribed");

	g_free(jap->who);
	g_free(jap);
}

static void
deny_add_cb(G_GNUC_UNUSED PurpleAuthorizationRequest *request,
            G_GNUC_UNUSED const char *message, gpointer data)
{
	struct _jabber_add_permit *jap = data;

	PURPLE_ASSERT_CONNECTION_IS_VALID(jap->gc);

	jabber_presence_subscription_set(purple_connection_get_protocol_data(jap->gc),
		jap->who, "unsubscribed");

	g_free(jap->who);
	g_free(jap);
}

static void
jabber_vcard_parse_avatar(JabberStream *js, const char *from,
                          G_GNUC_UNUSED JabberIqType type,
                          G_GNUC_UNUSED const char *id, PurpleXmlNode *packet,
                          G_GNUC_UNUSED gpointer data)
{
	JabberBuddy *jb = NULL;
	PurpleXmlNode *vcard, *photo, *binval, *fn, *nick;
	char *text;

	if(!from)
		return;

	jb = jabber_buddy_find(js, from, TRUE);

	js->pending_avatar_requests = g_slist_remove(js->pending_avatar_requests, jb);

	if((vcard = purple_xmlnode_get_child(packet, "vCard")) ||
			(vcard = purple_xmlnode_get_child_with_namespace(packet, "query", "vcard-temp"))) {
		/* The logic here regarding the nickname and full name is copied from
		 * buddy.c:jabber_vcard_parse. */
		gchar *nickname = NULL;
		if ((fn = purple_xmlnode_get_child(vcard, "FN")))
			nickname = purple_xmlnode_get_data(fn);

		if ((nick = purple_xmlnode_get_child(vcard, "NICKNAME"))) {
			char *tmp = purple_xmlnode_get_data(nick);
			char *bare_jid = jabber_get_bare_jid(from);
			if (tmp && strstr(bare_jid, tmp) == NULL) {
				g_free(nickname);
				nickname = tmp;
			} else
				g_free(tmp);

			g_free(bare_jid);
		}

		if (nickname) {
			purple_serv_got_alias(js->gc, from, nickname);
			g_free(nickname);
		}

		if ((photo = purple_xmlnode_get_child(vcard, "PHOTO"))) {
			guchar *data = NULL;
			gchar *hash = NULL;
			gsize size = 0;

			if ((binval = purple_xmlnode_get_child(photo, "BINVAL")) &&
					(text = purple_xmlnode_get_data(binval))) {
				data = g_base64_decode(text, &size);
				g_free(text);

				if (data) {
					hash = g_compute_checksum_for_data(G_CHECKSUM_SHA1, data,
					                                   size);
				}
			}

			purple_buddy_icons_set_for_user(purple_connection_get_account(js->gc), from, data, size, hash);

			g_free(hash);
		}
	}
}

typedef struct {
	JabberStream *js;
	JabberBuddy *jb;
	char *from;
} JabberPresenceCapabilities;

static void
jabber_presence_set_capabilities(JabberCapsClientInfo *info,
                                 JabberPresenceCapabilities *userdata)
{
	JabberBuddyResource *jbr;
	char *resource = strchr(userdata->from, '/');

	if (resource)
		resource += 1;

	jbr = jabber_buddy_find_resource(userdata->jb, resource);
	if (!jbr) {
		g_free(userdata->from);
		g_free(userdata);
		return;
	}

	jbr->caps = info;

	purple_protocol_got_media_caps(
			purple_connection_get_account(userdata->js->gc),
			userdata->from);
	if (info == NULL)
		goto out;

	if (!jbr->commands_fetched && jabber_resource_has_capability(jbr, "http://jabber.org/protocol/commands")) {
		JabberIq *iq = jabber_iq_new_query(userdata->js, JABBER_IQ_GET, NS_DISCO_ITEMS);
		PurpleXmlNode *query = purple_xmlnode_get_child_with_namespace(iq->node, "query", NS_DISCO_ITEMS);
		purple_xmlnode_set_attrib(iq->node, "to", userdata->from);
		purple_xmlnode_set_attrib(query, "node", "http://jabber.org/protocol/commands");
		jabber_iq_set_callback(iq, jabber_adhoc_disco_result_cb, NULL);
		jabber_iq_send(iq);

		jbr->commands_fetched = TRUE;
	}

out:
	g_free(userdata->from);
	g_free(userdata);
}

static gboolean
handle_presence_chat(JabberStream *js, JabberPresence *presence, PurpleXmlNode *packet)
{
	static int i = 1;
	PurpleChatUserFlags flags = PURPLE_CHAT_USER_NONE;
	JabberChat *chat = presence->chat;

	if (presence->state == JABBER_BUDDY_STATE_ERROR) {
		char *title, *msg = jabber_parse_error(js, packet, NULL);

		if (!chat->conv) {
			title = g_strdup_printf(_("Error joining chat %s"), presence->from);
			purple_serv_got_join_chat_failed(js->gc, chat->components);
		} else {
			title = g_strdup_printf(_("Error in chat %s"), presence->from);
			if (g_hash_table_size(chat->members) == 0)
				purple_serv_got_chat_left(js->gc, chat->id);
		}
		purple_notify_error(js->gc, title, title, msg,
			purple_request_cpar_from_connection(js->gc));
		g_free(title);
		g_free(msg);

		if (g_hash_table_size(chat->members) == 0)
			/* Only destroy the chat if the error happened while joining */
			jabber_chat_destroy(chat);
		return FALSE;
	}

	if (presence->type == JABBER_PRESENCE_AVAILABLE) {
		const char *jid = NULL;
		const char *affiliation = NULL;
		const char *role = NULL;
		gboolean is_our_resource = FALSE; /* Is the presence about us? */
		JabberBuddyResource *jbr;

		/*
		 * XEP-0045 mandates the presence to include a resource (which is
		 * treated as the chat nick). Some non-compliant servers allow
		 * joining without a nick.
		 */
		if (!presence->jid_from->resource)
			return FALSE;

		if (presence->chat_info.item) {
			jid = purple_xmlnode_get_attrib(presence->chat_info.item, "jid");
			affiliation = purple_xmlnode_get_attrib(presence->chat_info.item, "affiliation");
			role = purple_xmlnode_get_attrib(presence->chat_info.item, "role");
		}

		if (g_slist_find(presence->chat_info.codes, GINT_TO_POINTER(110)) ||
				purple_strequal(presence->jid_from->resource, chat->handle) ||
				purple_strequal(presence->to, jid))
			is_our_resource = TRUE;

		if (g_slist_find(presence->chat_info.codes, GINT_TO_POINTER(201))) {
			chat->config_dialog_type = PURPLE_REQUEST_ACTION;
			chat->config_dialog_handle =
				purple_request_action(js->gc,
						_("Create New Room"),
						_("Create New Room"),
						_("You are creating a new room.  Would"
							" you like to configure it, or"
							" accept the default settings?"),
						/* Default Action */ 1,
						purple_request_cpar_from_conversation(PURPLE_CONVERSATION(chat->conv)),
						chat, 2,
						_("_Configure Room"), G_CALLBACK(jabber_chat_request_room_configure),
						_("_Accept Defaults"), G_CALLBACK(jabber_chat_create_instant_room));
		}

		if (g_slist_find(presence->chat_info.codes, GINT_TO_POINTER(210))) {
			/* server rewrote room-nick */
			g_free(chat->handle);
			chat->handle = g_strdup(presence->jid_from->resource);
		}

		if (purple_strequal(affiliation, "owner"))
			flags |= PURPLE_CHAT_USER_FOUNDER;
		if (role) {
			if (purple_strequal(role, "moderator"))
				flags |= PURPLE_CHAT_USER_OP;
			else if (purple_strequal(role, "participant"))
				flags |= PURPLE_CHAT_USER_VOICE;
		}

		if(!chat->conv) {
			PurpleConversation *conv;
			char *room_jid = g_strdup_printf("%s@%s", presence->jid_from->node, presence->jid_from->domain);
			chat->id = i++;
			conv = purple_serv_got_joined_chat(js->gc, chat->id, room_jid);
			chat->conv = PURPLE_CHAT_CONVERSATION(conv);
			purple_chat_conversation_set_nick(chat->conv, chat->handle);

			jabber_chat_disco_traffic(chat);
			g_free(room_jid);
		}

		jbr = jabber_buddy_track_resource(presence->jb, presence->jid_from->resource, presence->priority, presence->state, presence->status);
		jbr->commands_fetched = TRUE;

		jabber_chat_track_handle(chat, presence->jid_from->resource, jid, affiliation, role);

		if(!jabber_chat_find_buddy(chat->conv, presence->jid_from->resource)) {
			gboolean new_arrival = FALSE;

			if(chat->joined != NULL) {
				gint newer = -1;

				if(presence->sent != NULL) {
					newer = g_date_time_compare(presence->sent, chat->joined);
				}

				if(!presence->delayed || newer > 0) {
					new_arrival = TRUE;
				}
			}

			purple_chat_conversation_add_user(chat->conv,
			                                  presence->jid_from->resource,
			                                  jid, flags, new_arrival);
		} else {
			purple_chat_user_set_flags(purple_chat_conversation_find_user(chat->conv, presence->jid_from->resource),
					flags);
		}

		if (is_our_resource && chat->joined == NULL) {
			chat->joined = g_date_time_new_now_utc();
		}

	} else if (presence->type == JABBER_PRESENCE_UNAVAILABLE) {
		gboolean nick_change = FALSE;
		gboolean kick = FALSE;
		gboolean is_our_resource = FALSE; /* Is the presence about us? */

		const char *jid = NULL;

		/* If the chat nick is invalid, we haven't yet joined, or we've
		 * already left (it was probably us leaving after we closed the
		 * chat), we don't care.
		 */
		if (!presence->jid_from->resource || !chat->conv || chat->left) {
			if (chat->left &&
					presence->jid_from->resource && chat->handle && purple_strequal(presence->jid_from->resource, chat->handle))
				jabber_chat_destroy(chat);
			return FALSE;
		}

		is_our_resource = purple_strequal(presence->jid_from->resource, chat->handle);

		jabber_buddy_remove_resource(presence->jb, presence->jid_from->resource);

		if (presence->chat_info.item)
			jid = purple_xmlnode_get_attrib(presence->chat_info.item, "jid");

		if (chat->muc) {
			if (g_slist_find(presence->chat_info.codes, GINT_TO_POINTER(110))) {
				is_our_resource = TRUE;
				chat->joined = 0;
			}

			if (g_slist_find(presence->chat_info.codes, GINT_TO_POINTER(301))) {
				/* XXX: We got banned.  YAY! (No GIR, that's bad) */
			}


			if (g_slist_find(presence->chat_info.codes, GINT_TO_POINTER(303))) {
				const char *nick = NULL;
				if (presence->chat_info.item)
					nick = purple_xmlnode_get_attrib(presence->chat_info.item, "nick");

				/* nick change */
				if (!nick) {
					purple_debug_warning("jabber", "Chat presence indicating a nick change, but no new nickname!\n");
				} else {
					nick_change = TRUE;

					if (purple_strequal(presence->jid_from->resource, chat->handle)) {
						/* Changing our own nickname */
						g_free(chat->handle);
						/* TODO: This should be resourceprep'd */
						chat->handle = g_strdup(nick);
					}

					purple_chat_conversation_rename_user(chat->conv,
					                             presence->jid_from->resource,
					                             nick);
					jabber_chat_remove_handle(chat,
					                          presence->jid_from->resource);
				}
			}

			if (g_slist_find(presence->chat_info.codes, GINT_TO_POINTER(307))) {
				/* Someone was kicked from the room */
				const char *actor = NULL;
				char *reason = NULL;
				char *tmp;

				kick = TRUE;

				if (presence->chat_info.item) {
					PurpleXmlNode *node;

					node = purple_xmlnode_get_child(presence->chat_info.item, "actor");
					if (node)
						actor = purple_xmlnode_get_attrib(node, "jid");
					node = purple_xmlnode_get_child(presence->chat_info.item, "reason");
					if (node)
						reason = purple_xmlnode_get_data(node);
				}

				if (reason == NULL)
					reason = g_strdup(_("No reason"));

				if (is_our_resource) {
					if (actor)
						tmp = g_strdup_printf(_("You have been kicked by %s: (%s)"),
								actor, reason);
					else
						tmp = g_strdup_printf(_("You have been kicked: (%s)"),
								reason);
				} else {
					if (actor)
						tmp = g_strdup_printf(_("Kicked by %s (%s)"),
								actor, reason);
					else
						tmp = g_strdup_printf(_("Kicked (%s)"),
								reason);
				}

				g_free(presence->status);
				presence->status = tmp;

				g_free(reason);
			}

			if (g_slist_find(presence->chat_info.codes, GINT_TO_POINTER(321))) {
				/* XXX: removed due to an affiliation change */
			}

			if (g_slist_find(presence->chat_info.codes, GINT_TO_POINTER(322))) {
				/* XXX: removed because room is now members-only */
			}

			if (g_slist_find(presence->chat_info.codes, GINT_TO_POINTER(332))) {
				/* XXX: removed due to system shutdown */
			}
		}

		/*
		 * Possibly another connected resource of our JID (see XEP-0045
		 * v1.24 section 7.1.10) being disconnected. Should be
		 * distinguished by the item_jid.
		 * Also possibly works around bits of an Openfire bug. See
		 * #8319.
		 */
		if (is_our_resource && jid && !purple_strequal(presence->to, jid)) {
			/* TODO: When the above is a loop, this needs to still act
			 * sanely for all cases (this code is a little fragile). */
			if (!kick && !nick_change)
				/* Presumably, kicks and nick changes also affect us. */
				is_our_resource = FALSE;
		}

		if(!nick_change) {
			if (is_our_resource) {
				if (kick) {
					gchar *msg = g_strdup_printf("%s: %s",
						presence->jid_from->resource,
						presence->status);
					purple_conversation_write_system_message(PURPLE_CONVERSATION(chat->conv), msg, 0);
				}

				purple_serv_got_chat_left(js->gc, chat->id);
				jabber_chat_destroy(chat);
			} else {
				purple_chat_conversation_remove_user(chat->conv, presence->jid_from->resource,
						presence->status);
				jabber_chat_remove_handle(chat, presence->jid_from->resource);
			}
		}
	}

	return TRUE;
}

static gboolean
handle_presence_contact(JabberStream *js, JabberPresence *presence)
{
	JabberBuddyResource *jbr;
	PurpleAccount *account;
	PurpleBuddy *b;
	char *buddy_name;
	PurpleConversation *im;
	PurpleConversationManager *manager;

	buddy_name = jabber_id_get_bare_jid(presence->jid_from);

	account = purple_connection_get_account(js->gc);
	b = purple_blist_find_buddy(account, buddy_name);

	manager = purple_conversation_manager_get_default();
	/*
	 * Unbind/unlock from sending messages to a specific resource on
	 * presence changes.  This is locked to a specific resource when
	 * receiving a message (in message.c).
	 */
	im = purple_conversation_manager_find_im(manager, account, buddy_name);
	if (im) {
		purple_debug_info("jabber", "Changed conversation binding from %s to %s",
				purple_conversation_get_name(im), buddy_name);
		purple_conversation_set_name(im, buddy_name);
	}

	if (b == NULL) {
		if (presence->jb != js->user_jb) {
			PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
			purple_debug_warning("jabber",
			                     "Got presence for unknown buddy %s on account %s (%p)",
			                     buddy_name,
			                     purple_contact_info_get_username(info),
			                     account);
			g_free(buddy_name);
			return FALSE;
		} else {
			/* this is a different resource of our own account. Resume even when this account isn't on our blist */
		}
	}

	if (b && presence->vcard_avatar_hash) {
		const char *ah = presence->vcard_avatar_hash[0] != '\0' ?
				presence->vcard_avatar_hash : NULL;
		const char *ah2 = purple_buddy_icons_get_checksum_for_user(b);
		if (!purple_strequal(ah, ah2)) {
			/* XXX this is a crappy way of trying to prevent
			 * someone from spamming us with presence packets
			 * and causing us to DoS ourselves...what we really
			 * need is a queue system that can throttle itself,
			 * but i'm too tired to write that right now */
			if(!g_slist_find(js->pending_avatar_requests, presence->jb)) {
				JabberIq *iq;
				PurpleXmlNode *vcard;

				js->pending_avatar_requests =
					g_slist_prepend(js->pending_avatar_requests, presence->jb);

				iq = jabber_iq_new(js, JABBER_IQ_GET);
				purple_xmlnode_set_attrib(iq->node, "to", buddy_name);
				vcard = purple_xmlnode_new_child(iq->node, "vCard");
				purple_xmlnode_set_namespace(vcard, "vcard-temp");

				jabber_iq_set_callback(iq, jabber_vcard_parse_avatar, NULL);
				jabber_iq_send(iq);
			}
		}
	}

	if (presence->state == JABBER_BUDDY_STATE_ERROR ||
			presence->type == JABBER_PRESENCE_UNAVAILABLE ||
			presence->type == JABBER_PRESENCE_UNSUBSCRIBED) {
		jabber_buddy_remove_resource(presence->jb, presence->jid_from->resource);
	} else {
		jbr = jabber_buddy_track_resource(presence->jb,
				presence->jid_from->resource, presence->priority,
				presence->state, presence->status);
		jbr->idle = presence->idle ? time(NULL) - presence->idle : 0;
	}

	jbr = jabber_buddy_find_resource(presence->jb, NULL);
	if (jbr) {
		purple_protocol_got_user_status(account, buddy_name,
				jabber_buddy_state_get_status_id(jbr->state),
				"priority", jbr->priority,
				"message", jbr->status,
				NULL);
		purple_protocol_got_user_idle(account, buddy_name,
				jbr->idle, jbr->idle);
		if (presence->nickname)
			purple_serv_got_alias(js->gc, buddy_name, presence->nickname);
	} else {
		purple_protocol_got_user_status(account, buddy_name,
				jabber_buddy_state_get_status_id(JABBER_BUDDY_STATE_UNAVAILABLE),
				presence->status ? "message" : NULL, presence->status,
				NULL);
	}
	g_free(buddy_name);

	return TRUE;
}

void jabber_presence_parse(JabberStream *js, PurpleXmlNode *packet)
{
	const char *type;
	JabberBuddyResource *jbr = NULL;
	gboolean signal_return, ret;
	JabberPresence presence;
	PurpleXmlNode *child;

	memset(&presence, 0, sizeof(presence));
	/* defaults */
	presence.state = JABBER_BUDDY_STATE_UNKNOWN;
	presence.sent = g_date_time_new_now_utc();
	/* interesting values */
	presence.from = purple_xmlnode_get_attrib(packet, "from");
	presence.to   = purple_xmlnode_get_attrib(packet, "to");
	type = purple_xmlnode_get_attrib(packet, "type");
	presence.type = str_to_presence_type(type);

	presence.jb = jabber_buddy_find(js, presence.from, TRUE);
	g_return_if_fail(presence.jb != NULL);

	presence.jid_from = jabber_id_new(presence.from);
	if (presence.jid_from == NULL) {
		purple_debug_error("jabber", "Ignoring presence with malformed 'from' "
		                   "JID: %s\n", presence.from);
		return;
	}

	signal_return = GPOINTER_TO_INT(purple_signal_emit_return_1(purple_connection_get_protocol(js->gc),
			"jabber-receiving-presence", js->gc, type, presence.from, packet));
	if (signal_return) {
		goto out;
	}

	if (presence.jid_from->node)
		presence.chat = jabber_chat_find(js, presence.jid_from->node,
		                                 presence.jid_from->domain);
	g_free(presence.jb->error_msg);
	presence.jb->error_msg = NULL;

	if (presence.type == JABBER_PRESENCE_AVAILABLE) {
		presence.state = JABBER_BUDDY_STATE_ONLINE;
	} else if (presence.type == JABBER_PRESENCE_ERROR) {
		/* TODO: Is this handled properly?  Should it be treated as per-jbr? */
		char *msg = jabber_parse_error(js, packet, NULL);
		presence.state = JABBER_BUDDY_STATE_ERROR;
		presence.jb->error_msg = msg ? msg : g_strdup(_("Unknown Error in presence"));
	} else if (presence.type == JABBER_PRESENCE_SUBSCRIBE) {
		/* TODO: Move to handle_subscribe() (so nick is extracted by the
		 * PresenceHandler */
		struct _jabber_add_permit *jap = g_new0(struct _jabber_add_permit, 1);
		gboolean onlist = FALSE;
		PurpleAccount *account;
		PurpleAuthorizationRequest *request = NULL;
		PurpleNotification *notification = NULL;
		PurpleNotificationManager *manager = NULL;
		PurpleBuddy *buddy;
		PurpleXmlNode *nick;

		account = purple_connection_get_account(js->gc);
		buddy = purple_blist_find_buddy(account, presence.from);
		nick = purple_xmlnode_get_child_with_namespace(packet, "nick", "http://jabber.org/protocol/nick");
		if (nick)
			presence.nickname = purple_xmlnode_get_data(nick);

		if (buddy) {
			if ((presence.jb->subscription & (JABBER_SUB_TO | JABBER_SUB_PENDING)))
				onlist = TRUE;
		}

		jap->gc = js->gc;
		jap->who = g_strdup(presence.from);
		jap->js = js;

		request = purple_authorization_request_new(account, presence.from);
		purple_authorization_request_set_alias(request, presence.nickname);
		purple_authorization_request_set_add(request, !onlist);
		g_signal_connect(request, "accepted", G_CALLBACK(authorize_add_cb),
		                 jap);
		g_signal_connect(request, "denied", G_CALLBACK(deny_add_cb), jap);

		notification = purple_notification_new_from_authorization_request(request);
		manager = purple_notification_manager_get_default();
		purple_notification_manager_add(manager, notification);

		g_object_unref(notification);

		goto out;
	} else if (presence.type == JABBER_PRESENCE_SUBSCRIBED) {
		/* This case (someone has approved our subscribe request) is handled
		 * by the roster push the server sends along with this.
		 */
		goto out;
	} else if (presence.type == JABBER_PRESENCE_UNSUBSCRIBE) {
		/* XXX I'm not sure this is the right way to handle this, it
		 * might be better to add "unsubscribe" to the presence status
		 * if lower down, but I'm not sure. */
		/* they are unsubscribing from our presence, we don't care */
		/* Well, maybe just a little, we might want/need to start
		 * acknowledging this (and the others) at some point. */
		goto out;
	} else if (presence.type == JABBER_PRESENCE_PROBE) {
		purple_debug_warning("jabber", "Ignoring presence probe\n");
		goto out;
	} else if (presence.type == JABBER_PRESENCE_UNAVAILABLE) {
		presence.state = JABBER_BUDDY_STATE_UNAVAILABLE;
	} else if (presence.type == JABBER_PRESENCE_UNSUBSCRIBED) {
		presence.state = JABBER_BUDDY_STATE_UNKNOWN;
	} else {
		purple_debug_warning("jabber", "Ignoring presence with invalid type "
		                     "'%s'\n", type);
		goto out;
	}

	for (child = packet->child; child; child = child->next) {
		const char *xmlns;
		char *key;
		JabberPresenceHandler *pih;
		if (child->type != PURPLE_XMLNODE_TYPE_TAG)
			continue;

		xmlns = purple_xmlnode_get_namespace(child);
		key = g_strdup_printf("%s %s", child->name, xmlns ? xmlns : "");
		pih = g_hash_table_lookup(presence_handlers, key);
		g_free(key);
		if (pih)
			pih(js, &presence, child);
	}

	if (presence.delayed && presence.idle && presence.adjust_idle_for_delay) {
		GDateTime *now = g_date_time_new_now_utc();
		GTimeSpan difference = 0;

		difference = g_date_time_difference(now, presence.sent);

		g_date_time_unref(now);

		/* Delayed and idle, so update idle time */
		presence.idle = presence.idle + (difference / G_TIME_SPAN_SECOND);
	}

	/* TODO: Handle tracking jb(r) here? */

	if (presence.chat)
		ret = handle_presence_chat(js, &presence, packet);
	else
		ret = handle_presence_contact(js, &presence);
	if (!ret)
		goto out;

	if (presence.caps && presence.type == JABBER_PRESENCE_AVAILABLE) {
		/* handle Entity Capabilities (XEP-0115) */
		const char *node = purple_xmlnode_get_attrib(presence.caps, "node");
		const char *ver  = purple_xmlnode_get_attrib(presence.caps, "ver");
		const char *hash = purple_xmlnode_get_attrib(presence.caps, "hash");

		/* v1.5 uses: node, ver, and hash. */
		if(node != NULL && *node != '\0' &&
		   ver != NULL && *ver != '\0' &&
		   hash != NULL && *hash != '\0')
		{
			jbr = jabber_buddy_find_resource(presence.jb, presence.jid_from->resource);

			/* Look it up if we don't already have all this information */
			if(jbr == NULL || jbr->caps == NULL ||
			   !purple_strequal(node, jbr->caps->tuple.node) ||
			   !purple_strequal(ver, jbr->caps->tuple.ver) ||
			   !purple_strequal(hash, jbr->caps->tuple.hash))
			{
				JabberPresenceCapabilities *userdata = g_new0(JabberPresenceCapabilities, 1);
				userdata->js = js;
				userdata->jb = presence.jb;
				userdata->from = g_strdup(presence.from);
				jabber_caps_get_info(js, presence.from, node, ver, hash,
				    (jabber_caps_get_info_cb)jabber_presence_set_capabilities,
				    userdata);
			}
		}
	}

out:
	g_slist_free(presence.chat_info.codes);
	g_free(presence.status);
	g_free(presence.vcard_avatar_hash);
	g_free(presence.nickname);
	jabber_id_free(presence.jid_from);
	g_clear_pointer(&presence.sent, g_date_time_unref);
}

void jabber_presence_subscription_set(JabberStream *js, const char *who, const char *type)
{
	PurpleXmlNode *presence = purple_xmlnode_new("presence");

	purple_xmlnode_set_attrib(presence, "to", who);
	purple_xmlnode_set_attrib(presence, "type", type);

	jabber_send(js, presence);
	purple_xmlnode_free(presence);
}

void purple_status_to_jabber(PurpleStatus *status, JabberBuddyState *state, char **msg, int *priority)
{
	const char *status_id = NULL;
	const char *formatted_msg = NULL;

	if(state) *state = JABBER_BUDDY_STATE_UNKNOWN;
	if(msg) *msg = NULL;
	if(priority) *priority = 0;

	if(!status) {
		if(state) *state = JABBER_BUDDY_STATE_UNAVAILABLE;
	} else {
		if(state) {
			status_id = purple_status_get_id(status);
			*state = jabber_buddy_status_id_get_state(status_id);
		}

		if(msg) {
			formatted_msg = purple_status_get_attr_string(status, "message");

			/* if the message is blank, then there really isn't a message */
			if(formatted_msg && *formatted_msg)
				*msg = purple_markup_strip_html(formatted_msg);
		}

		if(priority)
			*priority = purple_status_get_attr_int(status, "priority");
	}
}

/* Incoming presence handlers */
static void
parse_priority(G_GNUC_UNUSED JabberStream *js, JabberPresence *presence,
               PurpleXmlNode *priority)
{
	char *p = purple_xmlnode_get_data(priority);

	if (presence->priority != 0)
		purple_debug_warning("jabber", "presence stanza received with multiple "
		                     "priority children!?\n");

	if (p) {
		presence->priority = atoi(p);
		g_free(p);
	} else
		purple_debug_warning("jabber", "Empty <priority/> in presence!\n");
}

static void
parse_show(G_GNUC_UNUSED JabberStream *js, JabberPresence *presence,
           PurpleXmlNode *show)
{
	char *cdata;

	if (presence->type != JABBER_PRESENCE_AVAILABLE) {
		purple_debug_warning("jabber", "<show/> present on presence, but "
		                               "type is not default ('available')\n");
		return;
	}

	cdata = purple_xmlnode_get_data(show);
	if (cdata) {
		presence->state = jabber_buddy_show_get_state(cdata);
		g_free(cdata);
	} else
		purple_debug_warning("jabber", "<show/> present on presence, but "
		                               "no contents!\n");
}

static void
parse_status(G_GNUC_UNUSED JabberStream *js, JabberPresence *presence,
             PurpleXmlNode *status)
{
	/* TODO: Check/track language attribute? */

	g_free(presence->status);
	presence->status = purple_xmlnode_get_data(status);
}

static void
parse_delay(G_GNUC_UNUSED JabberStream *js, JabberPresence *presence,
            PurpleXmlNode *delay)
{
	GTimeZone *tz = g_time_zone_new_utc();
	const char *stamp = purple_xmlnode_get_attrib(delay, "stamp");

	presence->delayed = TRUE;
	presence->sent = g_date_time_new_from_iso8601(stamp, tz);

	g_time_zone_unref(tz);
}

static void
parse_idle(G_GNUC_UNUSED JabberStream *js, JabberPresence *presence,
           PurpleXmlNode *query)
{
	const gchar *seconds = purple_xmlnode_get_attrib(query, "seconds");
	if (seconds) {
		presence->idle = atoi(seconds);
		presence->adjust_idle_for_delay = TRUE;
		if (presence->idle < 0) {
			purple_debug_warning("jabber", "Received bogus idle time %s\n", seconds);
			presence->idle = 0;
		}
	}
}

static void
parse_caps(G_GNUC_UNUSED JabberStream *js, JabberPresence *presence,
           PurpleXmlNode *c)
{
	/* TODO: Move the rest of the caps handling in here, after changing the
	 * the "do we have details about this (node, ver) and exts" to not
	 * require the jbr to be present (since that happens later).
	 */
	presence->caps = c;
}

static void
parse_nickname(G_GNUC_UNUSED JabberStream *js, JabberPresence *presence,
               PurpleXmlNode *nick)
{
	g_free(presence->nickname);
	presence->nickname = purple_xmlnode_get_data(nick);
}

static void
parse_vcard_avatar(G_GNUC_UNUSED JabberStream *js, JabberPresence *presence,
                   PurpleXmlNode *x)
{
	PurpleXmlNode *photo = purple_xmlnode_get_child(x, "photo");

	if (photo) {
		char *hash_tmp = purple_xmlnode_get_data(photo);
		g_free(presence->vcard_avatar_hash);
		presence->vcard_avatar_hash =
			hash_tmp ? hash_tmp : g_strdup("");
	}
}

static void
parse_muc_user(G_GNUC_UNUSED JabberStream *js, JabberPresence *presence,
               PurpleXmlNode *x)
{
	PurpleXmlNode *status;

	if (presence->chat == NULL) {
		purple_debug_warning("jabber", "Ignoring MUC gloop on non-MUC presence\n");
		return;
	}

	if (presence->chat->conv == NULL)
		presence->chat->muc = TRUE;

	for (status = purple_xmlnode_get_child(x, "status"); status;
			status = purple_xmlnode_get_next_twin(status)) {
		const char *code = purple_xmlnode_get_attrib(status, "code");
		int val;
		if (!code)
			continue;

		val = atoi(code);
		if (val == 0 || val < 0) {
			purple_debug_warning("jabber", "Ignoring bogus status code '%s'\n",
			                               code);
			continue;
		}

		presence->chat_info.codes = g_slist_prepend(presence->chat_info.codes, GINT_TO_POINTER(val));
	}

	presence->chat_info.item = purple_xmlnode_get_child(x, "item");
}

static void
jabber_presence_register_handler(const char *node, const char *xmlns,
                                 JabberPresenceHandler *handler)
{
	/*
	 * This is valid because nodes nor namespaces cannot have spaces in them
	 * (see http://www.w3.org/TR/2006/REC-xml-20060816/ and
	 * http://www.w3.org/TR/REC-xml-names/)
	 */
	char *key = g_strdup_printf("%s %s", node, xmlns);
	g_hash_table_replace(presence_handlers, key, handler);
}

void jabber_presence_init(void)
{
	presence_handlers = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

	/* Core RFC things */
	jabber_presence_register_handler("priority", "jabber:client", parse_priority);
	jabber_presence_register_handler("show", "jabber:client", parse_show);
	jabber_presence_register_handler("status", "jabber:client", parse_status);

	/* XEPs */
	jabber_presence_register_handler("c", "http://jabber.org/protocol/caps", parse_caps);
	jabber_presence_register_handler("delay", NS_DELAYED_DELIVERY, parse_delay);
	jabber_presence_register_handler("nick", "http://jabber.org/protocol/nick", parse_nickname);
	jabber_presence_register_handler("query", NS_LAST_ACTIVITY, parse_idle);
	jabber_presence_register_handler("x", NS_DELAYED_DELIVERY_LEGACY, parse_delay);
	jabber_presence_register_handler("x", "http://jabber.org/protocol/muc#user", parse_muc_user);
	jabber_presence_register_handler("x", "vcard-temp:x:update", parse_vcard_avatar);
}

void jabber_presence_uninit(void)
{
	g_hash_table_destroy(presence_handlers);
	presence_handlers = NULL;
}
