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

#ifndef PURPLE_PROTOCOL_WHITEBOARD_H
#define PURPLE_PROTOCOL_WHITEBOARD_H

#include <glib.h>
#include <glib-object.h>

#include <libpurple/account.h>
#include <libpurple/purplewhiteboard.h>

#define PURPLE_TYPE_PROTOCOL_WHITEBOARD (purple_protocol_whiteboard_get_type())
G_DECLARE_INTERFACE(PurpleProtocolWhiteboard, purple_protocol_whiteboard, PURPLE,
                    PROTOCOL_WHITEBOARD, PurpleProtocol)

/**
 * PurpleProtocolWhiteboard:
 *
 * The #PurpleProtocolWhiteboard interface defines the behavior of a protocol's
 * whiteboard interface.
 *
 * Since: 3.0.0
 */

/**
 * PurpleProtocolWhiteboardInterface:
 * @create: Creates a new whiteboard.
 *
 * The protocol whiteboard interface.
 *
 * This interface provides a gateway between purple and the protocol.
 *
 * Since: 3.0.0
 */
struct _PurpleProtocolWhiteboardInterface {
	/*< private >*/
	GTypeInterface parent;

	/*< public >*/
	PurpleWhiteboard *(*create)(PurpleProtocolWhiteboard *whiteboard, PurpleAccount *account, const gchar *who, gint state);

	/*< private >*/
	gpointer reserved[4];
};

G_BEGIN_DECLS

/**
 * purple_protocol_whiteboard_create:
 * @whiteboard: The instance.
 * @account: The [class@Purple.Account].
 * @who: The username of the contact who the whiteboard is being created for.
 * @state: The initial state of the whiteboard.
 *
 * Creates a new [class@Purple.Whiteboard].
 *
 * Returns: (transfer full): The new whiteboard instance.
 *
 * Since: 3.0.0
 */
PurpleWhiteboard *purple_protocol_whiteboard_create(PurpleProtocolWhiteboard *whiteboard, PurpleAccount *account, const gchar *who, gint state);

G_END_DECLS

#endif /* PURPLE_PROTOCOL_WHITEBOARD_H */
