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

#include "adhoccommands.h"
#include "buddy.h"
#include "chat.h"
#include "data.h"
#include "message.h"
#include "pep.h"
#include "iq.h"

#include <string.h>

static GString *jm_body_with_oob(JabberMessage *jm) {
	GList *etc;
	GString *body = g_string_new("");

	if(jm->xhtml)
		g_string_append(body, jm->xhtml);
	else if(jm->body)
		g_string_append(body, jm->body);

	for(etc = jm->etc; etc; etc = etc->next) {
		PurpleXmlNode *x = etc->data;
		const char *xmlns = purple_xmlnode_get_namespace(x);
		if(purple_strequal(xmlns, NS_OOB_X_DATA)) {
			PurpleXmlNode *url, *desc;
			char *urltxt, *desctxt;

			url = purple_xmlnode_get_child(x, "url");
			desc = purple_xmlnode_get_child(x, "desc");

			if(!url)
				continue;

			urltxt = purple_xmlnode_get_data(url);
			desctxt = desc ? purple_xmlnode_get_data(desc) : urltxt;

			if(body->len && !purple_strequal(body->str, urltxt))
				g_string_append_printf(body, "<br/><a href='%s'>%s</a>",
						urltxt, desctxt);
			else
				g_string_printf(body, "<a href='%s'>%s</a>",
						urltxt, desctxt);

			g_free(urltxt);

			if(desctxt != urltxt)
				g_free(desctxt);
		}
	}

	return body;
}

void jabber_message_free(JabberMessage *jm)
{
	g_free(jm->from);
	g_free(jm->to);
	g_free(jm->id);
	g_free(jm->subject);
	g_free(jm->body);
	g_free(jm->xhtml);
	g_free(jm->password);
	g_free(jm->error);
	g_free(jm->thread_id);
	g_list_free(jm->etc);
	g_list_free(jm->eventitems);

	g_clear_pointer(&jm->sent, g_date_time_unref);

	g_free(jm);
}

static void handle_chat(JabberMessage *jm)
{
	const gchar *contact = jm->outgoing ? jm->to : jm->from;
	JabberID *jid = jabber_id_new(contact);

	PurpleConnection *gc;
	PurpleConversationManager *manager;
	PurpleAccount *account;
	PurpleMessageFlags flags = 0;
	JabberBuddy *jb;
	JabberBuddyResource *jbr;
	GString *body;

	if(!jid)
		return;

	manager = purple_conversation_manager_get_default();

	gc = jm->js->gc;
	account = purple_connection_get_account(gc);

	jb = jabber_buddy_find(jm->js, contact, TRUE);
	jbr = jabber_buddy_find_resource(jb, jid->resource);

	if (jbr && jm->chat_state != JM_STATE_NONE)
		jbr->chat_states = JABBER_CHAT_STATES_SUPPORTED;

	switch(jm->chat_state) {
		case JM_STATE_COMPOSING:
			purple_serv_got_typing(gc, contact, 0, PURPLE_IM_TYPING);
			break;
		case JM_STATE_PAUSED:
			purple_serv_got_typing(gc, contact, 0, PURPLE_IM_TYPED);
			break;
		case JM_STATE_GONE: {
			PurpleConversation *im = NULL;

			im = purple_conversation_manager_find_im(manager, account, contact);

			if (im && jid->node && jid->domain) {
				char buf[256];
				PurpleBuddy *buddy;

				g_snprintf(buf, sizeof(buf), "%s@%s", jid->node, jid->domain);

				if ((buddy = purple_blist_find_buddy(account, buf))) {
					const char *who;
					char *escaped;

					who = purple_buddy_get_alias(buddy);
					escaped = g_markup_escape_text(who, -1);

					g_snprintf(buf, sizeof(buf),
					           _("%s has left the conversation."), escaped);
					g_free(escaped);

					/* At some point when we restructure PurpleConversation,
					 * this should be able to be implemented by removing the
					 * user from the conversation like we do with chats now. */
					purple_conversation_write_system_message(im, buf, 0);
				}
			}
			purple_serv_got_typing_stopped(gc, contact);
			break;
		}
		default:
			purple_serv_got_typing_stopped(gc, contact);
	}

	body = jm_body_with_oob(jm);

	if(body && body->len) {
		if (jid->resource) {
			/*
			 * We received a message from a specific resource, so
			 * we probably want a reply to go to this specific
			 * resource (i.e. bind/lock the conversation to this
			 * resource).
			 *
			 * This works because purple_im_conversation_send gets the name
			 * from purple_conversation_get_name()
			 */
			PurpleConversation *im;

			im = purple_conversation_manager_find_im(manager, account, contact);
			if (im && !purple_strequal(contact,
					purple_conversation_get_name(im))) {
				purple_debug_info("jabber", "Binding conversation to %s\n",
				                  contact);
				purple_conversation_set_name(im, contact);
			}
		}

		if(jbr) {
			/* Treat SUPPORTED as a terminal with no escape :) */
			if (jbr->chat_states != JABBER_CHAT_STATES_SUPPORTED) {
				if (jm->chat_state != JM_STATE_NONE)
					jbr->chat_states = JABBER_CHAT_STATES_SUPPORTED;
				else
					jbr->chat_states = JABBER_CHAT_STATES_UNSUPPORTED;
			}

			g_free(jbr->thread_id);
			jbr->thread_id = g_strdup(jm->thread_id);
		}

		if(jm->forwarded) {
			flags |= PURPLE_MESSAGE_FORWARDED;
		}
		flags |= jm->outgoing ? PURPLE_MESSAGE_SEND : PURPLE_MESSAGE_RECV;

		purple_serv_got_im(gc, contact, body->str, flags,
		                   (time_t)g_date_time_to_unix(jm->sent));
	}

	jabber_id_free(jid);

	if(body)
		g_string_free(body, TRUE);
}

static void handle_headline(JabberMessage *jm)
{
	char *title;
	GString *body;

	if(!jm->xhtml && !jm->body)
		return; /* ignore headlines without any content */

	body = jm_body_with_oob(jm);
	title = g_strdup_printf(_("Message from %s"), jm->from);

	purple_notify_formatted(jm->js->gc, title, jm->subject ? jm->subject : title,
			NULL, body->str, NULL, NULL);

	g_free(title);
	g_string_free(body, TRUE);
}

static void handle_groupchat(JabberMessage *jm)
{
	JabberID *jid = jabber_id_new(jm->from);
	JabberChat *chat;
	PurpleMessageFlags messageFlags = 0;

	if(!jid)
		return;

	chat = jabber_chat_find(jm->js, jid->node, jid->domain);

	if(!chat)
		return;

	if(jm->subject) {
		purple_chat_conversation_set_topic(chat->conv, jid->resource,
				jm->subject);
		messageFlags |= PURPLE_MESSAGE_NO_LOG;
		if(!jm->xhtml && !jm->body) {
			char *msg, *tmp, *tmp2;
			tmp = g_markup_escape_text(jm->subject, -1);
			tmp2 = purple_markup_linkify(tmp);
			if(jid->resource)
				msg = g_strdup_printf(_("%s has set the topic to: %s"), jid->resource, tmp2);
			else
				msg = g_strdup_printf(_("The topic is: %s"), tmp2);
			purple_conversation_write_system_message(PURPLE_CONVERSATION(chat->conv),
				msg, messageFlags);
			g_free(tmp);
			g_free(tmp2);
			g_free(msg);
		}
	}

	if(jm->xhtml || jm->body) {
		if(jid->resource) {
			time_t sent = (time_t)g_date_time_to_unix(jm->sent);

			purple_serv_got_chat_in(jm->js->gc, chat->id, jid->resource,
							messageFlags | (jm->delayed ? PURPLE_MESSAGE_DELAYED : 0),
							jm->xhtml ? jm->xhtml : jm->body, sent);
		} else if(chat->muc) {
			purple_conversation_write_system_message(
				PURPLE_CONVERSATION(chat->conv),
				jm->xhtml ? jm->xhtml : jm->body, messageFlags);
		}
	}

	jabber_id_free(jid);
}

static void handle_groupchat_invite(JabberMessage *jm)
{
	GHashTable *components;
	JabberID *jid = jabber_id_new(jm->to);

	if(!jid)
		return;

	components = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

	g_hash_table_replace(components, "room", g_strdup(jid->node));
	g_hash_table_replace(components, "server", g_strdup(jid->domain));
	g_hash_table_replace(components, "handle", g_strdup(jm->js->user->node));
	g_hash_table_replace(components, "password", g_strdup(jm->password));

	jabber_id_free(jid);
	purple_serv_got_chat_invite(jm->js->gc, jm->to, jm->from, jm->body, components);
}

static void handle_error(JabberMessage *jm)
{
	char *buf;

	if(!jm->body)
		return;

	buf = g_strdup_printf(_("Message delivery to %s failed: %s"),
			jm->from, jm->error ? jm->error : "");

	purple_notify_formatted(jm->js->gc, _("XMPP Message Error"), _("XMPP Message Error"), buf,
			jm->xhtml ? jm->xhtml : jm->body, NULL, NULL);

	g_free(buf);
}

static gchar *
jabber_message_xml_to_string_strip_img_smileys(PurpleXmlNode *xhtml)
{
	gchar *markup = purple_xmlnode_to_str(xhtml, NULL);
	int len = strlen(markup);
	int pos = 0;
	GString *out = g_string_new(NULL);

	while (pos < len) {
		/* this is a bit cludgy, maybe there is a better way to do this...
		  we need to find all <img> tags within the XHTML and replace those
			tags with the value of their "alt" attributes */
		if (g_str_has_prefix(&(markup[pos]), "<img")) {
			PurpleXmlNode *img = NULL;
			int pos2 = pos;
			const gchar *src;

			for (; pos2 < len ; pos2++) {
				if (g_str_has_prefix(&(markup[pos2]), "/>")) {
					pos2 += 2;
					break;
				} else if (g_str_has_prefix(&(markup[pos2]), "</img>")) {
					pos2 += 5;
					break;
				}
			}

			/* note, if the above loop didn't find the end of the <img> tag,
			  it the parsed string will be until the end of the input string,
			  in which case purple_xmlnode_from_str will bail out and return NULL,
			  in this case the "if" statement below doesn't trigger and the
			  text is copied unchanged */
			img = purple_xmlnode_from_str(&(markup[pos]), pos2 - pos);
			src = purple_xmlnode_get_attrib(img, "src");

			if (g_str_has_prefix(src, "cid:")) {
				const gchar *alt = purple_xmlnode_get_attrib(img, "alt");
				/* if the "alt" attribute is empty, put the cid as smiley string */
				if (alt && alt[0] != '\0') {
					/* if the "alt" is the same as the CID, as Jabbim does,
					 this prevents linkification... */
					if (purple_email_is_valid(alt)) {
						gchar *safe_alt = g_strdup_printf("smiley:%s", alt);
						out = g_string_append(out, safe_alt);
						g_free(safe_alt);
					} else {
						gchar *alt_escaped = g_markup_escape_text(alt, -1);
						out = g_string_append(out, alt_escaped);
						g_free(alt_escaped);
					}
				} else {
					out = g_string_append(out, src);
				}
				pos += pos2 - pos;
			} else {
				out = g_string_append_c(out, markup[pos]);
				pos++;
			}

			purple_xmlnode_free(img);

		} else {
			out = g_string_append_c(out, markup[pos]);
			pos++;
		}
	}

	g_free(markup);
	return g_string_free(out, FALSE);
}

void jabber_message_parse(JabberStream *js, PurpleXmlNode *packet)
{
	JabberMessage *jm;
	const char *id, *from, *to, *type;
	PurpleXmlNode *child = NULL, *received = NULL;
	gboolean signal_return;
	gboolean delayed = FALSE, is_outgoing = FALSE, is_forwarded = FALSE;
	GDateTime *timestamp = g_date_time_new_now_utc();

	/* Check if we have a carbons received element from our own account. */
	from = purple_xmlnode_get_attrib(packet, "from");
	if(from != NULL && jabber_is_own_account(js, from)) {
		PurpleXmlNode *forwarded = NULL;

		/* We check if this is a received carbon first. */
		received = purple_xmlnode_get_child_with_namespace(packet, "received",
		                                                   NS_MESSAGE_CARBONS);
		if(received != NULL) {
			forwarded = purple_xmlnode_get_child_with_namespace(received,
			                                                    "forwarded",
			                                                    NS_FORWARD);
		} else {
			PurpleXmlNode *sent = NULL;

			sent = purple_xmlnode_get_child_with_namespace(packet, "sent",
			                                               NS_MESSAGE_CARBONS);
			if(sent != NULL) {
				forwarded = purple_xmlnode_get_child_with_namespace(sent,
				                                                    "forwarded",
				                                                    NS_FORWARD);
				is_outgoing = TRUE;
			}
		}

		if(forwarded != NULL) {
			PurpleXmlNode *fwd_msg = NULL;

			fwd_msg = purple_xmlnode_get_child_with_namespace(forwarded,
			                                                  "message",
			                                                  NS_XMPP_CLIENT);
			if(fwd_msg != NULL) {
				PurpleXmlNode *delay = NULL;

				/* We have a forwarded message, so update the packet to point
				 * to it directly.
				 */
				packet = fwd_msg;
				is_forwarded = TRUE;

				/* Now check if it was a delayed message and if so, grab the
				 * timestamp that the server sent.
				 */
				delay = purple_xmlnode_get_child_with_namespace(forwarded,
				                                                "delay",
				                                                NS_DELAYED_DELIVERY);
				if(delay != NULL) {
					GDateTime *delayed_ts = NULL;
					GTimeZone *tz = g_time_zone_new_utc();
					const gchar *ts = purple_xmlnode_get_attrib(delay,
					                                            "stamp");

					delayed_ts = g_date_time_new_from_iso8601(ts, tz);
					g_time_zone_unref(tz);

					if(delayed_ts != NULL) {
						delayed = TRUE;
						g_date_time_unref(timestamp);
						timestamp = delayed_ts;
					}
				}
			}
		}
	}

	/* If the message was forwarded, packet is now pointing to the forwarded
	 * message.
	 */
	from = purple_xmlnode_get_attrib(packet, "from");
	id = purple_xmlnode_get_attrib(packet, "id");
	to = purple_xmlnode_get_attrib(packet, "to");
	type = purple_xmlnode_get_attrib(packet, "type");

	signal_return = GPOINTER_TO_INT(purple_signal_emit_return_1(purple_connection_get_protocol(js->gc),
			"jabber-receiving-message", js->gc, type, id, from, to, packet));
	if (signal_return)
		return;

	jm = g_new0(JabberMessage, 1);
	jm->js = js;
	jm->sent = timestamp;
	jm->delayed = delayed;
	jm->chat_state = JM_STATE_NONE;
	jm->forwarded = is_forwarded;
	jm->outgoing = is_outgoing;

	if(type) {
		if(purple_strequal(type, "normal"))
			jm->type = JABBER_MESSAGE_NORMAL;
		else if(purple_strequal(type, "chat"))
			jm->type = JABBER_MESSAGE_CHAT;
		else if(purple_strequal(type, "groupchat"))
			jm->type = JABBER_MESSAGE_GROUPCHAT;
		else if(purple_strequal(type, "headline"))
			jm->type = JABBER_MESSAGE_HEADLINE;
		else if(purple_strequal(type, "error"))
			jm->type = JABBER_MESSAGE_ERROR;
		else
			jm->type = JABBER_MESSAGE_OTHER;
	} else {
		jm->type = JABBER_MESSAGE_NORMAL;
	}

	jm->from = g_strdup(from);
	jm->to   = g_strdup(to);
	jm->id   = g_strdup(id);

	for(child = packet->child; child; child = child->next) {
		const char *xmlns = purple_xmlnode_get_namespace(child);
		if(child->type != PURPLE_XMLNODE_TYPE_TAG)
			continue;

		if(purple_strequal(child->name, "error")) {
			const char *code = purple_xmlnode_get_attrib(child, "code");
			char *code_txt = NULL;
			char *text = purple_xmlnode_get_data(child);
			if (!text) {
				PurpleXmlNode *enclosed_text_node;

				if ((enclosed_text_node = purple_xmlnode_get_child(child, "text")))
					text = purple_xmlnode_get_data(enclosed_text_node);
			}

			if(code)
				code_txt = g_strdup_printf(_("(Code %s)"), code);

			if(!jm->error)
				jm->error = g_strdup_printf("%s%s%s",
						text ? text : "",
						text && code_txt ? " " : "",
						code_txt ? code_txt : "");

			g_free(code_txt);
			g_free(text);
		} else if (xmlns == NULL) {
			/* QuLogic: Not certain this is correct, but it would have happened
			   with the previous code. */
			if(purple_strequal(child->name, "x"))
				jm->etc = g_list_append(jm->etc, child);
			/* The following tests expect xmlns != NULL */
			continue;
		} else if(purple_strequal(child->name, "subject") && purple_strequal(xmlns, NS_XMPP_CLIENT)) {
			if(!jm->subject) {
				jm->subject = purple_xmlnode_get_data(child);
				if(!jm->subject)
					jm->subject = g_strdup("");
			}
		} else if(purple_strequal(child->name, "thread") && purple_strequal(xmlns, NS_XMPP_CLIENT)) {
			if(!jm->thread_id)
				jm->thread_id = purple_xmlnode_get_data(child);
		} else if(purple_strequal(child->name, "body") && purple_strequal(xmlns, NS_XMPP_CLIENT)) {
			if(!jm->body) {
				char *msg = purple_xmlnode_get_data(child);
				char *escaped = g_markup_escape_text(msg, -1);
				jm->body = purple_strdup_withhtml(escaped);
				g_free(escaped);
				g_free(msg);
			}
		} else if(purple_strequal(child->name, "html") && purple_strequal(xmlns, NS_XHTML_IM)) {
			if(!jm->xhtml && purple_xmlnode_get_child(child, "body")) {
				char *c;
				gchar *reformatted_xhtml;

				purple_xmlnode_strip_prefixes(child);

				/* reformat xhtml so that img tags with a "cid:" src gets
				  translated to the bare text of the emoticon (the "alt" attrib) */
				/* this is done also when custom smiley retrieval is turned off,
				  this way the receiver always sees the shortcut instead */
				reformatted_xhtml =
					jabber_message_xml_to_string_strip_img_smileys(child);

				jm->xhtml = reformatted_xhtml;

			    /* Convert all newlines to whitespace. Technically, even regular, non-XML HTML is supposed to ignore newlines, but Pidgin has, as convention
				 * treated \n as a newline for compatibility with other protocols
				 */
				for (c = jm->xhtml; *c != '\0'; c++) {
					if (*c == '\n')
						*c = ' ';
				}
			}
		} else if(purple_strequal(child->name, "active") && purple_strequal(xmlns,"http://jabber.org/protocol/chatstates")) {
			jm->chat_state = JM_STATE_ACTIVE;
		} else if(purple_strequal(child->name, "composing") && purple_strequal(xmlns,"http://jabber.org/protocol/chatstates")) {
			jm->chat_state = JM_STATE_COMPOSING;
		} else if(purple_strequal(child->name, "paused") && purple_strequal(xmlns,"http://jabber.org/protocol/chatstates")) {
			jm->chat_state = JM_STATE_PAUSED;
		} else if(purple_strequal(child->name, "inactive") && purple_strequal(xmlns,"http://jabber.org/protocol/chatstates")) {
			jm->chat_state = JM_STATE_INACTIVE;
		} else if(purple_strequal(child->name, "gone") && purple_strequal(xmlns,"http://jabber.org/protocol/chatstates")) {
			jm->chat_state = JM_STATE_GONE;
		} else if(purple_strequal(child->name, "event") && purple_strequal(xmlns,"http://jabber.org/protocol/pubsub#event")) {
			PurpleXmlNode *items;
			jm->type = JABBER_MESSAGE_EVENT;
			for(items = purple_xmlnode_get_child(child,"items"); items; items = items->next)
				jm->eventitems = g_list_append(jm->eventitems, items);
		} else if(purple_strequal(child->name, "delay") && purple_strequal(xmlns, NS_DELAYED_DELIVERY)) {
			const char *stamp = purple_xmlnode_get_attrib(child, "stamp");
			if(stamp != NULL) {
				GDateTime *delayed_ts = NULL;
				GTimeZone *tz = g_time_zone_new_utc();

				delayed_ts = g_date_time_new_from_iso8601(stamp, tz);
				g_time_zone_unref(tz);

				if(delayed_ts != NULL) {
					jm->delayed = TRUE;
					g_date_time_unref(jm->sent);
					jm->sent = delayed_ts;
				}
			}
		} else if(purple_strequal(child->name, "x")) {
			if(purple_strequal(xmlns, NS_DELAYED_DELIVERY_LEGACY)) {
				const char *stamp = purple_xmlnode_get_attrib(child, "stamp");

				if(stamp != NULL) {
					GDateTime *delayed_ts = NULL;
					GTimeZone *tz = g_time_zone_new_utc();

					delayed_ts = g_date_time_new_from_iso8601(stamp, tz);
					g_time_zone_unref(tz);

					if(delayed_ts != NULL) {
						jm->delayed = TRUE;
						g_date_time_unref(jm->sent);
						jm->sent = delayed_ts;
					}
				}
			} else if(purple_strequal(xmlns, "jabber:x:conference") &&
					jm->type != JABBER_MESSAGE_GROUPCHAT_INVITE &&
					jm->type != JABBER_MESSAGE_ERROR) {
				const char *jid = purple_xmlnode_get_attrib(child, "jid");
				if(jid) {
					const char *reason = purple_xmlnode_get_attrib(child, "reason");
					const char *password = purple_xmlnode_get_attrib(child, "password");

					jm->type = JABBER_MESSAGE_GROUPCHAT_INVITE;
					g_free(jm->to);
					jm->to = g_strdup(jid);

					if (reason) {
						g_free(jm->body);
						jm->body = g_strdup(reason);
					}

					if (password) {
						g_free(jm->password);
						jm->password = g_strdup(password);
					}
				}
			} else if(purple_strequal(xmlns, "http://jabber.org/protocol/muc#user") &&
					jm->type != JABBER_MESSAGE_ERROR) {
				PurpleXmlNode *invite = purple_xmlnode_get_child(child, "invite");
				if(invite) {
					PurpleXmlNode *reason, *password;
					const char *jid = purple_xmlnode_get_attrib(invite, "from");
					g_free(jm->to);
					jm->to = jm->from;
					jm->from = g_strdup(jid);
					if((reason = purple_xmlnode_get_child(invite, "reason"))) {
						g_free(jm->body);
						jm->body = purple_xmlnode_get_data(reason);
					}
					if((password = purple_xmlnode_get_child(child, "password"))) {
						g_free(jm->password);
						jm->password = purple_xmlnode_get_data(password);
					}

					jm->type = JABBER_MESSAGE_GROUPCHAT_INVITE;
				}
			} else {
				jm->etc = g_list_append(jm->etc, child);
			}
		} else if (purple_strequal(child->name, "query")) {
			const char *node = purple_xmlnode_get_attrib(child, "node");
			if (purple_strequal(xmlns, NS_DISCO_ITEMS)
					&& purple_strequal(node, "http://jabber.org/protocol/commands")) {
				jabber_adhoc_got_list(js, jm->from, child);
			}
		}
	}

	switch(jm->type) {
		case JABBER_MESSAGE_OTHER:
			purple_debug_info("jabber",
					"Received message of unknown type: %s\n", type);
			G_GNUC_FALLTHROUGH;
		case JABBER_MESSAGE_NORMAL:
		case JABBER_MESSAGE_CHAT:
			handle_chat(jm);
			break;
		case JABBER_MESSAGE_HEADLINE:
			handle_headline(jm);
			break;
		case JABBER_MESSAGE_GROUPCHAT:
			handle_groupchat(jm);
			break;
		case JABBER_MESSAGE_GROUPCHAT_INVITE:
			handle_groupchat_invite(jm);
			break;
		case JABBER_MESSAGE_EVENT:
			jabber_handle_event(jm);
			break;
		case JABBER_MESSAGE_ERROR:
			handle_error(jm);
			break;
	}
	jabber_message_free(jm);
}

void jabber_message_send(JabberMessage *jm)
{
	PurpleXmlNode *message, *child;
	const char *type = NULL;

	message = purple_xmlnode_new("message");

	switch(jm->type) {
		case JABBER_MESSAGE_NORMAL:
			type = "normal";
			break;
		case JABBER_MESSAGE_CHAT:
		case JABBER_MESSAGE_GROUPCHAT_INVITE:
			type = "chat";
			break;
		case JABBER_MESSAGE_HEADLINE:
			type = "headline";
			break;
		case JABBER_MESSAGE_GROUPCHAT:
			type = "groupchat";
			break;
		case JABBER_MESSAGE_ERROR:
			type = "error";
			break;
		case JABBER_MESSAGE_OTHER:
		default:
			type = NULL;
			break;
	}

	if(type)
		purple_xmlnode_set_attrib(message, "type", type);

	if (jm->id)
		purple_xmlnode_set_attrib(message, "id", jm->id);

	purple_xmlnode_set_attrib(message, "to", jm->to);

	if(jm->thread_id) {
		child = purple_xmlnode_new_child(message, "thread");
		purple_xmlnode_insert_data(child, jm->thread_id, -1);
	}

	child = NULL;
	switch(jm->chat_state)
	{
		case JM_STATE_ACTIVE:
			child = purple_xmlnode_new_child(message, "active");
			break;
		case JM_STATE_COMPOSING:
			child = purple_xmlnode_new_child(message, "composing");
			break;
		case JM_STATE_PAUSED:
			child = purple_xmlnode_new_child(message, "paused");
			break;
		case JM_STATE_INACTIVE:
			child = purple_xmlnode_new_child(message, "inactive");
			break;
		case JM_STATE_GONE:
			child = purple_xmlnode_new_child(message, "gone");
			break;
		case JM_STATE_NONE:
			/* yep, nothing */
			break;
	}
	if(child)
		purple_xmlnode_set_namespace(child, "http://jabber.org/protocol/chatstates");

	if(jm->subject) {
		child = purple_xmlnode_new_child(message, "subject");
		purple_xmlnode_insert_data(child, jm->subject, -1);
	}

	if(jm->body) {
		child = purple_xmlnode_new_child(message, "body");
		purple_xmlnode_insert_data(child, jm->body, -1);
	}

	if(jm->xhtml) {
		if ((child = purple_xmlnode_from_str(jm->xhtml, -1))) {
			purple_xmlnode_insert_child(message, child);
		} else {
			purple_debug_error("jabber",
					"XHTML translation/validation failed, returning: %s\n",
					jm->xhtml);
		}
	}

	jabber_send(jm->js, message);

	purple_xmlnode_free(message);
}

/*
 * Compare the XHTML and plain strings passed in for "equality". Any HTML markup
 * other than <br/> (matches a newline) in the XHTML will cause this to return
 * FALSE.
 */
static gboolean
jabber_xhtml_plain_equal(const char *xhtml_escaped, const char *plain)
{
	int i = 0;
	int j = 0;
	gboolean ret;
	char *xhtml = purple_unescape_html(xhtml_escaped);

	while (xhtml[i] && plain[j]) {
		if (xhtml[i] == plain[j]) {
			i += 1;
			j += 1;
			continue;
		}

		if (plain[j] == '\n' && !strncmp(xhtml+i, "<br/>", 5)) {
			i += 5;
			j += 1;
			continue;
		}

		g_free(xhtml);
		return FALSE;
	}

	/* Are we at the end of both strings? */
	ret = (xhtml[i] == plain[j]) && (xhtml[i] == '\0');
	g_free(xhtml);
	return ret;
}

int
jabber_message_send_im(G_GNUC_UNUSED PurpleProtocolIM *pim,
                       PurpleConnection *gc, PurpleMessage *msg)
{
	JabberMessage *jm;
	JabberBuddy *jb;
	JabberBuddyResource *jbr;
	char *xhtml;
	char *tmp;
	char *resource;
	const gchar *rcpt = purple_message_get_recipient(msg);

	if (!rcpt || purple_message_is_empty(msg))
		return 0;

	resource = jabber_get_resource(rcpt);

	jb = jabber_buddy_find(purple_connection_get_protocol_data(gc), rcpt, TRUE);
	jbr = jabber_buddy_find_resource(jb, resource);

	g_free(resource);

	jm = g_new0(JabberMessage, 1);
	jm->js = purple_connection_get_protocol_data(gc);
	jm->type = JABBER_MESSAGE_CHAT;
	jm->chat_state = JM_STATE_ACTIVE;
	jm->to = g_strdup(rcpt);
	jm->id = jabber_get_next_id(jm->js);

	if(jbr) {
		if(jbr->thread_id)
			jm->thread_id = jbr->thread_id;

		if (jbr->chat_states == JABBER_CHAT_STATES_UNSUPPORTED)
			jm->chat_state = JM_STATE_NONE;
		else {
			/* if(JABBER_CHAT_STATES_UNKNOWN == jbr->chat_states)
			   jbr->chat_states = JABBER_CHAT_STATES_UNSUPPORTED; */
		}
	}

	tmp = purple_utf8_strip_unprintables(purple_message_get_contents(msg));
	purple_markup_html_to_xhtml(tmp, &xhtml, &jm->body);
	g_free(tmp);

	/*
	 * For backward compatibility with user expectations or for those not on
	 * the user's roster, allow sending XHTML-IM markup.
	 */
	if (jbr == NULL || jbr->caps == NULL ||
			jabber_resource_has_capability(jbr, NS_XHTML_IM)) {
		if (!jabber_xhtml_plain_equal(xhtml, jm->body))
			/* Wrap the message in <p/> for great interoperability justice. */
			jm->xhtml = g_strdup_printf("<html xmlns='" NS_XHTML_IM "'><body xmlns='" NS_XHTML "'><p>%s</p></body></html>", xhtml);
	}

	g_free(xhtml);

	jabber_message_send(jm);
	jabber_message_free(jm);
	return 1;
}

gint
jabber_message_send_chat(G_GNUC_UNUSED PurpleProtocolChat *protocol_chat,
                         PurpleConnection *gc, gint id, PurpleMessage *msg)
{
	JabberChat *chat;
	JabberMessage *jm;
	JabberStream *js;
	char *xhtml;
	char *tmp;

	if (!gc || purple_message_is_empty(msg))
		return 0;

	js = purple_connection_get_protocol_data(gc);
	chat = jabber_chat_find_by_id(js, id);

	if(!chat)
		return 0;

	jm = g_new0(JabberMessage, 1);
	jm->js = purple_connection_get_protocol_data(gc);
	jm->type = JABBER_MESSAGE_GROUPCHAT;
	jm->to = g_strdup_printf("%s@%s", chat->room, chat->server);
	jm->id = jabber_get_next_id(jm->js);

	tmp = purple_utf8_strip_unprintables(purple_message_get_contents(msg));
	purple_markup_html_to_xhtml(tmp, &xhtml, &jm->body);
	g_free(tmp);

	if (chat->xhtml && !jabber_xhtml_plain_equal(xhtml, jm->body))
		/* Wrap the message in <p/> for greater interoperability justice. */
		jm->xhtml = g_strdup_printf("<html xmlns='" NS_XHTML_IM "'><body xmlns='" NS_XHTML "'><p>%s</p></body></html>", xhtml);

	g_free(xhtml);

	jabber_message_send(jm);
	jabber_message_free(jm);

	return 1;
}

unsigned int
jabber_send_typing(G_GNUC_UNUSED PurpleProtocolIM *pim, PurpleConnection *gc,
                   const char *who, PurpleIMTypingState state)
{
	JabberStream *js;
	JabberMessage *jm;
	JabberBuddy *jb;
	JabberBuddyResource *jbr;
	char *resource;

	js = purple_connection_get_protocol_data(gc);
	jb = jabber_buddy_find(js, who, TRUE);
	if (!jb)
		return 0;

	resource = jabber_get_resource(who);
	jbr = jabber_buddy_find_resource(jb, resource);
	g_free(resource);

	/* We know this entity doesn't support chat states */
	if (jbr && jbr->chat_states == JABBER_CHAT_STATES_UNSUPPORTED)
		return 0;

	/* *If* we don't have presence /and/ the buddy can't see our
	 * presence, don't send typing notifications.
	 */
	if (!jbr && !(jb->subscription & JABBER_SUB_FROM))
		return 0;

	/* TODO: figure out threading */
	jm = g_new0(JabberMessage, 1);
	jm->js = js;
	jm->type = JABBER_MESSAGE_CHAT;
	jm->to = g_strdup(who);
	jm->id = jabber_get_next_id(jm->js);

	if(PURPLE_IM_TYPING == state)
		jm->chat_state = JM_STATE_COMPOSING;
	else if(PURPLE_IM_TYPED == state)
		jm->chat_state = JM_STATE_PAUSED;
	else
		jm->chat_state = JM_STATE_ACTIVE;

	/* if(JABBER_CHAT_STATES_UNKNOWN == jbr->chat_states)
		jbr->chat_states = JABBER_CHAT_STATES_UNSUPPORTED; */

	jabber_message_send(jm);
	jabber_message_free(jm);

	return 0;
}
