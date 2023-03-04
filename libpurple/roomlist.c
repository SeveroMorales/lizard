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

#include "account.h"
#include "connection.h"
#include "debug.h"
#include "roomlist.h"
#include "server.h"

/* This must be after roomlist.h otherwise you'll get an include cycle. */
#include "purpleprotocolroomlist.h"

/*
 * Private data for a room list.
 */
typedef struct {
	PurpleAccount *account;  /* The account this list belongs to. */
	GList *rooms;            /* The list of rooms.                */
	gboolean in_progress;    /* The listing is in progress.       */
} PurpleRoomlistPrivate;

/* Room list property enums */
enum
{
	PROP_0,
	PROP_ACCOUNT,
	PROP_IN_PROGRESS,
	PROP_LAST
};

static GParamSpec *properties[PROP_LAST];
static PurpleRoomlistUiOps *ops = NULL;

G_DEFINE_TYPE_WITH_PRIVATE(PurpleRoomlist, purple_roomlist, G_TYPE_OBJECT);

/**************************************************************************/
/* Room List API                                                          */
/**************************************************************************/

void purple_roomlist_show_with_account(PurpleAccount *account)
{
	if (ops && ops->show_with_account)
		ops->show_with_account(account);
}

PurpleAccount *purple_roomlist_get_account(PurpleRoomlist *list)
{
	PurpleRoomlistPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_ROOMLIST(list), NULL);

	priv = purple_roomlist_get_instance_private(list);
	return priv->account;
}

void purple_roomlist_set_in_progress(PurpleRoomlist *list, gboolean in_progress)
{
	PurpleRoomlistPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_ROOMLIST(list));

	priv = purple_roomlist_get_instance_private(list);
	priv->in_progress = in_progress;

	g_object_notify_by_pspec(G_OBJECT(list), properties[PROP_IN_PROGRESS]);
}

gboolean purple_roomlist_get_in_progress(PurpleRoomlist *list)
{
	PurpleRoomlistPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_ROOMLIST(list), FALSE);

	priv = purple_roomlist_get_instance_private(list);
	return priv->in_progress;
}

void purple_roomlist_room_add(PurpleRoomlist *list, PurpleRoomlistRoom *room)
{
	PurpleRoomlistPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_ROOMLIST(list));
	g_return_if_fail(room != NULL);

	priv = purple_roomlist_get_instance_private(list);
	priv->rooms = g_list_append(priv->rooms, room);

	if (ops && ops->add_room)
		ops->add_room(list, room);
}

PurpleRoomlist *purple_roomlist_get_list(PurpleConnection *gc)
{
	PurpleProtocol *protocol = NULL;

	g_return_val_if_fail(PURPLE_IS_CONNECTION(gc), NULL);
	g_return_val_if_fail(PURPLE_CONNECTION_IS_CONNECTED(gc), NULL);

	protocol = purple_connection_get_protocol(gc);

	if(PURPLE_IS_PROTOCOL_ROOMLIST(protocol)) {
		return purple_protocol_roomlist_get_list(PURPLE_PROTOCOL_ROOMLIST(protocol),
		                                         gc);
	}

	return NULL;
}

void purple_roomlist_cancel_get_list(PurpleRoomlist *list)
{
	PurpleRoomlistPrivate *priv = NULL;
	PurpleProtocol *protocol = NULL;
	PurpleConnection *gc;

	g_return_if_fail(PURPLE_IS_ROOMLIST(list));

	priv = purple_roomlist_get_instance_private(list);

	gc = purple_account_get_connection(priv->account);
	g_return_if_fail(PURPLE_IS_CONNECTION(gc));

	if(gc)
		protocol = purple_connection_get_protocol(gc);

	if(PURPLE_IS_PROTOCOL_ROOMLIST(protocol)) {
		purple_protocol_roomlist_cancel(PURPLE_PROTOCOL_ROOMLIST(protocol), list);
	}
}

void
purple_roomlist_join_room(PurpleRoomlist *list, PurpleRoomlistRoom *room) {
	PurpleRoomlistPrivate *priv = NULL;
	PurpleConnection *connection = NULL;
	GHashTable *components = NULL, *adjusted = NULL;
	GHashTableIter iter;
	const gchar *name = NULL;
	gpointer key, value;

	g_return_if_fail(PURPLE_IS_ROOMLIST(list));
	g_return_if_fail(PURPLE_IS_ROOMLIST_ROOM(room));

	priv = purple_roomlist_get_instance_private(list);

	connection = purple_account_get_connection(priv->account);
	if(connection == NULL) {
		return;
	}

	components = purple_roomlist_room_get_components(room);

	/* Make a copy of the components as we make sure the name is included. */
	adjusted = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_iter_init(&iter, components);
	while(g_hash_table_iter_next(&iter, &key, &value)) {
		g_hash_table_insert(adjusted, key, value);
	}

	name = purple_roomlist_room_get_name(room);
	g_hash_table_replace(adjusted, "name", (gpointer)name);

	purple_serv_join_chat(connection, adjusted);

	g_hash_table_destroy(adjusted);
}

/**************************************************************************/
/* Room List GObject code                                                 */
/**************************************************************************/

/* Set method for GObject properties */
static void
purple_roomlist_set_property(GObject *obj, guint param_id, const GValue *value,
		GParamSpec *pspec)
{
	PurpleRoomlist *list = PURPLE_ROOMLIST(obj);
	PurpleRoomlistPrivate *priv =
			purple_roomlist_get_instance_private(list);

	switch (param_id) {
		case PROP_ACCOUNT:
			priv->account = g_value_get_object(value);
			break;
		case PROP_IN_PROGRESS:
			purple_roomlist_set_in_progress(list, g_value_get_boolean(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

/* Get method for GObject properties */
static void
purple_roomlist_get_property(GObject *obj, guint param_id, GValue *value,
		GParamSpec *pspec)
{
	PurpleRoomlist *list = PURPLE_ROOMLIST(obj);

	switch (param_id) {
		case PROP_ACCOUNT:
			g_value_set_object(value, purple_roomlist_get_account(list));
			break;
		case PROP_IN_PROGRESS:
			g_value_set_boolean(value, purple_roomlist_get_in_progress(list));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_roomlist_init(G_GNUC_UNUSED PurpleRoomlist *list)
{
}

/* Called when done constructing */
static void
purple_roomlist_constructed(GObject *object)
{
	PurpleRoomlist *list = PURPLE_ROOMLIST(object);

	G_OBJECT_CLASS(purple_roomlist_parent_class)->constructed(object);

	if (ops && ops->create)
		ops->create(list);
}

/* GObject finalize function */
static void
purple_roomlist_finalize(GObject *object)
{
	PurpleRoomlist *list = PURPLE_ROOMLIST(object);
	PurpleRoomlistPrivate *priv =
			purple_roomlist_get_instance_private(list);

	purple_debug_misc("roomlist", "destroying list %p\n", list);

	g_list_free_full(priv->rooms, g_object_unref);

	G_OBJECT_CLASS(purple_roomlist_parent_class)->finalize(object);
}

/* Class initializer function */
static void
purple_roomlist_class_init(PurpleRoomlistClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = purple_roomlist_finalize;
	obj_class->constructed = purple_roomlist_constructed;

	/* Setup properties */
	obj_class->get_property = purple_roomlist_get_property;
	obj_class->set_property = purple_roomlist_set_property;

	properties[PROP_ACCOUNT] = g_param_spec_object("account", "Account",
				"The account for the room list.",
				PURPLE_TYPE_ACCOUNT,
				G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				G_PARAM_STATIC_STRINGS);

	properties[PROP_IN_PROGRESS] = g_param_spec_boolean("in-progress",
				"In progress",
				"Whether the room list is being fetched.", FALSE,
				G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, PROP_LAST, properties);
}

PurpleRoomlist *purple_roomlist_new(PurpleAccount *account)
{
	return g_object_new(PURPLE_TYPE_ROOMLIST,
		"account", account,
		NULL
	);
}

/**************************************************************************/
/* UI Registration Functions                                              */
/**************************************************************************/

void purple_roomlist_set_ui_ops(PurpleRoomlistUiOps *ui_ops)
{
	ops = ui_ops;
}

PurpleRoomlistUiOps *purple_roomlist_get_ui_ops(void)
{
	return ops;
}

/**************************************************************************
 * UI Ops GBoxed code
 **************************************************************************/

static PurpleRoomlistUiOps *
purple_roomlist_ui_ops_copy(PurpleRoomlistUiOps *ops)
{
	PurpleRoomlistUiOps *ops_new;

	g_return_val_if_fail(ops != NULL, NULL);

	ops_new = g_new(PurpleRoomlistUiOps, 1);
	*ops_new = *ops;

	return ops_new;
}

GType
purple_roomlist_ui_ops_get_type(void)
{
	static GType type = 0;

	if (type == 0) {
		type = g_boxed_type_register_static("PurpleRoomlistUiOps",
				(GBoxedCopyFunc)purple_roomlist_ui_ops_copy,
				(GBoxedFreeFunc)g_free);
	}

	return type;
}
