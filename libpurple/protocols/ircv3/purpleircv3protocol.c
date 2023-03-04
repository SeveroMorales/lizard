/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#include <glib/gi18n-lib.h>

#include "purpleircv3protocol.h"

#include "purpleircv3connection.h"
#include "purpleircv3core.h"
#include "purpleircv3protocolim.h"

/******************************************************************************
 * PurpleProtocol Implementation
 *****************************************************************************/
static GList *
purple_ircv3_protocol_get_user_splits(G_GNUC_UNUSED PurpleProtocol *protocol) {
	PurpleAccountUserSplit *split = NULL;
	GList *splits = NULL;

	split = purple_account_user_split_new(_("Server"),
	                                      PURPLE_IRCV3_DEFAULT_SERVER,
	                                      '@');
	splits = g_list_append(splits, split);

	return splits;
}

static GList *
purple_ircv3_protocol_get_account_options(G_GNUC_UNUSED PurpleProtocol *protocol)
{
	PurpleAccountOption *option;
	GList *options = NULL;

	option = purple_account_option_int_new(_("Port"), "port",
	                                       PURPLE_IRCV3_DEFAULT_TLS_PORT);
	options = g_list_append(options, option);

	option = purple_account_option_bool_new(_("Use TLS"), "use-tls", TRUE);
	options = g_list_append(options, option);

	option = purple_account_option_string_new(_("Server password"),
	                                          "server-password", "");
	options = g_list_append(options, option);

	option = purple_account_option_string_new(_("Ident name"), "ident", "");
	options = g_list_append(options, option);

	option = purple_account_option_string_new(_("Real name"), "real-name", "");
	options = g_list_append(options, option);

	option = purple_account_option_string_new(_("SASL login name"),
	                                          "sasl-login-name", "");
	options = g_list_append(options, option);

	option = purple_account_option_string_new(_("SASL mechanisms"),
	                                          "sasl-mechanisms", "");
	options = g_list_append(options, option);

	option = purple_account_option_bool_new(_("Allow plaintext SASL auth over "
	                                          "unencrypted connection"),
	                                        "plain-sasl-in-clear", FALSE);
	options = g_list_append(options, option);

	option = purple_account_option_int_new(_("Seconds between sending "
	                                         "messages"),
	                                       "rate-limit-interval", 2);
	options = g_list_append(options, option);

	option = purple_account_option_int_new(_("Maximum messages to send at "
	                                         "once"),
	                                       "rate-limit-burst", 5);
	options = g_list_append(options, option);

	return options;
}

static PurpleConnection *
purple_ircv3_protocol_create_connection(PurpleProtocol *protocol,
                                        PurpleAccount *account,
                                        const char *password,
                                        GError **error)
{
	const char *username = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL(protocol), NULL);
	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);

	/* Make sure the username (which includes the servername via usersplits),
	 * does not contain any whitespace.
	 */
	username = purple_contact_info_get_username(PURPLE_CONTACT_INFO(account));
	if(strpbrk(username, " \t\v\r\n") != NULL) {
		g_set_error(error,
		            PURPLE_CONNECTION_ERROR,
		            PURPLE_CONNECTION_ERROR_INVALID_SETTINGS,
		            _("IRC nick and server may not contain whitespace"));

		return NULL;
	}

	return g_object_new(
		PURPLE_IRCV3_TYPE_CONNECTION,
		"protocol", protocol,
		"account", account,
		"password", password,
		NULL);
}

static GList *
purple_ircv3_protocol_status_types(G_GNUC_UNUSED PurpleProtocol *protocol,
                                   G_GNUC_UNUSED PurpleAccount *account)
{
	PurpleStatusType *type = NULL;
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

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_DYNAMIC_TYPE_EXTENDED(
	PurpleIRCv3Protocol,
	purple_ircv3_protocol,
	PURPLE_TYPE_PROTOCOL,
	0,
	G_IMPLEMENT_INTERFACE_DYNAMIC(PURPLE_TYPE_PROTOCOL_IM,
	                              purple_ircv3_protocol_im_init))

static void
purple_ircv3_protocol_init(G_GNUC_UNUSED PurpleIRCv3Protocol *protocol) {
}

static void
purple_ircv3_protocol_class_finalize(G_GNUC_UNUSED PurpleIRCv3ProtocolClass *klass) {
}

static void
purple_ircv3_protocol_class_init(PurpleIRCv3ProtocolClass *klass) {
	PurpleProtocolClass *protocol_class = PURPLE_PROTOCOL_CLASS(klass);

	protocol_class->get_user_splits = purple_ircv3_protocol_get_user_splits;
	protocol_class->get_account_options =
		purple_ircv3_protocol_get_account_options;
	protocol_class->create_connection =
		purple_ircv3_protocol_create_connection;
	protocol_class->status_types = purple_ircv3_protocol_status_types;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
void
purple_ircv3_protocol_register(GPluginNativePlugin *plugin) {
	purple_ircv3_protocol_register_type(G_TYPE_MODULE(plugin));
}

PurpleProtocol *
purple_ircv3_protocol_new(void) {
	return g_object_new(
		PURPLE_IRCV3_TYPE_PROTOCOL,
		"id", "prpl-ircv3",
		"name", "IRCv3",
		"description", _("Version 3 of Internet Relay Chat (IRC)."),
		"icon-name", "im-ircv3",
		"icon-resource-path", "/im/pidgin/libpurple/ircv3/icons",
		"options", OPT_PROTO_CHAT_TOPIC | OPT_PROTO_PASSWORD_OPTIONAL |
		           OPT_PROTO_SLASH_COMMANDS_NATIVE,
		NULL);
}
