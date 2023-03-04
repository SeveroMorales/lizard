/* purple
 *
 * STUN implementation inspired by jstun [http://jstun.javawi.de/]
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

#include "internal.h"

#ifndef _WIN32
#include <net/if.h>
#endif

#include <gio/gio.h>

#include "glibcompat.h"

#include "debug.h"
#include "account.h"
#include "network.h"
#include "proxy.h"
#include "stun.h"
#include "prefs.h"

#define MSGTYPE_BINDINGREQUEST 0x0001
#define MSGTYPE_BINDINGRESPONSE 0x0101

#define ATTRIB_MAPPEDADDRESS 0x0001

struct stun_header {
	guint16 type;
	guint16 len;
	guint32 transid[4];
};

struct stun_attrib {
	guint16 type;
	guint16 len;
};

struct stun_conn {
	GSocket *sock;
	GSocketAddress *addr;
	int retry;
	guint incb;
	guint timeout;
	struct stun_header *packet;
	gsize packetsize;
};

static PurpleStunNatDiscovery nattype = {
	PURPLE_STUN_STATUS_UNDISCOVERED,
	"\0", NULL, 0};

static GSList *callbacks = NULL;

static void close_stun_conn(struct stun_conn *sc) {
	g_clear_object(&sc->sock);
	g_clear_object(&sc->addr);

	if (sc->incb) {
		g_source_remove(sc->incb);
	}

	if (sc->timeout) {
		g_source_remove(sc->timeout);
	}

	g_clear_pointer(&sc->packet, g_free);
	g_free(sc);
}

static void do_callbacks(void) {
	while (callbacks) {
		PurpleStunCallback cb = callbacks->data;
		if (cb)
			cb(&nattype);
		callbacks = g_slist_delete_link(callbacks, callbacks);
	}
}

static gboolean timeoutfunc(gpointer data) {
	struct stun_conn *sc = data;
	GError *error = NULL;

	if(sc->retry >= 2) {
		guint id;

		purple_debug_warning("stun", "request timed out, giving up.\n");

		/* set unknown */
		nattype.status = PURPLE_STUN_STATUS_UNKNOWN;

		nattype.lookup_time = g_get_monotonic_time();

		/* callbacks */
		do_callbacks();

		/* we don't need to remove the timeout (returning FALSE), but
		 * we do need to remove the read callback, which will free the
		 * whole stun_conn. */
		id = sc->incb;
		sc->timeout = 0;
		sc->incb = 0;
		g_source_remove(id);

		return FALSE;
	}
	purple_debug_info("stun", "request timed out, retrying.\n");
	sc->retry++;
	if (g_socket_send_to(sc->sock, sc->addr, (const gchar *)sc->packet,
	                     sc->packetsize, NULL,
	                     &error) != (gssize)sc->packetsize) {
		purple_debug_warning("stun", "sendto failed: %s", error->message);
		g_clear_error(&error);
		return FALSE;
	}
	return TRUE;
}

static gboolean
reply_cb(GSocket *socket, G_GNUC_UNUSED GIOCondition condition, gpointer data)
{
	struct stun_conn *sc = data;
	gchar buffer[65536];
	gchar *it;
	gssize len;
	struct stun_attrib attrib;
	struct stun_header hdr;
	GError *error = NULL;

	len = g_socket_receive(socket, buffer, sizeof(buffer) - 1, NULL, &error);
	if (len <= 0) {
		purple_debug_warning("stun", "unable to read stun response: %s",
		                     error->message);
		g_clear_error(&error);
		sc->incb = 0;
		return FALSE;
	}
	buffer[len] = '\0';

	if ((gsize)len < sizeof(struct stun_header)) {
		purple_debug_warning("stun", "got invalid response\n");
		sc->incb = 0;
		return FALSE;
	}

	memcpy(&hdr, buffer, sizeof(hdr));
	if ((gsize)len != (g_ntohs(hdr.len) + sizeof(struct stun_header))) {
		purple_debug_warning("stun", "got incomplete response\n");
		sc->incb = 0;
		return FALSE;
	}

	/* wrong transaction */
	if(hdr.transid[0] != sc->packet->transid[0]
			|| hdr.transid[1] != sc->packet->transid[1]
			|| hdr.transid[2] != sc->packet->transid[2]
			|| hdr.transid[3] != sc->packet->transid[3]) {
		purple_debug_warning("stun", "got wrong transid\n");
		sc->incb = 0;
		return FALSE;
	}

	if (hdr.type != MSGTYPE_BINDINGRESPONSE) {
		purple_debug_warning("stun", "Expected Binding Response, got %d",
		                     hdr.type);
		sc->incb = 0;
		return FALSE;
	}

	it = buffer + sizeof(struct stun_header);
	while ((buffer + len) > (it + sizeof(struct stun_attrib))) {
		memcpy(&attrib, it, sizeof(attrib));
		it += sizeof(struct stun_attrib);

		if (!((buffer + len) > (it + g_ntohs(attrib.len)))) {
			break;
		}

		if (attrib.type == g_htons(ATTRIB_MAPPEDADDRESS) &&
		    g_ntohs(attrib.len) == 8) {
			GInetAddress *inet_addr = NULL;
			/* Skip the first unused byte,
			 * the family(1 byte), and the port(2 bytes);
			 * then read the 4 byte IPv4 address */
			inet_addr = g_inet_address_new_from_bytes((const guint8 *)it + 4,
			                                          G_SOCKET_FAMILY_IPV4);
			if (inet_addr) {
				gchar *ip = g_inet_address_to_string(inet_addr);
				g_strlcpy(nattype.publicip, ip, sizeof(nattype.publicip));
				g_free(ip);
				g_object_unref(inet_addr);
			}
		}

		it += g_ntohs(attrib.len);
	}
	purple_debug_info("stun", "got public ip %s\n", nattype.publicip);
	nattype.status = PURPLE_STUN_STATUS_DISCOVERED;
	nattype.lookup_time = g_get_monotonic_time();

	do_callbacks();

	/* sc will be freed by the GSource destruction. */
	sc->incb = 0;
	return FALSE;
}

static void
hbn_cb(GObject *sender, GAsyncResult *res, gpointer data)
{
	struct stun_conn *sc = NULL;
	GList *addresses = NULL;
	GSocketAddress *local_addr = NULL;
	GInetAddress *remote_addr = NULL;
	struct stun_header *hdr_data = NULL;
	GSource *read_source = NULL;
	GError *error = NULL;

	addresses =
	        g_resolver_lookup_by_name_finish(G_RESOLVER(sender), res, &error);
	if (error != NULL) {
		nattype.status = PURPLE_STUN_STATUS_UNDISCOVERED;
		nattype.lookup_time = g_get_monotonic_time();

		do_callbacks();

		g_clear_error(&error);
		return;
	}

	sc = g_new0(struct stun_conn, 1);
	sc->sock = g_socket_new(G_SOCKET_FAMILY_IPV4, G_SOCKET_TYPE_DATAGRAM,
	                        G_SOCKET_PROTOCOL_DEFAULT, &error);
	if (sc->sock == NULL) {
		purple_debug_error(
		        "stun", "Unable to create socket to connect to STUN server: %s",
		        error->message);
		nattype.status = PURPLE_STUN_STATUS_UNKNOWN;
		nattype.lookup_time = g_get_monotonic_time();

		do_callbacks();

		close_stun_conn(sc);
		g_resolver_free_addresses(addresses);
		g_clear_error(&error);
		return;
	}

	local_addr = g_inet_socket_address_new_from_string("0.0.0.0", 0);
	remote_addr = G_INET_ADDRESS(addresses->data);
	sc->addr = g_inet_socket_address_new(remote_addr, GPOINTER_TO_INT(data));
	g_resolver_free_addresses(addresses);

	g_socket_set_blocking(sc->sock, FALSE);
	if (!g_socket_bind(sc->sock, local_addr, TRUE, &error)) {
		purple_debug_error(
		        "stun", "Unable to bind socket to connect to STUN server: %s",
		        error->message);
		nattype.status = PURPLE_STUN_STATUS_UNKNOWN;
		nattype.lookup_time = g_get_monotonic_time();

		do_callbacks();

		g_object_unref(local_addr);
		close_stun_conn(sc);
		g_clear_error(&error);
		return;
	}

	g_object_unref(local_addr);

	read_source = g_socket_create_source(sc->sock, G_IO_IN, NULL);
	g_source_set_callback(read_source, G_SOURCE_FUNC(reply_cb), sc,
	                      (GDestroyNotify)close_stun_conn);
	sc->incb = g_source_attach(read_source, NULL);
	g_source_unref(read_source);

	hdr_data = g_new0(struct stun_header, 1);
	hdr_data->type = g_htons(MSGTYPE_BINDINGREQUEST);
	hdr_data->len = 0;
	hdr_data->transid[0] = g_random_int();
	hdr_data->transid[1] = g_ntohl(((int)'g' << 24) + ((int)'a' << 16) +
	                               ((int)'i' << 8) + (int)'m');
	hdr_data->transid[2] = g_random_int();
	hdr_data->transid[3] = g_random_int();
	sc->packet = hdr_data;
	sc->packetsize = sizeof(struct stun_header);

	if (g_socket_send_to(sc->sock, sc->addr, (const gchar *)sc->packet,
	                     sc->packetsize, NULL,
	                     &error) < (gssize)sc->packetsize) {
		purple_debug_warning("stun", "sendto failed: %s", error->message);
		nattype.status = PURPLE_STUN_STATUS_UNKNOWN;
		nattype.lookup_time = g_get_monotonic_time();
		do_callbacks();
		close_stun_conn(sc);
		return;
	}

	sc->timeout = g_timeout_add(500, (GSourceFunc)timeoutfunc, sc);
}

static void
do_test1(GObject *sender, GAsyncResult *res, gpointer data) {
	GList *services = NULL;
	GError *error = NULL;
	GResolver *resolver;
	const char *servername = data;
	int port = 3478;

	services = g_resolver_lookup_service_finish(G_RESOLVER(sender),
			res, &error);
	if(error != NULL) {
		purple_debug_info("stun", "Failed to look up srv record : %s\n", error->message);

		g_error_free(error);
	} else {
		servername = g_srv_target_get_hostname((GSrvTarget *)services->data);
		port = g_srv_target_get_port((GSrvTarget *)services->data);
	}

	purple_debug_info("stun", "connecting to %s:%d\n", servername, port);

	resolver = g_resolver_get_default();
	g_resolver_lookup_by_name_async(resolver,
	                                servername,
	                                NULL,
	                                hbn_cb,
	                                GINT_TO_POINTER(port));
	g_object_unref(resolver);

	g_resolver_free_targets(services);
}

static gboolean call_callback(gpointer data) {
	PurpleStunCallback cb = data;
	cb(&nattype);
	return FALSE;
}

PurpleStunNatDiscovery *purple_stun_discover(PurpleStunCallback cb) {
	const char *servername = purple_prefs_get_string("/purple/network/stun_server");
	GResolver *resolver;

	purple_debug_info("stun", "using server %s\n", servername);

	if(nattype.status == PURPLE_STUN_STATUS_DISCOVERING) {
		if(cb)
			callbacks = g_slist_append(callbacks, cb);
		return &nattype;
	}

	if(nattype.status != PURPLE_STUN_STATUS_UNDISCOVERED) {
		gboolean use_cached_result = TRUE;

		/* Deal with the server name having changed since we did the
		   lookup */
		if (servername && strlen(servername) > 1
				&& !purple_strequal(servername, nattype.servername)) {
			use_cached_result = FALSE;
		}

		/* If we don't have a successful status and it has been 5
		   minutes since we last did a lookup, redo the lookup */
		if (nattype.status != PURPLE_STUN_STATUS_DISCOVERED &&
		    (g_get_monotonic_time() - nattype.lookup_time) >
		            300 * G_USEC_PER_SEC) {
			use_cached_result = FALSE;
		}

		if (use_cached_result) {
			if(cb)
				g_timeout_add(10, call_callback, cb);
			return &nattype;
		}
	}

	if(!servername || (strlen(servername) < 2)) {
		nattype.status = PURPLE_STUN_STATUS_UNKNOWN;
		nattype.lookup_time = g_get_monotonic_time();
		if(cb)
			g_timeout_add(10, call_callback, cb);
		return &nattype;
	}

	nattype.status = PURPLE_STUN_STATUS_DISCOVERING;
	nattype.publicip[0] = '\0';
	g_free(nattype.servername);
	nattype.servername = g_strdup(servername);

	callbacks = g_slist_append(callbacks, cb);

	resolver = g_resolver_get_default();
	g_resolver_lookup_service_async(resolver,
	                                "stun",
	                                "udp",
	                                servername,
	                                NULL,
	                                do_test1,
	                                (gpointer)servername);
	g_object_unref(resolver);

	return &nattype;
}

void
purple_stun_init(void)
{
	purple_prefs_add_string("/purple/network/stun_server", "");
}
