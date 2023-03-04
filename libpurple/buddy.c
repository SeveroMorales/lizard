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

#include "buddy.h"

#include "debug.h"
#include "purplebuddypresence.h"
#include "purplecontactmanager.h"
#include "purpleconversationmanager.h"
#include "purpleprotocolclient.h"
#include "util.h"

typedef struct {
	gchar *id;
	gchar *name;
	gchar *local_alias;
	gchar *server_alias;

	gpointer proto_data;

	PurpleBuddyIcon *icon;
	PurpleAccount *account;
	PurplePresence *presence;

	PurpleMediaCaps media_caps;
} PurpleBuddyPrivate;

enum {
	PROP_0,
	PROP_ID,
	PROP_NAME,
	PROP_LOCAL_ALIAS,
	PROP_SERVER_ALIAS,
	PROP_ICON,
	PROP_ACCOUNT,
	PROP_PRESENCE,
	PROP_MEDIA_CAPS,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE(PurpleBuddy, purple_buddy, PURPLE_TYPE_BLIST_NODE)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_buddy_set_id(PurpleBuddy *buddy, const gchar *id) {
	PurpleBuddyPrivate *priv = purple_buddy_get_instance_private(buddy);

	g_free(priv->id);
	priv->id = g_strdup(id);

	g_object_notify_by_pspec(G_OBJECT(buddy), properties[PROP_ID]);
}

static void
purple_buddy_set_account(PurpleBuddy *buddy, PurpleAccount *account) {
	PurpleBuddyPrivate *priv = purple_buddy_get_instance_private(buddy);

	if(g_set_object(&priv->account, account)) {
		g_object_notify_by_pspec(G_OBJECT(buddy), properties[PROP_ACCOUNT]);
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_buddy_set_property(GObject *obj, guint param_id, const GValue *value,
                          GParamSpec *pspec)
{
	PurpleBuddy *buddy = PURPLE_BUDDY(obj);

	switch (param_id) {
		case PROP_ID:
			purple_buddy_set_id(buddy, g_value_get_string(value));
			break;
		case PROP_NAME:
			purple_buddy_set_name(buddy, g_value_get_string(value));
			break;
		case PROP_LOCAL_ALIAS:
			purple_buddy_set_local_alias(buddy, g_value_get_string(value));
			break;
		case PROP_SERVER_ALIAS:
			purple_buddy_set_server_alias(buddy, g_value_get_string(value));
			break;
		case PROP_ICON:
			purple_buddy_set_icon(buddy, g_value_get_pointer(value));
			break;
		case PROP_ACCOUNT:
			purple_buddy_set_account(buddy,
			                         PURPLE_ACCOUNT(g_value_get_object(value)));
			break;
		case PROP_MEDIA_CAPS:
			purple_buddy_set_media_caps(buddy, g_value_get_enum(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_buddy_get_property(GObject *obj, guint param_id, GValue *value,
                          GParamSpec *pspec)
{
	PurpleBuddy *buddy = PURPLE_BUDDY(obj);

	switch (param_id) {
		case PROP_ID:
			g_value_set_string(value, purple_buddy_get_id(buddy));
			break;
		case PROP_NAME:
			g_value_set_string(value, purple_buddy_get_name(buddy));
			break;
		case PROP_LOCAL_ALIAS:
			g_value_set_string(value, purple_buddy_get_local_alias(buddy));
			break;
		case PROP_SERVER_ALIAS:
			g_value_set_string(value, purple_buddy_get_server_alias(buddy));
			break;
		case PROP_ICON:
			g_value_set_pointer(value, purple_buddy_get_icon(buddy));
			break;
		case PROP_ACCOUNT:
			g_value_set_object(value, purple_buddy_get_account(buddy));
			break;
		case PROP_PRESENCE:
			g_value_set_object(value, purple_buddy_get_presence(buddy));
			break;
		case PROP_MEDIA_CAPS:
			g_value_set_enum(value, purple_buddy_get_media_caps(buddy));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_buddy_init(G_GNUC_UNUSED PurpleBuddy *buddy) {
}

static void
purple_buddy_constructed(GObject *object) {
	PurpleBuddy *buddy = PURPLE_BUDDY(object);
	PurpleBuddyPrivate *priv = purple_buddy_get_instance_private(buddy);

	G_OBJECT_CLASS(purple_buddy_parent_class)->constructed(object);

	if(priv->id == NULL) {
		/* If there is no id for the user, generate a SHA256 based on the
		 * account_id and the username.
		 */
		PurpleContactInfo *info = PURPLE_CONTACT_INFO(priv->account);
		GChecksum *sum = g_checksum_new(G_CHECKSUM_SHA256);
		const guchar *data = NULL;

		data = (const guchar *)purple_account_get_protocol_id(priv->account);
		g_checksum_update(sum, data, -1);

		data = (const guchar *)purple_contact_info_get_username(info);
		g_checksum_update(sum, data, -1);

		data = (const guchar *)priv->name;
		g_checksum_update(sum, data, -1);

		purple_buddy_set_id(buddy, g_checksum_get_string(sum));

		g_checksum_free(sum);
	}

	priv->presence = PURPLE_PRESENCE(purple_buddy_presence_new(buddy));
	purple_presence_set_status_active(priv->presence, "offline", TRUE);

	purple_blist_new_node(purple_blist_get_default(),
	                      PURPLE_BLIST_NODE(buddy));
}

static void
purple_buddy_dispose(GObject *object) {
	PurpleBuddyPrivate *priv = purple_buddy_get_instance_private(PURPLE_BUDDY(object));

	g_clear_pointer(&priv->icon, purple_buddy_icon_unref);
	g_clear_object(&priv->presence);

	G_OBJECT_CLASS(purple_buddy_parent_class)->dispose(object);
}

static void
purple_buddy_finalize(GObject *object) {
	PurpleBuddy *buddy = PURPLE_BUDDY(object);
	PurpleBuddyPrivate *priv = purple_buddy_get_instance_private(buddy);
	PurpleProtocol *protocol;

	/*
	 * Tell the owner protocol that we're about to free the buddy so it
	 * can free proto_data
	 */
	protocol = purple_account_get_protocol(priv->account);
	if(protocol) {
		purple_protocol_client_buddy_free(PURPLE_PROTOCOL_CLIENT(protocol),
		                                  buddy);
	}

	g_free(priv->id);
	g_free(priv->name);
	g_free(priv->local_alias);
	g_free(priv->server_alias);

	G_OBJECT_CLASS(purple_buddy_parent_class)->finalize(object);
}

static void purple_buddy_class_init(PurpleBuddyClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->constructed = purple_buddy_constructed;
	obj_class->dispose = purple_buddy_dispose;
	obj_class->finalize = purple_buddy_finalize;
	obj_class->get_property = purple_buddy_get_property;
	obj_class->set_property = purple_buddy_set_property;

	/**
	 * PurpleBuddy::id:
	 *
	 * A globally unique identifier for this specific buddy.
	 *
	 * If an id is not passed during instantiation a uuid4 string is set as the
	 * id.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ID] = g_param_spec_string(
		"id", "id",
		"The globally unique identifier of the buddy.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	properties[PROP_NAME] = g_param_spec_string(
		"name", "Name",
		"The name of the buddy.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	properties[PROP_LOCAL_ALIAS] = g_param_spec_string(
		"local-alias", "Local alias",
		"Local alias of thee buddy.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	properties[PROP_SERVER_ALIAS] = g_param_spec_string(
		"server-alias", "Server alias",
		"Server-side alias of the buddy.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	properties[PROP_ICON] = g_param_spec_pointer(
		"icon", "Buddy icon",
		"The icon for the buddy.",
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	properties[PROP_ACCOUNT] = g_param_spec_object(
		"account", "Account",
		"The account for the buddy.",
		PURPLE_TYPE_ACCOUNT,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	properties[PROP_PRESENCE] = g_param_spec_object(
		"presence", "Presence",
		"The status information for the buddy.",
		PURPLE_TYPE_PRESENCE,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	properties[PROP_MEDIA_CAPS] = g_param_spec_enum(
		"media-caps", "Media capabilities",
		"The media capabilities of the buddy.",
		PURPLE_MEDIA_TYPE_CAPS, PURPLE_MEDIA_CAPS_NONE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleBuddy *
purple_buddy_new(PurpleAccount *account, const gchar *name, const gchar *alias)
{
	PurpleBuddy *buddy = NULL;
	PurpleContactManager *manager = NULL;

	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);
	g_return_val_if_fail(name != NULL, NULL);

	buddy = g_object_new(
		PURPLE_TYPE_BUDDY,
		"account", account,
		"name", name,
		"local-alias", alias,
		NULL);

	manager = purple_contact_manager_get_default();
	G_GNUC_BEGIN_IGNORE_DEPRECATIONS
	purple_contact_manager_add_buddy(manager, buddy);
	G_GNUC_END_IGNORE_DEPRECATIONS

	return buddy;
}

const gchar *
purple_buddy_get_id(PurpleBuddy *buddy) {
	PurpleBuddyPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), NULL);

	priv = purple_buddy_get_instance_private(buddy);

	return priv->id;
}

void
purple_buddy_set_icon(PurpleBuddy *buddy, PurpleBuddyIcon *icon) {
	PurpleBuddyPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_BUDDY(buddy));

	priv = purple_buddy_get_instance_private(buddy);
	if(priv->icon == icon) {
		return;
	}

	g_clear_pointer(&priv->icon, purple_buddy_icon_unref);
	if(icon != NULL) {
		priv->icon = purple_buddy_icon_ref(icon);
	}

	g_object_notify_by_pspec(G_OBJECT(buddy), properties[PROP_ICON]);

	purple_signal_emit(purple_blist_get_handle(), "buddy-icon-changed", buddy);

	purple_blist_update_node(purple_blist_get_default(),
	                         PURPLE_BLIST_NODE(buddy));
}

PurpleBuddyIcon *
purple_buddy_get_icon(PurpleBuddy *buddy) {
	PurpleBuddyPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), NULL);

	priv = purple_buddy_get_instance_private(buddy);

	return priv->icon;
}

PurpleAccount *
purple_buddy_get_account(PurpleBuddy *buddy) {
	PurpleBuddyPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), NULL);

	priv = purple_buddy_get_instance_private(buddy);

	return priv->account;
}

void
purple_buddy_set_name(PurpleBuddy *buddy, const gchar *name) {
	PurpleBuddyList *blist = NULL;
	PurpleBuddyPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_BUDDY(buddy));

	priv = purple_buddy_get_instance_private(buddy);

	if(priv->name != NULL) {
		purple_blist_update_buddies_cache(buddy, name);
	}

	g_free(priv->name);
	priv->name = purple_utf8_strip_unprintables(name);

	g_object_notify_by_pspec(G_OBJECT(buddy), properties[PROP_NAME]);

	blist = purple_blist_get_default();
	purple_blist_save_node(blist, PURPLE_BLIST_NODE(buddy));
	purple_blist_update_node(blist, PURPLE_BLIST_NODE(buddy));
}

const gchar *
purple_buddy_get_name(PurpleBuddy *buddy) {
	PurpleBuddyPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), NULL);

	priv = purple_buddy_get_instance_private(buddy);

	return priv->name;
}

gpointer
purple_buddy_get_protocol_data(PurpleBuddy *buddy) {
	PurpleBuddyPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), NULL);

	priv = purple_buddy_get_instance_private(buddy);

	return priv->proto_data;
}

void
purple_buddy_set_protocol_data(PurpleBuddy *buddy, gpointer data) {
	PurpleBuddyPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_BUDDY(buddy));

	priv = purple_buddy_get_instance_private(buddy);

	priv->proto_data = data;
}

const gchar *
purple_buddy_get_alias_only(PurpleBuddy *buddy) {
	PurpleBuddyPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), NULL);

	priv = purple_buddy_get_instance_private(buddy);

	if((priv->local_alias != NULL) && (*priv->local_alias != '\0')) {
		return priv->local_alias;
	} else if((priv->server_alias != NULL) && (*priv->server_alias != '\0')) {
		return priv->server_alias;
	}

	return NULL;
}

const gchar *
purple_buddy_get_contact_alias(PurpleBuddy *buddy) {
	PurpleBuddyPrivate *priv = NULL;
	PurpleMetaContact *c = NULL;

	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), NULL);

	priv = purple_buddy_get_instance_private(buddy);

	/* Search for an alias for the buddy. In order of precedence: */
	/* The local buddy alias */
	if(priv->local_alias != NULL) {
		return priv->local_alias;
	}

	/* The contact alias */
	c = purple_buddy_get_contact(buddy);
	if((c != NULL) && (purple_meta_contact_get_alias(c) != NULL)) {
		return purple_meta_contact_get_alias(c);
	}

	/* The server alias */
	if((priv->server_alias) && (*priv->server_alias != '\0')) {
		return priv->server_alias;
	}

	/* The buddy's user name (i.e. no alias) */
	return priv->name;
}

const gchar *
purple_buddy_get_alias(PurpleBuddy *buddy) {
	PurpleBuddyPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), NULL);

	priv = purple_buddy_get_instance_private(buddy);

	/* Search for an alias for the buddy. In order of precedence: */
	/* The buddy alias */
	if(priv->local_alias != NULL) {
		return priv->local_alias;
	}

	/* The server alias */
	if((priv->server_alias) && (*priv->server_alias != '\0')) {
		return priv->server_alias;
	}

	/* The buddy's user name (i.e. no alias) */
	return priv->name;
}

void
purple_buddy_set_local_alias(PurpleBuddy *buddy, const gchar *alias) {
	PurpleBuddyList *blist = NULL;
	PurpleBuddyPrivate *priv = NULL;
	PurpleConversation *im = NULL;
	PurpleConversationManager *manager = NULL;
	gchar *old_alias = NULL, *new_alias = NULL;

	g_return_if_fail(PURPLE_IS_BUDDY(buddy));

	priv = purple_buddy_get_instance_private(buddy);

	if((alias != NULL) && (*alias != '\0')) {
		new_alias = purple_utf8_strip_unprintables(alias);
	}

	if(purple_strequal(priv->local_alias, new_alias)) {
		g_free(new_alias);
		return;
	}

	old_alias = priv->local_alias;

	if((new_alias != NULL) && (*new_alias != '\0')) {
		priv->local_alias = new_alias;
	} else {
		priv->local_alias = NULL;
		g_free(new_alias);
	}

	g_object_notify_by_pspec(G_OBJECT(buddy), properties[PROP_LOCAL_ALIAS]);

	blist = purple_blist_get_default();
	purple_blist_save_node(blist, PURPLE_BLIST_NODE(buddy));
	purple_blist_update_node(blist, PURPLE_BLIST_NODE(buddy));

	manager = purple_conversation_manager_get_default();
	im = purple_conversation_manager_find_im(manager, priv->account,
	                                         priv->name);
	if(PURPLE_IS_IM_CONVERSATION(im)) {
		purple_conversation_autoset_title(im);
	}

	purple_signal_emit(purple_blist_get_handle(), "blist-node-aliased", buddy,
	                   old_alias);
	g_free(old_alias);
}

const gchar *
purple_buddy_get_local_alias(PurpleBuddy *buddy) {
	PurpleBuddyPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), NULL);

	priv = purple_buddy_get_instance_private(buddy);

	return priv->local_alias;
}

void
purple_buddy_set_server_alias(PurpleBuddy *buddy, const gchar *alias) {
	PurpleBuddyList *blist = NULL;
	PurpleBuddyPrivate *priv = NULL;
	PurpleConversation *im = NULL;
	PurpleConversationManager *manager = NULL;
	gchar *old_alias = NULL, *new_alias = NULL;

	g_return_if_fail(PURPLE_IS_BUDDY(buddy));

	priv = purple_buddy_get_instance_private(buddy);

	if((alias != NULL) && (*alias != '\0') && g_utf8_validate(alias, -1, NULL))
	{
		new_alias = purple_utf8_strip_unprintables(alias);
	}

	if(purple_strequal(priv->server_alias, new_alias)) {
		g_free(new_alias);

		return;
	}

	old_alias = priv->server_alias;

	if((new_alias != NULL) && (*new_alias != '\0')) {
		priv->server_alias = new_alias;
	} else {
		priv->server_alias = NULL;
		g_free(new_alias);
	}

	g_object_notify_by_pspec(G_OBJECT(buddy), properties[PROP_SERVER_ALIAS]);

	blist = purple_blist_get_default();
	purple_blist_save_node(blist, PURPLE_BLIST_NODE(buddy));
	purple_blist_update_node(blist, PURPLE_BLIST_NODE(buddy));

	manager = purple_conversation_manager_get_default();
	im = purple_conversation_manager_find_im(manager, priv->account,
	                                         priv->name);
	if(PURPLE_IS_IM_CONVERSATION(im)) {
		purple_conversation_autoset_title(im);
	}

	purple_signal_emit(purple_blist_get_handle(), "blist-node-aliased", buddy,
	                   old_alias);
	g_free(old_alias);
}

const gchar *
purple_buddy_get_server_alias(PurpleBuddy *buddy) {
	PurpleBuddyPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), NULL);

	priv = purple_buddy_get_instance_private(buddy);

	if((priv->server_alias) && (*priv->server_alias != '\0')) {
	    return priv->server_alias;
	}

	return NULL;
}

PurpleMetaContact *
purple_buddy_get_contact(PurpleBuddy *buddy) {
	PurpleBlistNode *parent = NULL;

	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), NULL);

	parent = purple_blist_node_get_parent(PURPLE_BLIST_NODE(buddy));
	if(PURPLE_IS_META_CONTACT(parent)) {
		return PURPLE_META_CONTACT(parent);
	}

	return NULL;
}

PurplePresence *
purple_buddy_get_presence(PurpleBuddy *buddy) {
	PurpleBuddyPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), NULL);

	priv = purple_buddy_get_instance_private(buddy);

	return priv->presence;
}

void
purple_buddy_update_status(PurpleBuddy *buddy, PurpleStatus *old_status) {
	PurpleBuddyList *blist = NULL;
	PurpleBuddyPrivate *priv = NULL;
	PurpleStatus *status = NULL;
	gpointer handle = NULL;

	gboolean old_online = FALSE, new_online = FALSE;

	g_return_if_fail(PURPLE_IS_BUDDY(buddy));

	priv = purple_buddy_get_instance_private(buddy);
	status = purple_presence_get_active_status(priv->presence);
	blist = purple_blist_get_default();
	handle = purple_blist_get_handle();

	purple_debug_info("blistnodetypes", "Updating buddy status for %s (%s)\n",
	                  priv->name,
	                  purple_account_get_protocol_name(priv->account));

	old_online = purple_status_is_online(old_status);
	new_online = purple_status_is_online(status);

	if(old_online != new_online) {
		PurpleBlistNode *cnode = NULL, *gnode = NULL;
		PurpleMetaContact *contact = NULL;
		PurpleCountingNode *contact_counter = NULL, *group_counter = NULL;
		gint delta = 0, limit = 0;

		if(new_online) {
			purple_signal_emit(handle, "buddy-signed-on", buddy);
			delta = 1;
			limit = 1;
		} else {
			purple_blist_node_set_int(PURPLE_BLIST_NODE(buddy), "last_seen",
			                          time(NULL));

			purple_signal_emit(handle, "buddy-signed-off", buddy);
			delta = -1;
			limit = 0;
		}

		cnode = purple_blist_node_get_parent(PURPLE_BLIST_NODE(buddy));
		contact = PURPLE_META_CONTACT(cnode);
		contact_counter = PURPLE_COUNTING_NODE(contact);
		gnode = purple_blist_node_get_parent(cnode);
		group_counter = PURPLE_COUNTING_NODE(gnode);

		purple_counting_node_change_online_count(contact_counter, delta);
		if(purple_counting_node_get_online_count(contact_counter) == limit) {
			purple_counting_node_change_online_count(group_counter, delta);
		}
	} else {
		purple_signal_emit(handle, "buddy-status-changed", buddy, old_status,
		                   status);
	}

	/*
	 * This function used to only call the following two functions if one of
	 * the above signals had been triggered, but that's not good, because
	 * if someone's away message changes and they don't go from away to back
	 * to away then no signal is triggered.
	 *
	 * It's a safe assumption that SOMETHING called this function.  PROBABLY
	 * because something, somewhere changed.  Calling the stuff below
	 * certainly won't hurt anything.  Unless you're on a K6-2 300.
	 */
	purple_meta_contact_invalidate_priority_buddy(purple_buddy_get_contact(buddy));

	purple_blist_update_node(blist, PURPLE_BLIST_NODE(buddy));
}

PurpleMediaCaps
purple_buddy_get_media_caps(PurpleBuddy *buddy) {
	PurpleBuddyPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), 0);

	priv = purple_buddy_get_instance_private(buddy);

	return priv->media_caps;
}

void
purple_buddy_set_media_caps(PurpleBuddy *buddy, PurpleMediaCaps media_caps) {
	PurpleBuddyPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_BUDDY(buddy));

	priv = purple_buddy_get_instance_private(buddy);
	priv->media_caps = media_caps;

	g_object_notify_by_pspec(G_OBJECT(buddy), properties[PROP_MEDIA_CAPS]);
}

PurpleGroup *
purple_buddy_get_group(PurpleBuddy *buddy) {
	PurpleBlistNode *contact = NULL, *group = NULL;

	g_return_val_if_fail(PURPLE_IS_BUDDY(buddy), NULL);

	contact = purple_blist_node_get_parent(PURPLE_BLIST_NODE(buddy));
	if(!PURPLE_IS_META_CONTACT(contact)) {
		return purple_blist_get_default_group();
	}

	group = purple_blist_node_get_parent(contact);
	if(PURPLE_IS_GROUP(group)) {
		return PURPLE_GROUP(group);
	}

	return NULL;
}
