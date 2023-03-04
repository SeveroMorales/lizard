/* purple
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

#include <config.h>

#include <glib/gi18n-lib.h>

#include <purple.h>

#include "xmpp.h"

struct _XMPPProtocol {
	JabberProtocol parent;
};

static GList *
xmpp_protocol_get_account_options(G_GNUC_UNUSED PurpleProtocol *protocol) {
	PurpleAccountOption *option;
	PurpleKeyValuePair *kvp = NULL;
	GList *opts = NULL, *encryption_values = NULL;

	/* build the list of encryption types we support */
	kvp = purple_key_value_pair_new(_("Require encryption"), "require_tls");
	encryption_values = g_list_append(encryption_values, kvp);

	kvp = purple_key_value_pair_new(_("Use encryption if available"),
	                                "opportunistic_tls");
	encryption_values = g_list_append(encryption_values, kvp);

	kvp = purple_key_value_pair_new(_("Use old-style SSL"), "old_ssl");
	encryption_values = g_list_append(encryption_values, kvp);

	/* build all the options */
	option = purple_account_option_list_new(_("Connection security"),
	                                        "connection_security",
	                                        encryption_values);
	opts = g_list_append(opts, option);

	option = purple_account_option_bool_new(_("Allow plaintext auth over "
	                                          "unencrypted streams"),
	                                        "auth_plain_in_clear", FALSE);
	opts = g_list_append(opts, option);

	option = purple_account_option_int_new(_("Connect port"), "port", 5222);
	opts = g_list_append(opts, option);

	option = purple_account_option_string_new(_("Connect server"),
	                                          "connect_server", NULL);
	opts = g_list_append(opts, option);

	option = purple_account_option_string_new(_("File transfer proxies"),
	                                          "ft_proxies", NULL);
	opts = g_list_append(opts, option);

	option = purple_account_option_string_new(_("BOSH URL"), "bosh_url", NULL);
	opts = g_list_append(opts, option);

	return opts;
}

static GList *
xmpp_protocol_get_user_splits(G_GNUC_UNUSED PurpleProtocol *protocol) {
	GList *splits = NULL;
	PurpleAccountUserSplit *split;

	/* Translators: 'domain' is used here in the context of Internet domains,
	 * e.g. pidgin.im.
	 */
	split = purple_account_user_split_new(_("Domain"), NULL, '@');
	purple_account_user_split_set_reverse(split, FALSE);
	splits = g_list_append(splits, split);

	split = purple_account_user_split_new(_("Resource"), "", '/');
	purple_account_user_split_set_reverse(split, FALSE);
	splits = g_list_append(splits, split);

	return splits;
}

static void
xmpp_protocol_init(G_GNUC_UNUSED XMPPProtocol *self) {
	purple_prefs_remove("/plugins/prpl/jabber");
}

static void
xmpp_protocol_class_init(XMPPProtocolClass *klass) {
	PurpleProtocolClass *protocol_class = PURPLE_PROTOCOL_CLASS(klass);

	protocol_class->get_account_options = xmpp_protocol_get_account_options;
	protocol_class->get_user_splits = xmpp_protocol_get_user_splits;
}

static void
xmpp_protocol_class_finalize(G_GNUC_UNUSED XMPPProtocolClass *klass)
{
}

G_DEFINE_DYNAMIC_TYPE(XMPPProtocol, xmpp_protocol, JABBER_TYPE_PROTOCOL);

/* This exists solely because the above macro makes xmpp_protocol_register_type
 * static. */
void
xmpp_protocol_register(PurplePlugin *plugin)
{
	xmpp_protocol_register_type(G_TYPE_MODULE(plugin));
}

PurpleProtocol *
xmpp_protocol_new(void) {
	PurpleProtocolOptions options;

	options = OPT_PROTO_CHAT_TOPIC | OPT_PROTO_UNIQUE_CHATNAME |
	          OPT_PROTO_MAIL_CHECK | OPT_PROTO_SLASH_COMMANDS_NATIVE |
	          OPT_PROTO_PASSWORD_OPTIONAL;

	return PURPLE_PROTOCOL(g_object_new(
		XMPP_TYPE_PROTOCOL,
		"id", XMPP_PROTOCOL_ID,
		"name", "XMPP",
		"description", _("Extensible Messaging and Presence Protocol for IM, "
		                 "voice, and video."),
		"icon-name", "im-jabber",
		"icon-resource-path", "/im/pidgin/libpurple/xmpp/icons",
		"options", options,
		NULL
	));
}
