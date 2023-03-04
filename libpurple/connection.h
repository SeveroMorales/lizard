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

#ifndef PURPLE_CONNECTION_H
#define PURPLE_CONNECTION_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PURPLE_TYPE_CONNECTION  purple_connection_get_type()
G_DECLARE_DERIVABLE_TYPE(PurpleConnection, purple_connection, PURPLE,
                         CONNECTION, GObject)

#define PURPLE_TYPE_CONNECTION_UI_OPS  (purple_connection_ui_ops_get_type())
typedef struct _PurpleConnectionUiOps PurpleConnectionUiOps;

/* This is meant to track use-after-free errors.
 * TODO: it should be disabled in released code. */
#define PURPLE_ASSERT_CONNECTION_IS_VALID(gc) \
	_purple_assert_connection_is_valid(gc, __FILE__, __LINE__)


/**
 * PurpleConnectionFlags:
 * @PURPLE_CONNECTION_FLAG_HTML: Connection sends/receives in 'HTML'
 * @PURPLE_CONNECTION_FLAG_NO_BGCOLOR: Connection does not send/receive
 *                                     background colors
 * @PURPLE_CONNECTION_FLAG_AUTO_RESP: Send auto responses when away
 * @PURPLE_CONNECTION_FLAG_FORMATTING_WBFO: The text buffer must be formatted
 *                                          as a whole
 * @PURPLE_CONNECTION_FLAG_NO_NEWLINES: No new lines are allowed in outgoing
 *                                      messages
 * @PURPLE_CONNECTION_FLAG_NO_FONTSIZE: Connection does not send/receive font
 *                                      sizes
 * @PURPLE_CONNECTION_FLAG_NO_URLDESC: Connection does not support descriptions
 *                                     with links
 * @PURPLE_CONNECTION_FLAG_NO_IMAGES: Connection does not support sending of
 *                                    images
 * @PURPLE_CONNECTION_FLAG_SUPPORT_MOODS: Connection supports setting moods
 * @PURPLE_CONNECTION_FLAG_SUPPORT_MOOD_MESSAGES: Connection supports setting
 *                                                a message on moods
 *
 * Flags to change behavior of the client for a given connection.
 */
typedef enum /*< flags >*/
{
	PURPLE_CONNECTION_FLAG_HTML       = 0x0001,
	PURPLE_CONNECTION_FLAG_NO_BGCOLOR = 0x0002,
	PURPLE_CONNECTION_FLAG_AUTO_RESP  = 0x0004,
	PURPLE_CONNECTION_FLAG_FORMATTING_WBFO = 0x0008,
	PURPLE_CONNECTION_FLAG_NO_NEWLINES = 0x0010,
	PURPLE_CONNECTION_FLAG_NO_FONTSIZE = 0x0020,
	PURPLE_CONNECTION_FLAG_NO_URLDESC = 0x0040,
	PURPLE_CONNECTION_FLAG_NO_IMAGES = 0x0080,
	PURPLE_CONNECTION_FLAG_SUPPORT_MOODS = 0x0200,
	PURPLE_CONNECTION_FLAG_SUPPORT_MOOD_MESSAGES = 0x0400
} PurpleConnectionFlags;

/**
 * PurpleConnectionState:
 * @PURPLE_CONNECTION_STATE_DISCONNECTED: Disconnected
 * @PURPLE_CONNECTION_STATE_DISCONNECTING: Disconnecting
 * @PURPLE_CONNECTION_STATE_CONNECTED: Connected
 * @PURPLE_CONNECTION_STATE_CONNECTING: Connecting
 *
 * A representation of the state of a [class@Purple.Connection].
 */
typedef enum {
	PURPLE_CONNECTION_STATE_DISCONNECTED = 0,
	PURPLE_CONNECTION_STATE_DISCONNECTING,
	PURPLE_CONNECTION_STATE_CONNECTED,
	PURPLE_CONNECTION_STATE_CONNECTING
} PurpleConnectionState;

/**
 * PURPLE_CONNECTION_ERROR:
 *
 * Error domain for Purple connection errors. Errors in this domain will be
 * from the #PurpleConnectionError enum.
 *
 * Since: 3.0.0
 */
#define PURPLE_CONNECTION_ERROR (g_quark_from_static_string("purple-connection-error"))

#include <time.h>

#include "account.h"
#include "purpleconnectionerrorinfo.h"
#include "purpleprotocol.h"
#include "status.h"

struct _PurpleConnectionClass {
	/*< private >*/
	GObjectClass parent;

	gboolean (*connect)(PurpleConnection *connection, GError **error);
	gboolean (*disconnect)(PurpleConnection *connection, GError **error);

	/*< private >*/
	gpointer reserved[8];
};

/**
 * PurpleConnectionUiOps:
 * @connected: Called when a connection is established (just before the
 *   <link linkend="connections-signed-on"><literal>"signed-on"</literal></link>
 *             signal).
 * @disconnected: Called when a connection is ended (between the
 *   <link linkend="connections-signing-off"><literal>"signing-off"</literal></link>
 *   and <link linkend="connections-signed-off"><literal>"signed-off"</literal></link>
 *                signals).
 * @network_connected: Called when libpurple discovers that the computer's
 *                     network connection is active.  On Linux, this uses
 *                     Network Manager if available; on Windows, it uses
 *                     Win32's network change notification infrastructure.
 * @network_disconnected: Called when libpurple discovers that the computer's
 *                        network connection has gone away.
 * @report_disconnect: Called when an error causes a connection to be
 *                     disconnected. Called before @disconnected.
 *                     <sbr/>See purple_connection_error().
 *                     <sbr/>@reason: why the connection ended, if known, or
 *                                 #PURPLE_CONNECTION_ERROR_OTHER_ERROR, if not.
 *                     <sbr/>@text:   a localized message describing the
 *                                 disconnection in more detail to the user.
 *
 * Connection UI operations.  Used to notify the user of changes to
 * connections, such as being disconnected, and to respond to the
 * underlying network connection appearing and disappearing.  UIs should
 * call #purple_connections_set_ui_ops() with an instance of this struct.
 *
 * See <link linkend="chapter-ui-ops">List of <literal>UiOps</literal> Structures</link>
 */
struct _PurpleConnectionUiOps
{
	void (*connected)(PurpleConnection *gc);
	void (*disconnected)(PurpleConnection *gc);

	void (*report_disconnect)(PurpleConnection *gc,
	                          PurpleConnectionError reason,
	                          const char *text);

	/*< private >*/
	void (*_purple_reserved1)(void);
	void (*_purple_reserved2)(void);
	void (*_purple_reserved3)(void);
	void (*_purple_reserved4)(void);
};

/******************************************************************************
 * To be deleted in the future
 *****************************************************************************/
void
_purple_assert_connection_is_valid(PurpleConnection *gc,
	const gchar *file, int line);

/**************************************************************************/
/* Connection API                                                         */
/**************************************************************************/

/**
 * purple_connection_connect:
 * @connection: The instance.
 * @error: Return address for a #GError, or %NULL.
 *
 * Tells the connection to connect. This is done by calling the
 * [vfunc@Purple.Connection.connect] function. State is managed by this
 * function.
 *
 * The default [vfunc@Purple.Connection.connect] is to call
 * [vfunc@Purple.Protocol.login].
 *
 * Due to the asynchronous nature of network connections, the return value and
 * @error are to be used to do some initial validation before a connection is
 * actually attempted.
 *
 * Returns: %TRUE if the initial connection for @account was successful,
 *          otherwise %FALSE with @error possibly set.
 *
 * Since: 3.0.0
 */
gboolean purple_connection_connect(PurpleConnection *connection, GError **error);

/**
 * purple_connection_disconnect:
 * @connection: The instance.
 * @error: Return address for a #GError, or %NULL.
 *
 * Tells the connection to disconnect. This is done by calling the
 * [vfunc@Purple.Connection.disconnect] function. State is managed by this
 * function.
 *
 * The default [vfunc@Purple.Connection.disconnect] is to call
 * [vfunc@Purple.Protocol.close].
 *
 * Returns: %TRUE if the account was disconnected gracefully, otherwise %FALSE
 *          with @error possibly set.
 *
 * Since: 3.0.0
 */
gboolean purple_connection_disconnect(PurpleConnection *connection, GError **error);

/**
 * purple_connection_set_state:
 * @gc:    The connection.
 * @state: The connection state.
 *
 * Sets the connection state.  Protocols should call this and pass in
 * the state #PURPLE_CONNECTION_CONNECTED when the account is completely
 * signed on.  What does it mean to be completely signed on?  If
 * the core can call protocol's set_status, and it successfully changes
 * your status, then the account is online.
 */
void purple_connection_set_state(PurpleConnection *gc, PurpleConnectionState state);

/**
 * purple_connection_set_flags:
 * @gc:    The connection.
 * @flags: The flags.
 *
 * Sets the connection flags.
 *
 * Since: 3.0.0
 */
void purple_connection_set_flags(PurpleConnection *gc, PurpleConnectionFlags flags);

/**
 * purple_connection_set_display_name:
 * @gc:   The connection.
 * @name: The displayed name.
 *
 * Sets the connection's displayed name.
 */
void purple_connection_set_display_name(PurpleConnection *gc, const char *name);

/**
 * purple_connection_set_protocol_data:
 * @connection: The PurpleConnection.
 * @proto_data: The protocol data to set for the connection.
 *
 * Sets the protocol data for a connection.
 */
void purple_connection_set_protocol_data(PurpleConnection *connection, void *proto_data);

/**
 * purple_connection_get_state:
 * @gc: The connection.
 *
 * Returns the connection state.
 *
 * Returns: The connection state.
 */
PurpleConnectionState purple_connection_get_state(PurpleConnection *gc);

/**
 * purple_connection_get_flags:
 * @gc: The connection.
 *
 * Returns the connection flags.
 *
 * Returns: The connection flags.
 *
 * Since: 3.0.0
 */
PurpleConnectionFlags purple_connection_get_flags(PurpleConnection *gc);

/**
 * PURPLE_CONNECTION_IS_CONNECTED:
 *
 * Returns TRUE if the account is connected, otherwise returns FALSE.
 *
 * Returns: TRUE if the account is connected, otherwise returns FALSE.
 */
#define PURPLE_CONNECTION_IS_CONNECTED(gc) \
	(purple_connection_get_state(gc) == PURPLE_CONNECTION_STATE_CONNECTED)

/**
 * purple_connection_get_id:
 * @connection: The connection.
 *
 * Gets the identifier of the connection.
 *
 * Returns: The identifier of the connection.
 *
 * Since: 3.0.0
 */
const gchar *purple_connection_get_id(PurpleConnection *connection);

/**
 * purple_connection_get_account:
 * @gc: The connection.
 *
 * Returns the connection's account.
 *
 * Returns: (transfer none): The connection's account.
 */
PurpleAccount *purple_connection_get_account(PurpleConnection *gc);

/**
 * purple_connection_get_protocol:
 * @gc: The connection.
 *
 * Returns the protocol managing a connection.
 *
 * Returns: (transfer none): The protocol.
 */
PurpleProtocol *purple_connection_get_protocol(PurpleConnection *gc);

/**
 * purple_connection_get_password:
 * @gc: The connection.
 *
 * Returns the connection's password.
 *
 * Returns: The connection's password.
 */
const char *purple_connection_get_password(PurpleConnection *gc);

/**
 * purple_connection_set_password:
 * @connection: The instance.
 * @password: (nullable): The new password.
 *
 * Sets the password for @connection to @password.
 *
 * This will not change your password on the remote service. It just updates
 * the password that the protocol should use when connecting.
 *
 * This is generally used by protocol plugins that support multiple
 * authentication methods and need to prompt the user for a password.
 *
 * Since: 3.0.0
 */
void purple_connection_set_password(PurpleConnection *connection, const char *password);

/**
 * purple_connection_get_active_chats:
 * @gc: The connection.
 *
 * Returns a list of active chat conversations on a connection.
 *
 * Returns: (element-type PurpleChatConversation) (transfer none): The active
 *          chats on the connection.
 *
 * Since: 3.0.0
 */
GSList *purple_connection_get_active_chats(PurpleConnection *gc);

/**
 * purple_connection_get_display_name:
 * @gc: The connection.
 *
 * Returns the connection's displayed name.
 *
 * Returns: The connection's displayed name.
 */
const char *purple_connection_get_display_name(PurpleConnection *gc);

/**
 * purple_connection_get_protocol_data:
 * @gc: The PurpleConnection.
 *
 * Gets the protocol data from a connection.
 *
 * Returns: The protocol data for the connection.
 */
void *purple_connection_get_protocol_data(PurpleConnection *gc);

/**
 * purple_connection_error:
 * @gc:          the connection which is closing.
 * @reason:      why the connection is closing.
 * @description: a localized description of the error (not %NULL ).
 *
 * Closes a connection with an error and a human-readable description of the
 * error.
 */
void
purple_connection_error(PurpleConnection *gc,
                        PurpleConnectionError reason,
                        const char *description);

/**
 * purple_connection_get_error_info:
 * @gc: The connection.
 *
 * Returns the #PurpleConnectionErrorInfo instance of a connection if an
 * error exists.
 *
 * Returns: The #PurpleConnectionErrorInfo instance of the connection if an
 *          error exists, %NULL otherwise.
 *
 * Since: 3.0.0
 */
PurpleConnectionErrorInfo *
purple_connection_get_error_info(PurpleConnection *gc);

/**
 * purple_connection_g_error:
 * @pc: Connection the error is associated with
 * @error: Error information
 *
 * Closes a connection similar to [method@Purple.Connection.error], but takes a
 * [type@GLib.Error] which is then converted to purple error codes.
 *
 * This function ignores G_IO_ERROR_CANCELLED, returning without
 * closing the connection. This can be used as a shortcut when
 * cancelling connections, as this is commonly done when shutting
 * down a connection. If G_IO_ERROR_CANCELLED needs to be caught,
 * do so with [method@GLib.Error.matches] prior to calling this function.
 *
 * Since: 3.0.0
 */
void purple_connection_g_error(PurpleConnection *pc, const GError *error);

/**
 * purple_connection_take_error:
 * @pc: Connection the error is associated with
 * @error: Return address for a #GError, or %NULL.
 *
 * Closes a connection similar to [method@Purple.Connection.error], but takes a
 * [type@GLib.Error] which is then converted to purple error codes.
 *
 * This function is equivalent to [method@Purple.Connection.g_error], expect
 * that it takes ownership of the GError.
 *
 * Since: 3.0.0
 */
void purple_connection_take_error(PurpleConnection *pc, GError *error);

/**
 * purple_connection_error_is_fatal:
 * @reason: The connection error to check.
 *
 * Reports whether a disconnection reason is fatal (in which case the account
 * should probably not be automatically reconnected) or transient (so
 * auto-reconnection is a good idea).
 *
 * For instance, #PURPLE_CONNECTION_ERROR_NETWORK_ERROR is a temporary error,
 * which might be caused by losing the network connection, so <code>
 * purple_connection_error_is_fatal(PURPLE_CONNECTION_ERROR_NETWORK_ERROR)
 * </code> is %FALSE.
 *
 * On the other hand, #PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED probably
 * indicates a misconfiguration of the account which needs the user to go fix
 * it up, so <code>
 * purple_connection_error_is_fatal(PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED)
 * </code> is %TRUE.
 *
 * Returns: %TRUE if the account should not be automatically reconnected, and
 *         %FALSE otherwise.
 */
gboolean
purple_connection_error_is_fatal (PurpleConnectionError reason);

/**
 * purple_connection_update_last_received:
 * @gc:   The connection.
 *
 * Indicate that a packet was received on the connection.
 * Set by the protocol to avoid sending unneeded keepalives.
 *
 * Since: 3.0.0
 */
void purple_connection_update_last_received(PurpleConnection *gc);

/**************************************************************************/
/* Connections API                                                        */
/**************************************************************************/

/**
 * purple_connections_disconnect_all:
 *
 * Disconnects from all connections.
 */
void purple_connections_disconnect_all(void);

/**
 * purple_connections_get_all:
 *
 * Returns a list of all active connections.  This does not
 * include connections that are in the process of connecting.
 *
 * Returns: (element-type PurpleConnection) (transfer none): A list of all
 *          active connections.
 */
GList *purple_connections_get_all(void);

/**
 * purple_connections_is_online:
 *
 * Checks if at least one account is online.
 *
 * Returns: %TRUE if at least one account is online.
 */
gboolean purple_connections_is_online(void);

/**************************************************************************/
/* UI Registration Functions                                              */
/**************************************************************************/

/**
 * purple_connection_ui_ops_get_type:
 *
 * Returns: The #GType for the #PurpleConnectionUiOps boxed structure.
 */
GType purple_connection_ui_ops_get_type(void);

/**
 * purple_connections_set_ui_ops:
 * @ops: The UI operations structure.
 *
 * Sets the UI operations structure to be used for connections.
 */
void purple_connections_set_ui_ops(PurpleConnectionUiOps *ops);

/**
 * purple_connections_get_ui_ops:
 *
 * Returns the UI operations structure used for connections.
 *
 * Returns: The UI operations structure in use.
 */
PurpleConnectionUiOps *purple_connections_get_ui_ops(void);

/**************************************************************************/
/* Connections Subsystem                                                  */
/**************************************************************************/

/**
 * purple_connections_init:
 *
 * Initializes the connections subsystem.
 */
void purple_connections_init(void);

/**
 * purple_connections_uninit:
 *
 * Uninitializes the connections subsystem.
 */
void purple_connections_uninit(void);

/**
 * purple_connections_get_handle:
 *
 * Returns the handle to the connections subsystem.
 *
 * Returns: The connections subsystem handle.
 */
void *purple_connections_get_handle(void);

G_END_DECLS

#endif /* PURPLE_CONNECTION_H */
