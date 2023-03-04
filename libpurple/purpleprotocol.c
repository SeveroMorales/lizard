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

#include "purpleprotocol.h"

#include "purpleaccountmanager.h"
#include "purpleenums.h"

enum {
	PROP_0,
	PROP_ID,
	PROP_NAME,
	PROP_DESCRIPTION,
	PROP_ICON_NAME,
	PROP_ICON_SEARCH_PATH,
	PROP_ICON_RESOURCE_PATH,
	PROP_OPTIONS,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = { NULL, };

typedef struct {
	gchar *id;
	gchar *name;
	gchar *description;

	gchar *icon_name;
	gchar *icon_search_path;
	gchar *icon_resource_path;

	PurpleProtocolOptions options;
} PurpleProtocolPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PurpleProtocol, purple_protocol,
                                    G_TYPE_OBJECT)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_protocol_set_id(PurpleProtocol *protocol, const gchar *id) {
	PurpleProtocolPrivate *priv = NULL;

	priv = purple_protocol_get_instance_private(protocol);
	g_free(priv->id);
	priv->id = g_strdup(id);

	g_object_notify_by_pspec(G_OBJECT(protocol), properties[PROP_ID]);
}

static void
purple_protocol_set_name(PurpleProtocol *protocol, const gchar *name) {
	PurpleProtocolPrivate *priv = NULL;

	priv = purple_protocol_get_instance_private(protocol);
	g_free(priv->name);
	priv->name = g_strdup(name);

	g_object_notify_by_pspec(G_OBJECT(protocol), properties[PROP_NAME]);
}

static void
purple_protocol_set_description(PurpleProtocol *protocol, const gchar *description) {
	PurpleProtocolPrivate *priv = NULL;

	priv = purple_protocol_get_instance_private(protocol);
	g_free(priv->description);
	priv->description = g_strdup(description);

	g_object_notify_by_pspec(G_OBJECT(protocol), properties[PROP_DESCRIPTION]);
}

static void
purple_protocol_set_icon_name(PurpleProtocol *protocol,
                              const gchar *icon_name)
{
	PurpleProtocolPrivate *priv = NULL;

	priv = purple_protocol_get_instance_private(protocol);

	g_free(priv->icon_name);
	priv->icon_name = g_strdup(icon_name);

	g_object_notify_by_pspec(G_OBJECT(protocol), properties[PROP_ICON_NAME]);
}

static void
purple_protocol_set_icon_search_path(PurpleProtocol *protocol,
                                     const gchar *path)
{
	PurpleProtocolPrivate *priv = NULL;

	priv = purple_protocol_get_instance_private(protocol);

	g_free(priv->icon_search_path);
	priv->icon_search_path = g_strdup(path);

	g_object_notify_by_pspec(G_OBJECT(protocol),
	                         properties[PROP_ICON_SEARCH_PATH]);
}

static void
purple_protocol_set_icon_resource_path(PurpleProtocol *protocol,
                                       const gchar *path)
{
	PurpleProtocolPrivate *priv = NULL;

	priv = purple_protocol_get_instance_private(protocol);

	g_free(priv->icon_resource_path);
	priv->icon_resource_path = g_strdup(path);

	g_object_notify_by_pspec(G_OBJECT(protocol),
	                         properties[PROP_ICON_RESOURCE_PATH]);
}

static void
purple_protocol_set_options(PurpleProtocol *protocol,
                            PurpleProtocolOptions options)
{
	PurpleProtocolPrivate *priv = NULL;

	priv = purple_protocol_get_instance_private(protocol);
	priv->options = options;

	g_object_notify_by_pspec(G_OBJECT(protocol), properties[PROP_OPTIONS]);
}

static void
purple_protocol_foreach_account_disconnect(PurpleAccount *account,
                                           gpointer data)
{
	const gchar *protocol_id = (const gchar *)data;

	/* I'm not sure that we can finalize a protocol plugin if an account is
	 * still using it..  Right now accounts don't ref protocols, but maybe
	 * they should?
	 */
	if(purple_account_is_disconnected(account)) {
		return;
	}

	if(purple_strequal(protocol_id, purple_account_get_protocol_id(account))) {
		purple_account_disconnect(account);
	}
}

/******************************************************************************
 * PurpleProtocol Implementation
 *****************************************************************************/
static PurpleConnection *
purple_protocol_default_create_connection(PurpleProtocol *protocol,
                                          PurpleAccount *account,
                                          const char *password,
                                          G_GNUC_UNUSED GError **error)
{
	return g_object_new(
		PURPLE_TYPE_CONNECTION,
		"protocol", protocol,
		"account", account,
		"password", password,
		NULL);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_protocol_get_property(GObject *obj, guint param_id, GValue *value,
                             GParamSpec *pspec)
{
	PurpleProtocol *protocol = PURPLE_PROTOCOL(obj);

	switch(param_id) {
		case PROP_ID:
			g_value_set_string(value, purple_protocol_get_id(protocol));
			break;
		case PROP_NAME:
			g_value_set_string(value, purple_protocol_get_name(protocol));
			break;
		case PROP_DESCRIPTION:
			g_value_set_string(value,
			                   purple_protocol_get_description(protocol));
			break;
		case PROP_ICON_NAME:
			g_value_set_string(value, purple_protocol_get_icon_name(protocol));
			break;
		case PROP_ICON_SEARCH_PATH:
			g_value_set_string(value,
			                   purple_protocol_get_icon_search_path(protocol));
			break;
		case PROP_ICON_RESOURCE_PATH:
			g_value_set_string(value,
			                   purple_protocol_get_icon_resource_path(protocol));
			break;
		case PROP_OPTIONS:
			g_value_set_flags(value, purple_protocol_get_options(protocol));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_protocol_set_property(GObject *obj, guint param_id, const GValue *value,
                             GParamSpec *pspec)
{
	PurpleProtocol *protocol = PURPLE_PROTOCOL(obj);

	switch(param_id) {
		case PROP_ID:
			purple_protocol_set_id(protocol, g_value_get_string(value));
			break;
		case PROP_NAME:
			purple_protocol_set_name(protocol, g_value_get_string(value));
			break;
		case PROP_DESCRIPTION:
			purple_protocol_set_description(protocol,
			                                g_value_get_string(value));
			break;
		case PROP_ICON_NAME:
			purple_protocol_set_icon_name(protocol, g_value_get_string(value));
			break;
		case PROP_ICON_SEARCH_PATH:
			purple_protocol_set_icon_search_path(protocol,
			                                     g_value_get_string(value));
			break;
		case PROP_ICON_RESOURCE_PATH:
			purple_protocol_set_icon_resource_path(protocol,
			                                       g_value_get_string(value));
			break;
		case PROP_OPTIONS:
			purple_protocol_set_options(protocol, g_value_get_flags(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_protocol_init(G_GNUC_UNUSED PurpleProtocol *protocol) {
}

static void
purple_protocol_finalize(GObject *object) {
	PurpleAccountManager *manager = purple_account_manager_get_default();
	PurpleProtocol *protocol = PURPLE_PROTOCOL(object);
	PurpleProtocolPrivate *priv = NULL;

	priv = purple_protocol_get_instance_private(protocol);

	purple_account_manager_foreach(manager,
	                               purple_protocol_foreach_account_disconnect,
	                               priv->id);

	g_clear_pointer(&priv->id, g_free);
	g_clear_pointer(&priv->name, g_free);
	g_clear_pointer(&priv->description, g_free);
	g_clear_pointer(&priv->icon_name, g_free);
	g_clear_pointer(&priv->icon_search_path, g_free);
	g_clear_pointer(&priv->icon_resource_path, g_free);

	/* these seem to be fallbacks if the subclass protocol doesn't do it's own
	 * clean up?  I kind of want to delete them... - gk 2021-03-03
	 */
	purple_request_close_with_handle(protocol);
	purple_notify_close_with_handle(protocol);

	purple_signals_disconnect_by_handle(protocol);
	purple_signals_unregister_by_instance(protocol);

	purple_prefs_disconnect_by_handle(protocol);

	G_OBJECT_CLASS(purple_protocol_parent_class)->finalize(object);
}

static void
purple_protocol_class_init(PurpleProtocolClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = purple_protocol_get_property;
	obj_class->set_property = purple_protocol_set_property;
	obj_class->finalize = purple_protocol_finalize;

	klass->create_connection = purple_protocol_default_create_connection;

	/**
	 * PurpleProtocol::id:
	 *
	 * The identifier for the protocol.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ID] = g_param_spec_string(
		"id", "id",
		"The identifier for the protocol",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleProtocol::name:
	 *
	 * The name to show in user interface for the protocol.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_NAME] = g_param_spec_string(
		"name", "name",
		"The name of the protocol to show in the user interface",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleProtocol::description:
	 *
	 * The description to show in user interface for the protocol.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_DESCRIPTION] = g_param_spec_string(
		"description", "description",
		"The description of the protocol to show in the user interface",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleProtocol::icon-name:
	 *
	 * The name of an icon that has been installed to either the path specified
	 * via PurpleProtocol::icon-search-path or
	 * PurpleProtocol::icon-resource-path.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ICON_NAME] = g_param_spec_string(
		"icon-name", "icon-name",
		"The name of the XDG icon.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleProtocol::icon-search-path:
	 *
	 * The path to an XDG Icon Theme directory which contains the icons for the
	 * protocol. See purple_protocol_get_icon_search_path() for more
	 * information.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ICON_SEARCH_PATH] = g_param_spec_string(
		"icon-search-path", "icon-search-path",
		"The path to an XDG Icon Theme directory.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleProtocol::icon-resource-path:
	 *
	 * A #GResource path which contains the icons for the protocol. See
	 * purple_protocol_get_icon_resource_path() for more information.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ICON_RESOURCE_PATH] = g_param_spec_string(
		"icon-resource-path", "icon-resource-path",
		"The GResource path to the icons.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleProtocol::options:
	 *
	 * The #PurpleProtocolOptions for the protocol.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_OPTIONS] = g_param_spec_flags(
		"options", "options",
		"The options for the protocol",
		PURPLE_TYPE_PROTOCOL_OPTIONS,
		0,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
const gchar *
purple_protocol_get_id(PurpleProtocol *protocol) {
	PurpleProtocolPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL(protocol), NULL);

	priv = purple_protocol_get_instance_private(protocol);

	return priv->id;
}

const gchar *
purple_protocol_get_name(PurpleProtocol *protocol) {
	PurpleProtocolPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL(protocol), NULL);

	priv = purple_protocol_get_instance_private(protocol);

	return priv->name;
}

const gchar *
purple_protocol_get_description(PurpleProtocol *protocol) {
	PurpleProtocolPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL(protocol), NULL);

	priv = purple_protocol_get_instance_private(protocol);

	return priv->description;
}

const gchar *
purple_protocol_get_icon_name(PurpleProtocol *protocol) {
	PurpleProtocolPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL(protocol), NULL);

	priv = purple_protocol_get_instance_private(protocol);

	return priv->icon_name;
}

const gchar *
purple_protocol_get_icon_search_path(PurpleProtocol *protocol) {
	PurpleProtocolPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL(protocol), NULL);

	priv = purple_protocol_get_instance_private(protocol);

	return priv->icon_search_path;
}

const gchar *
purple_protocol_get_icon_resource_path(PurpleProtocol *protocol) {
	PurpleProtocolPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL(protocol), NULL);

	priv = purple_protocol_get_instance_private(protocol);

	return priv->icon_resource_path;
}

PurpleProtocolOptions
purple_protocol_get_options(PurpleProtocol *protocol) {
	PurpleProtocolPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL(protocol), 0);

	priv = purple_protocol_get_instance_private(protocol);

	return priv->options;
}

GList *
purple_protocol_get_user_splits(PurpleProtocol *protocol) {
	PurpleProtocolClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL(protocol), NULL);

	klass = PURPLE_PROTOCOL_GET_CLASS(protocol);
	if(klass != NULL && klass->get_user_splits != NULL) {
		return klass->get_user_splits(protocol);
	}

	return NULL;
}

GList *
purple_protocol_get_account_options(PurpleProtocol *protocol) {
	PurpleProtocolClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL(protocol), NULL);

	klass = PURPLE_PROTOCOL_GET_CLASS(protocol);
	if(klass != NULL && klass->get_account_options != NULL) {
		return klass->get_account_options(protocol);
	}

	return NULL;
}

PurpleBuddyIconSpec *
purple_protocol_get_icon_spec(PurpleProtocol *protocol) {
	PurpleProtocolClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL(protocol), NULL);

	klass = PURPLE_PROTOCOL_GET_CLASS(protocol);
	if(klass != NULL && klass->get_buddy_icon_spec != NULL) {
		return klass->get_buddy_icon_spec(protocol);
	}

	return NULL;
}

PurpleWhiteboardOps *
purple_protocol_get_whiteboard_ops(PurpleProtocol *protocol) {
	PurpleProtocolClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL(protocol), NULL);

	klass = PURPLE_PROTOCOL_GET_CLASS(protocol);
	if(klass != NULL && klass->get_whiteboard_ops != NULL) {
		return klass->get_whiteboard_ops(protocol);
	}

	return NULL;
}

void
purple_protocol_login(PurpleProtocol *protocol, PurpleAccount *account) {
	PurpleProtocolClass *klass = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL(protocol));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	klass = PURPLE_PROTOCOL_GET_CLASS(protocol);
	if(klass != NULL && klass->login != NULL) {
		klass->login(protocol, account);
	}
}

void
purple_protocol_close(PurpleProtocol *protocol, PurpleConnection *gc) {
	PurpleProtocolClass *klass = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL(protocol));
	g_return_if_fail(PURPLE_IS_CONNECTION(gc));

	klass = PURPLE_PROTOCOL_GET_CLASS(protocol);
	if(klass != NULL && klass->close != NULL) {
		klass->close(protocol, gc);
	}
}

void
purple_protocol_can_connect_async(PurpleProtocol *protocol,
                                  PurpleAccount *account,
                                  GCancellable *cancellable,
                                  GAsyncReadyCallback callback,
                                  gpointer data)
{
	PurpleProtocolClass *klass = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL(protocol));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	klass = PURPLE_PROTOCOL_GET_CLASS(protocol);
	if(klass != NULL && klass->can_connect_async != NULL) {
		klass->can_connect_async(protocol, account, cancellable, callback,
		                         data);
	}
}

gboolean
purple_protocol_can_connect_finish(PurpleProtocol *protocol,
                                   GAsyncResult *result,
                                   GError **error)
{
	PurpleProtocolClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL(protocol), FALSE);

	klass = PURPLE_PROTOCOL_GET_CLASS(protocol);
	if(klass != NULL && klass->can_connect_finish != NULL) {
		return klass->can_connect_finish(protocol, result, error);
	}

	return FALSE;
}

PurpleConnection *
purple_protocol_create_connection(PurpleProtocol *protocol,
                                  PurpleAccount *account,
                                  const char *password,
                                  GError **error)
{
	PurpleProtocolClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL(protocol), NULL);
	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);

	klass = PURPLE_PROTOCOL_GET_CLASS(protocol);
	if(klass != NULL && klass->create_connection != NULL) {
		return klass->create_connection(protocol, account, password, error);
	}

	g_set_error(error, PURPLE_CONNECTION_ERROR, 0,
	            "Protocol %s did not implement create_connection",
	            purple_protocol_get_name(protocol));

	return NULL;
}

GList *
purple_protocol_get_status_types(PurpleProtocol *protocol,
                                 PurpleAccount *account)
{
	PurpleProtocolClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL(protocol), NULL);
	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);

	klass = PURPLE_PROTOCOL_GET_CLASS(protocol);
	if(klass != NULL && klass->status_types != NULL) {
		return klass->status_types(protocol, account);
	}

	return NULL;
}
