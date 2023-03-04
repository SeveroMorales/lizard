/*
 * purple - Jabber Protocol Plugin
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
#include <purple.h>

#include "jabber.h"
#include "iq.h"
#include "oob.h"

/* Number of bytes to read asynchronously per chunk. */
#define JABBER_OOB_READ_SIZE (1024*1024)

struct _JabberOOBXfer {
	JabberStream *js;
	gchar *iq_id;
	gchar *url;
	GCancellable *cancellable;
	SoupMessage *msg;
};

G_DEFINE_DYNAMIC_TYPE(JabberOOBXfer, jabber_oob_xfer, PURPLE_TYPE_XFER);

static void jabber_oob_xfer_xfer_init(PurpleXfer *xfer)
{
	purple_xfer_start(xfer, -1, NULL, 0);
}

static void jabber_oob_xfer_end(PurpleXfer *xfer)
{
	JabberOOBXfer *jox = JABBER_OOB_XFER(xfer);
	JabberIq *iq;

	iq = jabber_iq_new(jox->js, JABBER_IQ_RESULT);
	purple_xmlnode_set_attrib(iq->node, "to", purple_xfer_get_remote_user(xfer));
	jabber_iq_set_id(iq, jox->iq_id);

	jabber_iq_send(iq);
}

static void
jabber_oob_xfer_got_content_length(SoupMessage *msg, gpointer user_data)
{
	PurpleXfer *xfer = user_data;
	SoupMessageHeaders *headers;
	goffset total;

	headers = soup_message_get_response_headers(msg);
	total = soup_message_headers_get_content_length(headers);

	purple_xfer_set_size(xfer, total);
}

#if SOUP_MAJOR_VERSION >= 3
static void
jabber_oob_xfer_writer(GObject *source, GAsyncResult *result, gpointer data) {
	GInputStream *input = G_INPUT_STREAM(source);
	PurpleXfer *xfer = data;
	JabberOOBXfer *jox = JABBER_OOB_XFER(xfer);
	GBytes *bytes = NULL;
	GError *error = NULL;
	gpointer buffer = NULL;
	gsize size = 0;

	bytes = g_input_stream_read_bytes_finish(input, result, &error);
	if(bytes == NULL || error != NULL) {
		purple_debug_error("jabber", "Error reading OOB data: %s",
		                   error ? error->message : "unknown error");
		purple_xfer_set_status(xfer, PURPLE_XFER_STATUS_CANCEL_REMOTE);
		purple_xfer_end(xfer);
		g_clear_pointer(&bytes, g_bytes_unref);
		g_clear_object(&input);
		jox->msg = NULL;
		return;
	}

	buffer = g_bytes_unref_to_data(bytes, &size);

	/* We've reached the end of the file. */
	if(size == 0) {
		purple_xfer_set_completed(xfer, TRUE);
		purple_xfer_end(xfer);
		g_clear_object(&input);
		jox->msg = NULL;
		return;
	}

	if (!purple_xfer_write_file(xfer, buffer, size)) {
		purple_xfer_set_status(xfer, PURPLE_XFER_STATUS_CANCEL_LOCAL);
		purple_xfer_end(xfer);
		g_clear_object(&input);
		jox->msg = NULL;
		return;
	}

	/* Asynchronously read the next chunk of data. */
	g_input_stream_read_bytes_async(input, JABBER_OOB_READ_SIZE,
	                                G_PRIORITY_DEFAULT, jox->cancellable,
	                                jabber_oob_xfer_writer, xfer);
}

static void
jabber_oob_xfer_send_cb(GObject *source, GAsyncResult *result, gpointer data) {
	PurpleXfer *xfer = data;
	JabberOOBXfer *jox = JABBER_OOB_XFER(xfer);
	GInputStream *input = NULL;
	GError *error = NULL;

	if(!SOUP_STATUS_IS_SUCCESSFUL(soup_message_get_status(jox->msg))) {
		purple_xfer_set_status(xfer, PURPLE_XFER_STATUS_CANCEL_REMOTE);
		purple_xfer_end(xfer);
		jox->msg = NULL;
		return;
	}

	input = soup_session_send_finish(SOUP_SESSION(source), result, &error);
	if(input == NULL || error != NULL) {
		purple_debug_error("jabber", "Error sending OOB request: %s",
		                   error ? error->message : "unknown error");
		purple_xfer_set_status(xfer, PURPLE_XFER_STATUS_CANCEL_REMOTE);
		purple_xfer_end(xfer);
		g_clear_object(&input);
		jox->msg = NULL;
		return;
	}

	/* Asynchronously read an initial chunk of data. */
	g_input_stream_read_bytes_async(input, JABBER_OOB_READ_SIZE,
	                                G_PRIORITY_DEFAULT, jox->cancellable,
	                                jabber_oob_xfer_writer, xfer);
}

#else /* SOUP_MAJOR_VERSION < 3 */

static void
jabber_oob_xfer_got(G_GNUC_UNUSED SoupSession *session, SoupMessage *msg,
                    gpointer user_data)
{
	PurpleXfer *xfer = user_data;
	JabberOOBXfer *jox;

	if (purple_xfer_is_cancelled(xfer))
		return;

	jox = JABBER_OOB_XFER(xfer);
	jox->msg = NULL;

	if (!SOUP_STATUS_IS_SUCCESSFUL(soup_message_get_status(msg)) ||
	    purple_xfer_get_bytes_remaining(xfer) > 0) {
		purple_xfer_set_status(xfer, PURPLE_XFER_STATUS_CANCEL_REMOTE);
		purple_xfer_end(xfer);
	} else {
		purple_xfer_set_completed(xfer, TRUE);
		purple_xfer_end(xfer);
	}
}

static void
jabber_oob_xfer_writer(SoupMessage *msg, SoupBuffer *chunk, gpointer user_data)
{
	PurpleXfer *xfer = user_data;

	if (!purple_xfer_write_file(xfer, (const guchar *)chunk->data,
	                            chunk->length)) {
		JabberOOBXfer *jox = JABBER_OOB_XFER(xfer);
		soup_session_cancel_message(jox->js->http_conns, msg,
		                            SOUP_STATUS_IO_ERROR);
	}
}
#endif

static void jabber_oob_xfer_start(PurpleXfer *xfer)
{
	SoupMessage *msg;
	JabberOOBXfer *jox = JABBER_OOB_XFER(xfer);

	msg = soup_message_new("GET", jox->url);
	soup_message_add_header_handler(
	        msg, "got-headers", "Content-Length",
	        G_CALLBACK(jabber_oob_xfer_got_content_length), xfer);
#if SOUP_MAJOR_VERSION >= 3
	soup_session_send_async(jox->js->http_conns, msg, G_PRIORITY_DEFAULT,
	                        jox->cancellable, jabber_oob_xfer_send_cb, xfer);
#else
	soup_message_body_set_accumulate(msg->response_body, FALSE);
	g_signal_connect(msg, "got-chunk", G_CALLBACK(jabber_oob_xfer_writer),
	                 xfer);
	soup_session_queue_message(jox->js->http_conns, msg, jabber_oob_xfer_got,
	                           xfer);
#endif
}

static void jabber_oob_xfer_recv_error(PurpleXfer *xfer, const char *code) {
	JabberOOBXfer *jox = JABBER_OOB_XFER(xfer);
	JabberIq *iq;
	PurpleXmlNode *y, *z;

	iq = jabber_iq_new(jox->js, JABBER_IQ_ERROR);
	purple_xmlnode_set_attrib(iq->node, "to", purple_xfer_get_remote_user(xfer));
	jabber_iq_set_id(iq, jox->iq_id);
	y = purple_xmlnode_new_child(iq->node, "error");
	purple_xmlnode_set_attrib(y, "code", code);
	if(purple_strequal(code, "406")) {
		z = purple_xmlnode_new_child(y, "not-acceptable");
		purple_xmlnode_set_attrib(y, "type", "modify");
		purple_xmlnode_set_namespace(z, NS_XMPP_STANZAS);
	} else if(purple_strequal(code, "404")) {
		z = purple_xmlnode_new_child(y, "not-found");
		purple_xmlnode_set_attrib(y, "type", "cancel");
		purple_xmlnode_set_namespace(z, NS_XMPP_STANZAS);
	}
	jabber_iq_send(iq);
}

static void jabber_oob_xfer_recv_denied(PurpleXfer *xfer) {
	jabber_oob_xfer_recv_error(xfer, "406");
}

static void jabber_oob_xfer_recv_cancelled(PurpleXfer *xfer) {
	JabberOOBXfer *jox = JABBER_OOB_XFER(xfer);

#if SOUP_MAJOR_VERSION >= 3
	g_cancellable_cancel(jox->cancellable);
#else
	soup_session_cancel_message(jox->js->http_conns, jox->msg,
	                            SOUP_STATUS_CANCELLED);
#endif

	jabber_oob_xfer_recv_error(xfer, "404");
}

void jabber_oob_parse(JabberStream *js, const char *from, JabberIqType type,
	const char *id, PurpleXmlNode *querynode) {
	JabberOOBXfer *jox;
	const gchar *filename, *slash;
	gchar *url;
	PurpleXmlNode *urlnode;

	if(type != JABBER_IQ_SET)
		return;

	if(!from)
		return;

	if(!(urlnode = purple_xmlnode_get_child(querynode, "url")))
		return;

	url = purple_xmlnode_get_data(urlnode);
	if (!url)
		return;

	jox = g_object_new(
		JABBER_TYPE_OOB_XFER,
		"account", purple_connection_get_account(js->gc),
		"type", PURPLE_XFER_TYPE_RECEIVE,
		"remote-user", from,
		NULL
	);

	jox->iq_id = g_strdup(id);
	jox->js = js;
	jox->url = url;

	slash = strrchr(url, '/');
	if (slash == NULL) {
		filename = url;
	} else {
		filename = slash + 1;
	}

	purple_xfer_set_filename(PURPLE_XFER(jox), filename);

	js->oob_file_transfers = g_list_append(js->oob_file_transfers, jox);

	purple_xfer_request(PURPLE_XFER(jox));
}

static void
jabber_oob_xfer_init(JabberOOBXfer *xfer) {
#if SOUP_MAJOR_VERSION >= 3
	xfer->cancellable = g_cancellable_new();
#endif
}

static void
jabber_oob_xfer_finalize(GObject *obj) {
	JabberOOBXfer *jox = JABBER_OOB_XFER(obj);

	jox->js->oob_file_transfers = g_list_remove(jox->js->oob_file_transfers,
			jox);

	g_free(jox->iq_id);
	g_free(jox->url);
	if(jox->cancellable != NULL) {
		g_cancellable_cancel(jox->cancellable);
	}
	g_clear_object(&jox->cancellable);

	G_OBJECT_CLASS(jabber_oob_xfer_parent_class)->finalize(obj);
}

static void
jabber_oob_xfer_class_finalize(G_GNUC_UNUSED JabberOOBXferClass *klass) {
}

static void
jabber_oob_xfer_class_init(JabberOOBXferClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	PurpleXferClass *xfer_class = PURPLE_XFER_CLASS(klass);

	obj_class->finalize = jabber_oob_xfer_finalize;

	xfer_class->init = jabber_oob_xfer_xfer_init;
	xfer_class->end = jabber_oob_xfer_end;
	xfer_class->request_denied = jabber_oob_xfer_recv_denied;
	xfer_class->cancel_recv = jabber_oob_xfer_recv_cancelled;
	xfer_class->start = jabber_oob_xfer_start;
}

void
jabber_oob_xfer_register(GTypeModule *module) {
	jabber_oob_xfer_register_type(module);
}
