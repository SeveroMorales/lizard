/**
 * purple
 *
 * Copyright (C) 2003, Robbert Haarman <purple@inglorion.net>
 * Copyright (C) 2003, 2012 Ethan Blanton <elb@pidgin.im>
 * Copyright (C) 2000-2003, Rob Flynn <rob@tgflinux.com>
 * Copyright (C) 1998-1999, Mark Spencer <markster@marko.net>
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

#include <errno.h>

#include <glib/gi18n-lib.h>

#include <gplugin.h>
#include <gplugin-native.h>

#include <purple.h>

#include "irc.h"

#define PING_TIMEOUT 60

struct _IRCProtocol {
	PurpleProtocol parent;
};

static void irc_ison_buddy_init(char *name, struct irc_buddy *ib, GList **list);

static GList *irc_status_types(PurpleProtocol *protocol, PurpleAccount *account);
/* static GList *irc_chat_info(PurpleConnection *gc); */
static void irc_login(PurpleProtocol *protocol, PurpleAccount *account);
static void irc_login_cb(GObject *source, GAsyncResult *res, gpointer user_data);
static void irc_close(PurpleProtocol *protocol, PurpleConnection *gc);
static int irc_im_send(PurpleProtocolIM *im, PurpleConnection *gc, PurpleMessage *msg);
static int irc_chat_send(PurpleProtocolChat *protocol_chat, PurpleConnection *gc, int id, PurpleMessage *msg);
static void irc_chat_join(PurpleProtocolChat *protocol_chat, PurpleConnection *gc, GHashTable *data);
static void irc_read_input_cb(GObject *source, GAsyncResult *res, gpointer data);

static guint irc_nick_hash(const char *nick);
static gboolean irc_nick_equal(const char *nick1, const char *nick2);
static void irc_buddy_free(struct irc_buddy *ib);

PurpleProtocol *_irc_protocol = NULL;

static gint
irc_uri_handler_match_server(PurpleAccount *account, const gchar *match_server)
{
	const gchar *protocol_id;
	const gchar *username;
	gchar *server;

	protocol_id = purple_account_get_protocol_id(account);

	if (!purple_strequal(protocol_id, "prpl-irc") ||
			!purple_account_is_connected(account)) {
		return -1;
	}

	if (match_server == NULL || match_server[0] == '\0') {
		/* No server specified, match any IRC account */
		return 0;
	}

	username = purple_contact_info_get_username(PURPLE_CONTACT_INFO(account));
	server = strchr(username, '@');

	/* +1 to skip '@' */
	if (server == NULL || !purple_strequal(match_server, server + 1)) {
		return -1;
	}

	return 0;
}

static gboolean
irc_uri_handler(const gchar *scheme, const gchar *uri, GHashTable *params)
{
	PurpleAccountManager *manager = NULL;
	gchar *target;
	gchar *server;
	gchar **target_tokens;
	PurpleAccount *account;
	gchar **modifier;
	gboolean isnick = FALSE;

	g_return_val_if_fail(uri != NULL, FALSE);

	if (!purple_strequal(scheme, "irc")) {
		/* Not a scheme we handle here */
		return FALSE;
	}

	if (g_str_has_prefix(uri, "//")) {
		/* Skip initial '//' if it exists */
		uri += 2;
	}

	/* Find the target (aka room or user) */
	target = strchr(uri, '/');

	/* [1] to skip the '/' */
	if (target == NULL || target[1] == '\0') {
		purple_debug_warning("irc",
				"URI missing valid target: %s", uri);
		return FALSE;
	}

	server = g_strndup(uri, target - uri);

	/* Find account with correct server */
	manager = purple_account_manager_get_default();
	account = purple_account_manager_find_custom(manager,
	                                             (GEqualFunc)irc_uri_handler_match_server,
	                                             server);

	if (account == NULL) {
		purple_debug_warning("irc",
				"No account online on '%s' for handling URI",
				server);
		g_free(server);
		return FALSE;
	}

	/* Tokenize modifiers, +1 to skip the initial '/' */
	target_tokens = g_strsplit(target + 1, ",", 0);
	target = g_strdup_printf("#%s", target_tokens[0]);

	/* Parse modifiers, start at 1 to skip the actual target */
	for (modifier = target_tokens + 1; *modifier != NULL; ++modifier) {
		if (purple_strequal(*modifier, "isnick")) {
			isnick = TRUE;
			break;
		}
	}

	g_strfreev(target_tokens);

	if (isnick) {
		PurpleConversation *im;

		/* 'server' isn't needed here. Free it immediately. */
		g_free(server);

		/* +1 to skip '#' target prefix */
		im = purple_im_conversation_new(account, target + 1);
		g_free(target);

		purple_conversation_present(im);

		if (params != NULL) {
			const gchar *msg = g_hash_table_lookup(params, "msg");

			if (msg != NULL) {
				purple_conversation_send_confirm(im, msg);
			}
		}

		return TRUE;
	} else {
		GHashTable *components;

		components = g_hash_table_new_full(g_str_hash, g_str_equal,
				NULL, g_free);

		/* Transfer ownership of these to the hash table */
		g_hash_table_insert(components, "server", server);
		g_hash_table_insert(components, "channel", target);

		if (params != NULL) {
			const gchar *key = g_hash_table_lookup(params, "key");

			if (key != NULL) {
				g_hash_table_insert(components, "password",
						g_strdup(key));
			}
		}

		purple_serv_join_chat(purple_account_get_connection(account),
				components);
		g_hash_table_destroy(components);
		return TRUE;
	}

	return FALSE;
}

static void
irc_view_motd(G_GNUC_UNUSED GSimpleAction *action,
              GVariant *parameter,
              G_GNUC_UNUSED gpointer data)
{
	const gchar *account_id = NULL;
	PurpleAccountManager *manager = NULL;
	PurpleAccount *account = NULL;
	PurpleConnection *gc = NULL;
	struct irc_conn *irc;
	char *title, *body;

	if(!g_variant_is_of_type(parameter, G_VARIANT_TYPE_STRING)) {
		g_critical("IRC View MOTD action parameter is of incorrect type %s",
		           g_variant_get_type_string(parameter));
	}

	account_id = g_variant_get_string(parameter, NULL);
	manager = purple_account_manager_get_default();
	account = purple_account_manager_find_by_id(manager, account_id);
	gc = purple_account_get_connection(account);

	if (gc == NULL || purple_connection_get_protocol_data(gc) == NULL) {
		purple_debug_error("irc", "got MOTD request for NULL gc");
		return;
	}
	irc = purple_connection_get_protocol_data(gc);
	if (irc->motd == NULL) {
		purple_notify_error(gc, _("Error displaying MOTD"),
			_("No MOTD available"),
			_("There is no MOTD associated with this connection."),
			purple_request_cpar_from_connection(gc));
		return;
	}
	title = g_strdup_printf(_("MOTD for %s"), irc->server);
	body = g_strdup_printf("<span style=\"font-family: monospace;\">%s</span>", irc->motd->str);
	purple_notify_formatted(gc, title, title, NULL, body, NULL, NULL);
	g_free(title);
	g_free(body);
}

static int
irc_send_raw(G_GNUC_UNUSED PurpleProtocolServer *protocol_server,
             PurpleConnection *gc, const gchar *buf, gint len)
{
	struct irc_conn *irc = purple_connection_get_protocol_data(gc);
	if (len == -1) {
		len = strlen(buf);
	}
	irc_send_len(irc, buf, len);
	return len;
}

static void
irc_push_bytes_cb(GObject *source, GAsyncResult *res, gpointer data)
{
	PurpleQueuedOutputStream *stream = PURPLE_QUEUED_OUTPUT_STREAM(source);
	PurpleConnection *gc = data;
	gboolean result;
	GError *error = NULL;

	result = purple_queued_output_stream_push_bytes_finish(stream,
			res, &error);

	if (!result) {
		purple_queued_output_stream_clear_queue(stream);

		g_prefix_error(&error, "%s", _("Lost connection with server: "));
		purple_connection_take_error(gc, error);
		return;
	}
}

int irc_send(struct irc_conn *irc, const char *buf)
{
    return irc_send_len(irc, buf, strlen(buf));
}

int
irc_send_len(struct irc_conn *irc, const char *buf, G_GNUC_UNUSED int buflen) {
 	char *tosend = g_strdup(buf);
	int len;
	GBytes *data;

	purple_signal_emit(_irc_protocol, "irc-sending-text", purple_account_get_connection(irc->account), &tosend);

	if (tosend == NULL)
		return 0;

	if (purple_debug_is_verbose()) {
		gchar *clean = g_utf8_make_valid(tosend, -1);
		clean = g_strstrip(clean);
		purple_debug_misc("irc", "<< %s\n", clean);
		g_free(clean);
	}

	len = strlen(tosend);
	data = g_bytes_new_take(tosend, len);
	purple_queued_output_stream_push_bytes_async(irc->output, data,
			G_PRIORITY_DEFAULT, irc->cancellable, irc_push_bytes_cb,
			purple_account_get_connection(irc->account));
	g_bytes_unref(data);

	return len;
}

/* XXX I don't like messing directly with these buddies */
gboolean irc_blist_timeout(struct irc_conn *irc)
{
	if (irc->ison_outstanding) {
		return TRUE;
	}

	g_hash_table_foreach(irc->buddies, (GHFunc)irc_ison_buddy_init,
	                     (gpointer *)&irc->buddies_outstanding);

	irc_buddy_query(irc);

	return TRUE;
}

void irc_buddy_query(struct irc_conn *irc)
{
	GList *lp;
	GString *string;
	struct irc_buddy *ib;
	char *buf;

	string = g_string_sized_new(512);

	while ((lp = g_list_first(irc->buddies_outstanding))) {
		ib = (struct irc_buddy *)lp->data;
		if (string->len + strlen(ib->name) + 1 > 450)
			break;
		g_string_append_printf(string, "%s ", ib->name);
		ib->new_online_status = FALSE;
		irc->buddies_outstanding = g_list_delete_link(irc->buddies_outstanding, lp);
	}

	if (string->len) {
		buf = irc_format(irc, "vn", "ISON", string->str);
		irc_send(irc, buf);
		g_free(buf);
		irc->ison_outstanding = TRUE;
	} else
		irc->ison_outstanding = FALSE;

	g_string_free(string, TRUE);
}

static void
irc_ison_buddy_init(G_GNUC_UNUSED char *name, struct irc_buddy *ib,
                    GList **list)
{
	*list = g_list_append(*list, ib);
}


static void irc_ison_one(struct irc_conn *irc, struct irc_buddy *ib)
{
	char *buf;

	if (irc->buddies_outstanding != NULL) {
		irc->buddies_outstanding = g_list_append(irc->buddies_outstanding, ib);
		return;
	}

	ib->new_online_status = FALSE;
	buf = irc_format(irc, "vn", "ISON", ib->name);
	irc_send(irc, buf);
	g_free(buf);
}

static GList *
irc_protocol_get_account_options(G_GNUC_UNUSED PurpleProtocol *protocol) {
	PurpleAccountOption *option;
	GList *opts = NULL;

	option = purple_account_option_int_new(_("Port"), "port",
	                                       IRC_DEFAULT_PORT);
	opts = g_list_append(opts, option);

	option = purple_account_option_string_new(_("Encodings"), "encoding",
	                                          IRC_DEFAULT_CHARSET);
	opts = g_list_append(opts, option);

	option = purple_account_option_bool_new(_("Auto-detect incoming UTF-8"),
	                                        "autodetect_utf8",
	                                        IRC_DEFAULT_AUTODETECT);
	opts = g_list_append(opts, option);

	option = purple_account_option_string_new(_("Ident name"), "username", "");
	opts = g_list_append(opts, option);

	option = purple_account_option_string_new(_("Real name"), "realname", "");
	opts = g_list_append(opts, option);

	/*
	option = purple_account_option_string_new(_("Quit message"), "quitmsg",
	                                          IRC_DEFAULT_QUIT);
	opts = g_list_append(opts, option);
	*/

	option = purple_account_option_bool_new(_("Use SSL"), "ssl", FALSE);
	opts = g_list_append(opts, option);

	option = purple_account_option_bool_new(_("Authenticate with SASL"),
	                                        "sasl", FALSE);
	opts = g_list_append(opts, option);

	option = purple_account_option_bool_new(_("Allow plaintext SASL auth over "
	                                          "unencrypted connection"),
	                                        "auth_plain_in_clear", FALSE);
	opts = g_list_append(opts, option);

	return opts;
}

static GList *
irc_protocol_get_user_splits(G_GNUC_UNUSED PurpleProtocol *protocol) {
	PurpleAccountUserSplit *split;

	split = purple_account_user_split_new(_("Server"), IRC_DEFAULT_SERVER,
	                                      '@');

	return g_list_append(NULL, split);
}

static GList *
irc_status_types(G_GNUC_UNUSED PurpleProtocol *protocol,
                 G_GNUC_UNUSED PurpleAccount *account)
{
	PurpleStatusType *type;
	GList *types = NULL;

	type = purple_status_type_new(PURPLE_STATUS_AVAILABLE, NULL, NULL, TRUE);
	types = g_list_append(types, type);

	type = purple_status_type_new_with_attrs(
		PURPLE_STATUS_AWAY, NULL, NULL, TRUE, TRUE, FALSE,
		"message", _("Message"), purple_value_new(G_TYPE_STRING),
		NULL);
	types = g_list_append(types, type);

	type = purple_status_type_new(PURPLE_STATUS_OFFLINE, NULL, NULL, TRUE);
	types = g_list_append(types, type);

	return types;
}

static const gchar *
irc_protocol_actions_get_prefix(G_GNUC_UNUSED PurpleProtocolActions *actions) {
	return "prpl-irc";
}

static GActionGroup *
irc_protocol_actions_get_action_group(G_GNUC_UNUSED PurpleProtocolActions *actions,
                                      G_GNUC_UNUSED PurpleConnection *connection)
{
	GSimpleActionGroup *group = NULL;
	GActionEntry entries[] = {
		{
			.name = "view-motd",
			.activate = irc_view_motd,
			.parameter_type = "s",
		},
	};
	gsize nentries = G_N_ELEMENTS(entries);

	group = g_simple_action_group_new();
	g_action_map_add_action_entries(G_ACTION_MAP(group), entries, nentries,
	                                NULL);

	return G_ACTION_GROUP(group);
}

static GMenu *
irc_protocol_actions_get_menu(G_GNUC_UNUSED PurpleProtocolActions *actions,
                              G_GNUC_UNUSED PurpleConnection *connection)
{
	GMenu *menu = NULL;
	GMenuItem *item = NULL;

	menu = g_menu_new();

	item = g_menu_item_new(_("View MOTD"), "prpl-irc.view-motd");
	g_menu_item_set_attribute(item, PURPLE_MENU_ATTRIBUTE_DYNAMIC_TARGET, "s",
	                          "account");
	g_menu_append_item(menu, item);

	g_object_unref(item);


	return menu;
}

static GList *
irc_chat_join_info(G_GNUC_UNUSED PurpleProtocolChat *protocol_chat,
                   G_GNUC_UNUSED PurpleConnection *gc)
{
	GList *m = NULL;
	PurpleProtocolChatEntry *pce;

	pce = g_new0(PurpleProtocolChatEntry, 1);
	pce->label = _("_Channel");
	pce->identifier = "channel";
	pce->required = TRUE;
	m = g_list_append(m, pce);

	pce = g_new0(PurpleProtocolChatEntry, 1);
	pce->label = _("_Password");
	pce->identifier = "password";
	pce->secret = TRUE;
	m = g_list_append(m, pce);

	return m;
}

static GHashTable *
irc_chat_info_defaults(G_GNUC_UNUSED PurpleProtocolChat *protocol_chat,
                       G_GNUC_UNUSED PurpleConnection *gc,
                       const gchar *chat_name)
{
	GHashTable *defaults;

	defaults = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

	if (chat_name != NULL)
		g_hash_table_insert(defaults, "channel", g_strdup(chat_name));

	return defaults;
}

static void
irc_login(G_GNUC_UNUSED PurpleProtocol *protocol, PurpleAccount *account) {
	PurpleConnection *gc;
	struct irc_conn *irc;
	char **userparts;
	const char *username = NULL;
	GSocketClient *client;
	GError *error = NULL;

	username = purple_contact_info_get_username(PURPLE_CONTACT_INFO(account));

	gc = purple_account_get_connection(account);
	purple_connection_set_flags(gc, PURPLE_CONNECTION_FLAG_NO_NEWLINES |
		PURPLE_CONNECTION_FLAG_NO_IMAGES);

	if (strpbrk(username, " \t\v\r\n") != NULL) {
		purple_connection_take_error(gc, g_error_new_literal(
			PURPLE_CONNECTION_ERROR,
			PURPLE_CONNECTION_ERROR_INVALID_SETTINGS,
			_("IRC nick and server may not contain whitespace")));
		return;
	}

	irc = g_new0(struct irc_conn, 1);
	purple_connection_set_protocol_data(gc, irc);
	irc->account = account;
	irc->cancellable = g_cancellable_new();

	userparts = g_strsplit(username, "@", 2);
	purple_connection_set_display_name(gc, userparts[0]);
	irc->server = g_strdup(userparts[1]);
	g_strfreev(userparts);

	irc->buddies = g_hash_table_new_full((GHashFunc)irc_nick_hash, (GEqualFunc)irc_nick_equal,
					     NULL, (GDestroyNotify)irc_buddy_free);
	irc->cmds = g_hash_table_new(g_str_hash, g_str_equal);
	irc_cmd_table_build(irc);
	irc->msgs = g_hash_table_new(g_str_hash, g_str_equal);
	irc_msg_table_build(irc);

	client = purple_gio_socket_client_new(account, &error);

	if (client == NULL) {
		purple_connection_take_error(gc, error);
		return;
	}

	/* Optionally use TLS if it's set in the account settings */
	g_socket_client_set_tls(client,
			purple_account_get_bool(account, "ssl", FALSE));

	g_socket_client_connect_to_host_async(client, irc->server,
			purple_account_get_int(account, "port",
					g_socket_client_get_tls(client) ?
							IRC_DEFAULT_SSL_PORT :
							IRC_DEFAULT_PORT),
			irc->cancellable, irc_login_cb, gc);
	g_object_unref(client);
}

static gboolean do_login(PurpleConnection *gc) {
	char *buf = NULL;
	char *server;
	const char *nickname, *identname, *realname;
	struct irc_conn *irc = purple_connection_get_protocol_data(gc);
	const char *pass = purple_connection_get_password(gc);
	const gboolean use_sasl = purple_account_get_bool(irc->account, "sasl", FALSE);

	if (pass && *pass) {
		if (use_sasl)
			buf = irc_format(irc, "vv:", "CAP", "REQ", "sasl");
		else
			buf = irc_format(irc, "v:", "PASS", pass);
		if (irc_send(irc, buf) < 0) {
			g_free(buf);
			return FALSE;
		}
		g_free(buf);
	}

	nickname = purple_connection_get_display_name(gc);

	realname = purple_account_get_string(irc->account, "realname", "");
	if(realname == NULL || *realname == '\0') {
		realname = IRC_DEFAULT_ALIAS;
	}

	identname = purple_account_get_string(irc->account, "username", "");
	if(identname == NULL || *identname == '\0') {
		identname = nickname;
	}

	if (*irc->server == ':') {
		/* Same as hostname, above. */
		server = g_strdup_printf("0%s", irc->server);
	} else {
		server = g_strdup(irc->server);
	}

	buf = irc_format(irc, "vvvv:", "USER", identname, "*", server, realname);
	g_free(server);
	if (irc_send(irc, buf) < 0) {
		g_free(buf);
		return FALSE;
	}
	g_free(buf);
	buf = irc_format(irc, "vn", "NICK", nickname);
	irc->reqnick = g_strdup(nickname);
	irc->nickused = FALSE;
	if (irc_send(irc, buf) < 0) {
		g_free(buf);
		return FALSE;
	}
	g_free(buf);

	irc->recv_time = time(NULL);

	return TRUE;
}

static void
irc_login_cb(GObject *source, GAsyncResult *res, gpointer user_data)
{
	PurpleConnection *gc = user_data;
	GSocketConnection *conn;
	GError *error = NULL;
	struct irc_conn *irc;

	conn = g_socket_client_connect_to_host_finish(G_SOCKET_CLIENT(source),
			res, &error);

	if (conn == NULL) {
		g_prefix_error(&error, "%s", _("Unable to connect: "));
		purple_connection_take_error(gc, error);
		return;
	}

	irc = purple_connection_get_protocol_data(gc);
	irc->conn = conn;
	irc->output = purple_queued_output_stream_new(
			g_io_stream_get_output_stream(G_IO_STREAM(irc->conn)));

	if (do_login(gc)) {
		irc->input = g_data_input_stream_new(
				g_io_stream_get_input_stream(
						G_IO_STREAM(irc->conn)));
		g_data_input_stream_read_line_async(irc->input,
				G_PRIORITY_DEFAULT, irc->cancellable,
				irc_read_input_cb, gc);
	}
}

static void
irc_close(G_GNUC_UNUSED PurpleProtocol *protocol, PurpleConnection *gc) {
	struct irc_conn *irc = purple_connection_get_protocol_data(gc);

	if (irc == NULL)
		return;

	if (irc->conn != NULL)
		irc_cmd_quit(irc, "quit", NULL, NULL);

	if (irc->cancellable != NULL) {
		g_cancellable_cancel(irc->cancellable);
		g_clear_object(&irc->cancellable);
	}

	if (irc->conn != NULL) {
		purple_gio_graceful_close(G_IO_STREAM(irc->conn),
				G_INPUT_STREAM(irc->input),
				G_OUTPUT_STREAM(irc->output));
	}

	g_clear_object(&irc->input);
	g_clear_object(&irc->output);
	g_clear_object(&irc->conn);

	if (irc->timer)
		g_source_remove(irc->timer);
	g_hash_table_destroy(irc->cmds);
	g_hash_table_destroy(irc->msgs);
	g_hash_table_destroy(irc->buddies);
	if (irc->motd)
		g_string_free(irc->motd, TRUE);
	g_free(irc->server);

	g_free(irc->mode_chars);
	g_free(irc->reqnick);

	g_clear_object(&irc->hasl_ctx);

	g_free(irc);
}

static int
irc_im_send(G_GNUC_UNUSED PurpleProtocolIM *im, PurpleConnection *gc,
            PurpleMessage *msg)
{
	struct irc_conn *irc = purple_connection_get_protocol_data(gc);
	char *plain;
	const char *args[2];

	args[0] = irc_nick_skip_mode(irc, purple_message_get_recipient(msg));

	purple_markup_html_to_xhtml(purple_message_get_contents(msg),
		NULL, &plain);
	args[1] = plain;

	irc_cmd_privmsg(irc, "msg", NULL, args);
	g_free(plain);
	return 1;
}

static void
irc_get_info(G_GNUC_UNUSED PurpleProtocolServer *protocol_server,
             PurpleConnection *gc, const gchar *who)
{
	struct irc_conn *irc = purple_connection_get_protocol_data(gc);
	const char *args[2];
	args[0] = who;
	args[1] = NULL;
	irc_cmd_whois(irc, "whois", NULL, args);
}

static void
irc_set_status(G_GNUC_UNUSED PurpleProtocolServer *protocol_server,
               PurpleAccount *account, PurpleStatus *status)
{
	PurpleConnection *gc = purple_account_get_connection(account);
	struct irc_conn *irc;
	const char *args[1];
	const char *status_id = purple_status_get_id(status);

	g_return_if_fail(gc != NULL);
	irc = purple_connection_get_protocol_data(gc);

	if (!purple_status_is_active(status))
		return;

	args[0] = NULL;

	if (purple_strequal(status_id, "away")) {
		args[0] = purple_status_get_attr_string(status, "message");
		if ((args[0] == NULL) || (*args[0] == '\0'))
			args[0] = _("Away");
		irc_cmd_away(irc, "away", NULL, args);
	} else if (purple_strequal(status_id, "available")) {
		irc_cmd_away(irc, "back", NULL, args);
	}
}

static void
irc_add_buddy(G_GNUC_UNUSED PurpleProtocolServer *protocol_server,
              PurpleConnection *gc, PurpleBuddy *buddy,
              G_GNUC_UNUSED PurpleGroup *group,
              G_GNUC_UNUSED const gchar *message)
{
	struct irc_conn *irc = purple_connection_get_protocol_data(gc);
	struct irc_buddy *ib;
	const char *bname = purple_buddy_get_name(buddy);

	ib = g_hash_table_lookup(irc->buddies, bname);
	if (ib != NULL) {
		ib->ref++;
		purple_protocol_got_user_status(irc->account, bname,
				ib->online ? "available" : "offline", NULL);
	} else {
		ib = g_new0(struct irc_buddy, 1);
		ib->name = g_strdup(bname);
		ib->ref = 1;
		g_hash_table_replace(irc->buddies, ib->name, ib);
	}

	/* if the timer isn't set, this is during signon, so we don't want to flood
	 * ourself off with ISON's, so we don't, but after that we want to know when
	 * someone's online asap */
	if (irc->timer)
		irc_ison_one(irc, ib);
}

static void
irc_remove_buddy(G_GNUC_UNUSED PurpleProtocolServer *protocol_server,
                 PurpleConnection *gc, PurpleBuddy *buddy,
                 G_GNUC_UNUSED PurpleGroup *group)
{
	struct irc_conn *irc = purple_connection_get_protocol_data(gc);
	struct irc_buddy *ib;

	ib = g_hash_table_lookup(irc->buddies, purple_buddy_get_name(buddy));
	if (ib && --ib->ref == 0) {
		g_hash_table_remove(irc->buddies, purple_buddy_get_name(buddy));
	}
}

static void
irc_read_input_cb(GObject *source, GAsyncResult *res, gpointer data)
{
	PurpleConnection *gc = data;
	struct irc_conn *irc;
	gchar *line;
	gsize len;
	gsize start = 0;
	GError *error = NULL;

	line = g_data_input_stream_read_line_finish(
			G_DATA_INPUT_STREAM(source), res, &len, &error);

	if (line == NULL && error != NULL) {
		g_prefix_error(&error, "%s", _("Lost connection with server: "));
		purple_connection_take_error(gc, error);
		return;
	} else if (line == NULL) {
		purple_connection_take_error(gc, g_error_new_literal(
			PURPLE_CONNECTION_ERROR,
			PURPLE_CONNECTION_ERROR_NETWORK_ERROR,
			_("Server closed the connection")));
		return;
	}

	irc = purple_connection_get_protocol_data(gc);

	purple_connection_update_last_received(gc);

	if (len > 0 && line[len - 1] == '\r')
		line[len - 1] = '\0';

	/* This is a hack to work around the fact that marv gets messages
	 * with null bytes in them while using some weird irc server at work
 	 */
	while (start < len && line[start] == '\0')
		++start;

	if (start < len) {
		irc_parse_msg(irc, line + start);
	}

	g_free(line);

	g_data_input_stream_read_line_async(irc->input,
			G_PRIORITY_DEFAULT, irc->cancellable,
			irc_read_input_cb, gc);
}

static void
irc_chat_join(G_GNUC_UNUSED PurpleProtocolChat *protocol_chat,
              PurpleConnection *gc, GHashTable *data)
{
	struct irc_conn *irc = purple_connection_get_protocol_data(gc);
	const char *args[2];

	args[0] = g_hash_table_lookup(data, "channel");
	args[1] = g_hash_table_lookup(data, "password");
	irc_cmd_join(irc, "join", NULL, args);
}

static gchar *
irc_get_chat_name(G_GNUC_UNUSED PurpleProtocolChat *protocol_chat,
                  GHashTable *data)
{
	return g_strdup(g_hash_table_lookup(data, "channel"));
}

static void
irc_chat_invite(G_GNUC_UNUSED PurpleProtocolChat *protocol_chat,
                PurpleConnection *gc, gint id,
                G_GNUC_UNUSED const gchar *message, const gchar *name)
{
	struct irc_conn *irc = purple_connection_get_protocol_data(gc);
	PurpleConversation *convo;
	PurpleConversationManager *manager;
	const char *args[2];

	manager = purple_conversation_manager_get_default();
	convo = purple_conversation_manager_find_chat_by_id(manager,
	                                                    purple_connection_get_account(gc),
	                                                    id);
	if (!convo) {
		purple_debug_error("irc", "Got chat invite request for bogus chat");
		return;
	}
	args[0] = name;
	args[1] = purple_conversation_get_name(convo);
	irc_cmd_invite(irc, "invite", purple_conversation_get_name(convo), args);
}


static void
irc_chat_leave(G_GNUC_UNUSED PurpleProtocolChat *protocol_chat,
               PurpleConnection *gc, gint id)
{
	struct irc_conn *irc = purple_connection_get_protocol_data(gc);
	PurpleConversation *convo;
	PurpleConversationManager *manager;
	const char *args[2];

	manager = purple_conversation_manager_get_default();
	convo = purple_conversation_manager_find_chat_by_id(manager,
	                                                    purple_connection_get_account(gc),
	                                                    id);
	if (!convo) {
		return;
	}

	args[0] = purple_conversation_get_name(convo);
	args[1] = NULL;
	irc_cmd_part(irc, "part", purple_conversation_get_name(convo), args);
	purple_serv_got_chat_left(gc, id);
}

static gint
irc_chat_send(G_GNUC_UNUSED PurpleProtocolChat *protocol_chat,
              PurpleConnection *gc, gint id, PurpleMessage *msg)
{
	struct irc_conn *irc = purple_connection_get_protocol_data(gc);
	PurpleConversation *convo;
	PurpleConversationManager *manager;
	const char *args[2];
	char *tmp;

	manager = purple_conversation_manager_get_default();
	convo = purple_conversation_manager_find_chat_by_id(manager,
	                                                    purple_connection_get_account(gc),
	                                                    id);

	if (!convo) {
		purple_debug_error("irc", "chat send on nonexistent chat");
		return -EINVAL;
	}
	purple_markup_html_to_xhtml(purple_message_get_contents(msg), NULL, &tmp);
	args[0] = purple_conversation_get_name(convo);
	args[1] = tmp;

	irc_cmd_privmsg(irc, "msg", NULL, args);

	/* TODO: use msg */
	purple_serv_got_chat_in(gc, id, purple_connection_get_display_name(gc),
		purple_message_get_flags(msg),
		purple_message_get_contents(msg), time(NULL));
	g_free(tmp);
	return 0;
}

static guint irc_nick_hash(const char *nick)
{
	char *lc;
	guint bucket;

	lc = g_utf8_strdown(nick, -1);
	bucket = g_str_hash(lc);
	g_free(lc);

	return bucket;
}

static gboolean irc_nick_equal(const char *nick1, const char *nick2)
{
	return (purple_utf8_strcasecmp(nick1, nick2) == 0);
}

static void irc_buddy_free(struct irc_buddy *ib)
{
	g_free(ib->name);
	g_free(ib);
}

static void
irc_chat_set_topic(G_GNUC_UNUSED PurpleProtocolChat *protocol_chat,
                   PurpleConnection *gc, gint id, const gchar *topic)
{
	PurpleConversation *conv;
	PurpleConversationManager *manager;
	char *buf;
	const char *name = NULL;
	struct irc_conn *irc;

	manager = purple_conversation_manager_get_default();
	conv = purple_conversation_manager_find_chat_by_id(manager,
	                                                   purple_connection_get_account(gc),
	                                                   id);

	irc = purple_connection_get_protocol_data(gc);
	name = purple_conversation_get_name(conv);

	if (name == NULL) {
		return;
	}

	buf = irc_format(irc, "vt:", "TOPIC", name, topic);
	irc_send(irc, buf);
	g_free(buf);
}

static PurpleRoomlist *
irc_roomlist_get_list(G_GNUC_UNUSED PurpleProtocolRoomlist *protocol_roomlist,
                      PurpleConnection *gc)
{
	struct irc_conn *irc;
	char *buf;

	irc = purple_connection_get_protocol_data(gc);

	if (irc->roomlist)
		g_object_unref(irc->roomlist);

	irc->roomlist = purple_roomlist_new(purple_connection_get_account(gc));

	buf = irc_format(irc, "v", "LIST");
	irc_send(irc, buf);
	g_free(buf);

	return irc->roomlist;
}

static void
irc_roomlist_cancel(G_GNUC_UNUSED PurpleProtocolRoomlist *protocol_roomlist,
                    PurpleRoomlist *list)
{
	PurpleAccount *account = purple_roomlist_get_account(list);
	PurpleConnection *gc = purple_account_get_connection(account);
	struct irc_conn *irc;

	if (gc == NULL)
		return;

	irc = purple_connection_get_protocol_data(gc);

	purple_roomlist_set_in_progress(list, FALSE);

	if (irc->roomlist == list) {
		irc->roomlist = NULL;
		g_object_unref(list);
	}
}

static void
irc_keepalive(G_GNUC_UNUSED PurpleProtocolServer *protocol_server,
              PurpleConnection *gc)
{
	struct irc_conn *irc = purple_connection_get_protocol_data(gc);
	if ((time(NULL) - irc->recv_time) > PING_TIMEOUT)
		irc_cmd_ping(irc, NULL, NULL, NULL);
}

static const char *
irc_normalize(G_GNUC_UNUSED PurpleProtocolClient *client,
              G_GNUC_UNUSED PurpleAccount *account,
              const char *who)
{
	return purple_normalize_nocase(who);
}

static gssize
irc_get_max_message_size(G_GNUC_UNUSED PurpleProtocolClient *client,
                         G_GNUC_UNUSED PurpleConversation *conv)
{
	/* TODO: this static value is got from pidgin-otr, but it depends on
	 * some factors, for example IRC channel name. */
	return 417;
}

static void
irc_protocol_init(G_GNUC_UNUSED IRCProtocol *self) {
}

static void
irc_protocol_class_init(IRCProtocolClass *klass)
{
	PurpleProtocolClass *protocol_class = PURPLE_PROTOCOL_CLASS(klass);

	protocol_class->login = irc_login;
	protocol_class->close = irc_close;
	protocol_class->status_types = irc_status_types;

	protocol_class->get_account_options = irc_protocol_get_account_options;
	protocol_class->get_user_splits = irc_protocol_get_user_splits;
}

static void
irc_protocol_class_finalize(G_GNUC_UNUSED IRCProtocolClass *klass)
{
}

static void
irc_protocol_actions_iface_init(PurpleProtocolActionsInterface *iface)
{
	iface->get_prefix = irc_protocol_actions_get_prefix;
	iface->get_action_group = irc_protocol_actions_get_action_group;
	iface->get_menu = irc_protocol_actions_get_menu;
}

static void
irc_protocol_client_iface_init(PurpleProtocolClientInterface *client_iface)
{
	client_iface->normalize            = irc_normalize;
	client_iface->get_max_message_size = irc_get_max_message_size;
}

static void
irc_protocol_server_iface_init(PurpleProtocolServerInterface *server_iface)
{
	server_iface->set_status   = irc_set_status;
	server_iface->get_info     = irc_get_info;
	server_iface->add_buddy    = irc_add_buddy;
	server_iface->remove_buddy = irc_remove_buddy;
	server_iface->keepalive    = irc_keepalive;
	server_iface->send_raw     = irc_send_raw;
}

static void
irc_protocol_im_iface_init(PurpleProtocolIMInterface *im_iface)
{
	im_iface->send = irc_im_send;
}

static void
irc_protocol_chat_iface_init(PurpleProtocolChatInterface *chat_iface)
{
	chat_iface->info          = irc_chat_join_info;
	chat_iface->info_defaults = irc_chat_info_defaults;
	chat_iface->join          = irc_chat_join;
	chat_iface->get_name      = irc_get_chat_name;
	chat_iface->invite        = irc_chat_invite;
	chat_iface->leave         = irc_chat_leave;
	chat_iface->send          = irc_chat_send;
	chat_iface->set_topic     = irc_chat_set_topic;
}

static void
irc_protocol_roomlist_iface_init(PurpleProtocolRoomlistInterface *roomlist_iface)
{
	roomlist_iface->get_list = irc_roomlist_get_list;
	roomlist_iface->cancel   = irc_roomlist_cancel;
}

static void
irc_protocol_xfer_iface_init(PurpleProtocolXferInterface *xfer_iface)
{
	xfer_iface->send_file = irc_dccsend_send_file;
	xfer_iface->new_xfer  = irc_dccsend_new_xfer;
}

G_DEFINE_DYNAMIC_TYPE_EXTENDED(
	IRCProtocol,
	irc_protocol,
	PURPLE_TYPE_PROTOCOL,
	G_TYPE_FLAG_FINAL,
	G_IMPLEMENT_INTERFACE_DYNAMIC(PURPLE_TYPE_PROTOCOL_ACTIONS,
	                              irc_protocol_actions_iface_init)
	G_IMPLEMENT_INTERFACE_DYNAMIC(PURPLE_TYPE_PROTOCOL_CLIENT,
	                              irc_protocol_client_iface_init)
	G_IMPLEMENT_INTERFACE_DYNAMIC(PURPLE_TYPE_PROTOCOL_SERVER,
	                              irc_protocol_server_iface_init)
	G_IMPLEMENT_INTERFACE_DYNAMIC(PURPLE_TYPE_PROTOCOL_IM,
	                              irc_protocol_im_iface_init)
	G_IMPLEMENT_INTERFACE_DYNAMIC(PURPLE_TYPE_PROTOCOL_CHAT,
	                              irc_protocol_chat_iface_init)
	G_IMPLEMENT_INTERFACE_DYNAMIC(PURPLE_TYPE_PROTOCOL_ROOMLIST,
	                              irc_protocol_roomlist_iface_init)
	G_IMPLEMENT_INTERFACE_DYNAMIC(PURPLE_TYPE_PROTOCOL_XFER,
	                              irc_protocol_xfer_iface_init))

static PurpleProtocol *
irc_protocol_new(void) {
	return PURPLE_PROTOCOL(g_object_new(
		IRC_TYPE_PROTOCOL,
		"id", "prpl-irc",
		"name", "IRC",
		"description", _("Internet Relay Chat (IRC) is a text-based chat "
		                 "system."),
		"icon-name", "im-irc",
		"icon-resource-path", "/im/pidgin/libpurple/irc/icons",
		"options", OPT_PROTO_CHAT_TOPIC | OPT_PROTO_PASSWORD_OPTIONAL |
		           OPT_PROTO_SLASH_COMMANDS_NATIVE,
		NULL));
}

static GPluginPluginInfo *
irc_query(G_GNUC_UNUSED GError **error) {
	return purple_plugin_info_new(
		"id",           "prpl-irc",
		"name",         "IRC Protocol",
		"version",      DISPLAY_VERSION,
		"category",     N_("Protocol"),
		"summary",      N_("IRC Protocol Plugin"),
		"description",  N_("The IRC Protocol Plugin that Sucks Less"),
		"website",      PURPLE_WEBSITE,
		"abi-version",  PURPLE_ABI_VERSION,
		"flags",        PURPLE_PLUGIN_INFO_FLAGS_INTERNAL |
		                PURPLE_PLUGIN_INFO_FLAGS_AUTO_LOAD,
		NULL
	);
}

static gboolean
irc_load(GPluginPlugin *plugin, GError **error)
{
	PurpleProtocolManager *manager = purple_protocol_manager_get_default();

	irc_protocol_register_type(G_TYPE_MODULE(plugin));

	irc_xfer_register(G_TYPE_MODULE(plugin));

	_irc_protocol = irc_protocol_new();
	if(!purple_protocol_manager_register(manager, _irc_protocol, error)) {
		g_clear_object(&_irc_protocol);

		return FALSE;
	}

	purple_prefs_remove("/plugins/prpl/irc/quitmsg");
	purple_prefs_remove("/plugins/prpl/irc");

	irc_register_commands();

	purple_signal_register(_irc_protocol, "irc-sending-text",
			     purple_marshal_VOID__POINTER_POINTER, G_TYPE_NONE, 2,
			     PURPLE_TYPE_CONNECTION,
			     G_TYPE_POINTER); /* pointer to a string */
	purple_signal_register(_irc_protocol, "irc-receiving-text",
			     purple_marshal_VOID__POINTER_POINTER, G_TYPE_NONE, 2,
			     PURPLE_TYPE_CONNECTION,
			     G_TYPE_POINTER); /* pointer to a string */

	purple_signal_connect(purple_get_core(), "uri-handler", plugin,
			G_CALLBACK(irc_uri_handler), NULL);

	return TRUE;
}

static gboolean
irc_unload(GPluginPlugin *plugin, G_GNUC_UNUSED gboolean shutdown,
           GError **error)
{
	PurpleProtocolManager *manager = purple_protocol_manager_get_default();

	if(!purple_protocol_manager_unregister(manager, _irc_protocol, error)) {
		return FALSE;
	}

	irc_unregister_commands();

	purple_signal_disconnect(purple_get_core(), "uri-handler", plugin,
			G_CALLBACK(irc_uri_handler));

	g_clear_object(&_irc_protocol);

	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(irc)
