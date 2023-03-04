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

#ifndef PURPLE_ROOMLIST_H
#define PURPLE_ROOMLIST_H

#define PURPLE_TYPE_ROOMLIST (purple_roomlist_get_type())
typedef struct _PurpleRoomlist PurpleRoomlist;

#define PURPLE_TYPE_ROOMLIST_FIELD (purple_roomlist_field_get_type())
typedef struct _PurpleRoomlistField PurpleRoomlistField;

#define PURPLE_TYPE_ROOMLIST_UI_OPS (purple_roomlist_ui_ops_get_type())
typedef struct _PurpleRoomlistUiOps PurpleRoomlistUiOps;

/**
 * PurpleRoomlistFieldType:
 * @PURPLE_ROOMLIST_FIELD_BOOL: The field is a boolean.
 * @PURPLE_ROOMLIST_FIELD_INT: The field is an integer.
 * @PURPLE_ROOMLIST_FIELD_STRING: We do a g_strdup on the passed value if it's
 *                                this type.
 *
 * The types of fields.
 */
typedef enum
{
	PURPLE_ROOMLIST_FIELD_BOOL,
	PURPLE_ROOMLIST_FIELD_INT,
	PURPLE_ROOMLIST_FIELD_STRING

} PurpleRoomlistFieldType;

#include "account.h"
#include <glib.h>
#include "purpleroomlistroom.h"

/**************************************************************************/
/* Data Structures                                                        */
/**************************************************************************/

/**
 * PurpleRoomlistUiOps:
 * @show_with_account: Force the ui to pop up a dialog and get the list.
 * @create:            A new list was created.
 * @set_fields:        Sets the columns.
 * @add_room:          Add a room to the list.
 *
 * The room list ops to be filled out by the UI.
 */
struct _PurpleRoomlistUiOps {
	void (*show_with_account)(PurpleAccount *account);
	void (*create)(PurpleRoomlist *list);
	void (*set_fields)(PurpleRoomlist *list, GList *fields);
	void (*add_room)(PurpleRoomlist *list, PurpleRoomlistRoom *room);

	/*< private >*/
	void (*_purple_reserved1)(void);
	void (*_purple_reserved2)(void);
	void (*_purple_reserved3)(void);
	void (*_purple_reserved4)(void);
};

/**
 * PurpleRoomlist:
 *
 * Represents a list of rooms for a given connection on a given protocol.
 */
struct _PurpleRoomlist {
	GObject gparent;
};

G_BEGIN_DECLS

/**************************************************************************/
/* Room List API                                                          */
/**************************************************************************/

/**
 * purple_roomlist_get_type:
 *
 * The standard _get_type function for #PurpleRoomlist.
 *
 * Returns: The #GType for the Room List object.
 */
G_DECLARE_FINAL_TYPE(PurpleRoomlist, purple_roomlist, PURPLE, ROOMLIST, GObject)

/**
 * purple_roomlist_show_with_account:
 * @account: The account to get the list on.
 *
 * This is used to get the room list on an account, asking the UI
 * to pop up a dialog with the specified account already selected,
 * and pretend the user clicked the get list button.
 * While we're pretending, predend I didn't say anything about dialogs
 * or buttons, since this is the core.
 */
void purple_roomlist_show_with_account(PurpleAccount *account);

/**
 * purple_roomlist_new:
 * @account: The account that's listing rooms.
 *
 * Returns a newly created room list object.
 *
 * Returns: The new room list handle.
 */
PurpleRoomlist *purple_roomlist_new(PurpleAccount *account);

/**
 * purple_roomlist_get_account:
 * @list: The room list.
 *
 * Retrieve the PurpleAccount that was given when the room list was
 * created.
 *
 * Returns: (transfer none): The PurpleAccount tied to this room list.
 */
PurpleAccount *purple_roomlist_get_account(PurpleRoomlist *list);

/**
 * purple_roomlist_set_fields:
 * @list: The room list.
 * @fields: (element-type PurpleRoomlistField) (transfer full): UI's are
 *          encouraged to default to displaying these fields in the order given.
 *
 * Set the different field types and their names for this protocol.
 *
 * This must be called before purple_roomlist_room_add().
 */
void purple_roomlist_set_fields(PurpleRoomlist *list, GList *fields);

/**
 * purple_roomlist_set_in_progress:
 * @list: The room list.
 * @in_progress: We're downloading it, or we're not.
 *
 * Set the "in progress" state of the room list.
 *
 * The UI is encouraged to somehow hint to the user
 * whether or not we're busy downloading a room list or not.
 */
void purple_roomlist_set_in_progress(PurpleRoomlist *list, gboolean in_progress);

/**
 * purple_roomlist_get_in_progress:
 * @list: The room list.
 *
 * Gets the "in progress" state of the room list.
 *
 * The UI is encouraged to somehow hint to the user
 * whether or not we're busy downloading a room list or not.
 *
 * Returns: True if we're downloading it, or false if we're not.
 */
gboolean purple_roomlist_get_in_progress(PurpleRoomlist *list);

/**
 * purple_roomlist_room_add:
 * @list: The room list.
 * @room: The room to add to the list. The GList of fields must be in the same
               order as was given in purple_roomlist_set_fields().
 *
 * Adds a room to the list of them.
*/
void purple_roomlist_room_add(PurpleRoomlist *list, PurpleRoomlistRoom *room);

/**
 * purple_roomlist_get_list:
 * @gc: The PurpleConnection to have get a list.
 *
 * Returns a PurpleRoomlist structure from the protocol, and
 * instructs the protocol to start fetching the list.
 *
 * Returns: (transfer full): A PurpleRoomlist* or %NULL if the protocol doesn't
 *          support that.
 */
PurpleRoomlist *purple_roomlist_get_list(PurpleConnection *gc);

/**
 * purple_roomlist_cancel_get_list:
 * @list: The room list to cancel a get_list on.
 *
 * Tells the protocol to stop fetching the list.
 * If this is possible and done, the protocol will
 * call set_in_progress with %FALSE and possibly
 * unref the list if it took a reference.
 */
void purple_roomlist_cancel_get_list(PurpleRoomlist *list);

/**
 * purple_roomlist_join_room:
 * @list: The room list whose room to join.
 * @room: The room to join.
 *
 * Create a new conversation for @room.
 *
 * Since: 3.0.0
 */
void purple_roomlist_join_room(PurpleRoomlist *list, PurpleRoomlistRoom *room);

/**************************************************************************/
/* UI Registration Functions                                              */
/**************************************************************************/

/**
 * purple_roomlist_ui_ops_get_type:
 *
 * The standard _get_type function for #PurpleRoomlistUiOps.
 *
 * Returns: The #GType for the #PurpleRoomlistUiOps boxed structure.
 */
GType purple_roomlist_ui_ops_get_type(void);

/**
 * purple_roomlist_set_ui_ops:
 * @ops: The UI operations structure.
 *
 * Sets the UI operations structure to be used in all purple room lists.
 */
void purple_roomlist_set_ui_ops(PurpleRoomlistUiOps *ops);

/**
 * purple_roomlist_get_ui_ops:
 *
 * Returns the purple window UI operations structure to be used in
 * new windows.
 *
 * Returns: A filled-out PurpleRoomlistUiOps structure.
 */
PurpleRoomlistUiOps *purple_roomlist_get_ui_ops(void);

G_END_DECLS

#endif /* PURPLE_ROOMLIST_H */
