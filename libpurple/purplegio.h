/*
 *
 * purple
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

#ifndef PURPLE_GIO_H
#define PURPLE_GIO_H

/**
 * purple_gio:
 *
 * The Purple Gio API provides helper functions for Gio operations which
 * are commonly used within libpurple and its consumers. These contain
 * such functions as setting up connections and shutting them down
 * gracefully.
 */

#include "account.h"

#include <gio/gio.h>

G_BEGIN_DECLS

/**
 * purple_gio_graceful_close:
 * @stream: A #GIOStream to close
 * @input: (nullable): A #GInputStream which wraps @stream's input stream
 * @output: (nullable): A #GOutputStream which wraps @stream's output stream
 *
 * Closes @input, @output, @stream. If there are pending operations, it
 * asynchronously waits for the operations to finish before closing the
 * arguments. Ensure the Gio callbacks can safely handle this being done
 * asynchronously.
 */
void
purple_gio_graceful_close(GIOStream *stream,
		GInputStream *input, GOutputStream *output);

/**
 * purple_gio_socket_client_new:
 * @account: The #PurpleAccount to use for this connection
 * @error: Return location for a GError, or NULL
 *
 * A helper function to simplify creating a #GSocketClient. It's intended
 * to be used in protocol plugins.
 *
 * Returns: (transfer full): A new #GSocketClient with the appropriate
 * GProxyResolver, based on the #PurpleAccount settings and
 * TLS Certificate handling, or NULL if an error occurred.
 */
GSocketClient *
purple_gio_socket_client_new(PurpleAccount *account, GError **error);

/**
 * purple_socket_listener_add_any_inet_port:
 * @listener: A #GSocketListener.
 * @source_object: (nullable): Optional GObject identifying this source.
 * @error: A #GError location to store the error occurring, or %NULL to ignore.
 *
 * Listens for TCP connections on any available port number for both IPv6 and
 * IPv4 (if each is available). This is a simple wrapper around
 * g_socket_listener_add_any_inet_port(), except if the user specified a port
 * range in the settings, than a port will be chosen from that range.
 *
 * Returns: The port number, or 0 in case of failure.
 */
guint16 purple_socket_listener_add_any_inet_port(GSocketListener *listener,
                                                 GObject *source_object,
                                                 GError **error);

G_END_DECLS

#endif /* PURPLE_GIO_H */
