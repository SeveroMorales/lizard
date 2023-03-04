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
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_ACCOUNT_H
#define PURPLE_ACCOUNT_H

#include <glib.h>
#include <glib-object.h>

#define PURPLE_TYPE_ACCOUNT             (purple_account_get_type())

typedef struct _PurpleAccount       PurpleAccount;

typedef gboolean (*PurpleFilterAccountFunc)(PurpleAccount *account);

#include "buddy.h"
#include "connection.h"
#include "group.h"
#include "purpleconnectionerrorinfo.h"
#include "purplecontactinfo.h"
#include "purpleprotocol.h"
#include "purpleproxyinfo.h"
#include "status.h"
#include "xmlnode.h"

/**
 * PurpleAccountRequestResponse:
 * @PURPLE_ACCOUNT_RESPONSE_IGNORE: Silently ignore the request.
 * @PURPLE_ACCOUNT_RESPONSE_DENY: Block the request potentially informing the
 *                                sender based on the protocol.
 * @PURPLE_ACCOUNT_RESPONSE_PASS: Prompt the user with the request.
 * @PURPLE_ACCOUNT_RESPONSE_ACCEPT: Grant the request.
 *
 * Account request response types
 */
typedef enum
{
	PURPLE_ACCOUNT_RESPONSE_IGNORE = -2,
	PURPLE_ACCOUNT_RESPONSE_DENY = -1,
	PURPLE_ACCOUNT_RESPONSE_PASS = 0,
	PURPLE_ACCOUNT_RESPONSE_ACCEPT = 1
} PurpleAccountRequestResponse;

G_BEGIN_DECLS

/**************************************************************************/
/* Account API                                                            */
/**************************************************************************/

/**
 * PurpleAccount:
 *
 * A #PurpleAccount is the core configuration for connecting to a specific
 * account. User interfaces typically allow users to create these in a dialog
 * or wizard.
 */

/**
 * purple_account_get_type:
 *
 * Returns: The #GType for the Account object.
 */
G_DECLARE_FINAL_TYPE(PurpleAccount, purple_account, PURPLE, ACCOUNT,
                     PurpleContactInfo)

/**
 * purple_account_new:
 * @username:    The username.
 * @protocol_id: The protocol ID.
 *
 * Creates a new account.
 *
 * Returns: The new account.
 */
PurpleAccount *purple_account_new(const char *username, const char *protocol_id);

/**
 * purple_account_connect:
 * @account: The account to connect to.
 *
 * Connects to an account.
 */
void purple_account_connect(PurpleAccount *account);

/**
 * purple_account_disconnect:
 * @account: The account to disconnect from.
 *
 * Disconnects from an account.
 */
void purple_account_disconnect(PurpleAccount *account);

/**
 * purple_account_is_disconnecting:
 * @account: The account
 *
 * Indicates if the account is currently being disconnected.
 *
 * Returns: TRUE if the account is being disconnected.
 *
 * Since: 3.0.0
 */
gboolean purple_account_is_disconnecting(PurpleAccount *account);

/**
 * purple_account_request_close_with_account:
 * @account:	   The account for which requests should be closed
 *
 * Close account requests registered for the given PurpleAccount
 */
void purple_account_request_close_with_account(PurpleAccount *account);

/**
 * purple_account_request_close:
 * @ui_handle:	   The ui specific handle for which requests should be closed
 *
 * Close the account request for the given ui handle
 */
void purple_account_request_close(void *ui_handle);

/**
 * purple_account_request_password:
 * @account:     The account to request the password for.
 * @ok_cb:       (scope call): The callback for the OK button.
 * @cancel_cb:   (scope call): The callback for the cancel button.
 * @user_data:   User data to be passed into callbacks.
 *
 * Requests a password from the user for the account. Does not set the
 * account password on success; do that in ok_cb if desired.
 */
void purple_account_request_password(PurpleAccount *account, GCallback ok_cb,
				     GCallback cancel_cb, void *user_data);

/**
 * purple_account_request_change_password:
 * @account: The account to change the password on.
 *
 * Requests information from the user to change the account's password.
 */
void purple_account_request_change_password(PurpleAccount *account);

/**
 * purple_account_request_change_user_info:
 * @account: The account to change the user information on.
 *
 * Requests information from the user to change the account's
 * user information.
 */
void purple_account_request_change_user_info(PurpleAccount *account);

/**
 * purple_account_set_user_info:
 * @account:   The account.
 * @user_info: The user information.
 *
 * Sets the account's user information
 */
void purple_account_set_user_info(PurpleAccount *account, const char *user_info);

/**
 * purple_account_set_buddy_icon_path:
 * @account: The account.
 * @path:	  The buddy icon non-cached path.
 *
 * Sets the account's buddy icon path.
 */
void purple_account_set_buddy_icon_path(PurpleAccount *account, const char *path);

/**
 * purple_account_set_protocol_id:
 * @account:     The account.
 * @protocol_id: The protocol ID.
 *
 * Sets the account's protocol ID.
 */
void purple_account_set_protocol_id(PurpleAccount *account,
								  const char *protocol_id);

/**
 * purple_account_set_connection:
 * @account: The account.
 * @gc:      The connection.
 *
 * Sets the account's connection.
 */
void purple_account_set_connection(PurpleAccount *account, PurpleConnection *gc);

/**
 * purple_account_set_remember_password:
 * @account: The account.
 * @value:   %TRUE if it should remember the password.
 *
 * Sets whether or not this account should save its password.
 */
void purple_account_set_remember_password(PurpleAccount *account, gboolean value);

/**
 * purple_account_set_enabled:
 * @account: The account.
 * @value:   %TRUE if it is enabled.
 *
 * Sets whether or not this account is enabled.
 */
void purple_account_set_enabled(PurpleAccount *account, gboolean value);

/**
 * purple_account_set_proxy_info:
 * @account: The account.
 * @info:    The proxy information.
 *
 * Sets the account's proxy information.
 */
void purple_account_set_proxy_info(PurpleAccount *account, PurpleProxyInfo *info);

/**
 * purple_account_set_status_types:
 * @account:      The account.
 * @status_types: (element-type PurpleStatusType): The list of status types.
 *
 * Sets the account's status types.
 */
void purple_account_set_status_types(PurpleAccount *account, GList *status_types);

/**
 * purple_account_set_status:
 * @account: The account.
 * @status_id: The ID of the status.
 * @active: Whether @status_id is to be activated (%TRUE) or deactivated
 *          (%FALSE).
 * @...: A %NULL-terminated list of pairs of `const char *` attribute name
 *       followed by `const char *` attribute value for the status. (For
 *       example, one pair might be `"message"` followed by `"hello, talk to
 *       me!"`.)
 *
 * Activates or deactivates a status.
 *
 * All changes to the statuses of an account go through this function or
 * [method@Purple.Account.set_status_attrs].
 *
 * You can only deactivate an exclusive status by activating another exclusive
 * status. So, if @status_id is an exclusive status and @active is %FALSE, this
 * function does nothing.
 *
 * Variadic version of [method@Purple.Account.set_status_attrs].
 */
void purple_account_set_status(PurpleAccount *account, const char *status_id,
	gboolean active, ...) G_GNUC_NULL_TERMINATED;


/**
 * purple_account_set_status_attrs: (rename-to purple_account_set_status):
 * @account: The account.
 * @status_id: The ID of the status.
 * @active: Whether @status_id is to be activated (%TRUE) or deactivated
 *          (%FALSE).
 * @attrs: (element-type utf8 utf8): A hash table with keys of attribute
 *         names and values of attributes for the status. (For example, one
 *         pair might be a key of `"message"` with a value of `"hello, talk to
 *         me!"`.)
 *
 * Activates or deactivates a status.
 *
 * All changes to the statuses of an account go through this function or
 * [method@Purple.Account.set_status].
 *
 * You can only deactivate an exclusive status by activating another exclusive
 * status. So, if @status_id is an exclusive status and @active is %FALSE, this
 * function does nothing.
 *
 * Since: 3.0.0
 */
void purple_account_set_status_attrs(PurpleAccount *account,
	const char *status_id, gboolean active, GHashTable *attrs);

/**
 * purple_account_clear_settings:
 * @account: The account.
 *
 * Clears all protocol-specific settings on an account.
 */
void purple_account_clear_settings(PurpleAccount *account);

/**
 * purple_account_set_int:
 * @account: The account.
 * @name:    The name of the setting.
 * @value:   The setting's value.
 *
 * Sets a protocol-specific integer setting for an account.
 */
void purple_account_set_int(PurpleAccount *account, const char *name, int value);

/**
 * purple_account_set_string:
 * @account: The account.
 * @name:    The name of the setting.
 * @value:   The setting's value.
 *
 * Sets a protocol-specific string setting for an account.
 */
void purple_account_set_string(PurpleAccount *account, const char *name,
							 const char *value);

/**
 * purple_account_set_bool:
 * @account: The account.
 * @name:    The name of the setting.
 * @value:   The setting's value.
 *
 * Sets a protocol-specific boolean setting for an account.
 */
void purple_account_set_bool(PurpleAccount *account, const char *name,
						   gboolean value);

/**
 * purple_account_is_connected:
 * @account: The account.
 *
 * Returns whether or not the account is connected.
 *
 * Returns: %TRUE if connected, or %FALSE otherwise.
 */
gboolean purple_account_is_connected(PurpleAccount *account);

/**
 * purple_account_is_connecting:
 * @account: The account.
 *
 * Returns whether or not the account is connecting.
 *
 * Returns: %TRUE if connecting, or %FALSE otherwise.
 */
gboolean purple_account_is_connecting(PurpleAccount *account);

/**
 * purple_account_is_disconnected:
 * @account: The account.
 *
 * Returns whether or not the account is disconnected.
 *
 * Returns: %TRUE if disconnected, or %FALSE otherwise.
 */
gboolean purple_account_is_disconnected(PurpleAccount *account);

/**
 * purple_account_get_user_info:
 * @account: The account.
 *
 * Returns the account's user information.
 *
 * Returns: The user information.
 */
const char *purple_account_get_user_info(PurpleAccount *account);

/**
 * purple_account_get_buddy_icon_path:
 * @account: The account.
 *
 * Gets the account's buddy icon path.
 *
 * Returns: The buddy icon's non-cached path.
 */
const char *purple_account_get_buddy_icon_path(PurpleAccount *account);

/**
 * purple_account_get_protocol_id:
 * @account: The account.
 *
 * Returns the account's protocol ID.
 *
 * Returns: The protocol ID.
 */
const char *purple_account_get_protocol_id(PurpleAccount *account);

/**
 * purple_account_get_protocol:
 * @account: The #PurpleAccount instance.
 *
 * Gets the #PurpleProtocol instance for @account.
 *
 * Returns: (transfer none): The #PurpleProtocol for @account or %NULL if it
 *          could not be found.
 *
 * Since: 3.0.0
 */
PurpleProtocol *purple_account_get_protocol(PurpleAccount *account);

/**
 * purple_account_get_protocol_name:
 * @account: The account.
 *
 * Returns the account's protocol name.
 *
 * Returns: The protocol name.
 */
const char *purple_account_get_protocol_name(PurpleAccount *account);

/**
 * purple_account_get_connection:
 * @account: The account.
 *
 * Returns the account's connection.
 *
 * Returns: (transfer none): The connection.
 */
PurpleConnection *purple_account_get_connection(PurpleAccount *account);

/**
 * purple_account_get_remember_password:
 * @account: The account.
 *
 * Returns whether or not this account should save its password.
 *
 * Returns: %TRUE if it should remember the password.
 */
gboolean purple_account_get_remember_password(PurpleAccount *account);

/**
 * purple_account_get_enabled:
 * @account: The account.
 *
 * Returns whether or not this account is enabled.
 *
 * Returns: %TRUE if it enabled on this UI.
 */
gboolean purple_account_get_enabled(PurpleAccount *account);

/**
 * purple_account_get_proxy_info:
 * @account: The account.
 *
 * Returns the account's proxy information.
 *
 * Returns: (transfer none): The proxy information.
 */
PurpleProxyInfo *purple_account_get_proxy_info(PurpleAccount *account);

/**
 * purple_account_get_active_status:
 * @account:   The account.
 *
 * Returns the active status for this account.  This looks through
 * the PurplePresence associated with this account and returns the
 * PurpleStatus that has its active flag set to "TRUE."  There can be
 * only one active PurpleStatus in a PurplePresence.
 *
 * Returns: (transfer none): The active status.
 */
PurpleStatus *purple_account_get_active_status(PurpleAccount *account);

/**
 * purple_account_get_status:
 * @account:   The account.
 * @status_id: The status ID.
 *
 * Returns the account status with the specified ID.
 *
 * Returns: (transfer none): The status, or %NULL if it was never registered.
 */
PurpleStatus *purple_account_get_status(PurpleAccount *account,
									const char *status_id);

/**
 * purple_account_get_status_type:
 * @account: The account.
 * @id:      The ID of the status type to find.
 *
 * Returns the account status type with the specified ID.
 *
 * Returns: The status type if found, or %NULL.
 */
PurpleStatusType *purple_account_get_status_type(PurpleAccount *account,
											 const char *id);

/**
 * purple_account_get_status_type_with_primitive:
 * @account:   The account.
 * @primitive: The type of the status type to find.
 *
 * Returns the account status type with the specified primitive.
 * Note: It is possible for an account to have more than one
 * PurpleStatusType with the same primitive.  In this case, the
 * first PurpleStatusType is returned.
 *
 * Returns: The status if found, or %NULL.
 */
PurpleStatusType *purple_account_get_status_type_with_primitive(
							PurpleAccount *account,
							PurpleStatusPrimitive primitive);

/**
 * purple_account_get_presence:
 * @account: The account.
 *
 * Returns the account's presence.
 *
 * Returns: (transfer none): The account's presence.
 */
PurplePresence *purple_account_get_presence(PurpleAccount *account);

/**
 * purple_account_is_status_active:
 * @account:   The account.
 * @status_id: The status ID.
 *
 * Returns whether or not an account status is active.
 *
 * Returns: TRUE if active, or FALSE if not.
 */
gboolean purple_account_is_status_active(PurpleAccount *account,
									   const char *status_id);

/**
 * purple_account_get_status_types:
 * @account: The account.
 *
 * Returns the account's status types.
 *
 * Returns: (transfer none) (element-type PurpleStatusType): The account's status types.
 */
GList *purple_account_get_status_types(PurpleAccount *account);

/**
 * purple_account_get_int:
 * @account:       The account.
 * @name:          The name of the setting.
 * @default_value: The default value.
 *
 * Returns a protocol-specific integer setting for an account.
 *
 * Returns: The value.
 */
int purple_account_get_int(PurpleAccount *account, const char *name,
						 int default_value);

/**
 * purple_account_get_string:
 * @account:       The account.
 * @name:          The name of the setting.
 * @default_value: The default value.
 *
 * Returns a protocol-specific string setting for an account.
 *
 * Returns: The value.
 */
const char *purple_account_get_string(PurpleAccount *account,
									const char *name,
									const char *default_value);

/**
 * purple_account_get_bool:
 * @account:       The account.
 * @name:          The name of the setting.
 * @default_value: The default value.
 *
 * Returns a protocol-specific boolean setting for an account.
 *
 * Returns: The value.
 */
gboolean purple_account_get_bool(PurpleAccount *account, const char *name,
							   gboolean default_value);

/**
 * purple_account_add_buddy:
 * @account: The account.
 * @buddy: The buddy to add.
 * @message: The invite message.  This may be ignored by a protocol.
 *
 * Adds a buddy to the server-side buddy list for the specified account.
 */
void purple_account_add_buddy(PurpleAccount *account, PurpleBuddy *buddy, const char *message);

/**
 * purple_account_add_buddies:
 * @account: The account.
 * @buddies: (element-type PurpleBuddy): The list of PurpleBlistNodes representing the buddies to add.
 * @message: The invite message.  This may be ignored by a protocol.
 *
 * Adds a list of buddies to the server-side buddy list.
 */
void purple_account_add_buddies(PurpleAccount *account, GList *buddies, const char *message);

/**
 * purple_account_remove_buddy:
 * @account: The account.
 * @buddy: The buddy to remove.
 * @group: The group to remove the buddy from.
 *
 * Removes a buddy from the server-side buddy list.
 */
void purple_account_remove_buddy(PurpleAccount *account, PurpleBuddy *buddy,
								PurpleGroup *group);

/**
 * purple_account_remove_buddies:
 * @account: The account.
 * @buddies: (element-type PurpleBuddy): The list of buddies to remove.
 * @groups: (element-type PurpleGroup): The list of groups to remove buddies from.  Each node of this
 *               list should match the corresponding node of buddies.
 *
 * Removes a list of buddies from the server-side buddy list.
 *
 * Note: The lists buddies and groups are parallel lists.  Be sure that node n of
 *       groups matches node n of buddies.
 */
void purple_account_remove_buddies(PurpleAccount *account, GList *buddies,
									GList *groups);

/**
 * purple_account_remove_group:
 * @account: The account.
 * @group: The group to remove.
 *
 * Removes a group from the server-side buddy list.
 */
void purple_account_remove_group(PurpleAccount *account, PurpleGroup *group);

/**
 * purple_account_change_password:
 * @account: The account.
 * @orig_pw: The old password.
 * @new_pw: The new password.
 *
 * Changes the password on the specified account.
 */
void purple_account_change_password(PurpleAccount *account, const char *orig_pw,
									const char *new_pw);

/**
 * purple_account_supports_offline_message:
 * @account: The account
 * @buddy:   The buddy
 *
 * Whether the account supports sending offline messages to buddy.
 */
gboolean purple_account_supports_offline_message(PurpleAccount *account, PurpleBuddy *buddy);

/**
 * purple_account_get_error:
 * @account: The account whose error should be retrieved.
 *
 * Get the error that caused the account to be disconnected, or %NULL if the
 * account is happily connected or disconnected without an error.
 *
 * Returns: (transfer none): The type of error and a human-readable description
 *          of the current error, or %NULL if there is no current error.  This
 *          pointer is guaranteed to remain valid until the @ref
 *          account-error-changed signal is emitted for @account.
 */
const PurpleConnectionErrorInfo *purple_account_get_error(PurpleAccount *account);

/**
 * purple_account_set_error:
 * @account: The account whose error should be set.
 * @info: (nullable) (transfer full): The [struct@Purple.ConnectionErrorInfo]
 *        to set.
 *
 * Sets the error of @account to @info. Note that setting this won't disconnect
 * the account. This is intended to be called by libpurple when there is a
 * connection failure, when invalid settings are entered in an account editor,
 * or similar situations.
 *
 * Since: 3.0.0
 */
void purple_account_set_error(PurpleAccount *account, PurpleConnectionErrorInfo *info);

/**
 * purple_account_set_require_password:
 * @account: The instance.
 * @require_password: Whether or not this account should require a password.
 *
 * For protocols that have an optional password, this settings tells libpurple
 * that it should look for a password in the [class@Purple.CredentialManager]
 * or prompt the user if a password can not be found.
 *
 * Since: 3.0.0
 */
void purple_account_set_require_password(PurpleAccount *account, gboolean require_password);

/**
 * purple_account_get_require_password:
 * @account: The instance.
 *
 * Gets whether or not @account requires a password.
 *
 * Returns: %TRUE if the account requires a password, %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_account_get_require_password(PurpleAccount *account);

/**
 * purple_account_freeze_notify_settings:
 * @account: The instance.
 *
 * Increment the freeze count for settings on @account. When the freeze count
 * is greater than 0, the [signal@Purple.Account::setting-changed] signal will
 * not be emitted until the freeze count returns to 0.
 *
 * This is intended to only notify once @account has reached a consistent
 * state. Most user interfaces change all of the properties on an account at
 * once and some of them may be co-dependent, so all values need to be updated
 * before the change can be acted upon.
 *
 * Call [method@Purple.Account.thaw_notify_settings] to decrement the freeze
 * counter.
 *
 * Since: 3.0.0
 */
void purple_account_freeze_notify_settings(PurpleAccount *account);

/**
 * purple_account_thaw_notify_settings:
 * @account: The instance.
 *
 * Decrements the freeze count for settings on @account.
 *
 * See [method@Purple.Account.freeze_notify_settings] for more information.
 *
 * Since: 3.0.0
 */
void purple_account_thaw_notify_settings(PurpleAccount *account);

G_END_DECLS

#endif /* PURPLE_ACCOUNT_H */
