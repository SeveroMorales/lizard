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
#include "purpledemoprotocolclient.h"

/******************************************************************************
 * PurpleProtocolClient Implementation
 *****************************************************************************/
static gchar *
purple_demo_protocol_status_text(G_GNUC_UNUSED PurpleProtocolClient *client,
                                 PurpleBuddy *buddy)
{
	PurplePresence *presence = NULL;
	PurpleStatus *status = NULL;
	const gchar *message = NULL;
	gchar *ret = NULL;

	presence = purple_buddy_get_presence(buddy);
	status = purple_presence_get_active_status(presence);

	message = purple_status_get_attr_string(status, "message");
	if(message != NULL) {
		ret = g_markup_escape_text(message, -1);
		purple_util_chrreplace(ret, '\n', ' ');
	}

	return ret;
}

void
purple_demo_protocol_client_init(PurpleProtocolClientInterface *iface) {
	iface->status_text = purple_demo_protocol_status_text;
}
