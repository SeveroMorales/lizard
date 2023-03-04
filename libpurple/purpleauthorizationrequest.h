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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_AUTHORIZATION_REQUEST_H
#define PURPLE_AUTHORIZATION_REQUEST_H

#include <glib.h>
#include <glib-object.h>

#include "account.h"

G_BEGIN_DECLS

/**
 * PurpleAuthorizationRequest:
 *
 * #PurpleAuthorizationRequest is a data structure that contains all of the
 * information when someone has requested authorization to add you to their
 * contact list.
 *
 * Since: 3.0.0
 */

#define PURPLE_TYPE_AUTHORIZATION_REQUEST (purple_authorization_request_get_type())

G_DECLARE_FINAL_TYPE(PurpleAuthorizationRequest, purple_authorization_request,
                     PURPLE, AUTHORIZATION_REQUEST, GObject)

/**
 * purple_authorization_request_new:
 * @account: The account that this request is for.
 * @username: The username of the user requesting authorization.
 *
 * Creates a new [class@Purple.AuthorizationRequest] for @username on @account.
 *
 * This is typically only used by libpurple itself.
 *
 * Returns: The new instance.
 *
 * Since: 3.0.0
 */
PurpleAuthorizationRequest *purple_authorization_request_new(PurpleAccount *account, const gchar *username);

/**
 * purple_authorization_request_get_account:
 * @request: The instance.
 *
 * Gets the [class@Account] for @request.
 *
 * Returns: (transfer none): The account.
 *
 * Since: 3.0.0
 */
PurpleAccount *purple_authorization_request_get_account(PurpleAuthorizationRequest *request);

/**
 * purple_authorization_request_get_username:
 * @request: The instance.
 *
 * Gets the username for the user requesting authorization.
 *
 * Returns: The username of the remote user.
 *
 * Since: 3.0.0
 */
const gchar *purple_authorization_request_get_username(PurpleAuthorizationRequest *request);

/**
 * purple_authorization_request_set_alias:
 * @request: The instance.
 * @alias: (nullable): The alias of the remote user.
 *
 * Sets the alias of the remote user to @alias. User interfaces can use this
 * when presenting the authorization request to the end user.
 *
 * Since: 3.0.0
 */
void purple_authorization_request_set_alias(PurpleAuthorizationRequest *request, const gchar *alias);

/**
 * purple_authorization_request_get_alias:
 * @request: The instance.
 *
 * Gets the alias of the remote user if one was set.
 *
 * Returns: (nullable): The alias if one was set.
 *
 * Since: 3.0.0
 */
const gchar *purple_authorization_request_get_alias(PurpleAuthorizationRequest *request);

/**
 * purple_authorization_request_set_message:
 * @request: The instance.
 * @message: (nullable): An optional message from the remote user.
 *
 * Sets an optional message from remote user, that the user interface can
 * display to the end user.
 *
 * Since: 3.0.0
 */
void purple_authorization_request_set_message(PurpleAuthorizationRequest *request, const gchar *message);

/**
 * purple_authorization_request_get_message:
 * @request: The instance.
 *
 * Gets the message that was optionally sent by the remote user.
 *
 * Returns: (nullable): The optional message.
 *
 * Since: 3.0.0
 */
const gchar *purple_authorization_request_get_message(PurpleAuthorizationRequest *request);

/**
 * purple_authorization_request_set_add:
 * @request: The instance.
 * @add: Whether or not to ask the user to add the remote user back.
 *
 * Sets whether or not the user interface should ask the end user to add the
 * remote user if the remote user was accepted.
 *
 * Since: 3.0.0
 */
void purple_authorization_request_set_add(PurpleAuthorizationRequest *request, gboolean add);

/**
 * purple_authorization_request_get_add:
 * @request: The instance.
 *
 * Gets whether or not the user interface should ask the end user to add the
 * remote user if the end user accepted the remote user's friend request.
 *
 * Returns: %TRUE if the user interface should request the end user to add the
 *          remote user back.
 *
 * Since: 3.0.0
 */
gboolean purple_authorization_request_get_add(PurpleAuthorizationRequest *request);

/**
 * purple_authorization_request_accept:
 * @request: The instance.
 *
 * Emits the [signal@AuthorizationRequest::accepted] signal. This is typically
 * called by the user interface when the user has clicked the accept button.
 *
 * If this is called multiple times, or called after
 * [method@AuthorizationRequest.deny] then this does nothing.
 *
 * Since: 3.0.0
 */
void purple_authorization_request_accept(PurpleAuthorizationRequest *request);

/**
 * purple_authorization_request_deny:
 * @request: The instance.
 * @message: (nullable): An optional denial message.
 *
 * Emits the [signal@AuthorizationRequest::denied] signal. This is typically
 * called by the user interface when the user has clicked the deny button.
 *
 * If this is called multiple times, or called after
 * [method@AuthorizationRequest.accept] then this does nothing.
 *
 * Since: 3.0.0
 */
void purple_authorization_request_deny(PurpleAuthorizationRequest *request, const gchar *message);

G_END_DECLS

#endif /* PURPLE_AUTHORIZATION_REQUEST_H */
