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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_ROOMLIST_ROOM_H
#define PURPLE_ROOMLIST_ROOM_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PURPLE_TYPE_ROOMLIST_ROOM (purple_roomlist_room_get_type())

/**
 * purple_roomlist_room_get_type:
 *
 * Gets the #GType of #PurpleRoomlistRoom.
 *
 * Returns: The #GType of #PurpleRoomlistRoom.
 *
 * Since: 3.0.0
 */

/**
 * PurpleRoomlistRoom:
 *
 * #PurpleRoomlistRoom keeps track of all #PurpleConversation's inside
 * of libpurple and allows searching of them.
 *
 * Since: 3.0.0
 */
G_DECLARE_DERIVABLE_TYPE(PurpleRoomlistRoom, purple_roomlist_room, PURPLE,
						 ROOMLIST_ROOM, GObject)

struct _PurpleRoomlistRoomClass {
	/*< private >*/
	GObjectClass parent;

	gpointer reserved[4];
};

/**
 * purple_roomlist_room_new:
 * @name: The name for the room.
 * @description: The description or topic of the room.
 *
 * Creates a new room to be added to a [class@Purple.Roomlist].
 *
 * Since: 3.0.0
 */
PurpleRoomlistRoom *purple_roomlist_room_new(const gchar *name, const gchar *description);

/**
 * purple_roomlist_room_get_name:
 * @room: The instance.
 *
 * Gets the name of @room.
 *
 * Returns: The name of @room.
 *
 * Since: 3.0.0
 */
const gchar *purple_roomlist_room_get_name(PurpleRoomlistRoom *room);

/**
 * purple_roomlist_room_get_description:
 * @room: The instance.
 *
 * Gets the description of @room.
 *
 * Returns: The description of @room.
 *
 * Since: 3.0.0
 */
const gchar *purple_roomlist_room_get_description(PurpleRoomlistRoom *room);

/**
 * purple_roomlist_get_category:
 * @room: The instance.
 *
 * Gets the category of @room. It is up to the user interface on whether or not
 * this will be used.
 *
 * Returns: The category of @room if set otherwise %NULL.
 *
 * Since: 3.0.0
 */
const gchar *purple_roomlist_room_get_category(PurpleRoomlistRoom *room);

/**
 * purple_roomlist_room_set_category:
 * @room: The instance.
 * @category: (nullable): The new category.
 *
 * Sets the category of @room.
 *
 * Since: 3.0.0
 */
void purple_roomlist_room_set_category(PurpleRoomlistRoom *room, const gchar *category);

/**
 * purple_roomlist_room_get_user_count:
 * @room: The instance.
 *
 * Gets the number of users in @room.
 *
 * Returns: The number of users in @room if set, otherwise 0.
 *
 * Since: 3.0.0
 */
guint purple_roomlist_room_get_user_count(PurpleRoomlistRoom *room);

/**
 * purple_roomlist_room_set_user_count:
 * @room: The instance.
 * @user_count: The new user count.
 *
 * Sets the user count of @room to @user_count.
 *
 * Since: 3.0.0
 */
void purple_roomlist_room_set_user_count(PurpleRoomlistRoom *room, guint user_count);

/**
 * purple_roomlist_room_add_field:
 * @room: This instance.
 * @field: The name of the field. This should be a static string.
 * @value: The value of the field. This should be a copy of the value.
 *
 * Adds a new field to @room with the name of @field and value of @value.
 *
 * Since: 3.0.0
 */
void purple_roomlist_room_add_field(PurpleRoomlistRoom *room, const gchar *field, const gchar *value);

/**
 * purple_roomlist_room_get_field:
 * @room: The instance.
 * @field: The name of the field to get.
 *
 * Gets the value of the field named @field in @room.
 *
 * Returns: The value of @field.
 *
 * Since: 3.0.0
 */
const gchar *purple_roomlist_room_get_field(PurpleRoomlistRoom *room, const gchar *field);

/**
 * purple_roomlist_room_get_components:
 * @room: The instance.
 *
 * Gets the components that can be passed to purple_serv_join_chat() to join
 * the room.
 *
 * Returns: (transfer none): The components used to join the room.
 *
 * Since: 3.0.0
 */
GHashTable *purple_roomlist_room_get_components(PurpleRoomlistRoom *room);

G_END_DECLS

#endif /* PURPLE_ROOMLIST_ROOM_H */
