/*
 * Purple - XMPP Service Disco Browser
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
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

#include <glib/gi18n-lib.h>

#include "xmppdiscoservice.h"
#include "xmppdiscoenums.h"

struct _XmppDiscoService {
	GObject parent;

	PidginDiscoList *list;
	GListStore *children;
	char *name;
	char *description;

	char *gateway_type;
	XmppDiscoServiceType type;
	XmppDiscoServiceFlags flags;

	char *jid;
	char *node;
	gboolean expanded;
};

enum {
	PROP_0,
	PROP_LIST,
	PROP_NAME,
	PROP_DESCRIPTION,
	PROP_SERVICE_TYPE,
	PROP_GATEWAY_TYPE,
	PROP_FLAGS,
	PROP_JID,
	PROP_NODE,
	PROP_EXPANDED,
	PROP_ICON_NAME,
	PROP_CHILD_MODEL,
	PROP_LAST
};

static GParamSpec *properties[PROP_LAST] = {NULL};

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
xmpp_disco_service_set_list(XmppDiscoService *service, PidginDiscoList *list) {
	service->list = list;

	g_object_notify_by_pspec(G_OBJECT(service), properties[PROP_LIST]);
}

static void
xmpp_disco_service_refresh_child_model(GObject *obj,
                                       G_GNUC_UNUSED GParamSpec *pspec,
                                       G_GNUC_UNUSED gpointer data)
{
	XmppDiscoService *service = XMPP_DISCO_SERVICE(obj);
	gboolean changed = FALSE;

	if((service->flags & XMPP_DISCO_BROWSE) != 0) {
		if(service->children == NULL) {
			service->children = g_list_store_new(XMPP_DISCO_TYPE_SERVICE);
			changed = TRUE;
		}
	} else {
		changed = service->children != NULL;
		g_clear_object(&service->children);
	}

	if(changed) {
		g_object_notify_by_pspec(G_OBJECT(service), properties[PROP_CHILD_MODEL]);
	}
}

/******************************************************************************
 * GObject implementation
 *****************************************************************************/
G_DEFINE_DYNAMIC_TYPE(XmppDiscoService, xmpp_disco_service, G_TYPE_OBJECT)

static void
xmpp_disco_service_get_property(GObject *object, guint prop_id, GValue *value,
                                GParamSpec *pspec)
{
	XmppDiscoService *service = XMPP_DISCO_SERVICE(object);

	switch(prop_id) {
		case PROP_LIST:
			g_value_set_pointer(value, xmpp_disco_service_get_list(service));
			break;
		case PROP_NAME:
			g_value_set_string(value, xmpp_disco_service_get_name(service));
			break;
		case PROP_DESCRIPTION:
			g_value_set_string(value,
			                   xmpp_disco_service_get_description(service));
			break;
		case PROP_SERVICE_TYPE:
			g_value_set_enum(value,
			                 xmpp_disco_service_get_service_type(service));
			break;
		case PROP_GATEWAY_TYPE:
			g_value_set_string(value,
			                   xmpp_disco_service_get_gateway_type(service));
			break;
		case PROP_FLAGS:
			g_value_set_flags(value, xmpp_disco_service_get_flags(service));
			break;
		case PROP_JID:
			g_value_set_string(value, xmpp_disco_service_get_jid(service));
			break;
		case PROP_NODE:
			g_value_set_string(value, xmpp_disco_service_get_node(service));
			break;
		case PROP_EXPANDED:
			g_value_set_boolean(value,
			                    xmpp_disco_service_get_expanded(service));
			break;
		case PROP_ICON_NAME:
			g_value_take_string(value,
			                    xmpp_disco_service_get_icon_name(service));
			break;
		case PROP_CHILD_MODEL:
			g_value_set_object(value,
			                   xmpp_disco_service_get_child_model(service));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
xmpp_disco_service_set_property(GObject *object, guint prop_id,
                                const GValue *value, GParamSpec *pspec)
{
	XmppDiscoService *service = XMPP_DISCO_SERVICE(object);

	switch(prop_id) {
		case PROP_LIST:
			xmpp_disco_service_set_list(service, g_value_get_pointer(value));
			break;
		case PROP_NAME:
			xmpp_disco_service_set_name(service, g_value_get_string(value));
			break;
		case PROP_DESCRIPTION:
			xmpp_disco_service_set_description(service,
			                                   g_value_get_string(value));
			break;
		case PROP_SERVICE_TYPE:
			xmpp_disco_service_set_service_type(service,
			                                    g_value_get_enum(value));
			break;
		case PROP_GATEWAY_TYPE:
			xmpp_disco_service_set_gateway_type(service,
			                                    g_value_get_string(value));
			break;
		case PROP_FLAGS:
			xmpp_disco_service_set_flags(service, g_value_get_flags(value));
			break;
		case PROP_JID:
			xmpp_disco_service_set_jid(service, g_value_get_string(value));
			break;
		case PROP_NODE:
			xmpp_disco_service_set_node(service, g_value_get_string(value));
			break;
		case PROP_EXPANDED:
			xmpp_disco_service_set_expanded(service,
			                                g_value_get_boolean(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
xmpp_disco_service_finalize(GObject *obj) {
	XmppDiscoService *service = XMPP_DISCO_SERVICE(obj);

	g_clear_pointer(&service->name, g_free);
	g_clear_pointer(&service->description, g_free);
	g_clear_pointer(&service->gateway_type, g_free);
	g_clear_pointer(&service->jid, g_free);
	g_clear_pointer(&service->node, g_free);
	g_clear_object(&service->children);

	G_OBJECT_CLASS(xmpp_disco_service_parent_class)->finalize(obj);
}

static void
xmpp_disco_service_class_init(XmppDiscoServiceClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	/* Properties */
	obj_class->get_property = xmpp_disco_service_get_property;
	obj_class->set_property = xmpp_disco_service_set_property;
	obj_class->finalize = xmpp_disco_service_finalize;

	properties[PROP_LIST] = g_param_spec_pointer(
	        "list", "list", "The list displaying this service.",
	        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	properties[PROP_NAME] = g_param_spec_string(
	        "name", "name", "The name of this service.",
	        NULL,
	        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	properties[PROP_DESCRIPTION] = g_param_spec_string(
	        "description", "description", "The description of this service.",
	        NULL,
	        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	properties[PROP_SERVICE_TYPE] = g_param_spec_enum(
	        "service-type", "service-type", "The service type of this service.",
	        XMPP_DISCO_TYPE_SERVICE_TYPE, XMPP_DISCO_SERVICE_TYPE_UNSET,
	        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	properties[PROP_GATEWAY_TYPE] = g_param_spec_string(
	        "gateway-type", "gateway-type", "The gateway type of this service.",
	        NULL,
	        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	properties[PROP_FLAGS] = g_param_spec_flags(
	        "flags", "flags", "The flags of this service.",
	        XMPP_DISCO_TYPE_SERVICE_FLAGS, XMPP_DISCO_NONE,
	        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	properties[PROP_JID] = g_param_spec_string(
	        "jid", "jid", "The jid of this service.",
	        NULL,
	        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	properties[PROP_NODE] = g_param_spec_string(
	        "node", "node", "The node of this service.",
	        NULL,
	        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	properties[PROP_EXPANDED] = g_param_spec_boolean(
	        "expanded", "expanded", "Whether this service expanded.",
	        FALSE,
	        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	properties[PROP_ICON_NAME] = g_param_spec_string(
	        "icon-name", "icon-name", "The icon name of this service.",
	        NULL,
	        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	properties[PROP_CHILD_MODEL] = g_param_spec_object(
	        "child-model", "child-model",
	        "The model containing children of this service.",
	        G_TYPE_LIST_MODEL,
	        G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, PROP_LAST, properties);
}

static void
xmpp_disco_service_class_finalize(G_GNUC_UNUSED XmppDiscoServiceClass *klass) {
}

static void
xmpp_disco_service_init(XmppDiscoService *service) {
	g_signal_connect(service, "notify::flags",
	                 G_CALLBACK(xmpp_disco_service_refresh_child_model), NULL);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

void
xmpp_disco_service_register(PurplePlugin *plugin) {
	xmpp_disco_service_register_type(G_TYPE_MODULE(plugin));
}

XmppDiscoService *
xmpp_disco_service_new(PidginDiscoList *list) {
	return g_object_new(
		XMPP_DISCO_TYPE_SERVICE,
		"list", list,
		NULL);
}

PidginDiscoList *
xmpp_disco_service_get_list(XmppDiscoService *service) {
	g_return_val_if_fail(XMPP_DISCO_IS_SERVICE(service), NULL);

	return service->list;
}

const char *
xmpp_disco_service_get_name(XmppDiscoService *service) {
	g_return_val_if_fail(XMPP_DISCO_IS_SERVICE(service), NULL);

	return service->name;
}

void
xmpp_disco_service_set_name(XmppDiscoService *service, const char *name) {
	g_return_if_fail(XMPP_DISCO_IS_SERVICE(service));

	g_free(service->name);
	service->name = g_strdup(name);

	g_object_notify_by_pspec(G_OBJECT(service), properties[PROP_NAME]);
}

const char *
xmpp_disco_service_get_description(XmppDiscoService *service) {
	g_return_val_if_fail(XMPP_DISCO_IS_SERVICE(service), NULL);

	return service->description;
}

void
xmpp_disco_service_set_description(XmppDiscoService *service,
                                   const char *description)
{
	g_return_if_fail(XMPP_DISCO_IS_SERVICE(service));

	g_free(service->description);
	service->description = g_strdup(description);

	g_object_notify_by_pspec(G_OBJECT(service), properties[PROP_DESCRIPTION]);
}

XmppDiscoServiceType
xmpp_disco_service_get_service_type(XmppDiscoService *service) {
	g_return_val_if_fail(XMPP_DISCO_IS_SERVICE(service),
	                     XMPP_DISCO_SERVICE_TYPE_UNSET);

	return service->type;
}

void
xmpp_disco_service_set_service_type(XmppDiscoService *service,
                                    XmppDiscoServiceType type)
{
	GObject *obj = NULL;
	g_return_if_fail(XMPP_DISCO_IS_SERVICE(service));

	service->type = type;

	obj = G_OBJECT(service);
	g_object_freeze_notify(obj);
	g_object_notify_by_pspec(obj, properties[PROP_SERVICE_TYPE]);
	g_object_notify_by_pspec(obj, properties[PROP_ICON_NAME]);
	g_object_thaw_notify(obj);
}

const char *
xmpp_disco_service_get_gateway_type(XmppDiscoService *service) {
	g_return_val_if_fail(XMPP_DISCO_IS_SERVICE(service), NULL);

	return service->gateway_type;
}

void
xmpp_disco_service_set_gateway_type(XmppDiscoService *service,
                                    const char *gateway_type)
{
	GObject *obj = NULL;
	g_return_if_fail(XMPP_DISCO_IS_SERVICE(service));

	g_free(service->gateway_type);
	service->gateway_type = g_strdup(gateway_type);

	obj = G_OBJECT(service);
	g_object_freeze_notify(obj);
	g_object_notify_by_pspec(obj, properties[PROP_GATEWAY_TYPE]);
	g_object_notify_by_pspec(obj, properties[PROP_ICON_NAME]);
	g_object_thaw_notify(obj);
}

XmppDiscoServiceFlags
xmpp_disco_service_get_flags(XmppDiscoService *service) {
	g_return_val_if_fail(XMPP_DISCO_IS_SERVICE(service), XMPP_DISCO_NONE);

	return service->flags;
}

void
xmpp_disco_service_set_flags(XmppDiscoService *service,
                             XmppDiscoServiceFlags flags)
{
	g_return_if_fail(XMPP_DISCO_IS_SERVICE(service));

	service->flags = flags;

	g_object_notify_by_pspec(G_OBJECT(service), properties[PROP_FLAGS]);
}

void
xmpp_disco_service_add_flags(XmppDiscoService *service,
                             XmppDiscoServiceFlags flags)
{
	g_return_if_fail(XMPP_DISCO_IS_SERVICE(service));

	service->flags |= flags;

	g_object_notify_by_pspec(G_OBJECT(service), properties[PROP_FLAGS]);
}

void
xmpp_disco_service_remove_flags(XmppDiscoService *service,
                                XmppDiscoServiceFlags flags)
{
	g_return_if_fail(XMPP_DISCO_IS_SERVICE(service));

	service->flags &= ~flags;

	g_object_notify_by_pspec(G_OBJECT(service), properties[PROP_FLAGS]);
}

const char *
xmpp_disco_service_get_jid(XmppDiscoService *service) {
	g_return_val_if_fail(XMPP_DISCO_IS_SERVICE(service), NULL);

	return service->jid;
}

void
xmpp_disco_service_set_jid(XmppDiscoService *service, const char *jid) {
	g_return_if_fail(XMPP_DISCO_IS_SERVICE(service));

	g_free(service->jid);
	service->jid = g_strdup(jid);

	g_object_notify_by_pspec(G_OBJECT(service), properties[PROP_JID]);
}

const char *
xmpp_disco_service_get_node(XmppDiscoService *service) {
	g_return_val_if_fail(XMPP_DISCO_IS_SERVICE(service), NULL);

	return service->node;
}

void
xmpp_disco_service_set_node(XmppDiscoService *service, const char *node) {
	g_return_if_fail(XMPP_DISCO_IS_SERVICE(service));

	g_free(service->node);
	service->node = g_strdup(node);

	g_object_notify_by_pspec(G_OBJECT(service), properties[PROP_NODE]);
}

gboolean
xmpp_disco_service_get_expanded(XmppDiscoService *service) {
	g_return_val_if_fail(XMPP_DISCO_IS_SERVICE(service), FALSE);

	return service->expanded;
}

void
xmpp_disco_service_set_expanded(XmppDiscoService *service, gboolean expanded) {
	g_return_if_fail(XMPP_DISCO_IS_SERVICE(service));

	service->expanded = expanded;

	g_object_notify_by_pspec(G_OBJECT(service), properties[PROP_EXPANDED]);
}

char *
xmpp_disco_service_get_icon_name(XmppDiscoService *service)
{
	char *icon_name = NULL;

	g_return_val_if_fail(XMPP_DISCO_IS_SERVICE(service), NULL);

	if(service->type == XMPP_DISCO_SERVICE_TYPE_GATEWAY && service->gateway_type != NULL) {
		icon_name = g_strconcat("im-", service->gateway_type, NULL);
#if 0
	} else if(service->type == XMPP_DISCO_SERVICE_TYPE_USER) {
		icon_name = g_strdup("person");
#endif
	} else if(service->type == XMPP_DISCO_SERVICE_TYPE_CHAT) {
		icon_name = g_strdup("chat");
	}

	return icon_name;
}

GListModel *
xmpp_disco_service_get_child_model(XmppDiscoService *service) {
	g_return_val_if_fail(XMPP_DISCO_IS_SERVICE(service), NULL);

	return G_LIST_MODEL(service->children);
}

void
xmpp_disco_service_add_child(XmppDiscoService *service,
                             XmppDiscoService *child)
{
	g_return_if_fail(XMPP_DISCO_IS_SERVICE(service));
	g_return_if_fail(XMPP_DISCO_IS_SERVICE(child));
	g_return_if_fail((service->flags & XMPP_DISCO_BROWSE) != 0);

	g_list_store_append(service->children, child);
}
