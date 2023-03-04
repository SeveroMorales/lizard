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

#ifndef PURPLE_PROTOCOL_ROOMLIST_H
#define PURPLE_PROTOCOL_ROOMLIST_H

#include <glib.h>

#include "connection.h"
#include "roomlist.h"
#include "purpleprotocol.h"

G_BEGIN_DECLS

#define PURPLE_TYPE_PROTOCOL_ROOMLIST (purple_protocol_roomlist_get_type())

/**
 * PurpleProtocolRoomlist:
 *
 * #PurpleProtocolRoomlist is an interface to abstract how to handle rooms
 * lists at the protocol level.
 */

G_DECLARE_INTERFACE(PurpleProtocolRoomlist, purple_protocol_roomlist, PURPLE,
                    PROTOCOL_ROOMLIST, PurpleProtocol)

/**
 * PurpleProtocolRoomlistInterface:
 *
 * The protocol roomlist interface.
 *
 * This interface provides callbacks for room listing.
 *
 * Since: 3.0.0
 */
struct _PurpleProtocolRoomlistInterface {
	/*< private >*/
	GTypeInterface parent_iface;

	/*< public >*/
	PurpleRoomlist *(*get_list)(PurpleProtocolRoomlist *protocol_roomlist, PurpleConnection *gc);

	void (*cancel)(PurpleProtocolRoomlist *protocol_roomlist, PurpleRoomlist *list);

	void (*expand_category)(PurpleProtocolRoomlist *protocol_roomlist, PurpleRoomlist *list, PurpleRoomlistRoom *category);

	gchar *(*room_serialize)(PurpleProtocolRoomlist *protocol_roomlist, PurpleRoomlistRoom *room);

	/*< private >*/
	gpointer reserved[4];
};

/**
 * purple_protocol_roomlist_get_list:
 * @protocol_roomlist: The #PurpleProtocolRoomlist instance.
 * @gc: The #PurpleAccount to get the roomlist for.
 *
 * Gets the list of rooms for @gc.
 *
 * Returns: (transfer full): The roomlist for @gc.
 *
 * Since: 3.0.0
 */
PurpleRoomlist *purple_protocol_roomlist_get_list(PurpleProtocolRoomlist *protocol_roomlist, PurpleConnection *gc);

/**
 * purple_protocol_roomlist_cancel:
 * @protocol_roomlist: The #PurpleProtocolRoomlist instance.
 * @list: The #PurpleRoomlist instance.
 *
 * Requesting a roomlist can take a long time. This function cancels a request
 * that's already in progress.
 *
 * Since: 3.0.0
 */
void purple_protocol_roomlist_cancel(PurpleProtocolRoomlist *protocol_roomlist, PurpleRoomlist *list);

/**
 * purple_protocol_roomlist_expand_category:
 * @protocol_roomlist: The #PurpleProtocolRoomlist instance.
 * @list: The #PurpleRoomlist instance.
 * @category: The category to expand.
 *
 * Expands the given @category for @list.
 *
 * Since: 3.0.0
 */
void purple_protocol_roomlist_expand_category(PurpleProtocolRoomlist *protocol_roomlist, PurpleRoomlist *list, PurpleRoomlistRoom *category);

/**
 * purple_protocol_roomlist_room_serialize:
 * @protocol_roomlist: The #PurpleProtocolRoomlist instance.
 * @room: The #PurpleRoomlistRoom instance.
 *
 * Serializes @room into a string that will be displayed in a user interface.
 *
 * Returns: (transfer full): The serialized form of @room.
 *
 * Since: 3.0.0
 */
char *purple_protocol_roomlist_room_serialize(PurpleProtocolRoomlist *protocol_roomlist, PurpleRoomlistRoom *room);

G_END_DECLS

#endif /* PURPLE_PROTOCOL_ROOMLIST_H */
