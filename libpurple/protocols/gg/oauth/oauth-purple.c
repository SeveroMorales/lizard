/* purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * Rewritten from scratch during Google Summer of Code 2012
 * by Tomek Wasilczyk (http://www.wasilczyk.pl).
 *
 * Previously implemented by:
 *  - Arkadiusz Miskiewicz <misiek@pld.org.pl> - first implementation (2001);
 *  - Bartosz Oler <bartosz@bzimage.us> - reimplemented during GSoC 2005;
 *  - Krzysztof Klinikowski <grommasher@gmail.com> - some parts (2009-2011).
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

#include "oauth-purple.h"
#include "gg.h"

#include "oauth.h"
#include "../utils.h"
#include "../xml.h"

#include <purple.h>

#define GGP_OAUTH_RESPONSE_MAX 10240

typedef struct
{
	SoupMessage *msg;
	PurpleConnection *gc;
	ggp_oauth_request_cb callback;
	gpointer user_data;
	gchar *token;
	gchar *token_secret;

	gchar *sign_method, *sign_url;
} ggp_oauth_data;

static void ggp_oauth_data_free(ggp_oauth_data *data)
{
	g_object_unref(data->msg);
	g_free(data->token);
	g_free(data->token_secret);
	g_free(data->sign_method);
	g_free(data->sign_url);
	g_free(data);
}

static void
ggp_oauth_access_token_got(GObject *source, GAsyncResult *result,
                           gpointer user_data)
{
	ggp_oauth_data *data = user_data;
	GBytes *response_body = NULL;
	const char *buffer = NULL;
	gsize size = 0;
	gchar *token = NULL, *token_secret = NULL;
	PurpleXmlNode *xml;
	gboolean succ = TRUE;
	GError *error = NULL;

	response_body = soup_session_send_and_read_finish(SOUP_SESSION(source),
	                                                  result, &error);
	if(response_body == NULL) {
		purple_debug_error("gg", "ggp_oauth_access_token_got: failed: %s",
		                   error->message);
		ggp_oauth_data_free(data);
		g_error_free(error);
		return;
	}

	buffer = g_bytes_get_data(response_body, &size);
	xml = purple_xmlnode_from_str(buffer, size);
	g_bytes_unref(response_body);

	if (xml == NULL) {
		purple_debug_error("gg", "ggp_oauth_access_token_got: invalid xml");
		ggp_oauth_data_free(data);
		return;
	}

	succ &= ggp_xml_get_string(xml, "oauth_token", &token);
	succ &= ggp_xml_get_string(xml, "oauth_token_secret", &token_secret);
	purple_xmlnode_free(xml);
	if (!succ || strlen(token) < 10) {
		purple_debug_error("gg", "ggp_oauth_access_token_got: invalid xml - "
		                         "token is not present");
		g_free(token);
		g_free(token_secret);
		ggp_oauth_data_free(data);
		return;
	}

	if (data->sign_url) {
		PurpleAccount *account;
		PurpleContactInfo *info = NULL;
		gchar *auth;

		purple_debug_misc("gg", "ggp_oauth_access_token_got: got access token, "
		                        "returning signed url");

		account = purple_connection_get_account(data->gc);
		info = PURPLE_CONTACT_INFO(account);
		auth = gg_oauth_generate_header(
		        data->sign_method, data->sign_url,
		        purple_contact_info_get_username(info),
		        purple_connection_get_password(data->gc), token, token_secret);
		data->callback(data->gc, auth, data->user_data);
	} else {
		purple_debug_misc(
		        "gg",
		        "ggp_oauth_access_token_got: got access token, returning it");
		data->callback(data->gc, token, data->user_data);
	}

	g_free(token);
	g_free(token_secret);
	ggp_oauth_data_free(data);
}

static void
ggp_oauth_authorization_done(GObject *source,
                             G_GNUC_UNUSED GAsyncResult *result,
                             gpointer user_data)
{
	ggp_oauth_data *data = user_data;
	PurpleAccount *account;
	PurpleContactInfo *info = NULL;
	SoupStatus status_code;
	char *auth;
	SoupMessage *msg = NULL;
	const char *method = "POST";
	const char *url = "http://api.gadu-gadu.pl/access_token";

	PURPLE_ASSERT_CONNECTION_IS_VALID(data->gc);

	account = purple_connection_get_account(data->gc);
	info = PURPLE_CONTACT_INFO(account);

	status_code = soup_message_get_status(data->msg);
	if (status_code != 302) {
		purple_debug_error("gg",
		                   "ggp_oauth_authorization_done: failed (code = %d)",
		                   status_code);
		ggp_oauth_data_free(data);
		return;
	}

	purple_debug_misc("gg", "ggp_oauth_authorization_done: authorization done, "
	                        "requesting access token...");

	auth = gg_oauth_generate_header(method, url,
	                                purple_contact_info_get_username(info),
	                                purple_connection_get_password(data->gc),
	                                data->token, data->token_secret);

	g_clear_object(&data->msg);
	data->msg = msg = soup_message_new(method, url);
	// purple_http_request_set_max_len(req, GGP_OAUTH_RESPONSE_MAX);
	soup_message_headers_replace(soup_message_get_request_headers(msg),
	                             "Authorization", auth);
	soup_session_send_and_read_async(SOUP_SESSION(source), msg,
	                                 G_PRIORITY_DEFAULT, NULL,
	                                 ggp_oauth_access_token_got, data);

	g_free(auth);
}

static void
ggp_oauth_request_token_got(GObject *source, GAsyncResult *result,
                            gpointer user_data)
{
	SoupSession *session = SOUP_SESSION(source);
	ggp_oauth_data *data = user_data;
	GBytes *response_body = NULL;
	const char *buffer = NULL;
	gsize size = 0;
	PurpleAccount *account;
	PurpleXmlNode *xml;
	SoupMessage *msg = NULL;
	gchar *request_data;
	GBytes *body = NULL;
	gboolean succ = TRUE;
	GError *error = NULL;

	PURPLE_ASSERT_CONNECTION_IS_VALID(data->gc);

	account = purple_connection_get_account(data->gc);

	if(!SOUP_STATUS_IS_SUCCESSFUL(soup_message_get_status(data->msg))) {
		purple_debug_error("gg", "ggp_oauth_request_token_got: "
			"requested token not received\n");
		ggp_oauth_data_free(data);
		return;
	}

	response_body = soup_session_send_and_read_finish(session, result, &error);
	if(response_body == NULL) {
		purple_debug_error("gg", "ggp_oauth_access_token_got: failed: %s",
		                   error->message);
		ggp_oauth_data_free(data);
		g_error_free(error);
		return;
	}

	purple_debug_misc("gg", "ggp_oauth_request_token_got: "
		"got request token, doing authorization...\n");

	buffer = g_bytes_get_data(response_body, &size);
	xml = purple_xmlnode_from_str(buffer, size);
	g_bytes_unref(response_body);

	if (xml == NULL) {
		purple_debug_error("gg", "ggp_oauth_request_token_got: "
			"invalid xml\n");
		ggp_oauth_data_free(data);
		return;
	}

	succ &= ggp_xml_get_string(xml, "oauth_token", &data->token);
	succ &= ggp_xml_get_string(xml, "oauth_token_secret",
		&data->token_secret);
	purple_xmlnode_free(xml);
	if (!succ) {
		purple_debug_error("gg", "ggp_oauth_request_token_got: "
			"invalid xml - token is not present\n");
		ggp_oauth_data_free(data);
		return;
	}

	request_data = g_strdup_printf(
		"callback_url=http://www.mojageneracja.pl&request_token=%s&"
		"uin=%s&password=%s", data->token,
		purple_contact_info_get_username(PURPLE_CONTACT_INFO(account)),
		purple_connection_get_password(data->gc));

	g_clear_object(&data->msg);
	data->msg = msg = soup_message_new("POST",
	                                   "https://login.gadu-gadu.pl/authorize");
	// purple_http_request_set_max_len(msg, GGP_OAUTH_RESPONSE_MAX);
	/* we don't need any results, nor 302 redirection */
	soup_message_set_flags(msg, SOUP_MESSAGE_NO_REDIRECT);
	body = g_bytes_new_take(request_data, strlen(request_data));
	soup_message_set_request_body_from_bytes(msg,
	                                         "application/x-www-form-urlencoded",
	                                         body);
	g_bytes_unref(body);
	soup_session_send_and_read_async(session, msg, G_PRIORITY_DEFAULT, NULL,
	                                 ggp_oauth_authorization_done, data);
}

void
ggp_oauth_request(PurpleConnection *gc, ggp_oauth_request_cb callback,
                  gpointer user_data, const gchar *sign_method,
                  const gchar *sign_url)
{
	GGPInfo *info = purple_connection_get_protocol_data(gc);
	PurpleAccount *account = purple_connection_get_account(gc);
	SoupMessage *msg;
	char *auth;
	const char *method = "POST";
	const char *url = "http://api.gadu-gadu.pl/request_token";
	ggp_oauth_data *data;

	purple_debug_misc("gg", "ggp_oauth_request: requesting token...\n");

	auth = gg_oauth_generate_header(
	        method, url,
	        purple_contact_info_get_username(PURPLE_CONTACT_INFO(account)),
	        purple_connection_get_password(gc), NULL, NULL);

	data = g_new0(ggp_oauth_data, 1);
	data->gc = gc;
	data->callback = callback;
	data->user_data = user_data;
	data->sign_method = g_strdup(sign_method);
	data->sign_url = g_strdup(sign_url);

	data->msg = msg = soup_message_new(method, url);
	// purple_http_request_set_max_len(req, GGP_OAUTH_RESPONSE_MAX);
	soup_message_headers_replace(soup_message_get_request_headers(msg),
	                             "Authorization", auth);
	soup_session_send_and_read_async(info->http, msg, G_PRIORITY_DEFAULT, NULL,
	                                 ggp_oauth_request_token_got, data);

	g_free(auth);
}
