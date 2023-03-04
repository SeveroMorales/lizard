/**
 * purple
 *
 * Copyright (C) 2003, 2012 Ethan Blanton <elb@pidgin.im>
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

/*
 * Note: If you change any of these functions to use additional args you
 * MUST ensure the arg count is correct in parse.c. Otherwise it may be
 * possible for a malicious server or man-in-the-middle to trigger a crash.
 */

#include <glib/gi18n-lib.h>

#include <ctype.h>

#include <purple.h>

#include "irc.h"

#include <stdio.h>
#include <stdlib.h>

#include <hasl.h>

static char *irc_mask_nick(const char *mask);
static char *irc_mask_userhost(const char *mask);
static void irc_chat_remove_buddy(PurpleChatConversation *chat, char *data[2]);
static void irc_buddy_status(char *name, struct irc_buddy *ib, struct irc_conn *irc);
static void irc_connected(struct irc_conn *irc, const char *nick);

static void irc_msg_handle_privmsg(struct irc_conn *irc, const char *name,
                                   const char *from, const char *to,
                                   const char *rawmsg, gboolean notice);

static void irc_sasl_finish(struct irc_conn *irc);

static char *irc_mask_nick(const char *mask)
{
	char *end, *buf;

	end = strchr(mask, '!');
	if (!end)
		buf = g_strdup(mask);
	else
		buf = g_strndup(mask, end - mask);

	return buf;
}

static char *irc_mask_userhost(const char *mask)
{
	return g_strdup(strchr(mask, '!') + 1);
}

static void irc_chat_remove_buddy(PurpleChatConversation *chat, char *data[2])
{
	char *message, *stripped;

	stripped = data[1] ? irc_mirc2txt(data[1]) : NULL;
	message = g_strdup_printf("quit: %s", stripped);
	g_free(stripped);

	if (purple_chat_conversation_has_user(chat, data[0]))
		purple_chat_conversation_remove_user(chat, data[0], message);

	g_free(message);
}

static void irc_connected(struct irc_conn *irc, const char *nick)
{
	PurpleConnection *gc;
	PurpleStatus *status;
	GSList *buddies;
	PurpleAccount *account;

	if ((gc = purple_account_get_connection(irc->account)) == NULL
	    || PURPLE_CONNECTION_IS_CONNECTED(gc))
		return;

	purple_connection_set_display_name(gc, nick);
	purple_connection_set_state(gc, PURPLE_CONNECTION_STATE_CONNECTED);
	account = purple_connection_get_account(gc);

	/* If we're away then set our away message */
	status = purple_account_get_active_status(irc->account);
	if (purple_status_type_get_primitive(purple_status_get_status_type(status)) != PURPLE_STATUS_AVAILABLE) {
		PurpleProtocol *protocol = purple_connection_get_protocol(gc);
		purple_protocol_server_set_status(PURPLE_PROTOCOL_SERVER(protocol), irc->account, status);
	}

	/* this used to be in the core, but it's not now */
	for (buddies = purple_blist_find_buddies(account, NULL); buddies;
			buddies = g_slist_delete_link(buddies, buddies))
	{
		PurpleBuddy *b = buddies->data;
		struct irc_buddy *ib = g_new0(struct irc_buddy, 1);
		ib->name = g_strdup(purple_buddy_get_name(b));
		ib->ref = 1;
		g_hash_table_replace(irc->buddies, ib->name, ib);
	}

	irc_blist_timeout(irc);
	if (!irc->timer)
		irc->timer = g_timeout_add_seconds(45, (GSourceFunc)irc_blist_timeout, (gpointer)irc);
}

/* This function is ugly, but it's really an error handler. */
void
irc_msg_default(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                G_GNUC_UNUSED const char *from, char **args)
{
	int i;
	const char *end, *cur, *numeric = NULL;
	char *clean, *tmp, *convname;
	PurpleConversation *convo;
	PurpleConversationManager *manager;

	for (cur = args[0], i = 0; i < 4; i++) {
		end = strchr(cur, ' ');
		if (end == NULL) {
			goto undirected;
		}
		/* Check for 3-digit numeric in second position */
		if (i == 1) {
			if (end - cur != 3
			    || !isdigit(cur[0]) || !isdigit(cur[1])
			    || !isdigit(cur[2])) {
				goto undirected;
			}
			/* Save the numeric for printing to the channel */
			numeric = cur;
		}
		/* Don't advance cur if we're on the final iteration. */
		if (i != 3) {
			cur = end + 1;
		}
	}

	/* At this point, cur is the beginning of the fourth position,
	 * end is the following space, and there are remaining
	 * arguments.  We'll check to see if this argument is a
	 * currently active conversation (private message or channel,
	 * either one), and print the numeric to that conversation if it
	 * is. */

	tmp = g_strndup(cur, end - cur);
	convname = g_utf8_make_valid(tmp, -1);
	g_free(tmp);

	/* Check for an existing conversation */
	manager = purple_conversation_manager_get_default();
	convo = purple_conversation_manager_find(manager, irc->account, convname);
	g_free(convname);

	if (convo == NULL) {
		goto undirected;
	}

	/* end + 1 is the first argument past the target.  The initial
	 * arguments we've skipped are routing info, numeric, recipient
	 * (this account's nick, most likely), and target (this
	 * channel).  If end + 1 is an ASCII :, skip it, because it's
	 * meaningless in this context.  This won't catch all
	 * :-arguments, but it'll catch the easy case. */
	if (*++end == ':') {
		end++;
	}

	/* We then print "numeric: remainder". */
	clean = g_utf8_make_valid(end, -1);
	tmp = g_strdup_printf("%.3s: %s", numeric, clean);
	g_free(clean);
	purple_conversation_write_system_message(convo, tmp,
		PURPLE_MESSAGE_NO_LOG | PURPLE_MESSAGE_RAW |
		PURPLE_MESSAGE_NO_LINKIFY);
	g_free(tmp);
	return;

  undirected:
	/* This, too, should be escaped somehow (smarter) */
	clean = g_utf8_make_valid(args[0], -1);
	purple_debug_info("irc", "Unrecognized message: %s", clean);
	g_free(clean);
}

void
irc_msg_features(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                 G_GNUC_UNUSED const char *from, char **args)
{
	gchar **features;
	int i;

	features = g_strsplit(args[1], " ", -1);
	for (i = 0; features[i]; i++) {
		char *val;
		if (!strncmp(features[i], "PREFIX=", 7)) {
			if ((val = strchr(features[i] + 7, ')')) != NULL)
				irc->mode_chars = g_strdup(val + 1);
		}
	}

	g_strfreev(features);
}

void
irc_msg_luser(struct irc_conn *irc, const char *name,
              G_GNUC_UNUSED const char *from, char **args)
{
	if (purple_strequal(name, "251")) {
		/* 251 is required, so we pluck our nick from here and
		 * finalize connection */
		irc_connected(irc, args[0]);
		/* Some IRC servers seem to not send a 255 numeric, so
		 * I guess we can't require it; 251 will do. */
	/* } else if (purple_strequal(name, "255")) { */
	}
}

void irc_msg_away(struct irc_conn *irc, const char *name, const char *from, char **args)
{
	PurpleConnection *gc;
	char *msg;

	if (irc->whois.nick && !purple_utf8_strcasecmp(irc->whois.nick, args[1])) {
		/* We're doing a whois, show this in the whois dialog */
		irc_msg_whois(irc, name, from, args);
		return;
	}

	gc = purple_account_get_connection(irc->account);
	if (gc) {
		msg = g_markup_escape_text(args[2], -1);
		purple_serv_got_im(gc, args[1], msg, PURPLE_MESSAGE_AUTO_RESP, time(NULL));
		g_free(msg);
	}
}

void
irc_msg_badmode(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);

	g_return_if_fail(gc);

	purple_notify_error(gc, NULL, _("Bad mode"), args[1],
		purple_request_cpar_from_connection(gc));
}

void
irc_msg_ban(struct irc_conn *irc, const char *name,
            G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConversation *chat;
	PurpleConversationManager *manager;

	manager = purple_conversation_manager_get_default();
	chat = purple_conversation_manager_find_chat(manager, irc->account,
	                                             args[1]);

	if (purple_strequal(name, "367")) {
		char *msg = NULL;
		/* Ban list entry */
		if (args[3] && args[4]) {
			/* This is an extended syntax, not in RFC 1459 */
			int t1 = atoi(args[4]);
			time_t t2 = time(NULL);
			char *time = purple_str_seconds_to_string(t2 - t1);
			msg = g_strdup_printf(_("Ban on %s by %s, set %s ago"),
			                      args[2], args[3], time);
			g_free(time);
		} else {
			msg = g_strdup_printf(_("Ban on %s"), args[2]);
		}
		if (chat) {
			purple_conversation_write_system_message(
				chat, msg, PURPLE_MESSAGE_NO_LOG);
		} else {
			purple_debug_info("irc", "%s\n", msg);
		}
		g_free(msg);
	} else if (purple_strequal(name, "368")) {
		if (!chat)
			return;
		/* End of ban list */
		purple_conversation_write_system_message(chat,
			_("End of ban list"), PURPLE_MESSAGE_NO_LOG);
	}
}

void
irc_msg_banned(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
               G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);
	char *buf;

	g_return_if_fail(gc);

	buf = g_strdup_printf(_("You are banned from %s."), args[1]);
	purple_notify_error(gc, _("Banned"), _("Banned"), buf,
		purple_request_cpar_from_connection(gc));
	g_free(buf);
}

void
irc_msg_banfull(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConversation *chat;
	PurpleConversationManager *manager;
	char *buf, *nick;

	manager = purple_conversation_manager_get_default();
	chat = purple_conversation_manager_find_chat(manager, irc->account,
	                                             args[1]);
	if (!chat) {
		return;
	}

	nick = g_markup_escape_text(args[2], -1);
	buf = g_strdup_printf(_("Cannot ban %s: banlist is full"), nick);
	g_free(nick);
	purple_conversation_write_system_message(chat, buf, PURPLE_MESSAGE_NO_LOG);
	g_free(buf);
}

void
irc_msg_chanmode(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                 G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConversation *chat;
	PurpleConversationManager *manager;
	char *buf, *escaped;

	manager = purple_conversation_manager_get_default();
	chat = purple_conversation_manager_find_chat(manager, irc->account,
	                                             args[1]);
	if (!chat) { /* XXX punt on channels we are not in for now */
		return;
	}

	escaped = (args[3] != NULL) ? g_markup_escape_text(args[3], -1) : NULL;
	buf = g_strdup_printf("mode for %s: %s %s", args[1], args[2], escaped ? escaped : "");
	purple_conversation_write_system_message(chat, buf, 0);
	g_free(escaped);
	g_free(buf);
}

void
irc_msg_whois(struct irc_conn *irc, const char *name,
              G_GNUC_UNUSED const char *from, char **args)
{
	if (!irc->whois.nick) {
		purple_debug_warning("irc", "Unexpected %s reply for %s",
		                     purple_strequal(name, "314") ? "WHOWAS" : "WHOIS", args[1]);
		return;
	}

	if (purple_utf8_strcasecmp(irc->whois.nick, args[1])) {
		purple_debug_warning("irc", "Got %s reply for %s while waiting for %s",
		                     purple_strequal(name, "314") ? "WHOWAS" : "WHOIS", args[1], irc->whois.nick);
		return;
	}

	if (purple_strequal(name, "301")) {
		irc->whois.away = g_strdup(args[2]);
	} else if (purple_strequal(name, "311") || purple_strequal(name, "314")) {
		irc->whois.ident = g_strdup(args[2]);
		irc->whois.host = g_strdup(args[3]);
		irc->whois.real = g_strdup(args[5]);
	} else if (purple_strequal(name, "312")) {
		irc->whois.server = g_strdup(args[2]);
		irc->whois.serverinfo = g_strdup(args[3]);
	} else if (purple_strequal(name, "313")) {
		irc->whois.ircop = 1;
	} else if (purple_strequal(name, "317")) {
		irc->whois.idle = atoi(args[2]);
		if (args[3])
			irc->whois.signon = (time_t)atoi(args[3]);
	} else if (purple_strequal(name, "319")) {
		if (irc->whois.channels == NULL) {
			irc->whois.channels = g_string_new(args[2]);
		} else {
			irc->whois.channels = g_string_append(irc->whois.channels, args[2]);
		}
	} else if (purple_strequal(name, "320")) {
		irc->whois.identified = 1;
	} else if (purple_strequal(name, "330")) {
		purple_debug_info("irc", "330 %s: 1=[%s] 2=[%s] 3=[%s]",
		                  name, args[1], args[2], args[3]);
		if (purple_strequal(args[3], "is logged in as"))
			irc->whois.login = g_strdup(args[2]);
	}
}

void
irc_msg_endwhois(struct irc_conn *irc, const char *name,
                 G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConnection *gc;
	char *tmp, *tmp2;
	PurpleNotifyUserInfo *user_info;

	if (!irc->whois.nick) {
		purple_debug_warning("irc", "Unexpected End of %s for %s",
		                     purple_strequal(name, "369") ? "WHOWAS" : "WHOIS", args[1]);
		return;
	}
	if (purple_utf8_strcasecmp(irc->whois.nick, args[1])) {
		purple_debug_warning("irc", "Received end of %s for %s, expecting %s",
		                     purple_strequal(name, "369") ? "WHOWAS" : "WHOIS", args[1], irc->whois.nick);
		return;
	}

	user_info = purple_notify_user_info_new();

	tmp2 = g_markup_escape_text(args[1], -1);
	tmp = g_strdup_printf("%s%s%s", tmp2,
				(irc->whois.ircop ? _(" <i>(ircop)</i>") : ""),
				(irc->whois.identified ? _(" <i>(identified)</i>") : ""));
	purple_notify_user_info_add_pair_html(user_info, _("Nick"), tmp);
	g_free(tmp2);
	g_free(tmp);

	if (irc->whois.away) {
		purple_notify_user_info_add_pair_plaintext(user_info, _("Away"), irc->whois.away);
		g_free(irc->whois.away);
	}
	if (irc->whois.real) {
		purple_notify_user_info_add_pair_plaintext(user_info, _("Real name"), irc->whois.real);
		g_free(irc->whois.real);
	}
	if (irc->whois.login) {
		purple_notify_user_info_add_pair_plaintext(user_info, _("Login name"), irc->whois.login);
		g_free(irc->whois.login);
	}
	if (irc->whois.ident) {
		purple_notify_user_info_add_pair_plaintext(user_info, _("Ident name"), irc->whois.ident);
		g_free(irc->whois.ident);
	}
	if (irc->whois.host) {
		purple_notify_user_info_add_pair_plaintext(user_info, _("Host name"), irc->whois.host);
		g_free(irc->whois.host);
	}
	if (irc->whois.server) {
		tmp = g_strdup_printf("%s (%s)", irc->whois.server, irc->whois.serverinfo);
		purple_notify_user_info_add_pair_plaintext(user_info, _("Server"), tmp);
		g_free(tmp);
		g_free(irc->whois.server);
		g_free(irc->whois.serverinfo);
	}
	if (irc->whois.channels) {
		purple_notify_user_info_add_pair_plaintext(user_info, _("Currently on"), irc->whois.channels->str);
		g_string_free(irc->whois.channels, TRUE);
	}
	if (irc->whois.idle) {
		GDateTime *signon = NULL;

		tmp = purple_str_seconds_to_string(irc->whois.idle);
		purple_notify_user_info_add_pair_plaintext(user_info, _("Idle for"),
		                                           tmp);
		g_free(tmp);

		signon = g_date_time_new_from_unix_local(irc->whois.signon);
		tmp = g_date_time_format(signon, "%c");
		purple_notify_user_info_add_pair_plaintext(user_info,
		                                           _("Online since"), tmp);
		g_free(tmp);
		g_date_time_unref(signon);
	}
	if (purple_strequal(irc->whois.nick, "elb")) {
		purple_notify_user_info_add_pair_plaintext(user_info,
																   _("<b>Defining adjective:</b>"), _("Glorious"));
	}

	gc = purple_account_get_connection(irc->account);

	purple_notify_userinfo(gc, irc->whois.nick, user_info, NULL, NULL);
	purple_notify_user_info_destroy(user_info);

	g_free(irc->whois.nick);
	memset(&irc->whois, 0, sizeof(irc->whois));
}

void
irc_msg_who(struct irc_conn *irc, const char *name,
            G_GNUC_UNUSED const char *from, char **args)
{
	if (purple_strequal(name, "352")) {
		PurpleConversation *chat;
		PurpleConversationManager *manager;
		PurpleChatUser *cb;

		char *cur, *userhost, *realname;

		PurpleChatUserFlags flags;

		manager = purple_conversation_manager_get_default();
		chat = purple_conversation_manager_find_chat(manager, irc->account,
		                                             args[1]);
		if (!chat) {
			purple_debug_error("irc", "Got a WHO response for %s, which doesn't exist", args[1]);
			return;
		}

		cb = purple_chat_conversation_find_user(PURPLE_CHAT_CONVERSATION(chat), args[5]);
		if (!cb) {
			purple_debug_error("irc", "Got a WHO response for %s who isn't a buddy.", args[5]);
			return;
		}

		userhost = g_strdup_printf("%s@%s", args[2], args[3]);

		/* The final argument is a :-argument, but annoyingly
		 * contains two "words", the hop count and real name. */
		for (cur = args[7]; *cur; cur++) {
			if (*cur == ' ') {
				cur++;
				break;
			}
		}
		realname = g_strdup(cur);

		g_object_set_data_full(G_OBJECT(cb), "userhost", userhost, g_free);
		g_object_set_data_full(G_OBJECT(cb), "realname", realname, g_free);

		flags = purple_chat_user_get_flags(cb);

		/* FIXME: I'm not sure this is really a good idea, now
		 * that we no longer do periodic WHO.  It seems to me
		 * like it's more likely to be confusing than not.
		 * Comments? */
		if (args[6][0] == 'G' && !(flags & PURPLE_CHAT_USER_AWAY)) {
			purple_chat_user_set_flags(cb, flags | PURPLE_CHAT_USER_AWAY);
		} else if(args[6][0] == 'H' && (flags & PURPLE_CHAT_USER_AWAY)) {
			purple_chat_user_set_flags(cb, flags & ~PURPLE_CHAT_USER_AWAY);
		}
	}
}

void
irc_msg_list(struct irc_conn *irc, const char *name,
             G_GNUC_UNUSED const char *from, char **args)
{
	if (!irc->roomlist)
		return;

	if (purple_strequal(name, "321")) {
		purple_roomlist_set_in_progress(irc->roomlist, TRUE);
		return;
	}

	if (purple_strequal(name, "323")) {
		purple_roomlist_set_in_progress(irc->roomlist, FALSE);
		g_object_unref(irc->roomlist);
		irc->roomlist = NULL;
		return;
	}

	if (purple_strequal(name, "322")) {
		PurpleRoomlistRoom *room;
		char *topic;

		if (!purple_roomlist_get_in_progress(irc->roomlist)) {
			purple_debug_warning("irc", "Buggy server didn't send RPL_LISTSTART.\n");
			purple_roomlist_set_in_progress(irc->roomlist, TRUE);
		}

		topic = irc_mirc2txt(args[3]);
		room = purple_roomlist_room_new(args[1], topic);
		g_free(topic);

		purple_roomlist_room_set_user_count(room, strtol(args[2], NULL, 10));
		purple_roomlist_room_add_field(room, "channel", args[1]);
		purple_roomlist_room_add(irc->roomlist, room);
		g_object_unref(room);
	}
}

void irc_msg_topic(struct irc_conn *irc, const char *name, const char *from, char **args)
{
	char *chan, *topic, *msg, *nick, *tmp, *tmp2;
	PurpleConversation *chat;
	PurpleConversationManager *manager;

	if (purple_strequal(name, "topic")) {
		chan = args[0];
		topic = irc_mirc2txt (args[1]);
	} else {
		chan = args[1];
		topic = irc_mirc2txt (args[2]);
	}

	manager = purple_conversation_manager_get_default();
	chat = purple_conversation_manager_find_chat(manager, irc->account, chan);
	if (!chat) {
		purple_debug_error("irc", "Got a topic for %s, which doesn't exist", chan);
		g_free(topic);
		return;
	}

	/* If this is an interactive update, print it out */
	tmp = g_markup_escape_text(topic, -1);
	tmp2 = purple_markup_linkify(tmp);
	g_free(tmp);
	if (purple_strequal(name, "topic")) {
		const char *current_topic = purple_chat_conversation_get_topic(PURPLE_CHAT_CONVERSATION(chat));
		if (!(current_topic != NULL && purple_strequal(tmp2, current_topic)))
		{
			char *nick_esc;
			nick = irc_mask_nick(from);
			nick_esc = g_markup_escape_text(nick, -1);
			purple_chat_conversation_set_topic(PURPLE_CHAT_CONVERSATION(chat), nick, topic);
			if (*tmp2)
				msg = g_strdup_printf(_("%s has changed the topic to: %s"), nick_esc, tmp2);
			else
				msg = g_strdup_printf(_("%s has cleared the topic."), nick_esc);
			g_free(nick_esc);
			g_free(nick);
			purple_conversation_write_system_message(
				chat, msg, 0);
			g_free(msg);
		}
	} else {
		char *chan_esc = g_markup_escape_text(chan, -1);
		msg = g_strdup_printf(_("The topic for %s is: %s"), chan_esc, tmp2);
		g_free(chan_esc);
		purple_chat_conversation_set_topic(PURPLE_CHAT_CONVERSATION(chat), NULL, topic);
		purple_conversation_write_system_message(chat, msg, 0);
		g_free(msg);
	}
	g_free(tmp2);
	g_free(topic);
}

void
irc_msg_topicinfo(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                  G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConversation *chat;
	PurpleConversationManager *manager;
	GDateTime *dt, *local;
	gint64 mtime;
	char *msg, *timestamp, *datestamp;

	manager = purple_conversation_manager_get_default();
	chat = purple_conversation_manager_find_chat(manager, irc->account,
	                                             args[1]);
	if (!chat) {
		purple_debug_error("irc", "Got topic info for %s, which doesn't exist", args[1]);
		return;
	}

	mtime = g_ascii_strtoll(args[3], NULL, 10);
	if(mtime == 0 || mtime == G_MININT64 || mtime == G_MAXINT64) {
		purple_debug_error("irc", "Got apparently nonsensical topic timestamp %s", args[3]);
		return;
	}

	dt = g_date_time_new_from_unix_utc(mtime);
	if(dt == NULL) {
		purple_debug_error("irc", "Failed to turn %" G_GINT64_FORMAT " into a GDateTime", mtime);
		return;
	}

	local = g_date_time_to_local(dt);
	g_date_time_unref(dt);

	timestamp = g_date_time_format(local, "%X");
	datestamp = g_date_time_format(local, "%x");
	msg = g_strdup_printf(_("Topic for %s set by %s at %s on %s"), args[1], args[2], timestamp, datestamp);
	purple_conversation_write_system_message(chat,
		msg, PURPLE_MESSAGE_NO_LINKIFY);
	g_free(timestamp);
	g_free(datestamp);
	g_free(msg);
	g_date_time_unref(local);
}

void
irc_msg_unknown(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);
	char *buf;

	g_return_if_fail(gc);

	buf = g_strdup_printf(_("Unknown message '%s'"), args[1]);
	purple_notify_error(gc, _("Unknown message"), buf, _("The IRC server "
		"received a message it did not understand."),
		purple_request_cpar_from_connection(gc));
	g_free(buf);
}

void
irc_msg_names(struct irc_conn *irc, const char *name,
              G_GNUC_UNUSED const char *from, char **args)
{
	char *names, *cur, *end, *tmp, *msg;

	if (purple_strequal(name, "366")) {
		PurpleConversation *convo;
		PurpleConversationManager *manager;

		manager = purple_conversation_manager_get_default();
		convo = purple_conversation_manager_find(manager, irc->account,
		                                         args[1]);
		if (!convo) {
			purple_debug_error("irc", "Got a NAMES list for %s, which doesn't exist", args[1]);
			g_string_free(irc->names, TRUE);
			irc->names = NULL;
			return;
		}

		names = cur = g_string_free(irc->names, FALSE);
		irc->names = NULL;
		if (g_object_get_data(G_OBJECT(convo), IRC_NAMES_FLAG)) {
			msg = g_strdup_printf(_("Users on %s: %s"), args[1], names ? names : "");
			purple_conversation_write_system_message(convo, msg, PURPLE_MESSAGE_NO_LOG);
			g_free(msg);
		} else if (cur != NULL) {
			GList *users = NULL;
			GList *flags = NULL;

			while (*cur) {
				PurpleChatUserFlags f = PURPLE_CHAT_USER_NONE;
				end = strchr(cur, ' ');
				if (!end)
					end = cur + strlen(cur);
				if (*cur == '@') {
					f = PURPLE_CHAT_USER_OP;
					cur++;
				} else if (*cur == '%') {
					f = PURPLE_CHAT_USER_HALFOP;
					cur++;
				} else if(*cur == '+') {
					f = PURPLE_CHAT_USER_VOICE;
					cur++;
				} else if(irc->mode_chars
					  && strchr(irc->mode_chars, *cur)) {
					if (*cur == '~')
						f = PURPLE_CHAT_USER_FOUNDER;
					cur++;
				}
				tmp = g_strndup(cur, end - cur);
				users = g_list_prepend(users, tmp);
				flags = g_list_prepend(flags, GINT_TO_POINTER(f));
				cur = end;
				if (*cur)
					cur++;
			}

			if (users != NULL) {
				purple_chat_conversation_add_users(PURPLE_CHAT_CONVERSATION(convo), users, NULL, flags, FALSE);

				g_list_free_full(users, g_free);
				g_list_free(flags);
			}

			g_object_set_data(G_OBJECT(convo), IRC_NAMES_FLAG,
						   GINT_TO_POINTER(TRUE));
		}
		g_free(names);
	} else {
		if (!irc->names)
			irc->names = g_string_new("");

		if (irc->names->len && irc->names->str[irc->names->len - 1] != ' ')
			irc->names = g_string_append_c(irc->names, ' ');
		irc->names = g_string_append(irc->names, args[3]);
	}
}

void
irc_msg_motd(struct irc_conn *irc, const char *name,
             G_GNUC_UNUSED const char *from, char **args)
{
	char *escaped;

	if (purple_strequal(name, "375")) {
		if (irc->motd) {
			g_string_free(irc->motd, TRUE);
			irc->motd = NULL;
		}
		irc->motd = g_string_new("");
		return;
	} else if (purple_strequal(name, "376")) {
		/* dircproxy 1.0.5 does not send 251 on reconnection, so
		 * finalize the connection here if it is not already done. */
		irc_connected(irc, args[0]);
		return;
	} else if (purple_strequal(name, "422")) {
		/* in case there is no 251, and no MOTD set, finalize the connection.
		 * (and clear the motd for good measure). */

		if (irc->motd) {
			g_string_free(irc->motd, TRUE);
			irc->motd = NULL;
		}

		irc_connected(irc, args[0]);
		return;
	}

	if (!irc->motd) {
		purple_debug_error("irc", "IRC server sent MOTD without STARTMOTD\n");
		return;
	}

	if (!args[1])
		return;

	escaped = g_markup_escape_text(args[1], -1);
	g_string_append_printf(irc->motd, "%s<br>", escaped);
	g_free(escaped);
}

void
irc_msg_time(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
             G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConnection *gc;

	gc = purple_account_get_connection(irc->account);

	g_return_if_fail(gc);

	purple_notify_message(gc, PURPLE_NOTIFY_MSG_INFO, _("Time Response"),
		_("The IRC server's local time is:"), args[2], NULL, NULL,
		purple_request_cpar_from_connection(gc));
}

void
irc_msg_nochan(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
               G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);

	g_return_if_fail(gc);

	purple_notify_error(gc, NULL, _("No such channel"), args[1],
		purple_request_cpar_from_connection(gc));
}

void
irc_msg_nonick(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
               G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConnection *gc;
	PurpleConversation *convo;
	PurpleConversationManager *manager;

	manager = purple_conversation_manager_get_default();
	convo = purple_conversation_manager_find(manager, irc->account, args[1]);
	if (convo) {
		purple_conversation_write_system_message(convo,
			PURPLE_IS_IM_CONVERSATION(convo) ? _("User is not logged in") : _("no such channel"),
			PURPLE_MESSAGE_NO_LOG);

	} else {
		if ((gc = purple_account_get_connection(irc->account)) == NULL)
			return;
		purple_notify_error(gc, NULL, _("No such nick or channel"),
			args[1], purple_request_cpar_from_connection(gc));
	}

	if (irc->whois.nick && !purple_utf8_strcasecmp(irc->whois.nick, args[1])) {
		g_free(irc->whois.nick);
		irc->whois.nick = NULL;
	}
}

void
irc_msg_nosend(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
               G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConnection *gc;
	PurpleConversation *chat;
	PurpleConversationManager *manager;

	manager = purple_conversation_manager_get_default();
	chat = purple_conversation_manager_find_chat(manager, irc->account,
	                                             args[1]);
	if (chat) {
		purple_conversation_write_system_message(chat, args[2],
			PURPLE_MESSAGE_NO_LOG);
	} else {
		if ((gc = purple_account_get_connection(irc->account)) == NULL)
			return;
		purple_notify_error(gc, NULL, _("Could not send"), args[2],
			purple_request_cpar_from_connection(gc));
	}
}

void
irc_msg_notinchan(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                  G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConversation *chat;
	PurpleConversationManager *manager;

	manager = purple_conversation_manager_get_default();
	chat = purple_conversation_manager_find_chat(manager, irc->account,
	                                             args[1]);

	purple_debug_info("irc", "We're apparently not in %s, but tried to use it", args[1]);
	if (chat) {
		/*g_slist_remove(irc->gc->buddy_chats, chat);
		  purple_conversation_set_account(chat, NULL);*/
		purple_conversation_write_system_message(chat,
			args[2], PURPLE_MESSAGE_NO_LOG);
	}
}

void
irc_msg_notop(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
              G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConversation *chat;
	PurpleConversationManager *manager;

	manager = purple_conversation_manager_get_default();
	chat = purple_conversation_manager_find_chat(manager, irc->account,
	                                             args[1]);
	if (!chat) {
		return;
	}

	purple_conversation_write_system_message(chat, args[2], 0);
}

void
irc_msg_invite(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
               const char *from, char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);
	GHashTable *components;
	gchar *nick;

	g_return_if_fail(gc);

	components = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
	nick = irc_mask_nick(from);

	g_hash_table_insert(components, g_strdup("channel"), g_strdup(args[1]));

	purple_serv_got_chat_invite(gc, args[1], nick, NULL, components);
	g_free(nick);
}

void
irc_msg_inviteonly(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                   G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);
	char *buf;

	g_return_if_fail(gc);

	buf = g_strdup_printf(_("Joining %s requires an invitation."), args[1]);
	purple_notify_error(gc, _("Invitation only"), _("Invitation only"), buf,
		purple_request_cpar_from_connection(gc));
	g_free(buf);
}

void
irc_msg_ison(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
             G_GNUC_UNUSED const char *from, char **args)
{
	char **nicks;
	struct irc_buddy *ib;
	int i;

	nicks = g_strsplit(args[1], " ", -1);
	for (i = 0; nicks[i]; i++) {
		if ((ib = g_hash_table_lookup(irc->buddies, (gconstpointer)nicks[i])) == NULL) {
			continue;
		}
		ib->new_online_status = TRUE;
	}
	g_strfreev(nicks);

	if (irc->ison_outstanding)
		irc_buddy_query(irc);

	if (!irc->ison_outstanding)
		g_hash_table_foreach(irc->buddies, (GHFunc)irc_buddy_status, (gpointer)irc);
}

static void irc_buddy_status(char *name, struct irc_buddy *ib, struct irc_conn *irc)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);
	PurpleBuddy *buddy = purple_blist_find_buddy(irc->account, name);

	if (!gc || !buddy)
		return;

	if (ib->online && !ib->new_online_status) {
		purple_protocol_got_user_status(irc->account, name, "offline", NULL);
		ib->online = FALSE;
	} else if (!ib->online && ib->new_online_status) {
		purple_protocol_got_user_status(irc->account, name, "available", NULL);
		ib->online = TRUE;
	}
}

void
irc_msg_join(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
             const char *from, char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);
	PurpleConversation *chat;
	PurpleConversationManager *manager;
	PurpleChatUser *cb;

	char *nick, *userhost, *buf;
	struct irc_buddy *ib;
	static int id = 1;

	g_return_if_fail(gc);

	nick = irc_mask_nick(from);

	manager = purple_conversation_manager_get_default();

	if (!purple_utf8_strcasecmp(nick, purple_connection_get_display_name(gc))) {
		/* We are joining a channel for the first time */
		purple_serv_got_joined_chat(gc, id++, args[0]);
		g_free(nick);
		chat = purple_conversation_manager_find_chat(manager, irc->account,
		                                             args[0]);

		if (chat == NULL) {
			purple_debug_error("irc", "tried to join %s but couldn't\n", args[0]);
			return;
		}
		g_object_set_data(G_OBJECT(chat), IRC_NAMES_FLAG,
					   GINT_TO_POINTER(FALSE));

		// Get the real name and user host for all participants.
		buf = irc_format(irc, "vc", "WHO", args[0]);
		irc_send(irc, buf);
		g_free(buf);

		/* Until purple_conversation_present does something that
		 * one would expect in Pidgin, this call produces buggy
		 * behavior both for the /join and auto-join cases. */
		/* purple_conversation_present(chat); */
		return;
	}

	chat = purple_conversation_manager_find_chat(manager, irc->account,
	                                             args[0]);
	if (chat == NULL) {
		purple_debug_error("irc", "JOIN for %s failed", args[0]);
		g_free(nick);
		return;
	}

	userhost = irc_mask_userhost(from);

	purple_chat_conversation_add_user(PURPLE_CHAT_CONVERSATION(chat), nick, userhost, PURPLE_CHAT_USER_NONE, TRUE);

	cb = purple_chat_conversation_find_user(PURPLE_CHAT_CONVERSATION(chat), nick);

	if (cb) {
		g_object_set_data_full(G_OBJECT(cb), "userhost", userhost, g_free);
	}

	if ((ib = g_hash_table_lookup(irc->buddies, nick)) != NULL) {
		ib->new_online_status = TRUE;
		irc_buddy_status(nick, ib, irc);
	}

	g_free(nick);
}

void
irc_msg_kick(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
             const char *from, char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);
	PurpleConversation *chat;
	PurpleConversationManager *manager;
	char *nick, *buf;

	g_return_if_fail(gc);

	manager = purple_conversation_manager_get_default();
	chat = purple_conversation_manager_find_chat(manager, irc->account,
	                                             args[0]);

	nick = irc_mask_nick(from);

	if (!chat) {
		purple_debug_error("irc", "Received a KICK for unknown channel %s", args[0]);
		g_free(nick);
		return;
	}

	if (!purple_utf8_strcasecmp(purple_connection_get_display_name(gc), args[1])) {
		buf = g_strdup_printf(_("You have been kicked by %s: (%s)"), nick, args[2]);
		purple_conversation_write_system_message(PURPLE_CONVERSATION(chat), buf, 0);
		g_free(buf);
		purple_serv_got_chat_left(gc, purple_chat_conversation_get_id(PURPLE_CHAT_CONVERSATION(chat)));
	} else {
		buf = g_strdup_printf(_("Kicked by %s (%s)"), nick, args[2]);
		purple_chat_conversation_remove_user(PURPLE_CHAT_CONVERSATION(chat), args[1], buf);
		g_free(buf);
	}

	g_free(nick);
}

void
irc_msg_mode(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
             const char *from, char **args)
{
	char *nick = irc_mask_nick(from), *buf;

	if (*args[0] == '#' || *args[0] == '&') {	/* Channel	*/
		PurpleConversation *chat;
		PurpleConversationManager *manager;
		char *escaped;

		manager = purple_conversation_manager_get_default();
		chat = purple_conversation_manager_find_chat(manager, irc->account,
		                                             args[0]);
		if (!chat) {
			purple_debug_error("irc", "MODE received for %s, which we are not in", args[0]);
			g_free(nick);
			return;
		}
		escaped = (args[2] != NULL) ? g_markup_escape_text(args[2], -1) : NULL;
		buf = g_strdup_printf(_("mode (%s %s) by %s"), args[1], escaped ? escaped : "", nick);
		purple_conversation_write_system_message(chat, buf, 0);
		g_free(escaped);
		g_free(buf);
		if(args[2]) {
			PurpleChatUser *cb;
			PurpleChatUserFlags newflag, flags;
			char *mcur, *cur, *end, *user;
			gboolean add = FALSE;
			mcur = args[1];
			cur = args[2];
			while (*cur && *mcur) {
				if ((*mcur == '+') || (*mcur == '-')) {
					add = (*mcur == '+') ? TRUE : FALSE;
					mcur++;
					continue;
				}
				end = strchr(cur, ' ');
				if (!end)
					end = cur + strlen(cur);
				user = g_strndup(cur, end - cur);
				cb = purple_chat_conversation_find_user(PURPLE_CHAT_CONVERSATION(chat), user);
				flags = purple_chat_user_get_flags(cb);
				newflag = PURPLE_CHAT_USER_NONE;
				if (*mcur == 'o')
					newflag = PURPLE_CHAT_USER_OP;
				else if (*mcur =='h')
					newflag = PURPLE_CHAT_USER_HALFOP;
				else if (*mcur == 'v')
					newflag = PURPLE_CHAT_USER_VOICE;
				else if(irc->mode_chars
					  && strchr(irc->mode_chars, '~') && (*mcur == 'q'))
					newflag = PURPLE_CHAT_USER_FOUNDER;
				if (newflag) {
					if (add)
						flags |= newflag;
					else
						flags &= ~newflag;
					purple_chat_user_set_flags(cb, flags);
				}
				g_free(user);
				cur = end;
				if (*cur)
					cur++;
				if (*mcur)
					mcur++;
			}
		}
	} else {					/* User		*/
	}
	g_free(nick);
}

void
irc_msg_nick(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
             const char *from, char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);
	PurpleConversation *im;
	PurpleConversationManager *manager;
	GSList *chats;
	char *nick = irc_mask_nick(from);

	irc->nickused = FALSE;

	if (!gc) {
		g_free(nick);
		return;
	}
	chats = purple_connection_get_active_chats(gc);

	if (!purple_utf8_strcasecmp(nick, purple_connection_get_display_name(gc))) {
		purple_connection_set_display_name(gc, args[0]);
	}

	while (chats) {
		PurpleChatConversation *chat = PURPLE_CHAT_CONVERSATION(chats->data);
		/* This is ugly ... */
		if (purple_chat_conversation_has_user(chat, nick))
			purple_chat_conversation_rename_user(chat, nick, args[0]);
		chats = chats->next;
	}

	manager = purple_conversation_manager_get_default();
	im = purple_conversation_manager_find_im(manager, irc->account, nick);
	if (im != NULL) {
		purple_conversation_set_name(im, args[0]);
	}

	g_free(nick);
}

void
irc_msg_badnick(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                G_GNUC_UNUSED const char *from, G_GNUC_UNUSED char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);
	if (purple_connection_get_state(gc) == PURPLE_CONNECTION_STATE_CONNECTED) {
		purple_notify_error(gc, _("Invalid nickname"), _("Invalid "
			"nickname"), _("Your selected nickname was rejected by "
			"the server.  It probably contains invalid characters."),
			purple_request_cpar_from_connection(gc));

	} else {
		purple_connection_take_error(gc, g_error_new_literal(
				PURPLE_CONNECTION_ERROR,
				PURPLE_CONNECTION_ERROR_INVALID_SETTINGS,
				_("Your selected account name was rejected by the server.  It probably contains invalid characters.")));
	}
}

void
irc_msg_nickused(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                 G_GNUC_UNUSED const char *from, char **args)
{
	char *newnick, *buf, *end;
	PurpleConnection *gc = purple_account_get_connection(irc->account);

	if (gc && purple_connection_get_state(gc) == PURPLE_CONNECTION_STATE_CONNECTED) {
		/* We only want to do the following dance if the connection
		   has not been successfully completed.  If it has, just
		   notify the user that their /nick command didn't go. */
		buf = g_strdup_printf(_("The nickname \"%s\" is already being used."),
				      irc->reqnick);
		purple_notify_error(gc, _("Nickname in use"), _("Nickname in "
			"use"), buf, purple_request_cpar_from_connection(gc));
		g_free(buf);
		g_free(irc->reqnick);
		irc->reqnick = NULL;
		return;
	}

	if (strlen(args[1]) < strlen(irc->reqnick) || irc->nickused)
		newnick = g_strdup(args[1]);
	else
		newnick = g_strdup_printf("%s0", args[1]);
	end = newnick + strlen(newnick) - 1;
	/* try fallbacks */
	if((*end < '9') && (*end >= '1')) {
			*end = *end + 1;
	} else *end = '1';

	g_free(irc->reqnick);
	irc->reqnick = newnick;
	irc->nickused = TRUE;

	purple_connection_set_display_name(
		purple_account_get_connection(irc->account), newnick);

	buf = irc_format(irc, "vn", "NICK", newnick);
	irc_send(irc, buf);
	g_free(buf);
}

void irc_msg_notice(struct irc_conn *irc, const char *name, const char *from, char **args)
{
	irc_msg_handle_privmsg(irc, name, from, args[0], args[1], TRUE);
}

void
irc_msg_nochangenick(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                     G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);

	g_return_if_fail(gc);

	purple_notify_error(gc, _("Cannot change nick"),
		_("Could not change nick"), args[2],
		purple_request_cpar_from_connection(gc));
}

void
irc_msg_part(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
             const char *from, char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);
	PurpleConversation *chat;
	PurpleConversationManager *manager;
	char *nick, *msg, *channel;

	g_return_if_fail(gc);

	/* Undernet likes to :-quote the channel name, for no good reason
	 * that I can see.  This catches that. */
	channel = (args[0][0] == ':') ? &args[0][1] : args[0];

	manager = purple_conversation_manager_get_default();
	chat = purple_conversation_manager_find_chat(manager, irc->account,
	                                             channel);
	if (!chat) {
		purple_debug_info("irc", "Got a PART on %s, which doesn't exist -- probably closed", channel);
		return;
	}

	nick = irc_mask_nick(from);
	if (!purple_utf8_strcasecmp(nick, purple_connection_get_display_name(gc))) {
		char *escaped = args[1] ? g_markup_escape_text(args[1], -1) : NULL;
		msg = g_strdup_printf(_("You have parted the channel%s%s"),
		                      (args[1] && *args[1]) ? ": " : "",
		                      (escaped && *escaped) ? escaped : "");
		g_free(escaped);
		purple_conversation_write_system_message(chat, msg, 0);
		g_free(msg);
		purple_serv_got_chat_left(gc, purple_chat_conversation_get_id(PURPLE_CHAT_CONVERSATION(chat)));
	} else {
		msg = args[1] ? irc_mirc2txt(args[1]) : NULL;
		purple_chat_conversation_remove_user(PURPLE_CHAT_CONVERSATION(chat), nick, msg);
		g_free(msg);
	}
	g_free(nick);
}

void
irc_msg_ping(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
             G_GNUC_UNUSED const char *from, char **args)
{
	char *buf;

	buf = irc_format(irc, "v:", "PONG", args[0]);
	irc_send(irc, buf);
	g_free(buf);
}

void
irc_msg_pong(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
             G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConversation *convo;
	PurpleConversationManager *manager;
	PurpleConnection *gc;
	char **parts, *msg;
	gint64 oldstamp;

	parts = g_strsplit(args[1], " ", 2);

	if (!parts[0] || !parts[1]) {
		g_strfreev(parts);
		return;
	}

	if (sscanf(parts[1], "%" G_GINT64_FORMAT, &oldstamp) != 1) {
		msg = g_strdup(_("Error: invalid PONG from server"));
	} else {
		msg = g_strdup_printf(_("PING reply -- Lag: %f seconds"),
		                      (g_get_monotonic_time() - oldstamp) /
		                              (gdouble)G_USEC_PER_SEC);
	}

	manager = purple_conversation_manager_get_default();
	convo = purple_conversation_manager_find(manager, irc->account, parts[0]);
	g_strfreev(parts);
	if (convo) {
		purple_conversation_write_system_message(convo, msg, PURPLE_MESSAGE_NO_LOG);
	} else {
		gc = purple_account_get_connection(irc->account);
		if (!gc) {
			g_free(msg);
			return;
		}
		purple_notify_info(gc, NULL, "PONG", msg,
			purple_request_cpar_from_connection(gc));
	}
	g_free(msg);
}

void irc_msg_privmsg(struct irc_conn *irc, const char *name, const char *from, char **args)
{
	irc_msg_handle_privmsg(irc, name, from, args[0], args[1], FALSE);
}

static void
irc_msg_handle_privmsg(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                       const char *from, const char *to, const char *rawmsg,
                       gboolean notice)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);
	PurpleConversation *chat;
	char *tmp;
	char *msg;
	char *nick;

	if (!gc)
		return;

	nick = irc_mask_nick(from);
	tmp = irc_parse_ctcp(irc, nick, to, rawmsg, notice);
	if (!tmp) {
		g_free(nick);
		return;
	}

	msg = irc_escape_privmsg(tmp, -1);
	g_free(tmp);

	tmp = irc_mirc2html(msg);
	g_free(msg);
	msg = tmp;
	if (notice) {
		tmp = g_strdup_printf("(notice) %s", msg);
		g_free(msg);
		msg = tmp;
	}

	if (!purple_utf8_strcasecmp(to, purple_connection_get_display_name(gc))) {
		purple_serv_got_im(gc, nick, msg, 0, time(NULL));
	} else {
		PurpleConversationManager *manager;

		manager = purple_conversation_manager_get_default();
		chat = purple_conversation_manager_find_chat(manager, irc->account,
		                                             irc_nick_skip_mode(irc, to));
		if (chat) {
			purple_serv_got_chat_in(gc, purple_chat_conversation_get_id(PURPLE_CHAT_CONVERSATION(chat)),
				nick, PURPLE_MESSAGE_RECV, msg, time(NULL));
		} else
			purple_debug_info("irc", "Got a %s on %s, which does not exist\n",
			                  notice ? "NOTICE" : "PRIVMSG", to);
	}
	g_free(msg);
	g_free(nick);
}

void
irc_msg_regonly(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);
	PurpleConversationManager *manager;
	char *msg;

	g_return_if_fail(gc);

	manager = purple_conversation_manager_get_default();

	if(purple_conversation_manager_find_chat(manager, irc->account, args[1])) {
		/* This is a channel we're already in; for some reason,
		 * freenode feels the need to notify us that in some
		 * hypothetical other situation this might not have
		 * succeeded.  Suppress that. */
		return;
	}

	msg = g_strdup_printf(_("Cannot join %s: Registration is required."), args[1]);
	purple_notify_error(gc, _("Cannot join channel"), msg, args[2],
		purple_request_cpar_from_connection(gc));
	g_free(msg);
}

void
irc_msg_quit(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
             const char *from, char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);
	struct irc_buddy *ib;
	char *data[2];

	g_return_if_fail(gc);

	data[0] = irc_mask_nick(from);
	data[1] = args[0];
	/* XXX this should have an API, I shouldn't grab this directly */
	g_slist_foreach(purple_connection_get_active_chats(gc),
			(GFunc)irc_chat_remove_buddy, data);

	if ((ib = g_hash_table_lookup(irc->buddies, data[0])) != NULL) {
		ib->new_online_status = FALSE;
		irc_buddy_status(data[0], ib, irc);
	}
	g_free(data[0]);
}

void
irc_msg_unavailable(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                    G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);

	purple_notify_error(gc, NULL, _("Nick or channel is temporarily "
		"unavailable."), args[1],
		purple_request_cpar_from_connection(gc));
}

void
irc_msg_wallops(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                const char *from, char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);
	char *nick, *msg;

	g_return_if_fail(gc);

	nick = irc_mask_nick(from);
	msg = g_strdup_printf (_("Wallops from %s"), nick);
	g_free(nick);
	purple_notify_info(gc, NULL, msg, args[0],
		purple_request_cpar_from_connection(gc));
	g_free(msg);
}

static void
irc_auth_sasl_attempt(struct irc_conn *irc) {
	PurpleAccount *account = irc->account;
	PurpleConnection *gc = purple_account_get_connection(account);
	char *buf;
	const char *current_mechanism = NULL;
	const char *next_mechanism = NULL;

	current_mechanism = hasl_context_get_current_mechanism(irc->hasl_ctx);
	if(current_mechanism != NULL) {
		g_message("SASL '%s' mechanism failed", current_mechanism);
	}

	next_mechanism = hasl_context_next(irc->hasl_ctx);
	if(next_mechanism == NULL) {
		purple_connection_take_error(gc,
		                             g_error_new_literal(
		                             PURPLE_CONNECTION_ERROR,
		                             PURPLE_CONNECTION_ERROR_AUTHENTICATION_IMPOSSIBLE,
		                             _("SASL authentication failed: No worthy authentication mechanisms found.")));
		irc_sasl_finish(irc);

		return;
	}

	g_message("Using SASL: %s", next_mechanism);
	buf = irc_format(irc, "vv", "AUTHENTICATE", next_mechanism);
	irc_send(irc, buf);
	g_free(buf);
}

/* SASL authentication */
void
irc_msg_cap(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
            G_GNUC_UNUSED const char *from, char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);
	gboolean allow_clear_text = FALSE;

	if (strncmp(g_strstrip(args[2]), "sasl", 5))
		return;
	if (strncmp(args[1], "ACK", 4)) {
		purple_connection_take_error(gc, g_error_new_literal(
			PURPLE_CONNECTION_ERROR,
			PURPLE_CONNECTION_ERROR_AUTHENTICATION_IMPOSSIBLE,
			_("SASL authentication failed: Server does not support SASL authentication.")));

		irc_sasl_finish(irc);
		return;
	}

	irc->hasl_ctx = hasl_context_new();
	hasl_context_set_allowed_mechanisms(irc->hasl_ctx, "PLAIN");

	hasl_context_set_username(irc->hasl_ctx,
	                          purple_connection_get_display_name(gc));
	hasl_context_set_password(irc->hasl_ctx,
	                          purple_connection_get_password(gc));
	hasl_context_set_authzid(irc->hasl_ctx, "");
	hasl_context_set_tls(irc->hasl_ctx, G_IS_TLS_CONNECTION(irc->conn));

	allow_clear_text = purple_account_get_bool(irc->account,
	                                           "auth_plain_in_clear",
	                                           FALSE);
	hasl_context_set_allow_clear_text(irc->hasl_ctx, allow_clear_text);

	irc_auth_sasl_attempt(irc);
}

void
irc_msg_auth(struct irc_conn *irc, char *arg)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);
	HaslMechanismResult res;
	GError *error = NULL;
	char *buf, *authinfo;
	char *serverin = NULL;
	gsize serverinlen = 0;
	guint8 *c_out;
	gsize clen;

	if(!arg) {
		return;
	}

	if(arg[0] != '+') {
		serverin = (char *)g_base64_decode(arg, &serverinlen);
	}

	res = hasl_context_step(irc->hasl_ctx, (guint8 *)serverin, serverinlen,
	                        &c_out, &clen, &error);
	g_free(serverin);

	if(res == HASL_MECHANISM_RESULT_ERROR) {
		const char *error_msg = "unknown error";

		if(error != NULL && error->message != NULL) {
			error_msg = error->message;
		}

		purple_connection_take_error(gc, g_error_new(
			PURPLE_CONNECTION_ERROR,
			PURPLE_CONNECTION_ERROR_AUTHENTICATION_IMPOSSIBLE,
			_("SASL authentication failed: %s"),
			error_msg));

		g_clear_error(&error);

		irc_sasl_finish(irc);

		return;
	}

	if(error != NULL) {
			g_warning("hasl_context_step returned an error without an error "
			          "status: %s", error->message);
			g_clear_error(&error);
	}

	if(clen > 0) {
		authinfo = g_base64_encode(c_out, clen);
		g_free(c_out);
	} else {
		authinfo = g_strdup("+");
	}

	buf = irc_format(irc, "vv", "AUTHENTICATE", authinfo);
	irc_send(irc, buf);
	g_free(buf);
	g_free(authinfo);
	g_free(serverin);
}

void
irc_msg_authenticate(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                     G_GNUC_UNUSED const char *from, char **args)
{
	irc_msg_auth(irc, args[0]);
}

void
irc_msg_authok(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
               G_GNUC_UNUSED const char *from, G_GNUC_UNUSED char **args)
{
	char *buf;

	g_clear_object(&irc->hasl_ctx);
	purple_debug_info("irc", "Successfully authenticated using SASL.\n");

	/* Finish auth session */
	buf = irc_format(irc, "vv", "CAP", "END");
	irc_send(irc, buf);
	g_free(buf);
}

void
irc_msg_authtryagain(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                     G_GNUC_UNUSED const char *from, G_GNUC_UNUSED char **args)
{
	irc_auth_sasl_attempt(irc);
}

void
irc_msg_authfail(struct irc_conn *irc, G_GNUC_UNUSED const char *name,
                 G_GNUC_UNUSED const char *from,
                 G_GNUC_UNUSED char **args)
{
	irc_auth_sasl_attempt(irc);
}

static void
irc_sasl_finish(struct irc_conn *irc)
{
	char *buf;

	g_clear_object(&irc->hasl_ctx);

	/* Auth failed, abort */
	buf = irc_format(irc, "vv", "CAP", "END");
	irc_send(irc, buf);
	g_free(buf);
}
