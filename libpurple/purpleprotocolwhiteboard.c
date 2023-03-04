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

#include "purpleprotocolwhiteboard.h"

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_INTERFACE(PurpleProtocolWhiteboard, purple_protocol_whiteboard,
                   PURPLE_TYPE_PROTOCOL)

static void
purple_protocol_whiteboard_default_init(G_GNUC_UNUSED PurpleProtocolWhiteboardInterface *iface)
{
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleWhiteboard *
purple_protocol_whiteboard_create(PurpleProtocolWhiteboard *whiteboard,
                                  PurpleAccount *account, const gchar *who,
                                  gint state)
{
	PurpleProtocolWhiteboardInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_WHITEBOARD(whiteboard), NULL);

	iface = PURPLE_PROTOCOL_WHITEBOARD_GET_IFACE(whiteboard);
	if(iface != NULL && iface->create != NULL) {
		return iface->create(whiteboard, account, who, state);
	}

	return NULL;
}
