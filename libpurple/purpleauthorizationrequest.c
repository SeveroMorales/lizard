/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#include "purpleauthorizationrequest.h"

struct _PurpleAuthorizationRequest {
	GObject parent;

	PurpleAccount *account;

	gchar *username;
	gchar *alias;
	gchar *message;
	gboolean add;

	/* This tracks whether _accept or _deny have been called. */
	gboolean handled;
};

enum {
	SIG_ACCEPTED,
	SIG_DENIED,
	N_SIGNALS
};
static guint signals[N_SIGNALS] = {0, };

enum {
	PROP_0,
	PROP_ACCOUNT,
	PROP_USERNAME,
	PROP_ALIAS,
	PROP_MESSAGE,
	PROP_ADD,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

G_DEFINE_TYPE(PurpleAuthorizationRequest, purple_authorization_request,
              G_TYPE_OBJECT)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_authorization_request_set_account(PurpleAuthorizationRequest *request,
                                         PurpleAccount *account)
{
	g_return_if_fail(PURPLE_IS_AUTHORIZATION_REQUEST(request));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	if(g_set_object(&request->account, account)) {
		g_object_notify_by_pspec(G_OBJECT(request), properties[PROP_ACCOUNT]);
	}
}

static void
purple_authorization_request_set_username(PurpleAuthorizationRequest *request,
                                          const gchar *username)
{
	g_return_if_fail(PURPLE_IS_AUTHORIZATION_REQUEST(request));

	g_free(request->username);
	request->username = g_strdup(username);

	g_object_notify_by_pspec(G_OBJECT(request), properties[PROP_USERNAME]);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_authorization_request_get_property(GObject *obj, guint param_id,
                                          GValue *value, GParamSpec *pspec)
{
	PurpleAuthorizationRequest *request = PURPLE_AUTHORIZATION_REQUEST(obj);

	switch(param_id) {
		case PROP_ACCOUNT:
			g_value_set_object(value,
			                   purple_authorization_request_get_account(request));
			break;
		case PROP_USERNAME:
			g_value_set_string(value,
			                   purple_authorization_request_get_username(request));
			break;
		case PROP_ALIAS:
			g_value_set_string(value,
			                   purple_authorization_request_get_alias(request));
			break;
		case PROP_MESSAGE:
			g_value_set_string(value,
			                   purple_authorization_request_get_message(request));
			break;
		case PROP_ADD:
			g_value_set_boolean(value,
			                    purple_authorization_request_get_add(request));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_authorization_request_set_property(GObject *obj, guint param_id,
                                          const GValue *value,
                                          GParamSpec *pspec)
{
	PurpleAuthorizationRequest *request = PURPLE_AUTHORIZATION_REQUEST(obj);

	switch(param_id) {
		case PROP_ACCOUNT:
			purple_authorization_request_set_account(request,
			                                         g_value_get_object(value));
			break;
		case PROP_USERNAME:
			purple_authorization_request_set_username(request,
			                                          g_value_get_string(value));
			break;
		case PROP_ALIAS:
			purple_authorization_request_set_alias(request,
			                                       g_value_get_string(value));
			break;
		case PROP_MESSAGE:
			purple_authorization_request_set_message(request,
			                                         g_value_get_string(value));
			break;
		case PROP_ADD:
			purple_authorization_request_set_add(request,
			                                     g_value_get_boolean(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_authorization_request_dispose(GObject *obj) {
	PurpleAuthorizationRequest *request = PURPLE_AUTHORIZATION_REQUEST(obj);

	g_clear_object(&request->account);

	G_OBJECT_CLASS(purple_authorization_request_parent_class)->dispose(obj);
}

static void
purple_authorization_request_finalize(GObject *obj) {
	PurpleAuthorizationRequest *request = PURPLE_AUTHORIZATION_REQUEST(obj);

	g_clear_pointer(&request->username, g_free);
	g_clear_pointer(&request->alias, g_free);
	g_clear_pointer(&request->message, g_free);

	G_OBJECT_CLASS(purple_authorization_request_parent_class)->finalize(obj);
}

static void
purple_authorization_request_init(G_GNUC_UNUSED PurpleAuthorizationRequest *request)
{
}

static void
purple_authorization_request_class_init(PurpleAuthorizationRequestClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = purple_authorization_request_get_property;
	obj_class->set_property = purple_authorization_request_set_property;
	obj_class->dispose = purple_authorization_request_dispose;
	obj_class->finalize = purple_authorization_request_finalize;

	/**
	 * PurpleAuthorizationRequest::accepted:
	 * @request: The [class@AuthorizationRequest] instance.
	 *
	 * Emitted when the user has accepted @request. This is typically emitted
	 * by the user interface calling [method@AuthorizationRequest.accept].
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_ACCEPTED] = g_signal_new_class_handler(
		"accepted",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		0);

	/**
	 * PurpleAuthorizationRequest::denied:
	 * @request: The [class@AuthorizationRequest] instance.
	 * @message: (nullable): An optional denial message.
	 *
	 * Emitted when the user has denied @request. This is typically emitted
	 * by the user interface calling [method@AuthorizationRequest.deny].
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_DENIED] = g_signal_new_class_handler(
		"denied",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		G_TYPE_STRING);

	/**
	 * PurpleAuthorizationRequest:account:
	 *
	 * The account that this authorization request is for.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ACCOUNT] = g_param_spec_object(
		"account", "account",
		"The account for this authorization request",
		PURPLE_TYPE_ACCOUNT,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleAuthorizationRequest:username:
	 *
	 * The username of the remote user that is requesting authorization.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_USERNAME] = g_param_spec_string(
		"username", "username",
		"The username of the remote user that is requesting authorization",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleAuthorizationRequest:alias:
	 *
	 * The alias of the remote user that is requesting authorization.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ALIAS] = g_param_spec_string(
		"alias", "alias",
		"The alias of the remote user that is requesting authorization",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleAuthorizationRequest:message:
	 *
	 * The optional message sent from the remote user.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_MESSAGE] = g_param_spec_string(
		"message", "message",
		"The optional message sent by the remote user",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleAuthorizationRequest:add:
	 *
	 * Whether or not the user interface should ask the end user to add the
	 * remote user after accepting the end user's friend request.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ADD] = g_param_spec_boolean(
		"add", "add",
		"Whether or not to add the remote user back",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleAuthorizationRequest *
purple_authorization_request_new(PurpleAccount *account, const gchar *username)
{
	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);
	g_return_val_if_fail(username != NULL, NULL);

	return g_object_new(
		PURPLE_TYPE_AUTHORIZATION_REQUEST,
		"account", account,
		"username", username,
		NULL);
}

PurpleAccount *
purple_authorization_request_get_account(PurpleAuthorizationRequest *request) {
	g_return_val_if_fail(PURPLE_IS_AUTHORIZATION_REQUEST(request), NULL);

	return request->account;
}

const gchar *
purple_authorization_request_get_username(PurpleAuthorizationRequest *request)
{
	g_return_val_if_fail(PURPLE_IS_AUTHORIZATION_REQUEST(request), NULL);

	return request->username;
}

void
purple_authorization_request_set_alias(PurpleAuthorizationRequest *request,
                                       const gchar *alias)
{
	g_return_if_fail(PURPLE_IS_AUTHORIZATION_REQUEST(request));

	g_free(request->alias);
	request->alias = g_strdup(alias);

	g_object_notify_by_pspec(G_OBJECT(request), properties[PROP_ALIAS]);
}

const gchar *
purple_authorization_request_get_alias(PurpleAuthorizationRequest *request) {
	g_return_val_if_fail(PURPLE_IS_AUTHORIZATION_REQUEST(request), NULL);

	return request->alias;
}

void
purple_authorization_request_set_message(PurpleAuthorizationRequest *request,
                                         const gchar *message)
{
	g_return_if_fail(PURPLE_IS_AUTHORIZATION_REQUEST(request));

	g_free(request->message);
	request->message = g_strdup(message);

	g_object_notify_by_pspec(G_OBJECT(request), properties[PROP_MESSAGE]);
}

const gchar *
purple_authorization_request_get_message(PurpleAuthorizationRequest *request) {
	g_return_val_if_fail(PURPLE_IS_AUTHORIZATION_REQUEST(request), NULL);

	return request->message;
}

void
purple_authorization_request_set_add(PurpleAuthorizationRequest *request,
                                     gboolean add)
{
	g_return_if_fail(PURPLE_IS_AUTHORIZATION_REQUEST(request));

	request->add = add;

	g_object_notify_by_pspec(G_OBJECT(request), properties[PROP_ADD]);
}

gboolean
purple_authorization_request_get_add(PurpleAuthorizationRequest *request) {
	g_return_val_if_fail(PURPLE_IS_AUTHORIZATION_REQUEST(request), FALSE);

	return request->add;
}

void
purple_authorization_request_accept(PurpleAuthorizationRequest *request) {
	g_return_if_fail(PURPLE_IS_AUTHORIZATION_REQUEST(request));

	/* Calling this multiple times is a programming error. */
	g_return_if_fail(request->handled == FALSE);

	request->handled = TRUE;

	g_signal_emit(request, signals[SIG_ACCEPTED], 0);
}

void
purple_authorization_request_deny(PurpleAuthorizationRequest *request,
                                  const gchar *message)
{
	g_return_if_fail(PURPLE_IS_AUTHORIZATION_REQUEST(request));

	/* Calling this multiple times is a programming error. */
	g_return_if_fail(request->handled == FALSE);

	request->handled = TRUE;

	g_signal_emit(request, signals[SIG_DENIED], 0, message);
}
