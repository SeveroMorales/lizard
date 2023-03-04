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

#include <glib/gi18n-lib.h>

#include "purpledemoprotocol.h"
#include "purpledemoprotocolim.h"

/******************************************************************************
 * PurpleProtocolIM Implementation
 *****************************************************************************/
typedef struct {
	PurpleConnection *connection;
	PurpleMessage *message;
} PurpleDemoProtocolIMInfo;

static void
purple_demo_protocol_im_info_free(PurpleDemoProtocolIMInfo *info) {
	g_object_unref(info->message);
	g_free(info);
}

static gboolean
purple_demo_protocol_echo_im_cb(gpointer data)
{
	PurpleDemoProtocolIMInfo *info = data;
	const char *who = NULL;
	PurpleMessageFlags flags;
	GDateTime *timestamp = NULL;

	/* Turn outgoing message back incoming. */
	who = purple_message_get_recipient(info->message);
	flags = purple_message_get_flags(info->message);
	flags &= ~PURPLE_MESSAGE_SEND;
	flags |= PURPLE_MESSAGE_RECV;
	timestamp = purple_message_get_timestamp(info->message);

	purple_serv_got_im(info->connection, who,
	                   purple_message_get_contents(info->message), flags,
	                   g_date_time_to_unix(timestamp));

	return FALSE;
}

static gint
purple_demo_protocol_send_im(G_GNUC_UNUSED PurpleProtocolIM *im,
                             PurpleConnection *conn, PurpleMessage *msg)
{
	const gchar *who = purple_message_get_recipient(msg);

	if(purple_strequal(who, "Echo")) {
		PurpleDemoProtocolIMInfo *info = g_new(PurpleDemoProtocolIMInfo, 1);

		info->connection = conn;
		info->message = g_object_ref(msg);

		g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,
		                purple_demo_protocol_echo_im_cb, info,
		                (GDestroyNotify)purple_demo_protocol_im_info_free);
	}

	return 1;
}

void
purple_demo_protocol_im_init(PurpleProtocolIMInterface *iface) {
	iface->send = purple_demo_protocol_send_im;
}
