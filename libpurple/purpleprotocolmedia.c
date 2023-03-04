/*
 * purple
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "purpleprotocolmedia.h"

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_INTERFACE(PurpleProtocolMedia, purple_protocol_media,
                   PURPLE_TYPE_PROTOCOL)

static void
purple_protocol_media_default_init(G_GNUC_UNUSED PurpleProtocolMediaInterface *iface)
{
}

/******************************************************************************
 * Public API
 *****************************************************************************/
gboolean
purple_protocol_media_initiate_session(PurpleProtocolMedia *media,
                                       PurpleAccount *account,
                                       const gchar *who,
                                       PurpleMediaSessionType type)
{
	PurpleProtocolMediaInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_MEDIA(media), FALSE);

	iface = PURPLE_PROTOCOL_MEDIA_GET_IFACE(media);
	if(iface && iface->initiate_session) {
		return iface->initiate_session(media, account, who, type);
	}

	return FALSE;
}

PurpleMediaCaps
purple_protocol_media_get_caps(PurpleProtocolMedia *media,
                               PurpleAccount *account, const gchar *who)
{
	PurpleProtocolMediaInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_MEDIA(media),
	                     PURPLE_MEDIA_CAPS_NONE);

	iface = PURPLE_PROTOCOL_MEDIA_GET_IFACE(media);
	if(iface && iface->get_caps) {
		return iface->get_caps(media, account, who);
	}

	return PURPLE_MEDIA_CAPS_NONE;
}

gboolean
purple_protocol_media_send_dtmf(PurpleProtocolMedia *protocol_media,
                                PurpleMedia *media, gchar dtmf, guint8 volume,
                                guint8 duration)
{
	PurpleProtocolMediaInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_MEDIA(protocol_media), FALSE);

	iface = PURPLE_PROTOCOL_MEDIA_GET_IFACE(protocol_media);
	if(iface && iface->send_dtmf) {
		return iface->send_dtmf(protocol_media, media, dtmf, volume, duration);
	}

	return FALSE;
}
