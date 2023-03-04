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

/* TODO list (a little bit of a brain dump):
	* Support more actions than "register" and "add" based on context.
		- Subscribe to pubsub nodes (just...because?)
		- Execute ad-hoc commands
		- Change 'Register' to 'Unregister' if we're registered?
		- Administer MUCs
	* Enumerate pubsub node contents.
		- PEP too? (useful development tool at times)
	* See if we can better handle the ad-hoc commands that ejabberd returns
	  when disco'ing a server as an administrator:
from disco#items:
	<item jid='darkrain42.org' node='announce' name='Announcements'/>
disco#info:
	<iq from='darkrain42.org' type='result'>
		<query xmlns='http://jabber.org/protocol/disco#info' node='announce'/>
	</iq>
	* For services that are a JID w/o a node, handle fetching ad-hoc commands?
*/

#include <glib/gi18n-lib.h>

#include <purple.h>

#include <pidgin.h>

#include "xmppdisco.h"
#include "gtkdisco.h"

/* Variables */
PurplePlugin *my_plugin = NULL;
static GHashTable *iq_callbacks = NULL;
static gboolean iq_listening = FALSE;

typedef void (*XmppIqCallback)(PurpleConnection *pc, const char *type,
                               const char *id, const char *from, PurpleXmlNode *iq,
                               gpointer data);

struct item_data {
	PidginDiscoList *list;
	XmppDiscoService *parent;
	char *name;
	char *node; /* disco#info replies don't always include the node */
};

struct xmpp_iq_cb_data
{
	/*
	 * Every IQ callback in this plugin uses the same structure for the
	 * callback data. It's a hack (it wouldn't scale), but it's used so that
	 * it's easy to clean up all the callbacks when the account disconnects
	 * (see remove_iq_callbacks_by_pc below).
	 */
	struct item_data *context;
	PurpleConnection *pc;
	XmppIqCallback cb;
};


static char *
generate_next_id(void)
{
	static guint32 index = 0;

	if (index == 0) {
		do {
			index = g_random_int();
		} while (index == 0);
	}

	return g_strdup_printf("purpledisco%x", index++);
}

static gboolean
remove_iq_callbacks_by_pc(G_GNUC_UNUSED gpointer key, gpointer value,
                          gpointer user_data)
{
	struct xmpp_iq_cb_data *cb_data = value;

	if (cb_data && cb_data->pc == user_data) {
		struct item_data *item_data = cb_data->context;

		if (item_data) {
			pidgin_disco_list_unref(item_data->list);
			g_free(item_data->name);
			g_free(item_data->node);
			g_free(item_data);
		}

		return TRUE;
	} else
		return FALSE;
}

static gboolean
xmpp_iq_received(PurpleConnection *pc, const char *type, const char *id,
                 const char *from, PurpleXmlNode *iq)
{
	struct xmpp_iq_cb_data *cb_data;

	cb_data = g_hash_table_lookup(iq_callbacks, id);
	if (!cb_data) {
		return FALSE;
	}

	cb_data->cb(cb_data->pc, type, id, from, iq, cb_data->context);

	g_hash_table_remove(iq_callbacks, id);
	if (g_hash_table_size(iq_callbacks) == 0) {
		PurpleProtocol *protocol = purple_connection_get_protocol(pc);
		iq_listening = FALSE;
		purple_signal_disconnect(protocol, "jabber-receiving-iq", my_plugin,
		                         G_CALLBACK(xmpp_iq_received));
	}

	/* Om nom nom nom */
	return TRUE;
}

static void
xmpp_iq_register_callback(PurpleConnection *pc, gchar *id, gpointer data,
                          XmppIqCallback cb)
{
	struct xmpp_iq_cb_data *cbdata = g_new0(struct xmpp_iq_cb_data, 1);

	cbdata->context = data;
	cbdata->cb = cb;
	cbdata->pc = pc;

	g_hash_table_insert(iq_callbacks, id, cbdata);

	if (!iq_listening) {
		PurpleProtocol *protocol = NULL;
		PurpleProtocolManager *manager = purple_protocol_manager_get_default();
		protocol = purple_protocol_manager_find(manager, XMPP_PROTOCOL_ID);

		iq_listening = TRUE;
		purple_signal_connect(protocol, "jabber-receiving-iq", my_plugin,
		                      G_CALLBACK(xmpp_iq_received), NULL);
	}
}

static void
xmpp_disco_info_do(PurpleConnection *pc, gpointer cbdata, const char *jid,
                   const char *node, XmppIqCallback cb)
{
	PurpleXmlNode *iq, *query;
	char *id = generate_next_id();

	iq = purple_xmlnode_new("iq");
	purple_xmlnode_set_attrib(iq, "type", "get");
	purple_xmlnode_set_attrib(iq, "to", jid);
	purple_xmlnode_set_attrib(iq, "id", id);

	query = purple_xmlnode_new_child(iq, "query");
	purple_xmlnode_set_namespace(query, NS_DISCO_INFO);
	if (node) {
		purple_xmlnode_set_attrib(query, "node", node);
	}

	/* Steals id */
	xmpp_iq_register_callback(pc, id, cbdata, cb);

	purple_signal_emit(purple_connection_get_protocol(pc), "jabber-sending-xmlnode",
	                   pc, &iq);
	if (iq != NULL) {
		purple_xmlnode_free(iq);
	}
}

static void
xmpp_disco_items_do(PurpleConnection *pc, gpointer cbdata, const char *jid,
                    const char *node, XmppIqCallback cb)
{
	PurpleXmlNode *iq, *query;
	char *id = generate_next_id();

	iq = purple_xmlnode_new("iq");
	purple_xmlnode_set_attrib(iq, "type", "get");
	purple_xmlnode_set_attrib(iq, "to", jid);
	purple_xmlnode_set_attrib(iq, "id", id);

	query = purple_xmlnode_new_child(iq, "query");
	purple_xmlnode_set_namespace(query, NS_DISCO_ITEMS);
	if (node) {
		purple_xmlnode_set_attrib(query, "node", node);
	}

	/* Steals id */
	xmpp_iq_register_callback(pc, id, cbdata, cb);

	purple_signal_emit(purple_connection_get_protocol(pc), "jabber-sending-xmlnode",
	                   pc, &iq);
	if (iq != NULL) {
		purple_xmlnode_free(iq);
	}
}

static XmppDiscoServiceType
disco_service_type_from_identity(PurpleXmlNode *identity)
{
	const char *category, *type;

	if (!identity) {
		return XMPP_DISCO_SERVICE_TYPE_OTHER;
	}

	category = purple_xmlnode_get_attrib(identity, "category");
	type = purple_xmlnode_get_attrib(identity, "type");

	if (!category) {
		return XMPP_DISCO_SERVICE_TYPE_OTHER;
	}

	if (purple_strequal(category, "conference")) {
		return XMPP_DISCO_SERVICE_TYPE_CHAT;
	} else if (purple_strequal(category, "directory")) {
		return XMPP_DISCO_SERVICE_TYPE_DIRECTORY;
	} else if (purple_strequal(category, "gateway")) {
		return XMPP_DISCO_SERVICE_TYPE_GATEWAY;
	} else if (purple_strequal(category, "pubsub")) {
		if (!type || purple_strequal(type, "collection")) {
			return XMPP_DISCO_SERVICE_TYPE_PUBSUB_COLLECTION;
		} else if (purple_strequal(type, "leaf")) {
			return XMPP_DISCO_SERVICE_TYPE_PUBSUB_LEAF;
		} else if (purple_strequal(type, "service")) {
			return XMPP_DISCO_SERVICE_TYPE_OTHER;
		} else {
			purple_debug_warning("xmppdisco", "Unknown pubsub type '%s'\n", type);
			return XMPP_DISCO_SERVICE_TYPE_OTHER;
		}
	}

	return XMPP_DISCO_SERVICE_TYPE_OTHER;
}

static const struct {
	const char *from;
	const char *to;
} disco_type_mappings[] = {
	{ "gadu-gadu", "gadu-gadu" }, /* the protocol is gg, but list_icon returns "gadu-gadu" */
	{ "xmpp",      "jabber" }, /* jabber (mentioned in case the protocol is renamed so this line will match) */
	{ NULL,        NULL }
};

static const gchar *
disco_type_from_string(const gchar *str)
{
	int i = 0;

	g_return_val_if_fail(str != NULL, "");

	for ( ; disco_type_mappings[i].from; ++i) {
		if (!g_ascii_strcasecmp(str, disco_type_mappings[i].from)) {
			return disco_type_mappings[i].to;
		}
	}

	/* fallback to the string itself */
	return str;
}

static void
got_info_cb(G_GNUC_UNUSED PurpleConnection *pc, const char *type,
            G_GNUC_UNUSED const char *id, const char *from, PurpleXmlNode *iq,
            gpointer data)
{
	struct item_data *item_data = data;
	PidginDiscoList *list = item_data->list;
	PurpleXmlNode *query;

	--list->fetch_count;

	if (!list->in_progress) {
		goto out;
	}

	if (purple_strequal(type, "result") &&
			(query = purple_xmlnode_get_child(iq, "query"))) {
		PurpleXmlNode *identity = purple_xmlnode_get_child(query, "identity");
		XmppDiscoService *service = NULL;
		PurpleXmlNode *feature;

		service = xmpp_disco_service_new(item_data->list);
		purple_debug_info("xmppdisco", "parent for %s is %p", from, item_data->parent);
		xmpp_disco_service_set_service_type(service,
		                                    disco_service_type_from_identity(identity));

		if (item_data->node) {
			if (item_data->name) {
				xmpp_disco_service_set_name(service, item_data->name);
				g_clear_pointer(&item_data->name, g_free);
			} else {
				xmpp_disco_service_set_name(service, item_data->node);
			}

			xmpp_disco_service_set_node(service, item_data->node);
			g_clear_pointer(&item_data->node, g_free);

			if(xmpp_disco_service_get_service_type(service) == XMPP_DISCO_SERVICE_TYPE_PUBSUB_COLLECTION) {
				xmpp_disco_service_add_flags(service, XMPP_DISCO_BROWSE);
			}
		} else {
			xmpp_disco_service_set_name(service, from);
		}

		if(!xmpp_disco_service_get_node(service)) {
			/* Only support adding JIDs, not JID+node combos */
			xmpp_disco_service_add_flags(service, XMPP_DISCO_ADD);
		}

		if (item_data->name) {
			xmpp_disco_service_set_description(service, item_data->name);
			g_clear_pointer(&item_data->name, g_free);
		} else if (identity) {
			xmpp_disco_service_set_description(service,
			                                   purple_xmlnode_get_attrib(identity, "name"));
		}

		/* TODO: Overlap with service->name a bit */
		xmpp_disco_service_set_jid(service, from);

		for (feature = purple_xmlnode_get_child(query, "feature"); feature;
				feature = purple_xmlnode_get_next_twin(feature)) {
			const char *var;
			if (!(var = purple_xmlnode_get_attrib(feature, "var"))) {
				continue;
			}

			if (purple_strequal(var, NS_REGISTER)) {
				xmpp_disco_service_add_flags(service, XMPP_DISCO_REGISTER);
			} else if (purple_strequal(var, NS_DISCO_ITEMS)) {
				xmpp_disco_service_add_flags(service, XMPP_DISCO_BROWSE);
			} else if (purple_strequal(var, NS_MUC)) {
				xmpp_disco_service_add_flags(service, XMPP_DISCO_BROWSE);
				xmpp_disco_service_set_service_type(service,
				                                    XMPP_DISCO_SERVICE_TYPE_CHAT);
			}
		}

		if(xmpp_disco_service_get_service_type(service) == XMPP_DISCO_SERVICE_TYPE_GATEWAY) {
			xmpp_disco_service_set_gateway_type(service,
				disco_type_from_string(
					purple_xmlnode_get_attrib(identity, "type")));
		}

		pidgin_disco_add_service(list, service, item_data->parent);
	}

out:
	if (list->fetch_count == 0) {
		pidgin_disco_list_set_in_progress(list, FALSE);
	}

	g_free(item_data->name);
	g_free(item_data->node);
	g_free(item_data);
	pidgin_disco_list_unref(list);
}

static void
got_items_cb(PurpleConnection *pc, const char *type,
             G_GNUC_UNUSED const char *id, G_GNUC_UNUSED const char *from,
             PurpleXmlNode *iq, gpointer data)
{
	struct item_data *item_data = data;
	PidginDiscoList *list = item_data->list;
	PurpleXmlNode *query;
	gboolean has_items = FALSE;

	--list->fetch_count;

	if (!list->in_progress) {
		goto out;
	}

	if (purple_strequal(type, "result") &&
			(query = purple_xmlnode_get_child(iq, "query"))) {
		PurpleXmlNode *item;

		for (item = purple_xmlnode_get_child(query, "item"); item;
				item = purple_xmlnode_get_next_twin(item)) {
			const char *jid = purple_xmlnode_get_attrib(item, "jid");
			const char *name = purple_xmlnode_get_attrib(item, "name");
			const char *node = purple_xmlnode_get_attrib(item, "node");

			has_items = TRUE;

			if(xmpp_disco_service_get_service_type(item_data->parent) == XMPP_DISCO_SERVICE_TYPE_CHAT) {
				/* This is a hacky first-order approximation. Any MUC
				 * component that has a >1 level hierarchy (a Yahoo MUC
				 * transport component probably does) will violate this.
				 *
				 * On the other hand, this is better than querying all the
				 * chats at conference.jabber.org to enumerate them.
				 */
				XmppDiscoService *service = xmpp_disco_service_new(item_data->list);
				xmpp_disco_service_set_flags(service, XMPP_DISCO_ADD);
				xmpp_disco_service_set_service_type(service,
				                                    XMPP_DISCO_SERVICE_TYPE_CHAT);

				xmpp_disco_service_set_name(service, jid);
				xmpp_disco_service_set_description(service, name);
				xmpp_disco_service_set_jid(service, jid);
				xmpp_disco_service_set_node(service, node);
				pidgin_disco_add_service(list, service, item_data->parent);
			} else {
				struct item_data *item_data2 = g_new0(struct item_data, 1);

				item_data2->list = item_data->list;
				item_data2->parent = item_data->parent;
				item_data2->name = g_strdup(name);
				item_data2->node = g_strdup(node);

				++list->fetch_count;
				pidgin_disco_list_ref(list);
				xmpp_disco_info_do(pc, item_data2, jid, node, got_info_cb);
			}
		}
	}

	if (!has_items) {
		pidgin_disco_add_service(list, NULL, item_data->parent);
	}

out:
	if (list->fetch_count == 0) {
		pidgin_disco_list_set_in_progress(list, FALSE);
	}

	g_free(item_data);
	pidgin_disco_list_unref(list);
}

static void
server_items_cb(PurpleConnection *pc, const char *type,
                G_GNUC_UNUSED const char *id,
                G_GNUC_UNUSED const char *from, PurpleXmlNode *iq,
                gpointer data)
{
	struct item_data *cb_data = data;
	PidginDiscoList *list = cb_data->list;
	PurpleXmlNode *query;

	g_free(cb_data);
	--list->fetch_count;

	if (purple_strequal(type, "result") &&
			(query = purple_xmlnode_get_child(iq, "query"))) {
		PurpleXmlNode *item;

		for (item = purple_xmlnode_get_child(query, "item"); item;
				item = purple_xmlnode_get_next_twin(item)) {
			const char *jid = purple_xmlnode_get_attrib(item, "jid");
			const char *name = purple_xmlnode_get_attrib(item, "name");
			const char *node = purple_xmlnode_get_attrib(item, "node");
			struct item_data *item_data;

			if (!jid) {
				continue;
			}

			item_data = g_new0(struct item_data, 1);
			item_data->list = list;
			item_data->name = g_strdup(name);
			item_data->node = g_strdup(node);

			++list->fetch_count;
			pidgin_disco_list_ref(list);
			xmpp_disco_info_do(pc, item_data, jid, node, got_info_cb);
		}
	}

	if (list->fetch_count == 0) {
		pidgin_disco_list_set_in_progress(list, FALSE);
	}

	pidgin_disco_list_unref(list);
}

static void
server_info_cb(PurpleConnection *pc, const char *type,
               G_GNUC_UNUSED const char *id,
               const char *from, PurpleXmlNode *iq, gpointer data)
{
	struct item_data *cb_data = data;
	PidginDiscoList *list = cb_data->list;
	PurpleXmlNode *query;
	PurpleXmlNode *error;
	gboolean items = FALSE;

	--list->fetch_count;

	if (purple_strequal(type, "result") &&
			(query = purple_xmlnode_get_child(iq, "query"))) {
		PurpleXmlNode *feature;

		for (feature = purple_xmlnode_get_child(query, "feature"); feature;
				feature = purple_xmlnode_get_next_twin(feature)) {
			const char *var = purple_xmlnode_get_attrib(feature, "var");
			if (purple_strequal(var, NS_DISCO_ITEMS)) {
				items = TRUE;
				break;
			}
		}

		if (items) {
			xmpp_disco_items_do(pc, cb_data, from, NULL /* node */, server_items_cb);
			++list->fetch_count;
			pidgin_disco_list_ref(list);
		}
		else {
			pidgin_disco_list_set_in_progress(list, FALSE);
			g_free(cb_data);
		}
	}
	else {
		error = purple_xmlnode_get_child(iq, "error");
		if (purple_xmlnode_get_child(error, "remote-server-not-found")
		 || purple_xmlnode_get_child(error, "jid-malformed")) {
			purple_notify_error(my_plugin, _("Error"),
			                    _("Server does not exist"),
 			                    NULL, NULL);
		} else {
			purple_notify_error(my_plugin, _("Error"),
			                    _("Server does not support service discovery"),
			                    NULL, NULL);
		}
		pidgin_disco_list_set_in_progress(list, FALSE);
		g_free(cb_data);
	}

	pidgin_disco_list_unref(list);
}

void xmpp_disco_start(PidginDiscoList *list)
{
	struct item_data *cb_data;

	g_return_if_fail(list != NULL);

	++list->fetch_count;
	pidgin_disco_list_ref(list);

	cb_data = g_new0(struct item_data, 1);
	cb_data->list = list;

	xmpp_disco_info_do(list->pc, cb_data, list->server, NULL, server_info_cb);
}

void xmpp_disco_service_expand(XmppDiscoService *service)
{
	PidginDiscoList *list = NULL;
	struct item_data *item_data;

	g_return_if_fail(XMPP_DISCO_IS_SERVICE(service));

	if(xmpp_disco_service_get_expanded(service)) {
		return;
	}

	list = xmpp_disco_service_get_list(service);

	item_data = g_new0(struct item_data, 1);
	item_data->list = list;
	item_data->parent = service;

	++list->fetch_count;
	pidgin_disco_list_ref(list);

	pidgin_disco_list_set_in_progress(list, TRUE);

	xmpp_disco_items_do(list->pc, item_data,
	                    xmpp_disco_service_get_jid(service),
	                    xmpp_disco_service_get_node(service),
	                    got_items_cb);
	xmpp_disco_service_set_expanded(service, TRUE);
}

void
xmpp_disco_register_service(XmppDiscoService *service)
{
	PidginDiscoList *list = NULL;
	PurpleXmlNode *iq, *query;
	char *id = generate_next_id();

	iq = purple_xmlnode_new("iq");
	purple_xmlnode_set_attrib(iq, "type", "get");
	purple_xmlnode_set_attrib(iq, "to", xmpp_disco_service_get_jid(service));
	purple_xmlnode_set_attrib(iq, "id", id);

	query = purple_xmlnode_new_child(iq, "query");
	purple_xmlnode_set_namespace(query, NS_REGISTER);

	list = xmpp_disco_service_get_list(service);
	purple_signal_emit(purple_connection_get_protocol(list->pc),
	                   "jabber-sending-xmlnode", list->pc, &iq);
	if (iq != NULL) {
		purple_xmlnode_free(iq);
	}
	g_free(id);
}

static void
create_dialog(G_GNUC_UNUSED GSimpleAction *action,
              G_GNUC_UNUSED GVariant *parameter,
              G_GNUC_UNUSED gpointer data)
{
	pidgin_disco_dialog_new();
}

static void
signed_off_cb(PurpleConnection *pc, G_GNUC_UNUSED gpointer data)
{
	/* Deal with any dialogs */
	pidgin_disco_signed_off_cb(pc);

	/* Remove all the IQ callbacks for this connection */
	g_hash_table_foreach_remove(iq_callbacks, remove_iq_callbacks_by_pc, pc);
}

static GPluginPluginInfo *
xmpp_disco_query(G_GNUC_UNUSED GError **error)
{
	GActionEntry entries[] = {
		{
			.name = "dialog",
			.activate = create_dialog,
		}
	};
	GMenu *menu = NULL;
	GSimpleActionGroup *group = NULL;
	const gchar * const authors[] = {
		"Paul Aurich <paul@darkrain42.org>",
		NULL
	};

	group = g_simple_action_group_new();
	g_action_map_add_action_entries(G_ACTION_MAP(group), entries,
	                                G_N_ELEMENTS(entries), NULL);

	menu = g_menu_new();
	g_menu_append(menu, _("XMPP Service Discovery"), "dialog");

	return pidgin_plugin_info_new(
		"id",           PLUGIN_ID,
		"name",         N_("XMPP Service Discovery"),
		"version",      DISPLAY_VERSION,
		"category",     N_("Protocol utility"),
		"summary",      N_("Allows browsing and registering services."),
		"description",  N_("This plugin is useful for registering with legacy "
		                   "transports or other XMPP services."),
		"authors",      authors,
		"website",      PURPLE_WEBSITE,
		"abi-version",  PURPLE_ABI_VERSION,
		"action-group", group,
		"action-menu",  menu,
		NULL
	);
}

static gboolean
xmpp_disco_load(GPluginPlugin *plugin, GError **error)
{
	PurpleProtocol *xmpp_protocol;
	PurpleProtocolManager *manager;

	my_plugin = plugin;

	manager = purple_protocol_manager_get_default();
	xmpp_protocol = purple_protocol_manager_find(manager, XMPP_PROTOCOL_ID);
	if (NULL == xmpp_protocol) {
		g_set_error_literal(error, PLUGIN_DOMAIN, 0,
		                    _("XMPP protocol is not loaded."));
		return FALSE;
	}

	xmpp_disco_service_register(plugin);
	pidgin_disco_dialog_register(plugin);

	purple_signal_connect(purple_connections_get_handle(), "signing-off",
	                      plugin, G_CALLBACK(signed_off_cb), NULL);

	iq_callbacks = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

	return TRUE;
}

static gboolean
xmpp_disco_unload(GPluginPlugin *plugin, G_GNUC_UNUSED gboolean unload,
                  G_GNUC_UNUSED GError **error)
{
	g_hash_table_destroy(iq_callbacks);
	iq_callbacks = NULL;

	purple_signals_disconnect_by_handle(plugin);
	pidgin_disco_dialogs_destroy_all();

	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(xmpp_disco)
