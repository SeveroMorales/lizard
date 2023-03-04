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

#include <time.h>

#include <glib/gi18n-lib.h>

#include "purpledemoprotocol.h"
#include "purpledemoprotocolmedia.h"

/******************************************************************************
 * PurpleProtocolMedia Implementation
 *****************************************************************************/
static PurpleMediaCaps
purple_demo_protocol_media_get_caps(G_GNUC_UNUSED PurpleProtocolMedia *media,
                                    G_GNUC_UNUSED PurpleAccount *account,
                                    const gchar *who)
{
	if(purple_strequal(who, "Echo")) {
		return PURPLE_MEDIA_CAPS_AUDIO | PURPLE_MEDIA_CAPS_VIDEO |
		       PURPLE_MEDIA_CAPS_AUDIO_VIDEO;
	}

	return PURPLE_MEDIA_CAPS_NONE;
}

static gboolean
purple_demo_protocol_media_initiate_session(G_GNUC_UNUSED PurpleProtocolMedia *media,
                                            PurpleAccount *account,
                                            const gchar *who,
                                            PurpleMediaSessionType type)
{
	PurpleConnection *connection = NULL;
	gchar *session_name = NULL;
	gchar *message = NULL;
	GDateTime *timestamp = NULL;

	connection = purple_account_get_connection(account);

	session_name = g_flags_to_string(PURPLE_MEDIA_TYPE_SESSION_TYPE, type);
	message = g_strdup_printf(_("Initiated demo %s session with %s"),
	                          session_name, who);
	timestamp = g_date_time_new_now_utc();

	purple_serv_got_im(connection, "Echo",
	                   message, PURPLE_MESSAGE_RECV,
	                   g_date_time_to_unix(timestamp));

	g_date_time_unref(timestamp);
	g_free(message);
	g_free(session_name);

	/* TODO: When libpurple gets a backend, we can implement more of this. */
	return FALSE;
}

static gboolean
purple_demo_protocol_media_send_dtmf(G_GNUC_UNUSED PurpleProtocolMedia *protocol_media,
                                     PurpleMedia *media, gchar dtmf,
                                     guint8 volume, guint8 duration)
{
	PurpleAccount *account = NULL;
	PurpleConnection *connection = NULL;
	gchar *message = NULL;
	GDateTime *timestamp = NULL;

	account = purple_media_get_account(media);
	connection = purple_account_get_connection(account);

	message = g_strdup_printf(_("Received DTMF %c at volume %d for %d seconds"),
	                          dtmf, volume, duration);
	timestamp = g_date_time_new_now_utc();

	purple_serv_got_im(connection, "Echo",
	                   message, PURPLE_MESSAGE_RECV,
	                   g_date_time_to_unix(timestamp));

	g_date_time_unref(timestamp);
	g_free(message);

	return TRUE;
}

void
purple_demo_protocol_media_init(PurpleProtocolMediaInterface *iface) {
	iface->get_caps = purple_demo_protocol_media_get_caps;
	iface->initiate_session = purple_demo_protocol_media_initiate_session;
	iface->send_dtmf = purple_demo_protocol_media_send_dtmf;
}
