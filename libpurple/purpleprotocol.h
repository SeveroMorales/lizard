/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_PROTOCOL_H
#define PURPLE_PROTOCOL_H

#include <glib.h>
#include <glib-object.h>

#define PURPLE_TYPE_PROTOCOL (purple_protocol_get_type())

/**
 * PurpleProtocol:
 *
 * #PurpleProtocol is the base type for all protocols in libpurple.
 *
 * Since: 3.0.0
 */

G_DECLARE_DERIVABLE_TYPE(PurpleProtocol, purple_protocol, PURPLE, PROTOCOL,
                         GObject)

#include "account.h"
#include "buddyicon.h"
#include "connection.h"
#include "image.h"
#include "purpleaccountoption.h"
#include "purpleaccountusersplit.h"
#include "purplemessage.h"
#include "purplewhiteboardops.h"
#include "status.h"

/**
 * PurpleProtocolOptions:
 * @OPT_PROTO_UNIQUE_CHATNAME: User names are unique to a chat and are not
 *           shared between rooms.<sbr/>
 *           XMPP lets you choose what name you want in chats, so it shouldn't
 *           be pulling the aliases from the buddy list for the chat list; it
 *           gets annoying.
 * @OPT_PROTO_CHAT_TOPIC: Chat rooms have topics.<sbr/>
 *           IRC and XMPP support this.
 * @OPT_PROTO_NO_PASSWORD: Don't require passwords for sign-in.<sbr/>
 *           IRC doesn't require passwords, so there's no need for a
 *           password prompt.
 * @OPT_PROTO_MAIL_CHECK: Notify on new mail.<sbr/>
 *           MSN and Yahoo notify you when you have new mail.
 * @OPT_PROTO_PASSWORD_OPTIONAL: Allow passwords to be optional.<sbr/>
 *           Passwords in IRC are optional, and are needed for certain
 *           functionality.
 * @OPT_PROTO_USE_POINTSIZE: Allows font size to be specified in sane point
 *           size.<sbr/>
 *           Probably just XMPP and Y!M
 * @OPT_PROTO_REGISTER_NOSCREENNAME: Set the Register button active even when
 *           the username has not been specified.<sbr/>
 *           Gadu-Gadu doesn't need a username to register new account (because
 *           usernames are assigned by the server).
 * @OPT_PROTO_SLASH_COMMANDS_NATIVE: Indicates that slash commands are native
 *           to this protocol.<sbr/>
 *           Used as a hint that unknown commands should not be sent as
 *           messages.
 * @OPT_PROTO_INVITE_MESSAGE: Indicates that this protocol supports sending a
 *           user-supplied message along with an invitation.
 * @OPT_PROTO_AUTHORIZATION_GRANTED_MESSAGE: Indicates that this protocol
 *           supports sending a user-supplied message along with an
 *           authorization acceptance.
 * @OPT_PROTO_AUTHORIZATION_DENIED_MESSAGE: Indicates that this protocol
 *           supports sending a user-supplied message along with an
 *           authorization denial.
 *
 * Protocol options
 *
 * These should all be stuff that some protocols can do and others can't.
 */
typedef enum  /*< flags >*/
{
    OPT_PROTO_UNIQUE_CHATNAME               = 0x00000004,
    OPT_PROTO_CHAT_TOPIC                    = 0x00000008,
    OPT_PROTO_NO_PASSWORD                   = 0x00000010,
    OPT_PROTO_MAIL_CHECK                    = 0x00000020,
    OPT_PROTO_PASSWORD_OPTIONAL             = 0x00000080,
    OPT_PROTO_USE_POINTSIZE                 = 0x00000100,
    OPT_PROTO_REGISTER_NOSCREENNAME         = 0x00000200,
    OPT_PROTO_SLASH_COMMANDS_NATIVE         = 0x00000400,
    OPT_PROTO_INVITE_MESSAGE                = 0x00000800,
    OPT_PROTO_AUTHORIZATION_GRANTED_MESSAGE = 0x00001000,
    OPT_PROTO_AUTHORIZATION_DENIED_MESSAGE  = 0x00002000

} PurpleProtocolOptions;

/**
 * PurpleProtocolClass:
 * @get_user_splits: Returns a list of all #PurpleAccountUserSplit's that the
 *                   procotol provides.
 * @get_account_options: Returns a list of all #PurpleAccountOption's for the
 *                       protocol.
 * @get_buddy_icon_spec: Returns a #PurpleBuddyIconSpec that should be used.
 * @get_whiteboard_ops: Return the #PurpleWhiteboardOps that should be used.
 * @login: Logs into the server.
 * @close: Close sconnection with the server.
 * @status_types: Returns a list of #PurpleStatusType which exist for this
 *                account; and must add at least the offline and online states.
 * @list_icon: Returns the base icon name for the given buddy and account. If
 *             buddy is %NULL and the account is non-%NULL, it will return the
 *             name to use for the account's icon. If both are %NULL, it will
 *             return the name to use for the protocol's icon.
 *
 * The base class for all protocols.
 *
 * All protocol types must implement the methods in this class.
 *
 * Since: 3.0.0
 */
struct _PurpleProtocolClass {
	GObjectClass parent_class;

	GList *(*get_user_splits)(PurpleProtocol *protocol);
	GList *(*get_account_options)(PurpleProtocol *protocol);
	PurpleBuddyIconSpec *(*get_buddy_icon_spec)(PurpleProtocol *protocol);
	PurpleWhiteboardOps *(*get_whiteboard_ops)(PurpleProtocol *protocol);

	void (*login)(PurpleProtocol *protocol, PurpleAccount *account);

	void (*close)(PurpleProtocol *protocol, PurpleConnection *connection);

	void (*can_connect_async)(PurpleProtocol *protocol, PurpleAccount *account, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
	gboolean (*can_connect_finish)(PurpleProtocol *protocol, GAsyncResult *result, GError **error);

	PurpleConnection *(*create_connection)(PurpleProtocol *protocol, PurpleAccount *account, const char *password, GError **error);

	GList *(*status_types)(PurpleProtocol *protocol, PurpleAccount *account);

	/*< private >*/
	gpointer reserved[4];
};

/**
 * PURPLE_PROTOCOL_IMPLEMENTS:
 * @protocol: The protocol in which to check
 * @IFACE:    The interface name in caps. e.g. <literal>CLIENT</literal>
 * @func:     The function to check
 *
 * Returns: %TRUE if a protocol implements a function in an interface,
 *          %FALSE otherwise.
 *
 * Since: 3.0.0
 */
#define PURPLE_PROTOCOL_IMPLEMENTS(protocol, IFACE, func) \
	(PURPLE_IS_PROTOCOL_##IFACE(protocol) && \
	 PURPLE_PROTOCOL_##IFACE##_GET_IFACE(protocol)->func != NULL)

G_BEGIN_DECLS

/**
 * purple_protocol_get_id:
 * @protocol: The #PurpleProtocol instance.
 *
 * Gets the ID of a protocol.
 *
 * Returns: The ID of the protocol.
 *
 * Since: 3.0.0
 */
const gchar *purple_protocol_get_id(PurpleProtocol *protocol);

/**
 * purple_protocol_get_name:
 * @protocol: The #PurpleProtocol instance.
 *
 * Gets the translated name of a protocol.
 *
 * Returns: The translated name of the protocol.
 *
 * Since: 3.0.0
 */
const gchar *purple_protocol_get_name(PurpleProtocol *protocol);

/**
 * purple_protocol_get_description:
 * @protocol: The #PurpleProtocol instance.
 *
 * Gets the description of a protocol.
 *
 * Returns: The Description of the protocol.
 *
 * Since: 3.0.0
 */
const gchar *purple_protocol_get_description(PurpleProtocol *protocol);

/**
 * purple_protocol_get_options:
 * @protocol: The #PurpleProtocol instance.
 *
 * Gets the options of a protocol.
 *
 * Returns: The options of the protocol.
 *
 * Since: 3.0.0
 */
PurpleProtocolOptions purple_protocol_get_options(PurpleProtocol *protocol);

/**
 * purple_protocol_get_user_splits:
 * @protocol: The #PurpleProtocol instance.
 *
 * Gets the user splits of a protocol.
 *
 * Returns: (element-type PurpleAccountUserSplit) (transfer full): The user
 *          splits of the protocol.
 *
 * Since: 3.0.0
 */
GList *purple_protocol_get_user_splits(PurpleProtocol *protocol);

/**
 * purple_protocol_get_account_options:
 * @protocol: The #PurpleProtocol instance.
 *
 * Gets the account options for a protocol.
 *
 * Returns: (element-type PurpleAccountOption) (transfer full): The account
 *          options for the protocol.
 *
 * Since: 3.0.0
 */
GList *purple_protocol_get_account_options(PurpleProtocol *protocol);

/**
 * purple_protocol_get_icon_spec:
 * @protocol: The #PurpleProtocol instance.
 *
 * Gets the icon spec of a protocol.
 *
 * Returns: (transfer full): The icon spec of the protocol.
 *
 * Since: 3.0.0
 */
PurpleBuddyIconSpec *purple_protocol_get_icon_spec(PurpleProtocol *protocol);

/**
 * purple_protocol_get_whiteboard_ops:
 * @protocol: The #PurpleProtocol instance.
 *
 * Gets the whiteboard ops of a protocol.
 *
 * Returns: (transfer none): The whiteboard ops of the protocol.
 *
 * Since: 3.0.0
 */
PurpleWhiteboardOps *purple_protocol_get_whiteboard_ops(PurpleProtocol *protocol);

/**
 * purple_protocol_login:
 * @protocol: The instance.
 * @account: The [class@Purple.Account] to login.
 *
 * Logs @account in to @protocol.
 *
 * Since: 3.0.0
 */
void purple_protocol_login(PurpleProtocol *protocol, PurpleAccount *account);

/**
 * purple_protocol_close:
 * @protocol: The #PurpleProtocol instance.
 * @connection: The #PurpleConnection to close.
 *
 * Closes @connection using @protocol.
 *
 * Since: 3.0.0
 */
void purple_protocol_close(PurpleProtocol *protocol, PurpleConnection *connection);

/**
 * purple_protocol_can_connect_async:
 * @protocol: The instance.
 * @account: The [class@Purple.Account] instance.
 * @cancellable: (nullable): The [class@Gio.Cancellable] instance.
 * @callback: (scope async): The [callback@Gio.AsyncReadyCallback] to call.
 * @data: (nullable): User data to pass to @callback.
 *
 * Asks @protocol if it can determine if @account can be connected.
 *
 * Most protocol plugins will call [method@Gio.NetworkMonitor.can_reach_async]
 * to determine if a connection is possible.
 *
 * Since: 3.0.0
 */
void purple_protocol_can_connect_async(PurpleProtocol *protocol, PurpleAccount *account, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);

/**
 * purple_protocol_can_connect_finish:
 * @protocol: The instance.
 * @result: The [iface@Gio.AsyncResult] of the operation.
 * @error: Return address for a #GError, or %NULL.
 *
 * This should be called from the callback of
 * [method@Purple.Protocol.can_connect_async] to get the result of the call.
 *
 * Returns: %TRUE on success, otherwise %FALSE with @error optionally set.
 *
 * Since: 3.0.0
 */
gboolean purple_protocol_can_connect_finish(PurpleProtocol *protocol, GAsyncResult *result, GError **error);


/**
 * purple_protocol_create_connection:
 * @protocol: The instance.
 * @account: The [class@Purple.Account] for the connection.
 * @password: The password for the account.
 * @error: A return address for a [type@GLib.GError].
 *
 * Creates a [class@PurpleConnection] for @account.
 *
 * A protocol may indicate an error by setting @error and returning %NULL.
 *
 * Returns: (transfer full): The new connection or %NULL with @error possibly
 *          set on error.
 *
 * Since: 3.0.0
 */
PurpleConnection *purple_protocol_create_connection(PurpleProtocol *protocol, PurpleAccount *account, const char *password, GError **error);

/**
 * purple_protocol_get_status_types:
 * @protocol: The #PurpleProtocol instance.
 * @account: The #PurpleAccount instance.
 *
 * Gets all of the #PurpleStatusType's for @account which uses @protocol.
 *
 * Returns: (transfer full) (element-type PurpleStatusType): A list of the
 *          available PurpleStatusType's for @account with @protocol.
 *
 * Since: 3.0.0
 */
GList *purple_protocol_get_status_types(PurpleProtocol *protocol, PurpleAccount *account);

/**
 * purple_protocol_get_icon_name:
 * @protocol: The #PurpleProtocol instance.
 *
 * Gets the name of the icon that the protocol made available via either
 * purple_protocol_get_icon_search_path() or
 * purple_protocol_get_resource_path().
 *
 * Returns: The name of the icon for @protocol.
 *
 * Since: 3.0.0
 */
const gchar *purple_protocol_get_icon_name(PurpleProtocol *protocol);

/**
 * purple_protocol_get_icon_search_path:
 * @protocol: The #PurpleProtocol instance.
 *
 * Gets the icon search path for @protocol. This is used to allow protocol
 * plugins to install their icons in any XDG icon theme compliant directory.
 * The returned value should be the path of where the icons are on disk. See
 * gtk_icon_theme_add_search_path() for additional information.
 *
 * User interfaces will look for icons in this path named
 * %chat-&lt;protocol-name&gt; where protocol-name is the name property of @protocol.
 *
 * Returns: The file system path where the icons can be found.
 *
 * Since: 3.0.0
 */
const gchar *purple_protocol_get_icon_search_path(PurpleProtocol *protocol);

/**
 * purple_protocol_get_icon_resource_path:
 * @protocol: The #PurpleProtocol instance.
 *
 * Gets the icon resource path for @protocol. This is used to make icons that
 * have been embedded into a plugin available to libpurple. The returned value
 * should be the path of where the icons are in the resource. See
 * gtk_icon_theme_add_resource_path() for additional information.
 *
 * User interfaces will look for icons in this path named
 * %chat-&lt;protocol-name&gt; where protocol-name is the name property of @protocol.
 *
 * Returns: The gresource path where the icons can be found.
 *
 * Since: 3.0.0
 */
const gchar *purple_protocol_get_icon_resource_path(PurpleProtocol *protocol);

G_END_DECLS

#endif /* PURPLE_PROTOCOL_H */
