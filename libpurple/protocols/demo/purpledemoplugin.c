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

#include <glib.h>
#include <glib/gi18n-lib.h>

#include <gplugin.h>
#include <gplugin-native.h>

#include <purple.h>

#include "purpledemoplugin.h"

#include "purpledemoconnection.h"
#include "purpledemoprotocol.h"

/******************************************************************************
 * Globals
 *****************************************************************************/
static PurpleProtocol *demo_protocol = NULL;

/******************************************************************************
 * GPlugin Implementation
 *****************************************************************************/
static GPluginPluginInfo *
purple_demo_plugin_query(G_GNUC_UNUSED GError **error) {
	const gchar *authors[] = {
		"Pidgin Developers <devel@pidgin.im>",
		NULL
	};

	return purple_plugin_info_new(
		"id", "prpl-demo",
		"name", "Demo Protocol Plugin",
		"authors", authors,
		"version", PURPLE_VERSION,
		"category", N_("Protocol"),
		"summary", N_("A protocol plugin used for demos."),
		"description", N_("A protocol plugin that helps to demonstrate "
		                  "features of libpurple and clients."),
		"website", PURPLE_WEBSITE,
		"abi-version", PURPLE_ABI_VERSION,
		"flags", PURPLE_PLUGIN_INFO_FLAGS_INTERNAL |
		         PURPLE_PLUGIN_INFO_FLAGS_AUTO_LOAD,
		NULL
	);
}

static gboolean
purple_demo_plugin_load(GPluginPlugin *plugin, GError **error) {
	PurpleProtocolManager *manager = NULL;

	if(PURPLE_IS_PROTOCOL(demo_protocol)) {
		g_set_error_literal(error, PURPLE_DEMO_DOMAIN, 0,
		                    "plugin was not cleaned up properly");

		return FALSE;
	}

	purple_demo_connection_register(GPLUGIN_NATIVE_PLUGIN(plugin));
	purple_demo_protocol_register(GPLUGIN_NATIVE_PLUGIN(plugin));

	manager = purple_protocol_manager_get_default();

	demo_protocol = purple_demo_protocol_new();
	if(!purple_protocol_manager_register(manager, demo_protocol, error)) {
		g_clear_object(&demo_protocol);

		return FALSE;
	}

	return TRUE;
}

static gboolean
purple_demo_plugin_unload(G_GNUC_UNUSED GPluginPlugin *plugin,
                          G_GNUC_UNUSED gboolean shutdown,
                          GError **error)
{
	PurpleProtocolManager *manager = purple_protocol_manager_get_default();

	if(!PURPLE_IS_PROTOCOL(demo_protocol)) {
		g_set_error_literal(error, PURPLE_DEMO_DOMAIN, 0,
		                    "plugin was not setup properly");

		return FALSE;
	}

	if(!purple_protocol_manager_unregister(manager, demo_protocol, error)) {
		return FALSE;
	}

	g_clear_object(&demo_protocol);

	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(purple_demo_plugin)
