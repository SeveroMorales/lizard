/**
 * purple
 *
 * Copyright (C) 2004, Timothy T Ringenbach <omarvo@hotmail.com>
 * Copyright (C) 2003, Robbert Haarman <purple@inglorion.net>
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

#include <errno.h>

#include <glib/gi18n-lib.h>

#ifndef _WIN32
# include <arpa/inet.h>
#endif

#include <purple.h>
#include "libpurple/glibcompat.h"

#include "irc.h"

struct _IrcXfer {
	PurpleXfer parent;

	/* receive properties */
	gchar *ip;
	guint remote_port;

	/* send properties */
	GSocketService *service;
	GSocketConnection *conn;
	gint inpa;
	guchar *rxqueue;
	guint rxlen;
};

G_DEFINE_DYNAMIC_TYPE(IrcXfer, irc_xfer, PURPLE_TYPE_XFER);

/***************************************************************************
 * Functions related to receiving files via DCC SEND
 ***************************************************************************/

/*
 * This function is called whenever data is received.
 * It sends the acknowledgement (in the form of a total byte count as an
 * unsigned 4 byte integer in network byte order)
 */
static void
irc_dccsend_recv_ack(PurpleXfer *xfer, G_GNUC_UNUSED const guchar *data,
                     G_GNUC_UNUSED size_t size)
{
	guint32 l;
	gssize result;

	if(purple_xfer_get_xfer_type(xfer) != PURPLE_XFER_TYPE_RECEIVE) {
		return;
	}

	l = g_htonl(purple_xfer_get_bytes_sent(xfer));
	result = purple_xfer_write(xfer, (guchar *)&l, sizeof(l));
	if (result != sizeof(l)) {
		purple_debug_error("irc", "unable to send acknowledgement: %s\n", g_strerror(errno));
		/* TODO: We should probably close the connection here or something. */
	}
}

static void irc_dccsend_recv_init(PurpleXfer *xfer) {
	IrcXfer *xd = IRC_XFER(xfer);

	purple_xfer_start(xfer, -1, xd->ip, xd->remote_port);
}

/* This function makes the necessary arrangements for receiving files */
void irc_dccsend_recv(struct irc_conn *irc, const char *from, const char *msg) {
	IrcXfer *xfer;
	gchar **token;
	GString *filename;
	int i = 0;
	guint32 nip;

	token = g_strsplit(msg, " ", 0);
	if (!token[0] || !token[1] || !token[2]) {
		g_strfreev(token);
		return;
	}

	filename = g_string_new("");
	if (token[0][0] == '"') {
		if (!strchr(&(token[0][1]), '"')) {
			g_string_append(filename, &(token[0][1]));
			for (i = 1; token[i]; i++)
				if (!strchr(token[i], '"')) {
					g_string_append_printf(filename, " %s", token[i]);
				} else {
					g_string_append_len(filename, token[i], strlen(token[i]) - 1);
					break;
				}
		} else {
			g_string_append_len(filename, &(token[0][1]), strlen(&(token[0][1])) - 1);
		}
	} else {
		g_string_append(filename, token[0]);
	}

	if (!token[i] || !token[i+1] || !token[i+2]) {
		g_strfreev(token);
		g_string_free(filename, TRUE);
		return;
	}
	i++;

	xfer = g_object_new(
		IRC_TYPE_XFER,
		"account", irc->account,
		"type", PURPLE_XFER_TYPE_RECEIVE,
		"remote-user", from,
		NULL
	);

	purple_xfer_set_filename(PURPLE_XFER(xfer), filename->str);

	xfer->remote_port = atoi(token[i+1]);

	nip = strtoul(token[i], NULL, 10);
	if (nip) {
		GInetAddress *addr = g_inet_address_new_from_bytes(
		        (const guchar *)&nip, G_SOCKET_FAMILY_IPV4);
		xfer->ip = g_inet_address_to_string(addr);
		g_object_unref(addr);
	} else {
		xfer->ip = g_strdup(token[i]);
	}

	purple_debug_info("irc", "Receiving file (%s) from %s", filename->str, xfer->ip);
	purple_xfer_set_size(PURPLE_XFER(xfer), token[i+2] ? atoi(token[i+2]) : 0);

	purple_xfer_request(PURPLE_XFER(xfer));

	g_strfreev(token);
	g_string_free(filename, TRUE);
}

/*******************************************************************
 * Functions related to sending files via DCC SEND
 *******************************************************************/

/* just in case you were wondering, this is why DCC is crappy */
static void
irc_dccsend_send_read(gpointer data, int source,
                      G_GNUC_UNUSED PurpleInputCondition cond)
{
	PurpleXfer *xfer = PURPLE_XFER(data);
	IrcXfer *xd = IRC_XFER(xfer);
	char buffer[64];
	int len;

	len = read(source, buffer, sizeof(buffer));

	if (len < 0 && errno == EAGAIN)
		return;
	else if (len <= 0) {
		/* XXX: Shouldn't this be canceling the transfer? */
		g_source_remove(xd->inpa);
		xd->inpa = 0;
		return;
	}

	xd->rxqueue = g_realloc(xd->rxqueue, len + xd->rxlen);
	memcpy(xd->rxqueue + xd->rxlen, buffer, len);
	xd->rxlen += len;

	while (1) {
		gint32 val;
		size_t acked;

		if (xd->rxlen < 4)
			break;

		memcpy(&val, xd->rxqueue, sizeof(val));
		acked = g_ntohl(val);

		xd->rxlen -= 4;
		if (xd->rxlen) {
			unsigned char *tmp = g_memdup2(xd->rxqueue + 4, xd->rxlen);
			g_free(xd->rxqueue);
			xd->rxqueue = tmp;
		} else {
			g_free(xd->rxqueue);
			xd->rxqueue = NULL;
		}

		if ((goffset)acked >= purple_xfer_get_size(xfer)) {
			g_source_remove(xd->inpa);
			xd->inpa = 0;
			purple_xfer_set_completed(xfer, TRUE);
			purple_xfer_end(xfer);
			return;
		}
	}
}

static gssize irc_dccsend_send_write(PurpleXfer *xfer, const guchar *buffer, size_t size)
{
	gssize s;
	gssize ret;

	s = MIN((gssize)purple_xfer_get_bytes_remaining(xfer), (gssize)size);
	if (!s) {
		return 0;
	}

	ret = PURPLE_XFER_CLASS(irc_xfer_parent_class)->write(xfer, buffer, s);

	if (ret < 0 && errno == EAGAIN) {
		ret = 0;
	}

	return ret;
}

static void
irc_dccsend_send_connected(G_GNUC_UNUSED GSocketService *service,
                           GSocketConnection *connection,
                           GObject *source_object, G_GNUC_UNUSED gpointer data)
{
	PurpleXfer *xfer = PURPLE_XFER(source_object);
	IrcXfer *xd = IRC_XFER(xfer);
	GSocket *sock;
	int fd = -1;

	xd->conn = g_object_ref(connection);
	g_socket_service_stop(xd->service);
	g_clear_object(&xd->service);

	sock = g_socket_connection_get_socket(connection);
	fd = g_socket_get_fd(sock);
	_purple_network_set_common_socket_flags(fd);

	xd->inpa = purple_input_add(fd, PURPLE_INPUT_READ, irc_dccsend_send_read,
	                            xfer);
	/* Start the transfer */
	purple_xfer_start(xfer, fd, NULL, 0);
}

/*
 * This function is called after the user has selected a file to send.
 */
static void
irc_dccsend_send_init(PurpleXfer *xfer)
{
	IrcXfer *xd = IRC_XFER(xfer);
	PurpleConnection *gc =
	        purple_account_get_connection(purple_xfer_get_account(xfer));
	struct irc_conn *irc;
	const char *arg[2];
	char *tmp;
	GInetAddress *addr = NULL;
	const guint8 *bytes = NULL;
	guint32 ip = 0;
	guint16 port;
	GError *error = NULL;

	purple_xfer_set_filename(
	        xfer, g_path_get_basename(purple_xfer_get_local_filename(xfer)));

	/* Create a listening socket */
	xd->service = g_socket_service_new();
	if (xd->service == NULL) {
		purple_notify_error(gc, NULL, _("File Transfer Failed"),
		                    _("Unable to open a listening port."),
		                    purple_request_cpar_from_connection(gc));
		purple_xfer_cancel_local(xfer);
		return;
	}

	/* Monitor the listening socket */
	g_signal_connect(xd->service, "incoming",
	                 G_CALLBACK(irc_dccsend_send_connected), NULL);

	port = purple_socket_listener_add_any_inet_port(
	        G_SOCKET_LISTENER(xd->service), G_OBJECT(xfer), &error);
	if (port != 0) {
		purple_xfer_set_local_port(xfer, port);
	} else {
		purple_notify_error(gc, NULL, _("File Transfer Failed"),
			_("Unable to open a listening port."),
			purple_request_cpar_from_connection(gc));
		g_error_free(error);
		g_clear_object(&xd->service);
		purple_xfer_cancel_local(xfer);
		return;
	}

	irc = purple_connection_get_protocol_data(gc);

	purple_debug_misc("irc", "port is %hu\n", port);

	/* Send the intended recipient the DCC request */
	arg[0] = purple_xfer_get_remote_user(xfer);
	tmp = purple_network_get_my_ip_from_gio(irc->conn);
	addr = g_inet_address_new_from_string(tmp);
	bytes = g_inet_address_to_bytes(addr);
	ip = (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
	g_object_unref(addr);
	g_free(tmp);
	arg[1] = tmp = g_strdup_printf(
	        "\001DCC SEND \"%s\" %u %hu %" G_GOFFSET_FORMAT "\001",
	        purple_xfer_get_filename(xfer), ip, port,
	        purple_xfer_get_size(xfer));

	irc_cmd_privmsg(purple_connection_get_protocol_data(gc), "msg", NULL, arg);
	g_free(tmp);
}

PurpleXfer *
irc_dccsend_new_xfer(G_GNUC_UNUSED PurpleProtocolXfer *prplxfer,
                     PurpleConnection *gc, const char *who)
{
	return g_object_new(
		IRC_TYPE_XFER,
		"account", purple_connection_get_account(gc),
		"type", PURPLE_XFER_TYPE_SEND,
		"remote-user", who,
		NULL
	);
}

/**
 * Purple calls this function when the user selects Send File from the
 * buddy menu
 * It sets up the PurpleXfer struct and tells Purple to go ahead
 */
void irc_dccsend_send_file(PurpleProtocolXfer *prplxfer, PurpleConnection *gc, const char *who, const char *file) {
	PurpleXfer *xfer = irc_dccsend_new_xfer(prplxfer, gc, who);

	/* Perform the request */
	if (file)
		purple_xfer_request_accepted(xfer, file);
	else
		purple_xfer_request(xfer);
}

/******************************************************************************
 * PurpleXfer Implementation
 *****************************************************************************/
static void
irc_dccsend_init(PurpleXfer *xfer) {
	PurpleXferType type = purple_xfer_get_xfer_type(xfer);

	if(type == PURPLE_XFER_TYPE_SEND) {
		irc_dccsend_send_init(xfer);
	} else if(type == PURPLE_XFER_TYPE_RECEIVE) {
		irc_dccsend_recv_init(xfer);
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
irc_xfer_init(G_GNUC_UNUSED IrcXfer *xfer) {
}

static void
irc_xfer_finalize(GObject *obj) {
	IrcXfer *xfer = IRC_XFER(obj);

	/* clean up the receiving proprties */
	g_free(xfer->ip);
	g_free(xfer->rxqueue);

	/* clean up the sending properties */
	if (xfer->service) {
		g_socket_service_stop(xfer->service);
	}
	g_clear_object(&xfer->service);
	if(xfer->inpa > 0) {
		g_source_remove(xfer->inpa);
	}
	g_clear_object(&xfer->conn);

	G_OBJECT_CLASS(irc_xfer_parent_class)->finalize(obj);
}

static void
irc_xfer_class_finalize(G_GNUC_UNUSED IrcXferClass *klass) {
}

static void
irc_xfer_class_init(IrcXferClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	PurpleXferClass *xfer_class = PURPLE_XFER_CLASS(klass);

	obj_class->finalize = irc_xfer_finalize;

	xfer_class->init = irc_dccsend_init;
	xfer_class->ack = irc_dccsend_recv_ack;
	xfer_class->write = irc_dccsend_send_write;
}

void
irc_xfer_register(GTypeModule *module) {
	irc_xfer_register_type(module);
}
