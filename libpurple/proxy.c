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
 *
 */

#include <glib/gi18n-lib.h>

#include "debug.h"
#include "notify.h"
#include "prefs.h"
#include "proxy.h"
#include "purplegio.h"
#include "util.h"

#include <gio/gio.h>
#include <libsoup/soup.h>

static PurpleProxyInfo *global_proxy_info = NULL;

/**************************************************************************
 * Global Proxy API
 **************************************************************************/
PurpleProxyInfo *
purple_global_proxy_get_info(void)
{
	return global_proxy_info;
}

void
purple_global_proxy_set_info(PurpleProxyInfo *info)
{
	g_return_if_fail(info != NULL);

	g_clear_object(&global_proxy_info);

	global_proxy_info = info;
}

/**************************************************************************
 * Proxy API
 **************************************************************************/

PurpleProxyInfo *
purple_proxy_get_setup(PurpleAccount *account)
{
	PurpleProxyInfo *gpi = NULL;
	const gchar *tmp;

	/* This is used as a fallback so we don't overwrite the selected proxy type */
	static PurpleProxyInfo *tmp_none_proxy_info = NULL;
	if (!tmp_none_proxy_info) {
		tmp_none_proxy_info = purple_proxy_info_new();
		purple_proxy_info_set_proxy_type(tmp_none_proxy_info, PURPLE_PROXY_TYPE_NONE);
	}

	if (account && purple_account_get_proxy_info(account) != NULL) {
		gpi = purple_account_get_proxy_info(account);
		if (purple_proxy_info_get_proxy_type(gpi) == PURPLE_PROXY_TYPE_USE_GLOBAL)
			gpi = NULL;
	}
	if (gpi == NULL) {
		gpi = purple_global_proxy_get_info();
	}

	if (purple_proxy_info_get_proxy_type(gpi) == PURPLE_PROXY_TYPE_USE_ENVVAR) {
		if ((tmp = g_getenv("HTTP_PROXY")) != NULL ||
			(tmp = g_getenv("http_proxy")) != NULL ||
			(tmp = g_getenv("HTTPPROXY")) != NULL)
		{
			gchar *scheme, *host, *username, *password;
			gint port;
			GError *error = NULL;

			/* http_proxy-format:
			 * export http_proxy="http://user:passwd@your.proxy.server:port/"
			 */
			if (!g_uri_split_with_user(tmp, G_URI_FLAGS_HAS_PASSWORD, &scheme,
			                           &username, &password, NULL,
			                           &host, &port, NULL, NULL, NULL, &error))
			{
				purple_debug_warning("proxy", "Couldn't parse URL: %s: %s", tmp, error->message);
				g_error_free(error);
				return gpi;
			}
			if (!purple_strequal(scheme, "http")) {
				purple_debug_warning("proxy", "Couldn't parse URL: %s", tmp);
				g_free(username);
				g_free(password);
				g_free(host);
				return gpi;
			}

			purple_proxy_info_set_hostname(gpi, host);
			purple_proxy_info_set_port(gpi, port);
			purple_proxy_info_set_username(gpi, username);
			purple_proxy_info_set_password(gpi, password);

			g_free(host);
			g_free(username);
			g_free(password);

			/* XXX: Do we want to skip this step if user/password/port were part of url? */
			if ((tmp = g_getenv("HTTP_PROXY_USER")) != NULL ||
				(tmp = g_getenv("http_proxy_user")) != NULL ||
				(tmp = g_getenv("HTTPPROXYUSER")) != NULL)
				purple_proxy_info_set_username(gpi, tmp);

			if ((tmp = g_getenv("HTTP_PROXY_PASS")) != NULL ||
				(tmp = g_getenv("http_proxy_pass")) != NULL ||
				(tmp = g_getenv("HTTPPROXYPASS")) != NULL)
				purple_proxy_info_set_password(gpi, tmp);

			if ((tmp = g_getenv("HTTP_PROXY_PORT")) != NULL ||
				(tmp = g_getenv("http_proxy_port")) != NULL ||
				(tmp = g_getenv("HTTPPROXYPORT")) != NULL)
				purple_proxy_info_set_port(gpi, atoi(tmp));
		} else {
			/* no proxy environment variable found, don't use a proxy */
			purple_debug_info("proxy", "No environment settings found, not using a proxy\n");
			gpi = tmp_none_proxy_info;
		}

	}

	return gpi;
}

GProxyResolver *
purple_proxy_get_proxy_resolver(PurpleAccount *account, GError **error)
{
	PurpleProxyInfo *info = purple_proxy_get_setup(account);
	const gchar *protocol;
	const gchar *username;
	const gchar *password;
	gchar *auth;
	gchar *proxy;
	GProxyResolver *resolver;

	if (purple_proxy_info_get_proxy_type(info) == PURPLE_PROXY_TYPE_NONE) {
		/* Return an empty simple resolver, which will resolve on direct
		 * connection. */
		return g_simple_proxy_resolver_new(NULL, NULL);
	}

	switch (purple_proxy_info_get_proxy_type(info))
	{
		/* PURPLE_PROXY_NONE already handled above */

		case PURPLE_PROXY_TYPE_USE_ENVVAR:
			/* Intentional passthrough */
		case PURPLE_PROXY_TYPE_HTTP:
			protocol = "http";
			break;
		case PURPLE_PROXY_TYPE_SOCKS4:
			protocol = "socks4";
			break;
		case PURPLE_PROXY_TYPE_SOCKS5:
			/* Intentional passthrough */
		case PURPLE_PROXY_TYPE_TOR:
			protocol = "socks5";
			break;

		default:
			g_set_error(error, PURPLE_CONNECTION_ERROR,
					PURPLE_CONNECTION_ERROR_INVALID_SETTINGS,
					_("Invalid Proxy type (%d) specified"),
					purple_proxy_info_get_proxy_type(info));
			return NULL;
	}


	if (purple_proxy_info_get_hostname(info) == NULL ||
			purple_proxy_info_get_port(info) <= 0) {
		g_set_error_literal(error, PURPLE_CONNECTION_ERROR,
				PURPLE_CONNECTION_ERROR_INVALID_SETTINGS,
				_("Either the host name or port number "
				"specified for your given proxy type is "
				"invalid."));
		return NULL;
	}

	/* Everything checks out. Create and return the GProxyResolver */

	username = purple_proxy_info_get_username(info);
	password = purple_proxy_info_get_password(info);

	/* Username and password are optional */
	if(username != NULL && *username != '\0') {
		if(password != NULL && *password != '\0') {
			auth = g_strdup_printf("%s:%s@", username, password);
		} else {
			auth = g_strdup_printf("%s@", username);
		}
	} else {
		auth = NULL;
	}

	proxy = g_strdup_printf("%s://%s%s:%i", protocol,
			auth != NULL ? auth : "",
			purple_proxy_info_get_hostname(info),
			purple_proxy_info_get_port(info));
	g_free(auth);

	resolver = g_simple_proxy_resolver_new(proxy, NULL);
	g_free(proxy);

	return resolver;
}

static void
proxy_pref_cb(const char *name, G_GNUC_UNUSED PurplePrefType type,
              gconstpointer value, G_GNUC_UNUSED gpointer data)
{
	PurpleProxyInfo *info = purple_global_proxy_get_info();

	if (purple_strequal(name, "/purple/proxy/type")) {
		int proxytype;
		const char *type = value;

		if (purple_strequal(type, "none"))
			proxytype = PURPLE_PROXY_TYPE_NONE;
		else if (purple_strequal(type, "http"))
			proxytype = PURPLE_PROXY_TYPE_HTTP;
		else if (purple_strequal(type, "socks4"))
			proxytype = PURPLE_PROXY_TYPE_SOCKS4;
		else if (purple_strequal(type, "socks5"))
			proxytype = PURPLE_PROXY_TYPE_SOCKS5;
		else if (purple_strequal(type, "tor"))
			proxytype = PURPLE_PROXY_TYPE_TOR;
		else if (purple_strequal(type, "envvar"))
			proxytype = PURPLE_PROXY_TYPE_USE_ENVVAR;
		else
			proxytype = -1;

		purple_proxy_info_set_proxy_type(info, proxytype);
	} else if (purple_strequal(name, "/purple/proxy/host"))
		purple_proxy_info_set_hostname(info, value);
	else if (purple_strequal(name, "/purple/proxy/port"))
		purple_proxy_info_set_port(info, GPOINTER_TO_INT(value));
	else if (purple_strequal(name, "/purple/proxy/username"))
		purple_proxy_info_set_username(info, value);
	else if (purple_strequal(name, "/purple/proxy/password"))
		purple_proxy_info_set_password(info, value);
}

static void *
purple_proxy_get_handle(void)
{
	static int handle;

	return &handle;
}

void
purple_proxy_init(void)
{
	void *handle;

	/* Initialize a default proxy info struct. */
	global_proxy_info = purple_proxy_info_new();

	/* Proxy */
	purple_prefs_add_none("/purple/proxy");
	purple_prefs_add_string("/purple/proxy/type", "none");
	purple_prefs_add_string("/purple/proxy/host", "");
	purple_prefs_add_int("/purple/proxy/port", 0);
	purple_prefs_add_string("/purple/proxy/username", "");
	purple_prefs_add_string("/purple/proxy/password", "");

	/* Setup callbacks for the preferences. */
	handle = purple_proxy_get_handle();
	purple_prefs_connect_callback(handle, "/purple/proxy/type", proxy_pref_cb,
		NULL);
	purple_prefs_connect_callback(handle, "/purple/proxy/host", proxy_pref_cb,
		NULL);
	purple_prefs_connect_callback(handle, "/purple/proxy/port", proxy_pref_cb,
		NULL);
	purple_prefs_connect_callback(handle, "/purple/proxy/username",
		proxy_pref_cb, NULL);
	purple_prefs_connect_callback(handle, "/purple/proxy/password",
		proxy_pref_cb, NULL);

	/* Load the initial proxy settings */
	purple_prefs_trigger_callback("/purple/proxy/type");
	purple_prefs_trigger_callback("/purple/proxy/host");
	purple_prefs_trigger_callback("/purple/proxy/port");
	purple_prefs_trigger_callback("/purple/proxy/username");
	purple_prefs_trigger_callback("/purple/proxy/password");
}

void
purple_proxy_uninit(void)
{
	purple_prefs_disconnect_by_handle(purple_proxy_get_handle());

	g_clear_object(&global_proxy_info);
}
