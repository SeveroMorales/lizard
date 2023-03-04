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

#ifndef PURPLE_ADD_CONTACT_REQUEST_H
#define PURPLE_ADD_CONTACT_REQUEST_H

#include <glib.h>
#include <glib-object.h>

#include "account.h"

G_BEGIN_DECLS

/**
 * PurpleAddContactRequest:
 *
 * #PurpleAddContactRequest is a data structure that contains all of the
 * information when someone has added you to their contact list, so the user
 * interface can ask the user to add the remote user to their contact list.
 *
 * Since: 3.0.0
 */

#define PURPLE_TYPE_ADD_CONTACT_REQUEST (purple_add_contact_request_get_type())
G_DECLARE_FINAL_TYPE(PurpleAddContactRequest, purple_add_contact_request,
                     PURPLE, ADD_CONTACT_REQUEST, GObject)

/**
 * purple_add_contact_request_new:
 * @account: The account that this request is for.
 * @username: The username of the user requesting authorization.
 *
 * Creates a new [class@Purple.AddContactRequest] for @username on @account.
 *
 * This is typically only used by libpurple itself.
 *
 * Returns: The new instance.
 *
 * Since: 3.0.0
 */
PurpleAddContactRequest *purple_add_contact_request_new(PurpleAccount *account, const gchar *username);

/**
 * purple_add_contact_request_get_account:
 * @request: The instance.
 *
 * Gets the [class@Account] for @request.
 *
 * Returns: (transfer none): The account.
 *
 * Since: 3.0.0
 */
PurpleAccount *purple_add_contact_request_get_account(PurpleAddContactRequest *request);

/**
 * purple_add_contact_request_get_username:
 * @request: The instance.
 *
 * Gets the username for the user to be added.
 *
 * Returns: The username of the remote user.
 *
 * Since: 3.0.0
 */
const gchar *purple_add_contact_request_get_username(PurpleAddContactRequest *request);

/**
 * purple_add_contact_request_set_alias:
 * @request: The instance.
 * @alias: (nullable): The alias of the remote user.
 *
 * Sets the alias of the remote user to @alias. User interfaces can use this
 * when presenting the authorization request to the end user.
 *
 * Since: 3.0.0
 */
void purple_add_contact_request_set_alias(PurpleAddContactRequest *request, const gchar *alias);

/**
 * purple_add_contact_request_get_alias:
 * @request: The instance.
 *
 * Gets the alias of the remote user if one was set.
 *
 * Returns: (nullable): The alias if one was set.
 *
 * Since: 3.0.0
 */
const gchar *purple_add_contact_request_get_alias(PurpleAddContactRequest *request);

/**
 * purple_add_contact_request_set_message:
 * @request: The instance.
 * @message: (nullable): An optional message from the remote user.
 *
 * Sets an optional message from remote user, that the user interface can
 * display to the end user.
 *
 * Since: 3.0.0
 */
void purple_add_contact_request_set_message(PurpleAddContactRequest *request, const gchar *message);

/**
 * purple_add_contact_request_get_message:
 * @request: The instance.
 *
 * Gets the message that was optionally sent by the remote user.
 *
 * Returns: (nullable): The optional message.
 *
 * Since: 3.0.0
 */
const gchar *purple_add_contact_request_get_message(PurpleAddContactRequest *request);

/**
 * purple_add_contact_request_add:
 * @request: The instance.
 *
 * Emits the [signal@AddContactRequest::add] signal. This is typically called
 * by the user interface when the user has clicked the add button.
 *
 * If this is called multiple times, then this does nothing.
 *
 * Since: 3.0.0
 */
void purple_add_contact_request_add(PurpleAddContactRequest *request);

G_END_DECLS

#endif /* PURPLE_ADD_CONTACT_REQUEST_H */
