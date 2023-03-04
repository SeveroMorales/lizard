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
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <time.h>

#include <glib/gi18n-lib.h>

#include "purpledemoprotocol.h"

#include "purpledemoconnection.h"
#include "purpledemoprotocolactions.h"
#include "purpledemoprotocolclient.h"
#include "purpledemoprotocolim.h"
#include "purpledemoprotocolmedia.h"

struct _PurpleDemoProtocol {
	PurpleProtocol parent;
};

/******************************************************************************
 * PurpleProtocol Implementation
 *****************************************************************************/
static PurpleConnection *
purple_demo_protocol_create_connection(PurpleProtocol *protocol,
                                       PurpleAccount *account,
                                       const char *password,
                                       G_GNUC_UNUSED GError **error)
{
	g_return_val_if_fail(PURPLE_IS_PROTOCOL(protocol), NULL);
	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);

	return g_object_new(
		PURPLE_DEMO_TYPE_CONNECTION,
		"protocol", protocol,
		"account", account,
		"password", password,
		NULL);

}

static GList *
purple_demo_protocol_status_types(G_GNUC_UNUSED PurpleProtocol *protocol,
                                  G_GNUC_UNUSED PurpleAccount *account)
{
	PurpleStatusType *type = NULL;
	GList *status_types = NULL;

	type = purple_status_type_new_with_attrs(
		PURPLE_STATUS_AVAILABLE, "available", NULL,
		TRUE, TRUE, FALSE,
		"message", _("Message"), purple_value_new(G_TYPE_STRING),
		NULL);
	status_types = g_list_append(status_types, type);

	type = purple_status_type_new_with_attrs(
		PURPLE_STATUS_AWAY, "away", NULL,
		TRUE, TRUE, FALSE,
		"message", _("Message"), purple_value_new(G_TYPE_STRING),
		NULL);
	status_types = g_list_append(status_types, type);

	type = purple_status_type_new_with_attrs(
		PURPLE_STATUS_EXTENDED_AWAY, "extended_away", NULL,
		TRUE, TRUE, FALSE,
		"message", _("Message"), purple_value_new(G_TYPE_STRING),
		NULL);
	status_types = g_list_append(status_types, type);

	type = purple_status_type_new_full(
		PURPLE_STATUS_OFFLINE, NULL, NULL,
		TRUE, TRUE, FALSE);
	status_types = g_list_append(status_types, type);

	return status_types;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_DYNAMIC_TYPE_EXTENDED(
	PurpleDemoProtocol,
	purple_demo_protocol,
	PURPLE_TYPE_PROTOCOL,
	G_TYPE_FLAG_FINAL,
	G_IMPLEMENT_INTERFACE_DYNAMIC(PURPLE_TYPE_PROTOCOL_ACTIONS,
	                              purple_demo_protocol_actions_init)
	G_IMPLEMENT_INTERFACE_DYNAMIC(PURPLE_TYPE_PROTOCOL_CLIENT,
	                              purple_demo_protocol_client_init)
	G_IMPLEMENT_INTERFACE_DYNAMIC(PURPLE_TYPE_PROTOCOL_IM,
	                              purple_demo_protocol_im_init)
	G_IMPLEMENT_INTERFACE_DYNAMIC(PURPLE_TYPE_PROTOCOL_MEDIA,
	                              purple_demo_protocol_media_init))

static void
purple_demo_protocol_init(G_GNUC_UNUSED PurpleDemoProtocol *protocol) {
}

static void
purple_demo_protocol_class_finalize(G_GNUC_UNUSED PurpleDemoProtocolClass *klass) {
}

static void
purple_demo_protocol_class_init(PurpleDemoProtocolClass *klass) {
	PurpleProtocolClass *protocol_class = PURPLE_PROTOCOL_CLASS(klass);

	protocol_class->status_types = purple_demo_protocol_status_types;
	protocol_class->create_connection = purple_demo_protocol_create_connection;
}

/******************************************************************************
 * Local Exports
 *****************************************************************************/
void
purple_demo_protocol_register(GPluginNativePlugin *plugin) {
	purple_demo_protocol_register_type(G_TYPE_MODULE(plugin));
}

PurpleProtocol *
purple_demo_protocol_new(void) {
	return g_object_new(
		PURPLE_DEMO_TYPE_PROTOCOL,
		"id", "prpl-demo",
		"name", "Demo",
		"description", "A protocol plugin with static data to be used in "
		               "screen shots.",
		"icon-name", "im-purple-demo",
		"icon-resource-path", "/im/pidgin/purple/demo/icons",
		"options", OPT_PROTO_NO_PASSWORD,
		NULL);
}
