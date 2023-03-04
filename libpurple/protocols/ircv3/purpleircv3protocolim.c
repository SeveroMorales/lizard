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

#include "purpleircv3protocolim.h"

#include "purpleircv3connection.h"

/******************************************************************************
 * PurpleProtocolIM Implementation
 *****************************************************************************/
static gint
purple_ircv3_protocol_send_im(G_GNUC_UNUSED PurpleProtocolIM *im,
                              PurpleConnection *conn,
                              PurpleMessage *message)
{
	PurpleIRCv3Connection *connection = PURPLE_IRCV3_CONNECTION(conn);
	const char *contents = NULL;
	const char *recipient = NULL;

	contents = purple_message_get_contents(message);
	recipient = purple_message_get_recipient(message);

	purple_ircv3_connection_writef(connection, "PRIVMSG %s :%s", recipient,
	                               contents);

	return 1;
}

void
purple_ircv3_protocol_im_init(PurpleProtocolIMInterface *iface) {
	iface->send = purple_ircv3_protocol_send_im;
}
