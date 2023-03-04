/*
 * Copyright (C) 2023 Hasl Developers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include "haslmechanismplain.h"

#include "haslcore.h"

struct _HaslMechanismPlain {
	HaslMechanism parent;
};

G_DEFINE_TYPE(HaslMechanismPlain, hasl_mechanism_plain, HASL_TYPE_MECHANISM)

/******************************************************************************
 * HaslMechanism Implementation
 *****************************************************************************/
static gboolean
hasl_mechanism_plain_possible(G_GNUC_UNUSED HaslMechanism *mechanism,
                              HaslContext *context,
                              GError **error)
{
	const char *value = NULL;

	value = hasl_context_get_username(context);
	if(value == NULL || value[0] == '\0') {
		g_set_error(error, HASL_MECHANISM_PLAIN_DOMAIN, 0,
		            "missing username");

		return FALSE;
	}

	value = hasl_context_get_password(context);
	if(value == NULL || value[0] == '\0') {
		g_set_error(error, HASL_MECHANISM_PLAIN_DOMAIN, 0,
		            "missing password");

		return FALSE;
	}

	if(!hasl_context_get_allow_clear_text(context)) {
		if(!hasl_context_get_tls(context)) {
			g_set_error(error, HASL_MECHANISM_PLAIN_DOMAIN, 0,
			            "plain text is not allowed without TLS");

			return FALSE;
		}
	}

	return TRUE;
}

static HaslMechanismResult
hasl_mechanism_plain_step(G_GNUC_UNUSED HaslMechanism *mechanism,
                          HaslContext *ctx,
                          G_GNUC_UNUSED const guint8 *server_in,
                          G_GNUC_UNUSED gsize server_in_length,
                          guint8 **client_out,
                          gsize *client_out_length,
                          GError **error)
{
	GByteArray *data = NULL;
	const char *authzid = NULL;
	const char *username = NULL;
	const char *password = NULL;

	authzid = hasl_context_get_authzid(ctx);

	username = hasl_context_get_username(ctx);
	if(username == NULL || username[0] == '\0') {
		g_set_error(error, HASL_DOMAIN, 0, "no username provided");

		return HASL_MECHANISM_RESULT_ERROR;
	}

	password = hasl_context_get_password(ctx);
	if(password == NULL || password[0] == '\0') {
		g_set_error(error, HASL_DOMAIN, 0, "no password provided");

		return HASL_MECHANISM_RESULT_ERROR;
	}

	/* We have at least a username and a password, so build our payload. */
	data = g_byte_array_new();

	/* authzid can be null, so we need to check it before appending it. */
	if(authzid != NULL && authzid[0] != '\0') {
		g_byte_array_append(data, (guint8 *)authzid, strlen(authzid));
	}

	/* Add the rest of the values that we already validated and their null
	 * delimiters.
	 */
	g_byte_array_append(data, (guint8 *)"\0", 1);
	g_byte_array_append(data, (guint8 *)username, strlen(username));
	g_byte_array_append(data, (guint8 *)"\0", 1);
	g_byte_array_append(data, (guint8 *)password, strlen(password));

	/* Set our out variables and return. */
	if(client_out_length != NULL) {
		*client_out_length = data->len;
	}

	*client_out = g_byte_array_free(data, FALSE);

	return HASL_MECHANISM_RESULT_SUCCESS;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
hasl_mechanism_plain_init(G_GNUC_UNUSED HaslMechanismPlain *plain) {
}

static void
hasl_mechanism_plain_class_init(HaslMechanismPlainClass *klass) {
	HaslMechanismClass *mechanism_class = HASL_MECHANISM_CLASS(klass);

	mechanism_class->possible = hasl_mechanism_plain_possible;
	mechanism_class->step = hasl_mechanism_plain_step;
}
