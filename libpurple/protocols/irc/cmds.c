/**
 * purple
 *
 * Copyright (C) 2003, Ethan Blanton <eblanton@cs.purdue.edu>
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

#include <purple.h>

#include "irc.h"


static void irc_do_mode(struct irc_conn *irc, const char *target, const char *sign, char **ops);

int
irc_cmd_default(struct irc_conn *irc, const char *cmd, const char *target,
                G_GNUC_UNUSED const char **args)
{
	PurpleConversation *convo;
	PurpleConversationManager *manager;
	char *buf;

	manager = purple_conversation_manager_get_default();
	convo = purple_conversation_manager_find(manager, irc->account, target);

	if (!convo) {
		return 1;
	}

	buf = g_strdup_printf(_("Unknown command: %s"), cmd);
	purple_conversation_write_system_message(convo, buf, PURPLE_MESSAGE_NO_LOG);
	g_free(buf);

	return 1;
}

int
irc_cmd_away(struct irc_conn *irc, const char *cmd,
             G_GNUC_UNUSED const char *target, const char **args)
{
	char *buf, *message;

	if (args[0] && !purple_strequal(cmd, "back")) {
		message = purple_markup_strip_html(args[0]);
		purple_util_chrreplace(message, '\n', ' ');
		buf = irc_format(irc, "v:", "AWAY", message);
		g_free(message);
	} else {
		buf = irc_format(irc, "v", "AWAY");
	}
	irc_send(irc, buf);
	g_free(buf);

	return 0;
}

int
irc_cmd_ctcp(struct irc_conn *irc, G_GNUC_UNUSED const char *cmd,
             G_GNUC_UNUSED const char *target, const char **args)
{
	/* we have defined args as args[0] is target and args[1] is ctcp command */
	char *buf;
	GString *string;

	/* check if we have args */
	if (!args || !args[0] || !args[1])
		return 0;

	/* TODO:strip newlines or send each line as separate ctcp or something
	 * actually, this shouldn't be done here but somewhere else since irc should support escaping newlines */

	string = g_string_new(args[1]);
	g_string_prepend_c (string,'\001');
	g_string_append_c (string,'\001');
	buf = irc_format(irc, "vn:", "PRIVMSG", args[0], string->str);
	g_string_free(string,TRUE);

	irc_send(irc, buf);
	g_free(buf);

	return 1;
}

int irc_cmd_ctcp_action(struct irc_conn *irc, const char *cmd, const char *target, const char **args)
{
	PurpleConnection *gc = purple_account_get_connection(irc->account);
	gchar *action, *escaped, *dst, **newargs;
	const gchar *src, *me;
	gchar *msg;
	PurpleContactInfo *info = NULL;
	PurpleConversation *convo;
	PurpleConversationManager *manager;
	PurpleMessage *pmsg;

	if (!args || !args[0] || !gc) {
		return 0;
	}

	info = PURPLE_CONTACT_INFO(irc->account);
	me = purple_contact_info_get_name_for_display(info);

	manager = purple_conversation_manager_get_default();
	convo = purple_conversation_manager_find(manager, irc->account, target);

	msg = g_strdup_printf("/me %s", args[0]);

	/* XXX: we'd prefer to keep this in conversation.c */
	if (PURPLE_IS_IM_CONVERSATION(convo)) {
		const gchar *conv_name = purple_conversation_get_name(convo);
		pmsg = purple_message_new_outgoing(irc->account, me, conv_name, msg,
		                                   0);

		purple_signal_emit(purple_conversations_get_handle(),
			"sending-im-msg", irc->account, pmsg);
	} else {
		pmsg = purple_message_new_outgoing(irc->account, me, NULL, msg, 0);

		purple_signal_emit(purple_conversations_get_handle(),
			"sending-chat-msg", irc->account, pmsg,
			purple_chat_conversation_get_id(PURPLE_CHAT_CONVERSATION(convo)));
	}

	/* We free the original message because it could have been changed by the
	 * sending-*-msg signals above.
	 */
	g_free(msg);

	/* if the message was eaten by a signal we bail */
	if(purple_message_is_empty(pmsg)) {
		g_object_unref(G_OBJECT(pmsg));

		return 0;
	}

	/* create a copy of the updated message, which should not be null because
	 * we just tested if it was empty in the above conditional.
	 */
	msg = g_strdup(purple_message_get_contents(pmsg));

	if (strncmp(msg, "/me ", 4) != 0) {
		newargs = g_new0(char *, 2);
		newargs[0] = g_strdup(target);
		newargs[1] = msg;

		irc_cmd_privmsg(irc, cmd, target, (const char **)newargs);

		g_free(newargs[0]);
		g_free(newargs);
	} else {
		action = g_malloc(strlen(&msg[4]) + 10);

		sprintf(action, "\001ACTION ");

		src = &msg[4];
		dst = action + 8;
		while (*src) {
			if (*src == '\n') {
				if (*(src + 1) == '\0') {
					break;
				} else {
					*dst++ = ' ';
					src++;
					continue;
				}
			}
			*dst++ = *src++;
		}
		*dst++ = '\001';
		*dst = '\0';

		newargs = g_new0(char *, 2);
		newargs[0] = g_strdup(target);
		newargs[1] = action;
		irc_cmd_privmsg(irc, cmd, target, (const char **)newargs);
		g_free(newargs[0]);
		g_free(newargs);
		g_free(action);
	}

	/* XXX: we'd prefer to keep this in conversation.c */
	if (PURPLE_IS_IM_CONVERSATION(convo)) {
		purple_signal_emit(purple_conversations_get_handle(),
			"sent-im-msg", irc->account, pmsg);
	} else {
		purple_signal_emit(purple_conversations_get_handle(),
			"sent-chat-msg", irc->account, pmsg,
			purple_chat_conversation_get_id(PURPLE_CHAT_CONVERSATION(convo)));
	}

	g_free(msg);

	if (convo) {
		escaped = g_markup_escape_text(args[0], -1);
		action = g_strdup_printf("/me %s", escaped);
		g_free(escaped);
		if (action[strlen(action) - 1] == '\n') {
			action[strlen(action) - 1] = '\0';
		}
		if (PURPLE_IS_CHAT_CONVERSATION(convo)) {
			purple_serv_got_chat_in(gc, purple_chat_conversation_get_id(PURPLE_CHAT_CONVERSATION(convo)),
			                 purple_connection_get_display_name(gc),
			                 PURPLE_MESSAGE_SEND, action, time(NULL));
		} else {
			purple_message_set_recipient(pmsg,
			                             purple_connection_get_display_name(gc));
			purple_conversation_write_message(convo, pmsg);
		}
		g_free(action);
	}

	g_object_unref(G_OBJECT(pmsg));

	return 1;
}

int
irc_cmd_ctcp_version(struct irc_conn *irc, G_GNUC_UNUSED const char *cmd,
                     G_GNUC_UNUSED const char *target, const char **args)
{
	char *buf;

	if (!args || !args[0])
		return 0;

	buf = irc_format(irc, "vn:", "PRIVMSG", args[0], "\001VERSION\001");
	irc_send(irc, buf);
	g_free(buf);

	return 0;
}

int
irc_cmd_invite(struct irc_conn *irc, G_GNUC_UNUSED const char *cmd,
               const char *target, const char **args)
{
	char *buf;

	if (!args || !args[0] || !(args[1] || target))
		return 0;

	buf = irc_format(irc, "vnc", "INVITE", args[0], args[1] ? args[1] : target);
	irc_send(irc, buf);
	g_free(buf);

	return 0;
}

int
irc_cmd_join(struct irc_conn *irc, G_GNUC_UNUSED const char *cmd,
             G_GNUC_UNUSED const char *target, const char **args)
{
	char *buf;

	if (!args || !args[0])
		return 0;

	if (args[1])
		buf = irc_format(irc, "vcv", "JOIN", args[0], args[1]);
	else
		buf = irc_format(irc, "vc", "JOIN", args[0]);
	irc_send(irc, buf);
	g_free(buf);

	return 0;
}

int
irc_cmd_kick(struct irc_conn *irc, G_GNUC_UNUSED const char *cmd,
             const char *target, const char **args)
{
	PurpleConversationManager *manager;
	char *buf;

	if(!args || !args[0]) {
		return 0;
	}

	manager = purple_conversation_manager_get_default();
	if(!purple_conversation_manager_find_chat(manager, irc->account, target)) {
		return 0;
	}

	if(args[1]) {
		buf = irc_format(irc, "vcn:", "KICK", target, args[0], args[1]);
	} else {
		buf = irc_format(irc, "vcn", "KICK", target, args[0]);
	}
	irc_send(irc, buf);
	g_free(buf);

	return 0;
}

int
irc_cmd_list(struct irc_conn *irc, G_GNUC_UNUSED const char *cmd,
             G_GNUC_UNUSED const char *target, G_GNUC_UNUSED const char **args)
{
	purple_roomlist_show_with_account(irc->account);

	return 0;
}

int irc_cmd_mode(struct irc_conn *irc, const char *cmd, const char *target, const char **args)
{
	PurpleConnection *gc;
	char *buf;

	if (!args)
		return 0;

	if (purple_strequal(cmd, "mode")) {
		if (!args[0] && irc_ischannel(target))
			buf = irc_format(irc, "vc", "MODE", target);
		else if (args[0] && (*args[0] == '+' || *args[0] == '-'))
			buf = irc_format(irc, "vcn", "MODE", target, args[0]);
		else if (args[0])
			buf = irc_format(irc, "vn", "MODE", args[0]);
		else
			return 0;
	} else if (purple_strequal(cmd, "umode")) {
		if (!args[0])
			return 0;
		gc = purple_account_get_connection(irc->account);
		buf = irc_format(irc, "vnc", "MODE", purple_connection_get_display_name(gc), args[0]);
	} else {
		return 0;
	}

	irc_send(irc, buf);
	g_free(buf);

	return 0;
}

int
irc_cmd_names(struct irc_conn *irc, G_GNUC_UNUSED const char *cmd,
              const char *target, const char **args)
{
	char *buf;

	if (!args || (!args[0] && !irc_ischannel(target)))
		return 0;

	buf = irc_format(irc, "vc", "NAMES", args[0] ? args[0] : target);
	irc_send(irc, buf);
	g_free(buf);

	return 0;
}

int
irc_cmd_nick(struct irc_conn *irc, G_GNUC_UNUSED const char *cmd,
             G_GNUC_UNUSED const char *target, const char **args)
{
	char *buf;

	if (!args || !args[0])
		return 0;

	buf = irc_format(irc, "v:", "NICK", args[0]);
	g_free(irc->reqnick);
	irc->reqnick = g_strdup(args[0]);
	irc->nickused = FALSE;
	irc_send(irc, buf);
	g_free(buf);

	return 0;
}

int irc_cmd_op(struct irc_conn *irc, const char *cmd, const char *target, const char **args)
{
	char **nicks, **ops, *sign, *mode;
	int i = 0, used = 0;

	if (!args || !args[0] || !*args[0])
		return 0;

	if (purple_strequal(cmd, "op")) {
		sign = "+";
		mode = "o";
	} else if (purple_strequal(cmd, "deop")) {
		sign = "-";
		mode = "o";
	} else if (purple_strequal(cmd, "voice")) {
		sign = "+";
		mode = "v";
	} else if (purple_strequal(cmd, "devoice")) {
		sign = "-";
		mode = "v";
	} else {
		purple_debug_error("irc", "invalid 'op' command '%s'", cmd);
		return 0;
	}

	nicks = g_strsplit(args[0], " ", -1);

	for (i = 0; nicks[i]; i++)
		/* nothing */;
	ops = g_new0(char *, i * 2 + 1);

	for (i = 0; nicks[i]; i++) {
		if (*nicks[i]) {
			ops[used++] = mode;
			ops[used++] = nicks[i];
		}
	}

	irc_do_mode(irc, target, sign, ops);
	g_free(ops);
	g_strfreev(nicks);

	return 0;
}

int
irc_cmd_part(struct irc_conn *irc, G_GNUC_UNUSED const char *cmd,
             const char *target, const char **args)
{
	char *buf;

	if (!args)
		return 0;

	if (args[1])
		buf = irc_format(irc, "vc:", "PART", args[0] ? args[0] : target, args[1]);
	else
		buf = irc_format(irc, "vc", "PART", args[0] ? args[0] : target);
	irc_send(irc, buf);
	g_free(buf);

	return 0;
}

int
irc_cmd_ping(struct irc_conn *irc, G_GNUC_UNUSED const char *cmd,
             const char *target, const char **args)
{
	char *stamp;
	char *buf;

	if (args && args[0]) {
		if (irc_ischannel(args[0]))
			return 0;
		stamp = g_strdup_printf("\001PING %" G_GINT64_FORMAT "\001",
		                        g_get_monotonic_time());
		buf = irc_format(irc, "vn:", "PRIVMSG", args[0], stamp);
		g_free(stamp);
	} else if (target) {
		stamp = g_strdup_printf("%s %" G_GINT64_FORMAT, target,
		                        g_get_monotonic_time());
		buf = irc_format(irc, "v:", "PING", stamp);
		g_free(stamp);
	} else {
		stamp = g_strdup_printf("%" G_GUINT64_FORMAT, g_get_monotonic_time());
		buf = irc_format(irc, "vv", "PING", stamp);
		g_free(stamp);
	}
	irc_send(irc, buf);
	g_free(buf);

	return 0;
}

int
irc_cmd_privmsg(struct irc_conn *irc, const char *cmd,
                G_GNUC_UNUSED const char *target, const char **args)
{
	int max_privmsg_arg_len;
	const char *cur, *end;
	gchar *salvaged;
	char *msg, *buf;

	if (!args || !args[0] || !args[1])
		return 0;

	max_privmsg_arg_len = IRC_MAX_MSG_SIZE - strlen(args[0]) - 64;
	salvaged = g_utf8_make_valid(args[1], -1);
	cur = salvaged;
	end = salvaged;
	while (*end && *cur) {
		end = strchr(cur, '\n');
		if (!end)
			end = cur + strlen(cur);
		if (end - cur > max_privmsg_arg_len) {
			/* this call is used to find the last valid character position in the first
			 * max_privmsg_arg_len bytes of the utf-8 message
			 */
			g_utf8_validate(cur, max_privmsg_arg_len, &end);
		}

		msg = g_strndup(cur, end - cur);

		if(purple_strequal(cmd, "notice"))
			buf = irc_format(irc, "vt:", "NOTICE", args[0], msg);
		else
			buf = irc_format(irc, "vt:", "PRIVMSG", args[0], msg);

		irc_send(irc, buf);
		g_free(msg);
		g_free(buf);
		cur = end;
		if(*cur == '\n') {
			cur++;
		}
	}

	g_free(salvaged);

	return 0;
}

int
irc_cmd_quit(struct irc_conn *irc, G_GNUC_UNUSED const char *cmd,
             G_GNUC_UNUSED const char *target, const char **args)
{
	char *buf;

	if (!irc->quitting) {
		/*
		 * Use purple_account_get_string(irc->account, "quitmsg", IRC_DEFAULT_QUIT)
		 * and uncomment the appropriate account preference in irc.c if we
		 * decide we want custom quit messages.
		 */
		buf = irc_format(irc, "v:", "QUIT", (args && args[0]) ? args[0] : IRC_DEFAULT_QUIT);
		irc_send(irc, buf);
		g_free(buf);

		irc->quitting = TRUE;

		if (!purple_account_is_disconnecting(irc->account))
			purple_account_set_status(irc->account, "offline", TRUE, NULL);
	}

	return 0;
}

int
irc_cmd_quote(struct irc_conn *irc, G_GNUC_UNUSED const char *cmd,
              G_GNUC_UNUSED const char *target, const char **args)
{
	char *buf;

	if (!args || !args[0])
		return 0;

	buf = irc_format(irc, "n", args[0]);
	irc_send(irc, buf);
	g_free(buf);

	return 0;
}

int irc_cmd_query(struct irc_conn *irc, const char *cmd, const char *target, const char **args)
{
	PurpleConversation *im;
	PurpleConnection *gc;

	if (!args || !args[0])
		return 0;

	im = purple_im_conversation_new(irc->account, args[0]);
	purple_conversation_present(im);

	if (args[1]) {
		PurpleMessage *message = NULL;
		PurpleContactInfo *info = PURPLE_CONTACT_INFO(irc->account);
		const gchar *me = NULL;
		const gchar *recipient = NULL;

		gc = purple_account_get_connection(irc->account);
		irc_cmd_privmsg(irc, cmd, target, args);

		me = purple_contact_info_get_name_for_display(info);
		recipient = purple_connection_get_display_name(gc);
		message = purple_message_new_outgoing(irc->account, me, recipient,
		                                      args[1], 0);

		purple_conversation_write_message(im, message);

		g_object_unref(G_OBJECT(message));
	}

	return 0;
}

int
irc_cmd_remove(struct irc_conn *irc, G_GNUC_UNUSED const char *cmd,
               const char *target, const char **args)
{
	char *buf;

	if (!args || !args[0])
		return 0;

	if (!irc_ischannel(target)) /* not a channel, punt */
		return 0;

	if (args[1])
		buf = irc_format(irc, "vcn:", "REMOVE", target, args[0], args[1]);
	else
		buf = irc_format(irc, "vcn", "REMOVE", target, args[0]);
	irc_send(irc, buf);
	g_free(buf);

	return 0;
}

int
irc_cmd_service(struct irc_conn *irc, const char *cmd,
                G_GNUC_UNUSED const char *target, const char **args)
{
	char *capital_cmd, *buf;

	if (!args || !args[0])
		return 0;

	/* cmd will be one of nickserv, chanserv, memoserv or operserv */
	capital_cmd = g_ascii_strup(cmd, -1);
	buf = irc_format(irc, "v:", capital_cmd, args[0]);
	irc_send(irc, buf);
	g_free(capital_cmd);
	g_free(buf);

	return 0;
}

int
irc_cmd_time(struct irc_conn *irc, G_GNUC_UNUSED const char *cmd,
             G_GNUC_UNUSED const char *target, G_GNUC_UNUSED const char **args)
{
	char *buf;

	buf = irc_format(irc, "v", "TIME");
	irc_send(irc, buf);
	g_free(buf);

	return 0;
}

int
irc_cmd_topic(struct irc_conn *irc, G_GNUC_UNUSED const char *cmd,
              const char *target, const char **args)
{
	PurpleConversation *chat;
	PurpleConversationManager *manager;
	char *buf;
	const char *topic;

	if (!args) {
		return 0;
	}

	manager = purple_conversation_manager_get_default();
	chat = purple_conversation_manager_find_chat(manager, irc->account, target);
	if (!chat) {
		return 0;
	}

	if (!args[0]) {
		topic = purple_chat_conversation_get_topic (PURPLE_CHAT_CONVERSATION(chat));

		if (topic) {
			char *tmp, *tmp2;
			tmp = g_markup_escape_text(topic, -1);
			tmp2 = purple_markup_linkify(tmp);
			buf = g_strdup_printf(_("current topic is: %s"), tmp2);
			g_free(tmp);
			g_free(tmp2);
		} else
			buf = g_strdup(_("No topic is set"));
		purple_conversation_write_system_message(
			chat, buf, PURPLE_MESSAGE_NO_LOG);
		g_free(buf);

		return 0;
	}

	buf = irc_format(irc, "vt:", "TOPIC", target, args[0]);
	irc_send(irc, buf);
	g_free(buf);

	return 0;
}

int
irc_cmd_wallops(struct irc_conn *irc, const char *cmd,
                G_GNUC_UNUSED const char *target, const char **args)
{
	char *buf;

	if (!args || !args[0])
		return 0;

	if (purple_strequal(cmd, "wallops"))
		buf = irc_format(irc, "v:", "WALLOPS", args[0]);
	else if (purple_strequal(cmd, "operwall"))
		buf = irc_format(irc, "v:", "OPERWALL", args[0]);
	else
		return 0;

	irc_send(irc, buf);
	g_free(buf);

	return 0;
}

int
irc_cmd_whois(struct irc_conn *irc, G_GNUC_UNUSED const char *cmd,
              G_GNUC_UNUSED const char *target, const char **args)
{
	char *buf;

	if (!args || !args[0])
		return 0;

	if (args[1]) {
		buf = irc_format(irc, "vvn", "WHOIS", args[0], args[1]);
		irc->whois.nick = g_strdup(args[1]);
	} else {
		buf = irc_format(irc, "vn", "WHOIS", args[0]);
		irc->whois.nick = g_strdup(args[0]);
	}

	irc_send(irc, buf);
	g_free(buf);

	return 0;
}

int
irc_cmd_whowas(struct irc_conn *irc, G_GNUC_UNUSED const char *cmd,
               G_GNUC_UNUSED const char *target, const char **args)
{
	char *buf;

	if (!args || !args[0])
		return 0;

	buf = irc_format(irc, "vn", "WHOWAS", args[0]);

	irc->whois.nick = g_strdup(args[0]);
	irc_send(irc, buf);
	g_free(buf);

	return 0;
}

static void irc_do_mode(struct irc_conn *irc, const char *target, const char *sign, char **ops)
{
	char *buf, mode[5];
	int i = 0;

	if (!sign)
		return;

	while (ops[i]) {
		if (ops[i + 2] && ops[i + 4]) {
			g_snprintf(mode, sizeof(mode), "%s%s%s%s", sign,
				   ops[i], ops[i + 2], ops[i + 4]);
			buf = irc_format(irc, "vcvnnn", "MODE", target, mode,
					 ops[i + 1], ops[i + 3], ops[i + 5]);
			i += 6;
		} else if (ops[i + 2]) {
			g_snprintf(mode, sizeof(mode), "%s%s%s",
				   sign, ops[i], ops[i + 2]);
			buf = irc_format(irc, "vcvnn", "MODE", target, mode,
					 ops[i + 1], ops[i + 3]);
			i += 4;
		} else {
			g_snprintf(mode, sizeof(mode), "%s%s", sign, ops[i]);
			buf = irc_format(irc, "vcvn", "MODE", target, mode, ops[i + 1]);
			i += 2;
		}
		irc_send(irc, buf);
		g_free(buf);
	}
}
