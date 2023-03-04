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

#ifndef PURPLE_NETWORK_H
#define PURPLE_NETWORK_H

#include <glib.h>
#include <gio/gio.h>

#ifndef _WIN32
# include <netinet/in.h>
# include <sys/socket.h>
#endif

G_BEGIN_DECLS

/**************************************************************************/
/* Network API                                                            */
/**************************************************************************/

/**
 * purple_network_set_public_ip:
 * @ip: The local IP address.
 *
 * Sets the IP address of the local system in preferences.  This
 * is the IP address that should be used for incoming connections
 * (file transfer, direct IM, etc.) and should therefore be
 * publicly accessible.
 */
void purple_network_set_public_ip(const char *ip);

/**
 * purple_network_get_public_ip:
 *
 * Returns the IP address of the local system set in preferences.
 *
 * This returns the value set via purple_network_set_public_ip().
 *
 * Returns: The local IP address set in preferences.
 */
const char *purple_network_get_public_ip(void);

/**
 * purple_network_discover_my_ip:
 *
 * Discovers the IP address that should be used anywhere a public IP addresses
 * is needed (listening for an incoming file transfer, etc).
 *
 * If the user has manually specified an IP address via preferences, then this
 * is used.  Otherwise STUN, UPnP, and NAT-PMP will be attempted to discover
 * the local IP address depending on what's available.
 *
 * Since: 3.0.0
 */
void purple_network_discover_my_ip(void);

/**
 * purple_network_get_my_ip_from_gio:
 * @sockconn: The socket connection to use to help figure out the IP, or %NULL.
 *
 * Returns the IP address that should be used anywhere a public IP address is
 * needed (listening for an incoming file transfer, etc).
 *
 * If the user has manually specified an IP address via preferences, then this
 * IP is returned.  Otherwise STUN, UPnP, NAT-PMP, and finally GIO will be
 * attempted to discover the local IP address depending on what's available.
 *
 * Returns: The local IP address to be used.
 */
gchar *purple_network_get_my_ip_from_gio(GSocketConnection *sockconn);

/**
 * purple_network_is_available:
 *
 * Detects if there is an available network connection.
 *
 * Returns: TRUE if the network is available
 */
gboolean purple_network_is_available(void);

/**
 * purple_network_force_online:
 *
 * Makes purple_network_is_available() always return %TRUE.
 *
 * This is what backs the --force-online command line argument in Pidgin,
 * for example.  This is useful for offline testing, especially when
 * combined with nullprotocol.
 */
void purple_network_force_online(void);

/**
 * purple_network_set_stun_server:
 * @stun_server: The host name of the STUN server to set
 *
 * Update the STUN server IP given the host name
 * Will result in a DNS query being executed asynchronous
 */
void purple_network_set_stun_server(const gchar *stun_server);

/**
 * purple_network_get_stun_ip:
 *
 * Get the IP address of the STUN server as a string representation
 *
 * Returns: the IP address
 */
const gchar *purple_network_get_stun_ip(void);

/**
 * _purple_network_set_common_socket_flags:
 * @fd: The file descriptor for the socket.
 *
 * Sets most commonly used socket flags: O_NONBLOCK and FD_CLOEXEC.
 *
 * Returns: %TRUE if succeeded, %FALSE otherwise.
 */
gboolean _purple_network_set_common_socket_flags(int fd);

/**
 * purple_network_init:
 *
 * Initializes the network subsystem.
 */
void purple_network_init(void);

/**
 * purple_network_uninit:
 *
 * Shuts down the network subsystem.
 */
void purple_network_uninit(void);

G_END_DECLS

#endif /* PURPLE_NETWORK_H */
