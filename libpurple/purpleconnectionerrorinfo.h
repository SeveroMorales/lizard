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

#ifndef PURPLE_CONNECTION_ERROR_INFO_H
#define PURPLE_CONNECTION_ERROR_INFO_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

/**
 * PurpleConnectionError:
 * @PURPLE_CONNECTION_ERROR_NETWORK_ERROR: There was an error sending or
 *         receiving on the network socket, or there was some protocol error
 *         (such as the server sending malformed data).
 * @PURPLE_CONNECTION_ERROR_INVALID_USERNAME: The username supplied was not
 *         valid.
 * @PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED: The username, password or
 *         some other credential was incorrect.  Use
 *         #PURPLE_CONNECTION_ERROR_INVALID_USERNAME instead if the username
 *         is known to be invalid.
 * @PURPLE_CONNECTION_ERROR_AUTHENTICATION_IMPOSSIBLE: libpurple doesn't speak
 *         any of the authentication methods the server offered.
 * @PURPLE_CONNECTION_ERROR_NO_SSL_SUPPORT: libpurple was built without SSL
 *         support, and the connection needs SSL.
 * @PURPLE_CONNECTION_ERROR_ENCRYPTION_ERROR: There was an error negotiating
 *         SSL on this connection, or the server does not support encryption
 *         but an account option was set to require it.
 * @PURPLE_CONNECTION_ERROR_NAME_IN_USE: Someone is already connected to the
 *         server using the name you are trying to connect with.
 * @PURPLE_CONNECTION_ERROR_INVALID_SETTINGS: The username/server/other
 *         preference for the account isn't valid.  For instance, on IRC the
 *         username cannot contain white space.  This reason should not be used
 *         for incorrect passwords etc: use
 *         #PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED for that.
 * @PURPLE_CONNECTION_ERROR_CERT_NOT_PROVIDED: The server did not provide a
 *         SSL certificate.
 * @PURPLE_CONNECTION_ERROR_CERT_UNTRUSTED: The server's SSL certificate could
 *         not be trusted.
 * @PURPLE_CONNECTION_ERROR_CERT_EXPIRED: The server's SSL certificate has
 *         expired.
 * @PURPLE_CONNECTION_ERROR_CERT_NOT_ACTIVATED: The server's SSL certificate is
 *         not yet valid.
 * @PURPLE_CONNECTION_ERROR_CERT_HOSTNAME_MISMATCH: The server's SSL
 *         certificate did not match its hostname.
 * @PURPLE_CONNECTION_ERROR_CERT_FINGERPRINT_MISMATCH: The server's SSL
 *         certificate does not have the expected fingerprint.
 * @PURPLE_CONNECTION_ERROR_CERT_SELF_SIGNED: The server's SSL certificate is
 *         self-signed.
 * @PURPLE_CONNECTION_ERROR_CERT_OTHER_ERROR: There was some other error
 *         validating the server's SSL certificate.
 * @PURPLE_CONNECTION_ERROR_OTHER_ERROR: Some other error occurred which fits
 *         into none of the other categories.
 *
 * Possible errors that can cause a connection to be closed.
 */
typedef enum
{
	PURPLE_CONNECTION_ERROR_NETWORK_ERROR = 0,
	PURPLE_CONNECTION_ERROR_INVALID_USERNAME = 1,
	PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED = 2,
	PURPLE_CONNECTION_ERROR_AUTHENTICATION_IMPOSSIBLE = 3,
	PURPLE_CONNECTION_ERROR_NO_SSL_SUPPORT = 4,
	PURPLE_CONNECTION_ERROR_ENCRYPTION_ERROR = 5,
	PURPLE_CONNECTION_ERROR_NAME_IN_USE = 6,

	/* TODO This reason really shouldn't be necessary. Usernames and
	 *      other account preferences should be validated when the
	 *      account is created. */
	PURPLE_CONNECTION_ERROR_INVALID_SETTINGS = 7,

	PURPLE_CONNECTION_ERROR_CERT_NOT_PROVIDED = 8,
	PURPLE_CONNECTION_ERROR_CERT_UNTRUSTED = 9,
	PURPLE_CONNECTION_ERROR_CERT_EXPIRED = 10,
	PURPLE_CONNECTION_ERROR_CERT_NOT_ACTIVATED = 11,
	PURPLE_CONNECTION_ERROR_CERT_HOSTNAME_MISMATCH = 12,
	PURPLE_CONNECTION_ERROR_CERT_FINGERPRINT_MISMATCH = 13,
	PURPLE_CONNECTION_ERROR_CERT_SELF_SIGNED = 14,
	PURPLE_CONNECTION_ERROR_CERT_OTHER_ERROR = 15,

	PURPLE_CONNECTION_ERROR_CUSTOM_TEMPORARY = 16,
	PURPLE_CONNECTION_ERROR_CUSTOM_FATAL = 17,

	/* purple_connection_error() in connection.c uses the fact that
	 * this is the last member of the enum when sanity-checking; if other
	 * reasons are added after it, the check must be updated.
	 */
	PURPLE_CONNECTION_ERROR_OTHER_ERROR = 18
} PurpleConnectionError;

/**
 * PurpleConnectionErrorInfo:
 * @type: The type of error.
 * @description: A localised, human-readable description of the error.
 *
 * Holds the type of an error along with its description.
 */
typedef struct
{
	PurpleConnectionError type;
	char *description;
} PurpleConnectionErrorInfo;


#define PURPLE_TYPE_CONNECTION_ERROR_INFO (purple_connection_error_info_get_type())
GType purple_connection_error_info_get_type(void);

/**
 * purple_connection_error_info_new:
 * @type: The [enum@Purple.ConnectionError] of the error.
 * @description: The description of the error.
 *
 * Creates a new error info with the given properties.
 *
 * Returns: (transfer full): The new instance.
 *
 * Since: 3.0.0
 */
PurpleConnectionErrorInfo *purple_connection_error_info_new(PurpleConnectionError type, const gchar *description);

/**
 * purple_connection_error_info_copy:
 * @info: The instance to copy.
 *
 * Creates a copy of @info.
 *
 * Returns: (transfer full): A new copy of @info.
 *
 * Since: 3.0.0
 */
PurpleConnectionErrorInfo *purple_connection_error_info_copy(PurpleConnectionErrorInfo *info);

/**
 * purple_connection_error_info_free:
 * @info: The instance to free.
 *
 * Frees the memory associated with @info.
 *
 * Since: 3.0.0
 */
void purple_connection_error_info_free(PurpleConnectionErrorInfo *info);

G_END_DECLS

#endif /* PURPLE_CONNECTION_ERROR_INFO_H */
