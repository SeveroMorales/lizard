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

#include "jabber.h"
#include "auth.h"

static PurpleXmlNode *finish_plaintext_authentication(JabberStream *js)
{
	PurpleXmlNode *auth;
	GString *response;
	gchar *enc_out;

	auth = purple_xmlnode_new("auth");
	purple_xmlnode_set_namespace(auth, NS_XMPP_SASL);

	response = g_string_new("");
	response = g_string_append_c(response, '\0');
	response = g_string_append(response, js->user->node);
	response = g_string_append_c(response, '\0');
	response = g_string_append(response,
			purple_connection_get_password(js->gc));

	enc_out = g_base64_encode((guchar *)response->str, response->len);

	purple_xmlnode_set_attrib(auth, "mechanism", "PLAIN");
	purple_xmlnode_insert_data(auth, enc_out, -1);
	g_free(enc_out);
	g_string_free(response, TRUE);

	return auth;
}

static void allow_plaintext_auth(PurpleAccount *account)
{
	PurpleConnection *gc = purple_account_get_connection(account);
	JabberStream *js = purple_connection_get_protocol_data(gc);
	PurpleXmlNode *response;

	purple_account_set_bool(account, "auth_plain_in_clear", TRUE);

	response = finish_plaintext_authentication(js);
	jabber_send(js, response);
	purple_xmlnode_free(response);
}

static void disallow_plaintext_auth(PurpleAccount *account)
{
	purple_connection_error(purple_account_get_connection(account),
		PURPLE_CONNECTION_ERROR_ENCRYPTION_ERROR,
		_("Server requires plaintext authentication over an unencrypted stream"));
}

static JabberSaslState
jabber_plain_start(JabberStream *js, G_GNUC_UNUSED PurpleXmlNode *packet,
                   PurpleXmlNode **response, G_GNUC_UNUSED char **error)
{
	PurpleAccount *account = purple_connection_get_account(js->gc);
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
	char *msg;

	if (jabber_stream_is_ssl(js) || purple_account_get_bool(account, "auth_plain_in_clear", FALSE)) {
		*response = finish_plaintext_authentication(js);
		return JABBER_SASL_STATE_OK;
	}

	msg = g_strdup_printf(_("%s requires plaintext authentication over an unencrypted connection.  Allow this and continue authentication?"),
			purple_contact_info_get_username(info));
	purple_request_yes_no(js->gc, _("Plaintext Authentication"),
			_("Plaintext Authentication"),
			msg,
			1,
			purple_request_cpar_from_account(account),
			account, allow_plaintext_auth, disallow_plaintext_auth);
	g_free(msg);
	return JABBER_SASL_STATE_CONTINUE;
}

static JabberSaslMech plain_mech = {
	0, /* priority */
	"PLAIN", /* name */
	jabber_plain_start,
	NULL, /* handle_challenge */
	NULL, /* handle_success */
	NULL, /* handle_failure */
	NULL  /* dispose */
};

JabberSaslMech *jabber_auth_get_plain_mech(void)
{
	return &plain_mech;
}
