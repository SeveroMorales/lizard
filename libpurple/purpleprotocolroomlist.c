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
 * GNU General Public License for more dteails.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "purpleprotocolroomlist.h"

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_INTERFACE(PurpleProtocolRoomlist, purple_protocol_roomlist,
                   PURPLE_TYPE_PROTOCOL)

static void
purple_protocol_roomlist_default_init(G_GNUC_UNUSED PurpleProtocolRoomlistInterface *iface)
{
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleRoomlist *
purple_protocol_roomlist_get_list(PurpleProtocolRoomlist *protocol_roomlist,
                                  PurpleConnection *gc)
{
	PurpleProtocolRoomlistInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_ROOMLIST(protocol_roomlist), NULL);
	g_return_val_if_fail(PURPLE_IS_CONNECTION(gc), NULL);

	iface = PURPLE_PROTOCOL_ROOMLIST_GET_IFACE(protocol_roomlist);
	if(iface != NULL && iface->get_list != NULL) {
		return iface->get_list(protocol_roomlist, gc);
	}

	return NULL;
}

void
purple_protocol_roomlist_cancel(PurpleProtocolRoomlist *protocol_roomlist,
                                PurpleRoomlist *list)
{
	PurpleProtocolRoomlistInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_ROOMLIST(protocol_roomlist));
	g_return_if_fail(PURPLE_IS_ROOMLIST(list));

	iface = PURPLE_PROTOCOL_ROOMLIST_GET_IFACE(protocol_roomlist);
	if(iface != NULL && iface->cancel != NULL) {
		iface->cancel(protocol_roomlist, list);
	}
}

void
purple_protocol_roomlist_expand_category(PurpleProtocolRoomlist *protocol_roomlist,
                                         PurpleRoomlist *list,
                                         PurpleRoomlistRoom *category)
{
	PurpleProtocolRoomlistInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_ROOMLIST(protocol_roomlist));
	g_return_if_fail(PURPLE_IS_ROOMLIST(list));

	iface = PURPLE_PROTOCOL_ROOMLIST_GET_IFACE(protocol_roomlist);
	if(iface != NULL && iface->expand_category != NULL) {
		iface->expand_category(protocol_roomlist, list, category);
	}
}

gchar *
purple_protocol_roomlist_room_serialize(PurpleProtocolRoomlist *protocol_roomlist,
                                        PurpleRoomlistRoom *room)
{
	PurpleProtocolRoomlistInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_ROOMLIST(protocol_roomlist), NULL);
	g_return_val_if_fail(room != NULL, NULL);

	iface = PURPLE_PROTOCOL_ROOMLIST_GET_IFACE(protocol_roomlist);
	if(iface != NULL && iface->room_serialize != NULL) {
		return iface->room_serialize(protocol_roomlist, room);
	}

	return NULL;
}
