/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#ifndef PURPLE_PROXY_INFO_H
#define PURPLE_PROXY_INFO_H

#include <glib.h>
#include <glib-object.h>

/**
 * PurpleProxyType:
 * @PURPLE_PROXY_TYPE_USE_GLOBAL: Use the global proxy information.
 * @PURPLE_PROXY_TYPE_NONE:  No proxy.
 * @PURPLE_PROXY_TYPE_HTTP: HTTP proxy.
 * @PURPLE_PROXY_TYPE_SOCKS4: SOCKS 4 proxy.
 * @PURPLE_PROXY_TYPE_SOCKS5: SOCKS 5 proxy.
 * @PURPLE_PROXY_TYPE_USE_ENVVAR: Use environmental settings.
 * @PURPLE_PROXY_TYPE_TOR: Use a Tor proxy (SOCKS 5 really).
 *
 * A type of proxy connection.
 */
typedef enum {
    PURPLE_PROXY_TYPE_USE_GLOBAL = -1,
    PURPLE_PROXY_TYPE_NONE = 0,
    PURPLE_PROXY_TYPE_HTTP,
    PURPLE_PROXY_TYPE_SOCKS4,
    PURPLE_PROXY_TYPE_SOCKS5,
    PURPLE_PROXY_TYPE_USE_ENVVAR,
    PURPLE_PROXY_TYPE_TOR
} PurpleProxyType;

G_BEGIN_DECLS

#define PURPLE_TYPE_PROXY_INFO (purple_proxy_info_get_type())
G_DECLARE_FINAL_TYPE(PurpleProxyInfo, purple_proxy_info, PURPLE, PROXY_INFO,
                     GObject)

/**
 * purple_proxy_info_new:
 *
 * Creates a new proxy info instance.
 *
 * Returns: (transfer full): The new instance.
 */
PurpleProxyInfo *purple_proxy_info_new(void);

/**
 * purple_proxy_info_set_proxy_type:
 * @info: The instance.
 * @proxy_type: The new type.
 *
 * Sets the type of @info.
 */
void purple_proxy_info_set_proxy_type(PurpleProxyInfo *info, PurpleProxyType proxy_type);

/**
 * purple_proxy_info_get_proxy_type:
 * @info: The instance.
 *
 * Gets the type of the proxy.
 *
 * Returns: The type of the proxy.
 */
PurpleProxyType purple_proxy_info_get_proxy_type(PurpleProxyInfo *info);

/**
 * purple_proxy_info_set_hostname:
 * @info: The instance.
 * @hostname: The new hostname.
 *
 * Sets the hostname for the proxy.
 *
 * Since: 3.0.0
 */
void purple_proxy_info_set_hostname(PurpleProxyInfo *info, const gchar *hostname);

/**
 * purple_proxy_info_get_hostname:
 * @info: The instance.
 *
 * Gets the hostname of the proxy.
 *
 * Returns: The hostname.
 *
 * Since: 3.0.0
 */
const gchar *purple_proxy_info_get_hostname(PurpleProxyInfo *info);

/**
 * purple_proxy_info_set_port:
 * @info: The instance.
 * @port: The new port.
 *
 * Sets the port for the proxy.
 */
void purple_proxy_info_set_port(PurpleProxyInfo *info, gint port);

/**
 * purple_proxy_info_get_port:
 * @info: The instance.
 *
 * Gets the port of the proxy.
 *
 * Returns: The port of the proxy.
 */
gint purple_proxy_info_get_port(PurpleProxyInfo *info);

/**
 * purple_proxy_info_set_username:
 * @info: The instance.
 * @username: The new username.
 *
 * Sets the username of the proxy.
 */
void purple_proxy_info_set_username(PurpleProxyInfo *info, const gchar *username);

/**
 * purple_proxy_info_get_username:
 * @info: The instance.
 *
 * Gets the username of the proxy.
 *
 * Returns: The username of the proxy.
 */
const gchar *purple_proxy_info_get_username(PurpleProxyInfo *info);

/**
 * purple_proxy_info_set_password:
 * @info: The instance.
 * @password: The new password.
 *
 * Sets the password for the proxy.
 */
void purple_proxy_info_set_password(PurpleProxyInfo *info, const gchar *password);

/**
 * purple_proxy_info_get_password:
 * @info: The instance.
 *
 * Gets the password of the proxy.
 *
 * Returns: The password of the proxy.
 */
const gchar *purple_proxy_info_get_password(PurpleProxyInfo *info);

G_END_DECLS

#endif /* PURPLE_PROXY_INFO_H */
