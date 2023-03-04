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

#ifndef PURPLE_FACEBOOK_FACEBOOK_H
#define PURPLE_FACEBOOK_FACEBOOK_H

#include <glib.h>
#include <gmodule.h>

#define FACEBOOK_TYPE_PROTOCOL (facebook_protocol_get_type())
G_DECLARE_FINAL_TYPE(FacebookProtocol, facebook_protocol, FACEBOOK, PROTOCOL,
                     PurpleProtocol)

/**
 * FB_PROTOCOL_ID:
 *
 * The Facebook protocol identifier.
 */
#define FB_PROTOCOL_ID  "prpl-facebook"

/**
 * FacebookProtocol:
 *
 * Represents the Facebook #PurpleProtocol.
 */

/**
 * FacebookProtocolClass:
 *
 * The base class for all #FacebookProtocol's.
 */

#endif /* PURPLE_FACEBOOK_FACEBOOK_H */
