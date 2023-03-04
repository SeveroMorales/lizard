/**
 * @file bonjour.h The Purple interface to mDNS and peer to peer XMPP.
 *
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 */

#ifndef PURPLE_BONJOUR_BONJOUR_H
#define PURPLE_BONJOUR_BONJOUR_H

#include <gmodule.h>

#include <purple.h>

#include "mdns_common.h"
#include "xmpp.h"

#define BONJOUR_GROUP_NAME (bonjour_get_group_name())
#define BONJOUR_PROTOCOL_NAME "bonjour"

#define BONJOUR_STATUS_ID_OFFLINE   "offline"
#define BONJOUR_STATUS_ID_AVAILABLE "available"
#define BONJOUR_STATUS_ID_AWAY      "away"

#define BONJOUR_DEFAULT_PORT 5298

#define BONJOUR_TYPE_PROTOCOL (bonjour_protocol_get_type())
G_DECLARE_FINAL_TYPE(BonjourProtocol, bonjour_protocol, BONJOUR, PROTOCOL,
                     PurpleProtocol)

typedef struct
{
	BonjourDnsSd *dns_sd_data;
	BonjourXMPP *xmpp_data;
	GSList *xfer_lists;
	gchar *jid;
} BonjourData;

/**
 * Returns the GType for the BonjourProtocol object.
 */
G_MODULE_EXPORT GType bonjour_protocol_get_type(void);

/**
 *  This will always be username@machinename
 */
const char *bonjour_get_jid(PurpleAccount *account);

const gchar *bonjour_get_group_name(void);

#endif /* PURPLE_BONJOUR_BONJOUR_H */
