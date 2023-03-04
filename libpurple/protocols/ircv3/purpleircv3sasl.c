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

#include <hasl.h>

#include "purpleircv3sasl.h"

#include "purpleircv3capabilities.h"
#include "purpleircv3connection.h"
#include "purpleircv3core.h"

#define PURPLE_IRCV3_SASL_DATA_KEY ("sasl-data")

typedef struct {
	PurpleConnection *connection;

	HaslContext *ctx;

	GString *server_in_buffer;
} PurpleIRCv3SASLData;

/******************************************************************************
 * Helpers
 *****************************************************************************/
static const char *
purple_ircv3_sasl_get_username(PurpleConnection *connection) {
	PurpleAccount *account = NULL;
	const char *username = NULL;

	account = purple_connection_get_account(connection);

	username = purple_account_get_string(account, "sasl-login-name", "");
	if(username != NULL && username[0] != '\0') {
		return username;
	}

	return purple_connection_get_display_name(connection);
}

/******************************************************************************
 * SASL Helpers
 *****************************************************************************/
static void
purple_ircv3_sasl_data_free(PurpleIRCv3SASLData *data) {
	g_clear_object(&data->ctx);

	g_string_free(data->server_in_buffer, TRUE);

	g_free(data);
}

static void
purple_ircv3_sasl_data_add(PurpleConnection *connection, HaslContext *ctx) {
	PurpleIRCv3SASLData *data = NULL;

	data = g_new0(PurpleIRCv3SASLData, 1);
	g_object_set_data_full(G_OBJECT(connection), PURPLE_IRCV3_SASL_DATA_KEY,
	                       data, (GDestroyNotify)purple_ircv3_sasl_data_free);

	/* We don't reference this because the life cycle of this data is tied
	 * directly to the connection and adding a reference to the connection
	 * would keep both alive forever.
	 */
	data->connection = connection;
	data->ctx = ctx;

	/* We truncate the server_in_buffer when we need to so that we can minimize
	 * allocations and simplify the logic involved with it.
	 */
	data->server_in_buffer = g_string_new("");
}

static void
purple_ircv3_sasl_attempt(PurpleIRCv3Connection *connection) {
	PurpleIRCv3SASLData *data = NULL;
	const char *next_mechanism = NULL;
	const char *current = NULL;

	data = g_object_get_data(G_OBJECT(connection), PURPLE_IRCV3_SASL_DATA_KEY);

	current = hasl_context_get_current_mechanism(data->ctx);
	if(current != NULL) {
		g_message("SASL '%s' mechanism failed", current);
	}

	next_mechanism = hasl_context_next(data->ctx);
	if(next_mechanism == NULL) {
		GError *error = g_error_new(PURPLE_CONNECTION_ERROR,
		                            PURPLE_CONNECTION_ERROR_AUTHENTICATION_IMPOSSIBLE,
		                            _("No valid SASL mechanisms found"));

		purple_connection_take_error(PURPLE_CONNECTION(connection), error);

		return;
	}

	g_message("trying SASL '%s' mechanism", next_mechanism);

	purple_ircv3_connection_writef(connection, "AUTHENTICATE %s",
	                               next_mechanism);
}

static void
purple_ircv3_sasl_start(PurpleIRCv3Capabilities *caps) {
	PurpleIRCv3Connection *connection = NULL;
	PurpleAccount *account = NULL;
	PurpleConnection *purple_connection = NULL;
	HaslContext *ctx = NULL;
	const char *mechanisms = NULL;
	gboolean toggle = FALSE;

	connection = purple_ircv3_capabilities_get_connection(caps);
	purple_connection = PURPLE_CONNECTION(connection);
	account = purple_connection_get_account(purple_connection);

	ctx = hasl_context_new();

	/* At this point we are ready to start our SASL negotiation, so add a wait
	 * counter to the capabilities and start the negotiations!
	 */
	purple_ircv3_capabilities_add_wait(caps);

	/* Determine what mechanisms we're allowing and tell the context. */
	mechanisms = purple_account_get_string(account, "sasl-mechanisms", "");
	if(purple_strempty(mechanisms)) {
		/* If the user didn't specify any mechanisms, grab the mechanisms that
		 * the server advertised.
		 */
		mechanisms = purple_ircv3_capabilities_lookup(caps, "sasl", NULL);
	}
	hasl_context_set_allowed_mechanisms(ctx, mechanisms);

	/* Add the values we know to the context. */
	hasl_context_set_username(ctx, purple_ircv3_sasl_get_username(purple_connection));
	hasl_context_set_password(ctx, purple_connection_get_password(purple_connection));

	toggle = purple_account_get_bool(account, "use-tls", TRUE);
	hasl_context_set_tls(ctx, toggle);

	toggle = purple_account_get_bool(account, "plain-sasl-in-clear", FALSE);
	hasl_context_set_allow_clear_text(ctx, toggle);

	/* Create our SASLData object, add it to the connection. */
	purple_ircv3_sasl_data_add(purple_connection, ctx);

	/* Make it go! */
	purple_ircv3_sasl_attempt(connection);
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
purple_ircv3_sasl_ack_cb(PurpleIRCv3Capabilities *caps,
                         G_GNUC_UNUSED const char *capability,
                         G_GNUC_UNUSED gpointer data)
{
	purple_ircv3_sasl_start(caps);
}

/******************************************************************************
 * Internal API
 *****************************************************************************/
void
purple_ircv3_sasl_request(PurpleIRCv3Capabilities *capabilities) {
	purple_ircv3_capabilities_request(capabilities, "sasl");

	g_signal_connect(capabilities, "ack::sasl",
	                 G_CALLBACK(purple_ircv3_sasl_ack_cb), NULL);
}

gboolean
purple_ircv3_sasl_logged_in(G_GNUC_UNUSED GHashTable *tags,
                            G_GNUC_UNUSED const char *source,
                            G_GNUC_UNUSED const char *command,
                            G_GNUC_UNUSED guint n_params,
                            G_GNUC_UNUSED GStrv params,
                            G_GNUC_UNUSED GError **error,
                            gpointer user_data)
{
	PurpleIRCv3Connection *connection = user_data;
	PurpleIRCv3SASLData *data = NULL;

	data = g_object_get_data(G_OBJECT(connection), PURPLE_IRCV3_SASL_DATA_KEY);
	if(data == NULL) {
		g_set_error_literal(error, PURPLE_IRCV3_DOMAIN, 0,
		                    "RPL_LOGGEDIN received with no SASL data "
		                    "present");

		return FALSE;
	}

	/* At this point, we have the users authenticated username, we _may_ want
	 * to update the account's ID to this, but we'll need more testing to
	 * verify that.
	 * -- GK 2023-01-12
	 */

	return TRUE;
}

gboolean
purple_ircv3_sasl_logged_out(G_GNUC_UNUSED GHashTable *tags,
                             G_GNUC_UNUSED const char *source,
                             G_GNUC_UNUSED const char *command,
                             G_GNUC_UNUSED guint n_params,
                             G_GNUC_UNUSED GStrv params,
                             G_GNUC_UNUSED GError **error,
                             gpointer user_data)
{
	PurpleIRCv3Connection *connection = user_data;
	PurpleIRCv3SASLData *data = NULL;

	data = g_object_get_data(G_OBJECT(connection), PURPLE_IRCV3_SASL_DATA_KEY);
	if(data == NULL) {
		g_set_error_literal(error, PURPLE_IRCV3_DOMAIN, 0,
		                    "RPL_LOGGEDOUT received with no SASL data "
		                    "present");

		return FALSE;
	}

	/* Not sure how to trigger this or what we should do in this case to be
	 * honest, so just note it for now.
	 * -- GK 2023-01-12
	 */
	g_warning("Server sent SASL logged out");

	return TRUE;
}

gboolean
purple_ircv3_sasl_nick_locked(G_GNUC_UNUSED GHashTable *tags,
                              G_GNUC_UNUSED const char *source,
                              G_GNUC_UNUSED const char *command,
                              G_GNUC_UNUSED guint n_params,
                              GStrv params,
                              GError **error,
                              gpointer user_data)
{
	PurpleIRCv3Connection *connection = user_data;
	PurpleIRCv3SASLData *data = NULL;
	char *message = NULL;

	data = g_object_get_data(G_OBJECT(connection), PURPLE_IRCV3_SASL_DATA_KEY);
	if(data == NULL) {
		g_set_error_literal(error, PURPLE_IRCV3_DOMAIN, 0,
		                    "ERR_NICKLOCKED received with no SASL data "
		                    "present");

		return FALSE;
	}

	message = g_strjoinv(" ", params);

	g_set_error(error, PURPLE_CONNECTION_ERROR,
	            PURPLE_CONNECTION_ERROR_AUTHENTICATION_IMPOSSIBLE,
	            _("Nick name is locked: %s"), message);

	g_free(message);

	return FALSE;
}


gboolean
purple_ircv3_sasl_success(G_GNUC_UNUSED GHashTable *tags,
                          G_GNUC_UNUSED const char *source,
                          G_GNUC_UNUSED const char *command,
                          G_GNUC_UNUSED guint n_params,
                          G_GNUC_UNUSED GStrv params,
                          GError **error,
                          gpointer user_data)
{
	PurpleIRCv3Capabilities *capabilities = NULL;
	PurpleIRCv3Connection *connection = user_data;
	PurpleIRCv3SASLData *data = NULL;

	capabilities = purple_ircv3_connection_get_capabilities(connection);

	data = g_object_get_data(G_OBJECT(connection), PURPLE_IRCV3_SASL_DATA_KEY);
	if(data == NULL) {
		g_set_error_literal(error, PURPLE_IRCV3_DOMAIN, 0,
		                    "RPL_SASLSUCCESS received with no SASL data "
		                    "present");

		return FALSE;
	}

	/* This needs to be after we've checked the SASL data otherwise we might
	 * end up removing a wait that we don't own.
	 */
	purple_ircv3_capabilities_remove_wait(capabilities);

	g_message("successfully authenticated with SASL '%s' mechanism.",
	          hasl_context_get_current_mechanism(data->ctx));

	return TRUE;
}

gboolean
purple_ircv3_sasl_failed(G_GNUC_UNUSED GHashTable *tags,
                         G_GNUC_UNUSED const char *source,
                         G_GNUC_UNUSED const char *command,
                         G_GNUC_UNUSED guint n_params,
                         G_GNUC_UNUSED GStrv params,
                         G_GNUC_UNUSED GError **error,
                         gpointer user_data)
{
	PurpleIRCv3Connection *connection = user_data;
	PurpleIRCv3SASLData *data = NULL;

	data = g_object_get_data(G_OBJECT(connection), PURPLE_IRCV3_SASL_DATA_KEY);
	if(data == NULL) {
		g_set_error_literal(error, PURPLE_IRCV3_DOMAIN, 0,
		                    "ERR_SASLFAIL received with no SASL data present");

		return FALSE;
	}

	purple_ircv3_sasl_attempt(connection);

	return TRUE;
}

gboolean
purple_ircv3_sasl_message_too_long(G_GNUC_UNUSED GHashTable *tags,
                                   G_GNUC_UNUSED const char *source,
                                   G_GNUC_UNUSED const char *command,
                                   G_GNUC_UNUSED guint n_params,
                                   G_GNUC_UNUSED GStrv params,
                                   G_GNUC_UNUSED GError **error,
                                   gpointer user_data)
{
	PurpleIRCv3Connection *connection = user_data;
	PurpleIRCv3SASLData *data = NULL;

	data = g_object_get_data(G_OBJECT(connection), PURPLE_IRCV3_SASL_DATA_KEY);
	if(data == NULL) {
		g_set_error_literal(error, PURPLE_IRCV3_DOMAIN, 0,
		                    "ERR_SASLTOOLONG received with no SASL data "
		                    "present");

		return FALSE;
	}

	return TRUE;
}

gboolean
purple_ircv3_sasl_aborted(G_GNUC_UNUSED GHashTable *tags,
                          G_GNUC_UNUSED const char *source,
                          G_GNUC_UNUSED const char *command,
                          G_GNUC_UNUSED guint n_params,
                          G_GNUC_UNUSED GStrv params,
                          G_GNUC_UNUSED GError **error,
                          gpointer user_data)
{
	PurpleIRCv3Connection *connection = user_data;
	PurpleIRCv3SASLData *data = NULL;

	data = g_object_get_data(G_OBJECT(connection), PURPLE_IRCV3_SASL_DATA_KEY);
	if(data == NULL) {
		g_set_error_literal(error, PURPLE_IRCV3_DOMAIN, 0,
		                    "ERR_SASLABORTED received with no SASL data "
		                    "present");

		return FALSE;
	}

	/* This is supposed to get sent, when the client sends `AUTHENTICATE *`,
	 * but we don't do this, so I guess we'll note it for now...?
	 * --GK 2023-01-12
	 */
	g_warning("The server claims we aborted SASL authentication.");

	return TRUE;
}

gboolean
purple_ircv3_sasl_already_authed(G_GNUC_UNUSED GHashTable *tags,
                                 G_GNUC_UNUSED const char *source,
                                 G_GNUC_UNUSED const char *command,
                                 G_GNUC_UNUSED guint n_params,
                                 G_GNUC_UNUSED GStrv params,
                                 G_GNUC_UNUSED GError **error,
                                 gpointer user_data)
{
	PurpleIRCv3Connection *connection = user_data;
	PurpleIRCv3SASLData *data = NULL;

	data = g_object_get_data(G_OBJECT(connection), PURPLE_IRCV3_SASL_DATA_KEY);
	if(data == NULL) {
		g_set_error_literal(error, PURPLE_IRCV3_DOMAIN, 0,
		                    "ERR_SASLALREADY received with no SASL data "
		                    "present");

		return FALSE;
	}

	/* Similar to aborted above, we don't allow this, so just note that it
	 * happened.
	 * -- GK 2023-01-12
	 */
	g_warning("Server claims we tried to SASL authenticate again.");

	return TRUE;
}

gboolean
purple_ircv3_sasl_mechanisms(G_GNUC_UNUSED GHashTable *tags,
                             G_GNUC_UNUSED const char *source,
                             G_GNUC_UNUSED const char *command,
                             guint n_params,
                             GStrv params,
                             G_GNUC_UNUSED GError **error,
                             gpointer user_data)
{
	PurpleIRCv3Connection *connection = user_data;
	PurpleIRCv3SASLData *data = NULL;

	data = g_object_get_data(G_OBJECT(connection), PURPLE_IRCV3_SASL_DATA_KEY);
	if(data == NULL) {
		g_set_error_literal(error, PURPLE_IRCV3_DOMAIN, 0,
		                    "RPL_SASLMECHS received with no SASL data "
		                    "present");

		return FALSE;
	}

	/* We need to find a server that sends this message. The specification says
	 * it _may_ be sent when the client sends AUTHENTICATE with an unknown
	 * mechanism, but ergo doesn't.
	 *
	 * We can't just blindly accept the new list either, as depending on how
	 * the server implements it, we'll need to remove mechanisms we've already
	 * tried in the event the server just dumps the entire list. As we're not
	 * currently tracking which mechanisms we've tried, this will have to be
	 * addressed as well.
	 */
	if(n_params > 0) {
		char *message = g_strjoinv(" ", params);

		g_message("Server sent the following SASL mechanisms: %s", message);

		g_free(message);
	} else {
		g_message("Server sent an empty list of SASL mechanisms");
	}

	return TRUE;
}

gboolean
purple_ircv3_sasl_authenticate(G_GNUC_UNUSED GHashTable *tags,
                               G_GNUC_UNUSED const char *source,
                               G_GNUC_UNUSED const char *command,
                               guint n_params,
                               GStrv params,
                               GError **error,
                               gpointer user_data)
{
	PurpleIRCv3Connection *connection = user_data;
	PurpleIRCv3SASLData *data = NULL;
	char *payload = NULL;
	gboolean done = FALSE;

	if(n_params != 1) {
		g_set_error(error, PURPLE_IRCV3_DOMAIN, 0,
		            "ignoring AUTHENTICATE with %d parameters", n_params);

		return FALSE;
	}

	payload = params[0];
	data = g_object_get_data(G_OBJECT(connection), PURPLE_IRCV3_SASL_DATA_KEY);
	if(data == NULL) {
		g_set_error_literal(error, PURPLE_IRCV3_DOMAIN, 0,
		                    "AUTHENTICATE received with no SASL data present");

		return FALSE;
	}

	/* If the server sent us a payload, combine the chunks. */
	if(payload[0] != '+') {
		g_string_append(data->server_in_buffer, payload);

		if(strlen(payload) < 400) {
			done = TRUE;
		}
	} else {
		/* The server sent a + which is an empty message or the final message
		 * ended on a 400 byte barrier. */
		done = TRUE;
	}

	if(done) {
		HaslMechanismResult res = 0;
		GError *local_error = NULL;
		guint8 *server_in = NULL;
		guint8 *client_out = NULL;
		gsize server_in_length = 0;
		size_t client_out_length = 0;

		/* If we have a buffer, base64 decode it, and then truncate it. */
		if(data->server_in_buffer->len > 0) {
			server_in = g_base64_decode(data->server_in_buffer->str,
			                            &server_in_length);
			g_string_truncate(data->server_in_buffer, 0);
		}

		/* Try to move to the next step of the sasl client. */
		res = hasl_context_step(data->ctx, server_in, server_in_length,
		                        &client_out, &client_out_length, &local_error);

		/* We should be done with server_in, so free it.*/
		g_clear_pointer(&server_in, g_free);

		if(res == HASL_MECHANISM_RESULT_ERROR) {
			g_propagate_error(error, local_error);

			return FALSE;
		}

		if(local_error != NULL) {
			g_warning("hasl_context_step returned an error without an error "
			          "status: %s", local_error->message);

			g_clear_error(&local_error);
		}

		/* If we got an output for the client, write it out. */
		if(client_out_length > 0) {
			char *encoded = NULL;

			encoded = g_base64_encode(client_out, client_out_length);
			g_clear_pointer(&client_out, g_free);

			purple_ircv3_connection_writef(connection, "AUTHENTICATE %s",
			                               encoded);
			g_free(encoded);
		} else {
			purple_ircv3_connection_writef(connection, "AUTHENTICATE +");
		}
	}

	return TRUE;
}
