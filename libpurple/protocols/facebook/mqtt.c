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

#include <glib/gi18n-lib.h>

#include <glib/gprintf.h>
#include <stdarg.h>
#include <string.h>

#include <purple.h>

#include "mqtt.h"
#include "util.h"

/**
 * FbMqtt:
 *
 * Represents an MQTT connection.
 */
struct _FbMqtt
{
	GObject parent;

	PurpleConnection *gc;
	GIOStream *conn;
	GBufferedInputStream *input;
	PurpleQueuedOutputStream *output;
	GCancellable *cancellable;
	gboolean connected;
	guint16 mid;

	GByteArray *rbuf;
	gsize remz;

	gint tev;
};

G_DEFINE_TYPE(FbMqtt, fb_mqtt, G_TYPE_OBJECT);

/**
 * FbMqttMessage:
 *
 * Represents a reader/writer for an MQTT message.
 */
struct _FbMqttMessage
{
	GObject parent;

	FbMqttMessageType type;
	FbMqttMessageFlags flags;

	GByteArray *bytes;
	guint offset;
	guint pos;

	gboolean local;
};

G_DEFINE_TYPE(FbMqttMessage, fb_mqtt_message, G_TYPE_OBJECT);

static void fb_mqtt_read_packet(FbMqtt *mqtt);

static void
fb_mqtt_dispose(GObject *obj)
{
	FbMqtt *mqtt = FB_MQTT(obj);

	fb_mqtt_close(mqtt);
	g_byte_array_free(mqtt->rbuf, TRUE);
}

static void
fb_mqtt_class_init(FbMqttClass *klass)
{
	GObjectClass *gklass = G_OBJECT_CLASS(klass);

	gklass->dispose = fb_mqtt_dispose;
	/**
	 * FbMqtt::connect:
	 * @mqtt: The #FbMqtt.
	 *
	 * Emitted upon the successful completion of the connection
	 * process. This is emitted as a result of #fb_mqtt_connect().
	 */
	g_signal_new("connect",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             0);

	/**
	 * FbMqtt::error:
	 * @mqtt: The #FbMqtt.
	 * @error: The #GError.
	 *
	 * Emitted whenever an error is hit within the #FbMqtt. This
	 * should close the #FbMqtt with #fb_mqtt_close().
	 */
	g_signal_new("error",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             1, G_TYPE_ERROR);

	/**
	 * FbMqtt::open:
	 * @mqtt: The #FbMqtt.
	 *
	 * Emitted upon the successful opening of the remote socket.
	 * This is emitted as a result of #fb_mqtt_open(). This should
	 * call #fb_mqtt_connect().
	 */
	g_signal_new("open",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             0);

	/**
	 * FbMqtt::publish:
	 * @mqtt: The #FbMqtt.
	 * @topic: The topic.
	 * @pload: The payload.
	 *
	 * Emitted upon an incoming message from the steam.
	 */
	g_signal_new("publish",
	             G_TYPE_FROM_CLASS(klass),
	             G_SIGNAL_ACTION,
	             0,
	             NULL, NULL, NULL,
	             G_TYPE_NONE,
	             2, G_TYPE_STRING, G_TYPE_BYTE_ARRAY);
}

static void
fb_mqtt_init(FbMqtt *mqtt)
{
	mqtt->rbuf = g_byte_array_new();
}

static void
fb_mqtt_message_dispose(GObject *obj)
{
	FbMqttMessage *msg = FB_MQTT_MESSAGE(obj);

	if(msg->bytes != NULL && msg->local) {
		g_byte_array_free(msg->bytes, TRUE);
		msg->bytes = NULL;
	}
}

static void
fb_mqtt_message_class_init(FbMqttMessageClass *klass)
{
	GObjectClass *gklass = G_OBJECT_CLASS(klass);

	gklass->dispose = fb_mqtt_message_dispose;
}

static void
fb_mqtt_message_init(G_GNUC_UNUSED FbMqttMessage *msg)
{
}

GQuark
fb_mqtt_error_quark(void)
{
	static GQuark q = 0;

	if (G_UNLIKELY(q == 0)) {
		q = g_quark_from_static_string("fb-mqtt-error-quark");
	}

	return q;
}

FbMqtt *
fb_mqtt_new(PurpleConnection *gc)
{
	FbMqtt *mqtt;

	g_return_val_if_fail(PURPLE_IS_CONNECTION(gc), NULL);

	mqtt = g_object_new(FB_TYPE_MQTT,  NULL);
	mqtt->gc = gc;

	return mqtt;
};

void
fb_mqtt_close(FbMqtt *mqtt)
{
	g_return_if_fail(FB_IS_MQTT(mqtt));

	if(mqtt->tev > 0) {
		g_source_remove(mqtt->tev);
		mqtt->tev = 0;
	}

	if(mqtt->cancellable != NULL) {
		g_cancellable_cancel(mqtt->cancellable);
		g_clear_object(&mqtt->cancellable);
	}

	if(mqtt->conn != NULL) {
		purple_gio_graceful_close(mqtt->conn,
				G_INPUT_STREAM(mqtt->input),
				G_OUTPUT_STREAM(mqtt->output));
		g_clear_object(&mqtt->input);
		g_clear_object(&mqtt->output);
		g_clear_object(&mqtt->conn);
	}

	mqtt->connected = FALSE;
	g_byte_array_set_size(mqtt->rbuf, 0);
}

static void
fb_mqtt_take_error(FbMqtt *mqtt, GError *err, const gchar *prefix)
{
	if (g_error_matches(err, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
		/* Return as cancelled means the connection is closing */
		g_error_free(err);
		return;
	}

	/* Now we can check for programming errors */
	g_return_if_fail(FB_IS_MQTT(mqtt));

	if (prefix != NULL) {
		g_prefix_error(&err, "%s: ", prefix);
	}

	g_signal_emit_by_name(mqtt, "error", err);
	g_error_free(err);
}

static void
fb_mqtt_error_literal(FbMqtt *mqtt, FbMqttError error, const gchar *msg)
{
	GError *err;

	g_return_if_fail(FB_IS_MQTT(mqtt));

	err = g_error_new_literal(FB_MQTT_ERROR, error, msg);

	g_signal_emit_by_name(mqtt, "error", err);
	g_error_free(err);
}

void
fb_mqtt_error(FbMqtt *mqtt, FbMqttError error, const gchar *format, ...)
{
	GError *err;
	va_list ap;

	g_return_if_fail(FB_IS_MQTT(mqtt));

	va_start(ap, format);
	err = g_error_new_valist(FB_MQTT_ERROR, error, format, ap);
	va_end(ap);

	g_signal_emit_by_name(mqtt, "error", err);
	g_error_free(err);
}

static gboolean
fb_mqtt_cb_timeout(gpointer data)
{
	FbMqtt *mqtt = data;

	mqtt->tev = 0;
	fb_mqtt_error_literal(mqtt, FB_MQTT_ERROR_GENERAL,
	                      _("Connection timed out"));
	return FALSE;
}

static void
fb_mqtt_timeout_clear(FbMqtt *mqtt)
{
	if(mqtt->tev > 0) {
		g_source_remove(mqtt->tev);
		mqtt->tev = 0;
	}
}

static void
fb_mqtt_timeout(FbMqtt *mqtt)
{
	fb_mqtt_timeout_clear(mqtt);
	mqtt->tev = g_timeout_add_seconds(FB_MQTT_TIMEOUT_CONN, fb_mqtt_cb_timeout,
	                                  mqtt);
}

static gboolean
fb_mqtt_cb_ping(gpointer data)
{
	FbMqtt *mqtt = data;
	FbMqttMessage *msg;

	msg = fb_mqtt_message_new(FB_MQTT_MESSAGE_TYPE_PINGREQ, 0);
	fb_mqtt_write(mqtt, msg);
	g_object_unref(msg);

	mqtt->tev = 0;
	fb_mqtt_timeout(mqtt);
	return FALSE;
}

static void
fb_mqtt_ping(FbMqtt *mqtt)
{
	fb_mqtt_timeout_clear(mqtt);
	mqtt->tev = g_timeout_add_seconds(FB_MQTT_TIMEOUT_PING, fb_mqtt_cb_ping,
	                                  mqtt);
}

static void
fb_mqtt_cb_fill(GObject *source, GAsyncResult *res, gpointer data)
{
	GBufferedInputStream *input = G_BUFFERED_INPUT_STREAM(source);
	FbMqtt *mqtt = data;
	gssize ret;
	GError *err = NULL;

	ret = g_buffered_input_stream_fill_finish(input, res, &err);

	if (ret < 1) {
		if (ret == 0) {
			err = g_error_new_literal(G_IO_ERROR,
					G_IO_ERROR_CONNECTION_CLOSED,
					_("Connection closed"));
		}

		fb_mqtt_take_error(mqtt, err, _("Failed to read fixed header"));
		return;
	}

	fb_mqtt_read_packet(mqtt);
}

static void
fb_mqtt_cb_read_packet(GObject *source, GAsyncResult *res, gpointer data)
{
	FbMqtt *mqtt = data;
	gssize ret;
	FbMqttMessage *msg;
	GError *err = NULL;

	ret = g_input_stream_read_finish(G_INPUT_STREAM(source), res, &err);

	if (ret < 1) {
		if (ret == 0) {
			err = g_error_new_literal(G_IO_ERROR,
					G_IO_ERROR_CONNECTION_CLOSED,
					_("Connection closed"));
		}

		fb_mqtt_take_error(mqtt, err, _("Failed to read packet data"));
		return;
	}

	mqtt->remz -= ret;

	if(mqtt->remz > 0) {
		g_input_stream_read_async(G_INPUT_STREAM(source),
		                          mqtt->rbuf->data + mqtt->rbuf->len - mqtt->remz,
		                          mqtt->remz, G_PRIORITY_DEFAULT,
		                          mqtt->cancellable, fb_mqtt_cb_read_packet,
		                          mqtt);
		return;
	}

	msg = fb_mqtt_message_new_bytes(mqtt->rbuf);

	if (G_UNLIKELY(msg == NULL)) {
		fb_mqtt_error_literal(mqtt, FB_MQTT_ERROR_GENERAL,
		                      _("Failed to parse message"));
		return;
	}

	fb_mqtt_read(mqtt, msg);
	g_object_unref(msg);

	/* Read another packet if connection wasn't reset in fb_mqtt_read() */
	if (fb_mqtt_connected(mqtt, FALSE)) {
		fb_mqtt_read_packet(mqtt);
	}
}

static void
fb_mqtt_read_packet(FbMqtt *mqtt)
{
	const guint8 *buf;
	gsize count = 0;
	gsize pos;
	guint mult = 1;
	guint8 byte;
	gsize size = 0;

	buf = g_buffered_input_stream_peek_buffer(mqtt->input, &count);

	/* Start at 1 to skip the first byte */
	pos = 1;

	do {
		if (pos >= count) {
			/* Not enough data yet, try again later */
			g_buffered_input_stream_fill_async(mqtt->input, -1,
			                                   G_PRIORITY_DEFAULT,
			                                   mqtt->cancellable,
			                                   fb_mqtt_cb_fill, mqtt);
			return;
		}

		byte = *(buf + pos++);

		size += (byte & 127) * mult;
		mult *= 128;
	} while ((byte & 128) != 0);

	/* Add header to size */
	size += pos;

	g_byte_array_set_size(mqtt->rbuf, size);
	mqtt->remz = size;

	/* TODO: Use g_input_stream_read_all_async() when available. */
	/* TODO: Alternately, it would be nice to let the
	 * FbMqttMessage directly use the GBufferedInputStream
	 * buffer instead of copying it, provided it's consumed
	 * before the next read.
	 */
	g_input_stream_read_async(G_INPUT_STREAM(mqtt->input), mqtt->rbuf->data,
	                          mqtt->rbuf->len, G_PRIORITY_DEFAULT,
	                          mqtt->cancellable, fb_mqtt_cb_read_packet, mqtt);
}

void
fb_mqtt_read(FbMqtt *mqtt, FbMqttMessage *msg)
{
	FbMqttMessage *nsg;
	GByteArray *wytes;
	gchar *str;
	guint8 chr;
	guint16 mid;

	g_return_if_fail(FB_IS_MQTT(mqtt));

	fb_util_debug_hexdump(FB_UTIL_DEBUG_INFO, msg->bytes,
	                      "Reading %d (flags: 0x%0X)",
	                      msg->type, msg->flags);

	switch (msg->type) {
	case FB_MQTT_MESSAGE_TYPE_CONNACK:
		if (!fb_mqtt_message_read_byte(msg, NULL) ||
		    !fb_mqtt_message_read_byte(msg, &chr))
		{
			break;
		}

		if (chr != FB_MQTT_ERROR_SUCCESS) {
			fb_mqtt_error(mqtt, chr, _("Connection failed (%u)"),
			              chr);
			return;
		}

		mqtt->connected = TRUE;
		fb_mqtt_ping(mqtt);
		g_signal_emit_by_name(mqtt, "connect");
		return;

	case FB_MQTT_MESSAGE_TYPE_PUBLISH:
		if (!fb_mqtt_message_read_str(msg, &str)) {
			break;
		}

		if((msg->flags & FB_MQTT_MESSAGE_FLAG_QOS1) ||
		   (msg->flags & FB_MQTT_MESSAGE_FLAG_QOS2))
		{
			if(msg->flags & FB_MQTT_MESSAGE_FLAG_QOS1) {
				chr = FB_MQTT_MESSAGE_TYPE_PUBACK;
			} else {
				chr = FB_MQTT_MESSAGE_TYPE_PUBREC;
			}

			if (!fb_mqtt_message_read_mid(msg, &mid)) {
				g_free(str);
				break;
			}

			nsg = fb_mqtt_message_new(chr, 0);
			fb_mqtt_message_write_u16(nsg, mid);
			fb_mqtt_write(mqtt, nsg);
			g_object_unref(nsg);
		}

		wytes = g_byte_array_new();
		fb_mqtt_message_read_r(msg, wytes);
		g_signal_emit_by_name(mqtt, "publish", str, wytes);
		g_byte_array_free(wytes, TRUE);
		g_free(str);
		return;

	case FB_MQTT_MESSAGE_TYPE_PUBREL:
		if (!fb_mqtt_message_read_mid(msg, &mid)) {
			break;
		}

		nsg = fb_mqtt_message_new(FB_MQTT_MESSAGE_TYPE_PUBCOMP, 0);
		fb_mqtt_message_write_u16(nsg, mid); /* Message identifier */
		fb_mqtt_write(mqtt, nsg);
		g_object_unref(nsg);
		return;

	case FB_MQTT_MESSAGE_TYPE_PINGRESP:
		fb_mqtt_ping(mqtt);
		return;

	case FB_MQTT_MESSAGE_TYPE_PUBACK:
	case FB_MQTT_MESSAGE_TYPE_PUBCOMP:
	case FB_MQTT_MESSAGE_TYPE_SUBACK:
	case FB_MQTT_MESSAGE_TYPE_UNSUBACK:
		return;

	default:
		fb_mqtt_error(mqtt, FB_MQTT_ERROR_GENERAL,
		              _("Unknown packet (%u)"), msg->type);
		return;
	}

	/* Since no case returned, there was a parse error. */
	fb_mqtt_error_literal(mqtt, FB_MQTT_ERROR_GENERAL,
	                      _("Failed to parse message"));
}

static void
fb_mqtt_cb_push_bytes(GObject *source, GAsyncResult *res, gpointer data)
{
	PurpleQueuedOutputStream *stream = PURPLE_QUEUED_OUTPUT_STREAM(source);
	FbMqtt *mqtt = data;
	GError *err = NULL;

	if (!purple_queued_output_stream_push_bytes_finish(stream,
			res, &err)) {
		purple_queued_output_stream_clear_queue(stream);

		fb_mqtt_take_error(mqtt, err, _("Failed to write data"));
		return;
	}
}

void
fb_mqtt_write(FbMqtt *mqtt, FbMqttMessage *msg)
{
	const GByteArray *bytes;
	GBytes *gbytes;

	g_return_if_fail(FB_IS_MQTT(mqtt));
	g_return_if_fail(FB_IS_MQTT_MESSAGE(msg));

	bytes = fb_mqtt_message_bytes(msg);

	if (G_UNLIKELY(bytes == NULL)) {
		fb_mqtt_error_literal(mqtt, FB_MQTT_ERROR_GENERAL,
		                      _("Failed to format data"));
		return;
	}

	fb_util_debug_hexdump(FB_UTIL_DEBUG_INFO, msg->bytes,
	                      "Writing %d (flags: 0x%0X)",
	                      msg->type, msg->flags);

 	/* TODO: Would be nice to refactor this to not require copying bytes */
	gbytes = g_bytes_new(bytes->data, bytes->len);
	purple_queued_output_stream_push_bytes_async(mqtt->output, gbytes,
	                                             G_PRIORITY_DEFAULT,
	                                             mqtt->cancellable,
	                                             fb_mqtt_cb_push_bytes, mqtt);
	g_bytes_unref(gbytes);
}

static void
fb_mqtt_cb_open(GObject *source, GAsyncResult *res, gpointer data)
{
	FbMqtt *mqtt = data;
	GSocketConnection *conn;
	GError *err = NULL;

	conn = g_socket_client_connect_to_host_finish(G_SOCKET_CLIENT(source),
			res, &err);

	if (conn == NULL) {
		fb_mqtt_take_error(mqtt, err, NULL);
		return;
	}

	fb_mqtt_timeout_clear(mqtt);

	mqtt->conn = G_IO_STREAM(conn);
	mqtt->input = G_BUFFERED_INPUT_STREAM(g_buffered_input_stream_new(
			g_io_stream_get_input_stream(mqtt->conn)));
	mqtt->output = purple_queued_output_stream_new(
			g_io_stream_get_output_stream(mqtt->conn));

	fb_mqtt_read_packet(mqtt);

	g_signal_emit_by_name(mqtt, "open");
}

void
fb_mqtt_open(FbMqtt *mqtt, const gchar *host, gint port)
{
	PurpleAccount *acc;
	GSocketClient *client;
	GError *err = NULL;

	g_return_if_fail(FB_IS_MQTT(mqtt));

	acc = purple_connection_get_account(mqtt->gc);
	fb_mqtt_close(mqtt);

	client = purple_gio_socket_client_new(acc, &err);

	if (client == NULL) {
		fb_mqtt_take_error(mqtt, err, NULL);
		return;
	}

	mqtt->cancellable = g_cancellable_new();

	g_socket_client_set_tls(client, TRUE);
	g_socket_client_connect_to_host_async(client, host, port,
	                                      mqtt->cancellable, fb_mqtt_cb_open,
	                                      mqtt);
	g_object_unref(client);

	fb_mqtt_timeout(mqtt);
}

void
fb_mqtt_connect(FbMqtt *mqtt, guint8 flags, const GByteArray *pload)
{
	FbMqttMessage *msg;

	g_return_if_fail(!fb_mqtt_connected(mqtt, FALSE));
	g_return_if_fail(pload != NULL);

	/* Facebook always sends a CONNACK, use QoS1 */
	flags |= FB_MQTT_CONNECT_FLAG_QOS1;

	msg = fb_mqtt_message_new(FB_MQTT_MESSAGE_TYPE_CONNECT, 0);
	fb_mqtt_message_write_str(msg, FB_MQTT_NAME);   /* Protocol name */
	fb_mqtt_message_write_byte(msg, FB_MQTT_LEVEL); /* Protocol level */
	fb_mqtt_message_write_byte(msg, flags);         /* Flags */
	fb_mqtt_message_write_u16(msg, FB_MQTT_KA);     /* Keep alive */

	fb_mqtt_message_write(msg, pload->data, pload->len);
	fb_mqtt_write(mqtt, msg);

	fb_mqtt_timeout(mqtt);
	g_object_unref(msg);
}

gboolean
fb_mqtt_connected(FbMqtt *mqtt, gboolean error)
{
	gboolean connected;

	g_return_val_if_fail(FB_IS_MQTT(mqtt), FALSE);
	connected = (mqtt->conn != NULL) && mqtt->connected;

	if (!connected && error) {
		fb_mqtt_error_literal(mqtt, FB_MQTT_ERROR_GENERAL, _("Not connected"));
	}

	return connected;
}

void
fb_mqtt_disconnect(FbMqtt *mqtt)
{
	FbMqttMessage *msg;

	if (G_UNLIKELY(!fb_mqtt_connected(mqtt, FALSE))) {
		return;
	}

	msg = fb_mqtt_message_new(FB_MQTT_MESSAGE_TYPE_DISCONNECT, 0);
	fb_mqtt_write(mqtt, msg);
	g_object_unref(msg);
	fb_mqtt_close(mqtt);
}

void
fb_mqtt_publish(FbMqtt *mqtt, const gchar *topic, const GByteArray *pload)
{
	FbMqttMessage *msg;

	g_return_if_fail(FB_IS_MQTT(mqtt));
	g_return_if_fail(fb_mqtt_connected(mqtt, FALSE));

	/* Message identifier not required, but for consistency use QoS1 */
	msg = fb_mqtt_message_new(FB_MQTT_MESSAGE_TYPE_PUBLISH,
	                          FB_MQTT_MESSAGE_FLAG_QOS1);

	fb_mqtt_message_write_str(msg, topic);      /* Message topic */
	fb_mqtt_message_write_mid(msg, &mqtt->mid); /* Message identifier */

	if (pload != NULL) {
		fb_mqtt_message_write(msg, pload->data, pload->len);
	}

	fb_mqtt_write(mqtt, msg);
	g_object_unref(msg);
}

void
fb_mqtt_subscribe(FbMqtt *mqtt, ...)
{
	const gchar *topic;
	FbMqttMessage *msg;
	guint16 qos;
	va_list ap;

	g_return_if_fail(FB_IS_MQTT(mqtt));
	g_return_if_fail(fb_mqtt_connected(mqtt, FALSE));

	/* Facebook requires a message identifier, use QoS1 */
	msg = fb_mqtt_message_new(FB_MQTT_MESSAGE_TYPE_SUBSCRIBE,
	                          FB_MQTT_MESSAGE_FLAG_QOS1);

	fb_mqtt_message_write_mid(msg, &mqtt->mid); /* Message identifier */

	va_start(ap, mqtt);

	while ((topic = va_arg(ap, const gchar*)) != NULL) {
		qos = va_arg(ap, guint);
		fb_mqtt_message_write_str(msg, topic);
		fb_mqtt_message_write_byte(msg, qos);
	}

	va_end(ap);

	fb_mqtt_write(mqtt, msg);
	g_object_unref(msg);
}

void
fb_mqtt_unsubscribe(FbMqtt *mqtt, const gchar *topic1, ...)
{
	const gchar *topic;
	FbMqttMessage *msg;
	va_list ap;

	g_return_if_fail(FB_IS_MQTT(mqtt));
	g_return_if_fail(fb_mqtt_connected(mqtt, FALSE));

	/* Facebook requires a message identifier, use QoS1 */
	msg = fb_mqtt_message_new(FB_MQTT_MESSAGE_TYPE_UNSUBSCRIBE,
	                          FB_MQTT_MESSAGE_FLAG_QOS1);

	fb_mqtt_message_write_mid(msg, &mqtt->mid); /* Message identifier */
	fb_mqtt_message_write_str(msg, topic1);     /* First topic */

	va_start(ap, topic1);

	while ((topic = va_arg(ap, const gchar*)) != NULL) {
		fb_mqtt_message_write_str(msg, topic); /* Remaining topics */
	}

	va_end(ap);

	fb_mqtt_write(mqtt, msg);
	g_object_unref(msg);
}

FbMqttMessage *
fb_mqtt_message_new(FbMqttMessageType type, FbMqttMessageFlags flags)
{
	FbMqttMessage *msg;

	msg = g_object_new(FB_TYPE_MQTT_MESSAGE, NULL);

	msg->type = type;
	msg->flags = flags;
	msg->bytes = g_byte_array_new();
	msg->local = TRUE;

	return msg;
}

FbMqttMessage *
fb_mqtt_message_new_bytes(GByteArray *bytes)
{
	FbMqttMessage *msg;
	guint8 *byte;

	g_return_val_if_fail(bytes != NULL, NULL);
	g_return_val_if_fail(bytes->len >= 2, NULL);

	msg = g_object_new(FB_TYPE_MQTT_MESSAGE, NULL);

	msg->bytes = bytes;
	msg->local = FALSE;
	msg->type = (*bytes->data & 0xF0) >> 4;
	msg->flags = *bytes->data & 0x0F;

	/* Skip the fixed header */
	byte = msg->bytes->data + 1;
	while((*byte & 128) != 0) {
		byte++;
	}
	byte++;
	msg->offset = byte - bytes->data;
	msg->pos = msg->offset;

	return msg;
}

void
fb_mqtt_message_reset(FbMqttMessage *msg)
{
	g_return_if_fail(FB_IS_MQTT_MESSAGE(msg));

	if(msg->offset > 0) {
		g_byte_array_remove_range(msg->bytes, 0, msg->offset);
		msg->offset = 0;
		msg->pos = 0;
	}
}

const GByteArray *
fb_mqtt_message_bytes(FbMqttMessage *msg)
{
	guint i;
	guint8 byte;
	guint8 sbuf[4];
	guint32 size;

	g_return_val_if_fail(FB_IS_MQTT_MESSAGE(msg), NULL);

	i = 0;
	size = msg->bytes->len - msg->offset;

	do {
		if (G_UNLIKELY(i >= G_N_ELEMENTS(sbuf))) {
			return NULL;
		}

		byte = size % 128;
		size /= 128;

		if (size > 0) {
			byte |= 128;
		}

		sbuf[i++] = byte;
	} while (size > 0);

	fb_mqtt_message_reset(msg);
	g_byte_array_prepend(msg->bytes, sbuf, i);

	byte = ((msg->type & 0x0F) << 4) | (msg->flags & 0x0F);
	g_byte_array_prepend(msg->bytes, &byte, sizeof byte);

	msg->pos = (i + 1) * (sizeof byte);
	return msg->bytes;
}

gboolean
fb_mqtt_message_read(FbMqttMessage *msg, gpointer data, guint size)
{
	g_return_val_if_fail(FB_IS_MQTT_MESSAGE(msg), FALSE);

	if((msg->pos + size) > msg->bytes->len) {
		return FALSE;
	}

	if ((data != NULL) && (size > 0)) {
		memcpy(data, msg->bytes->data + msg->pos, size);
	}

	msg->pos += size;
	return TRUE;
}

gboolean
fb_mqtt_message_read_r(FbMqttMessage *msg, GByteArray *bytes)
{
	guint size;

	g_return_val_if_fail(FB_IS_MQTT_MESSAGE(msg), FALSE);
	size = msg->bytes->len - msg->pos;

	if (G_LIKELY(size > 0)) {
		g_byte_array_append(bytes, msg->bytes->data + msg->pos, size);
	}

	return TRUE;
}

gboolean
fb_mqtt_message_read_byte(FbMqttMessage *msg, guint8 *value)
{
	return fb_mqtt_message_read(msg, value, sizeof *value);
}

gboolean
fb_mqtt_message_read_mid(FbMqttMessage *msg, guint16 *value)
{
	return fb_mqtt_message_read_u16(msg, value);
}

gboolean
fb_mqtt_message_read_u16(FbMqttMessage *msg, guint16 *value)
{
	if (!fb_mqtt_message_read(msg, value, sizeof *value)) {
		return FALSE;
	}

	if (value != NULL) {
		*value = g_ntohs(*value);
	}

	return TRUE;
}

gboolean
fb_mqtt_message_read_str(FbMqttMessage *msg, gchar **value)
{
	guint8 *data;
	guint16 size;

	if (!fb_mqtt_message_read_u16(msg, &size)) {
		return FALSE;
	}

	if (value != NULL) {
		data = g_new(guint8, size + 1);
		data[size] = 0;
	} else {
		data = NULL;
	}

	if (!fb_mqtt_message_read(msg, data, size)) {
		g_free(data);
		return FALSE;
	}

	if (value != NULL) {
		*value = (gchar *) data;
	}

	return TRUE;
}

void
fb_mqtt_message_write(FbMqttMessage *msg, gconstpointer data, guint size)
{
	g_return_if_fail(FB_IS_MQTT_MESSAGE(msg));

	g_byte_array_append(msg->bytes, data, size);
	msg->pos += size;
}

void
fb_mqtt_message_write_byte(FbMqttMessage *msg, guint8 value)
{
	fb_mqtt_message_write(msg, &value, sizeof value);
}

void
fb_mqtt_message_write_mid(FbMqttMessage *msg, guint16 *value)
{
	g_return_if_fail(value != NULL);
	fb_mqtt_message_write_u16(msg, ++(*value));
}

void
fb_mqtt_message_write_u16(FbMqttMessage *msg, guint16 value)
{
	value = g_htons(value);
	fb_mqtt_message_write(msg, &value, sizeof value);
}

void
fb_mqtt_message_write_str(FbMqttMessage *msg, const gchar *value)
{
	gint16 size;

	g_return_if_fail(value != NULL);

	size = strlen(value);
	fb_mqtt_message_write_u16(msg, size);
	fb_mqtt_message_write(msg, value, size);
}
