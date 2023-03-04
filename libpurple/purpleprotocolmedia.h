/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_PROTOCOL_MEDIA_H
#define PURPLE_PROTOCOL_MEDIA_H

#include <glib.h>
#include <glib-object.h>

#include <libpurple/account.h>
#include <libpurple/media.h>
#include <libpurple/purpleprotocol.h>

#define PURPLE_TYPE_PROTOCOL_MEDIA (purple_protocol_media_get_type())
G_DECLARE_INTERFACE(PurpleProtocolMedia, purple_protocol_media, PURPLE,
                    PROTOCOL_MEDIA, PurpleProtocol)

G_BEGIN_DECLS

/**
 * PurpleProtocolMedia:
 *
 * #PurpleProtocolMedia describes the multimedia api that is available for
 * protocols.
 */

/**
 * PurpleProtocolMediaInterface:
 * @initiate_session: Initiate a media session with the given contact.
 *                    <sbr/>@account: The account to initiate the media session
 *                                    on.
 *                    <sbr/>@who: The remote user to initiate the session with.
 *                    <sbr/>@type: The type of media session to initiate.
 *                    <sbr/>Returns: %TRUE if the call succeeded else %FALSE.
 *                                   (Doesn't imply the media session or stream
 *                                   will be successfully created)
 * @get_caps: Checks to see if the given contact supports the given type of
 *            media session.
 *            <sbr/>@account: The account the contact is on.
 *            <sbr/>@who: The remote user to check for media capability with.
 *            <sbr/>Returns: The media caps the contact supports.
 * @send_dtmf: Sends DTMF codes out-of-band in a protocol-specific way if the
 *             protocol supports it, or failing that in-band if the media backend
 *             can do so. See purple_media_send_dtmf().
 *
 * The protocol media interface.
 *
 * This interface provides callbacks for media sessions on the protocol.
 */
struct _PurpleProtocolMediaInterface {
	/*< private >*/
	GTypeInterface parent;

	/*< public >*/
	gboolean (*initiate_session)(PurpleProtocolMedia *media, PurpleAccount *account, const gchar *who, PurpleMediaSessionType type);

	PurpleMediaCaps (*get_caps)(PurpleProtocolMedia *media, PurpleAccount *account, const gchar *who);

	gboolean (*send_dtmf)(PurpleProtocolMedia *protocol_media, PurpleMedia *media, gchar dtmf, guint8 volume, guint8 duration);

	/*< private >*/
	gpointer reserved[4];
};

/**
 * purple_protocol_media_initiate_session:
 * @media: The #PurpleProtocolMedia instance.
 * @account: The #PurpleAccount instance.
 * @who: The user to initiate a media session with.
 * @type: The type of media session to create.
 *
 * Initiates a media connection of @type to @who.
 *
 * Returns: %TRUE if successful, %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_protocol_media_initiate_session(PurpleProtocolMedia *media, PurpleAccount *account, const gchar *who, PurpleMediaSessionType type);

/**
 * purple_protocol_media_get_caps:
 * @media: The #PurpleProtocolMedia instance.
 * @account: The #PurpleAccount instance.
 * @who: The user to get the media capabilities for.
 *
 * Gets the #PurpleMediaCaps for @who which determine what types of media are
 * available.
 *
 * Returns: the media capabilities of @who.
 *
 * Since: 3.0.0
 */
PurpleMediaCaps purple_protocol_media_get_caps(PurpleProtocolMedia *media, PurpleAccount *account, const gchar *who);

/**
 * purple_protocol_media_send_dtmf:
 * @protocol_media: The #PurpleProtocolMedia instance.
 * @media: The #PurpleMedia instance.
 * @dtmf: A DTMF to send.
 * @volume: The volume to send @dtmf at.
 * @duration: The duration to send @dtmf (in ms?)
 *
 * Sends a DTMF (dual-tone multi-frequency) signal via the established @media
 * for the given @duration at the given @volume.
 *
 * It is up to the specific implementation if DTMF is send in or out of band.
 *
 * Returns: %TRUE if successful, %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_protocol_media_send_dtmf(PurpleProtocolMedia *protocol_media, PurpleMedia *media, gchar dtmf, guint8 volume, guint8 duration);

G_END_DECLS

#endif /* PURPLE_PROTOCOL_MEDIA_H */
