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

#include "purpleroomlistroom.h"

typedef struct {
	gchar *name;
	gchar *description;
	gchar *category;
	guint user_count;

	GHashTable *components;
} PurpleRoomlistRoomPrivate;

enum {
	PROP_0,
	PROP_NAME,
	PROP_DESCRIPTION,
	PROP_CATEGORY,
	PROP_USER_COUNT,
	N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

G_DEFINE_TYPE_WITH_PRIVATE(PurpleRoomlistRoom, purple_roomlist_room,
                           G_TYPE_OBJECT)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_roomlist_room_set_name(PurpleRoomlistRoom *room, const gchar *name) {
	PurpleRoomlistRoomPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_ROOMLIST_ROOM(room));

	priv = purple_roomlist_room_get_instance_private(room);

	g_clear_pointer(&priv->name, g_free);
	priv->name = g_strdup(name);

	g_object_notify_by_pspec(G_OBJECT(room), properties[PROP_NAME]);
}

static void
purple_roomlist_room_set_description(PurpleRoomlistRoom *room,
                                     const gchar *description)
{
	PurpleRoomlistRoomPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_ROOMLIST_ROOM(room));

	priv = purple_roomlist_room_get_instance_private(room);

	g_clear_pointer(&priv->description, g_free);
	priv->description = g_strdup(description);

	g_object_notify_by_pspec(G_OBJECT(room), properties[PROP_DESCRIPTION]);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_roomlist_room_get_property(GObject *obj, guint param_id, GValue *value,
                                  GParamSpec *pspec)
{
	PurpleRoomlistRoom *room = PURPLE_ROOMLIST_ROOM(obj);

	switch(param_id) {
		case PROP_NAME:
			g_value_set_string(value, purple_roomlist_room_get_name(room));
			break;
		case PROP_DESCRIPTION:
			g_value_set_string(value,
			                   purple_roomlist_room_get_description(room));
			break;
		case PROP_CATEGORY:
			g_value_set_string(value, purple_roomlist_room_get_category(room));
			break;
		case PROP_USER_COUNT:
			g_value_set_uint(value, purple_roomlist_room_get_user_count(room));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_roomlist_room_set_property(GObject *obj, guint param_id,
                                  const GValue *value, GParamSpec *pspec)
{
	PurpleRoomlistRoom *room = PURPLE_ROOMLIST_ROOM(obj);

	switch(param_id) {
		case PROP_NAME:
			purple_roomlist_room_set_name(room, g_value_get_string(value));
			break;
		case PROP_DESCRIPTION:
			purple_roomlist_room_set_description(room,
			                                     g_value_get_string(value));
			break;
		case PROP_CATEGORY:
			purple_roomlist_room_set_category(room, g_value_get_string(value));
			break;
		case PROP_USER_COUNT:
			purple_roomlist_room_set_user_count(room, g_value_get_uint(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_roomlist_room_finalize(GObject *obj) {
	PurpleRoomlistRoom *room = NULL;
	PurpleRoomlistRoomPrivate *priv = NULL;;

	room = PURPLE_ROOMLIST_ROOM(obj);
	priv = purple_roomlist_room_get_instance_private(room);

	g_clear_pointer(&priv->name, g_free);
	g_clear_pointer(&priv->description, g_free);
	g_clear_pointer(&priv->category, g_free);

	g_clear_pointer(&priv->components, g_hash_table_destroy);

	G_OBJECT_CLASS(purple_roomlist_room_parent_class)->finalize(obj);
}

static void
purple_roomlist_room_init(PurpleRoomlistRoom *room) {
	PurpleRoomlistRoomPrivate *priv = NULL;

	priv = purple_roomlist_room_get_instance_private(room);

	priv->components = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
	                                         g_free);
}

static void
purple_roomlist_room_class_init(PurpleRoomlistRoomClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = purple_roomlist_room_get_property;
	obj_class->set_property = purple_roomlist_room_set_property;
	obj_class->finalize = purple_roomlist_room_finalize;

	properties[PROP_NAME] = g_param_spec_string(
		"name", "name",
		"The name of the room",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	properties[PROP_DESCRIPTION] = g_param_spec_string(
		"description", "description",
		"The description of the room",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	properties[PROP_CATEGORY] = g_param_spec_string(
		"category", "category",
		"The category of the room",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	properties[PROP_USER_COUNT] = g_param_spec_uint(
		"user-count", "user-count",
		"The user count of the room",
		0, G_MAXUINT, 0,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleRoomlistRoom *
purple_roomlist_room_new(const gchar *name, const gchar *description) {
	return g_object_new(
		PURPLE_TYPE_ROOMLIST_ROOM,
		"name", name,
		"description", description,
		NULL);
}

const gchar *
purple_roomlist_room_get_name(PurpleRoomlistRoom *room) {
	PurpleRoomlistRoomPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_ROOMLIST_ROOM(room), NULL);

	priv = purple_roomlist_room_get_instance_private(room);

	return priv->name;
}

const gchar *
purple_roomlist_room_get_description(PurpleRoomlistRoom *room) {
	PurpleRoomlistRoomPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_ROOMLIST_ROOM(room), NULL);

	priv = purple_roomlist_room_get_instance_private(room);

	return priv->description;
}

const gchar *
purple_roomlist_room_get_category(PurpleRoomlistRoom *room) {
	PurpleRoomlistRoomPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_ROOMLIST_ROOM(room), NULL);

	priv = purple_roomlist_room_get_instance_private(room);

	return priv->category;
}

void
purple_roomlist_room_set_category(PurpleRoomlistRoom *room,
                                  const gchar *category)
{
	PurpleRoomlistRoomPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_ROOMLIST_ROOM(room));

	priv = purple_roomlist_room_get_instance_private(room);

	g_clear_pointer(&priv->category, g_free);
	priv->category = g_strdup(category);

	g_object_notify_by_pspec(G_OBJECT(room), properties[PROP_CATEGORY]);
}

guint
purple_roomlist_room_get_user_count(PurpleRoomlistRoom *room) {
	PurpleRoomlistRoomPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_ROOMLIST_ROOM(room), 0);

	priv = purple_roomlist_room_get_instance_private(room);

	return priv->user_count;
}

void
purple_roomlist_room_set_user_count(PurpleRoomlistRoom *room,
                                    guint user_count)
{
	PurpleRoomlistRoomPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_ROOMLIST_ROOM(room));

	priv = purple_roomlist_room_get_instance_private(room);

	priv->user_count = user_count;

	g_object_notify_by_pspec(G_OBJECT(room), properties[PROP_USER_COUNT]);
}

void
purple_roomlist_room_add_field(PurpleRoomlistRoom *room, const gchar *field,
                               const gchar *value)
{
	PurpleRoomlistRoomPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_ROOMLIST_ROOM(room));
	g_return_if_fail(field != NULL);
	g_return_if_fail(value != NULL);

	priv = purple_roomlist_room_get_instance_private(room);

	g_hash_table_replace(priv->components, (gpointer)field, g_strdup(value));
}

const gchar *
purple_roomlist_room_get_field(PurpleRoomlistRoom *room, const gchar *field) {
	PurpleRoomlistRoomPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_ROOMLIST_ROOM(room), NULL);
	g_return_val_if_fail(field != NULL, NULL);

	priv = purple_roomlist_room_get_instance_private(room);

	return g_hash_table_lookup(priv->components, field);
}

GHashTable *
purple_roomlist_room_get_components(PurpleRoomlistRoom *room) {
	PurpleRoomlistRoomPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_ROOMLIST_ROOM(room), NULL);

	priv = purple_roomlist_room_get_instance_private(room);

	return priv->components;
}
