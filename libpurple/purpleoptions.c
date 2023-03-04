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

#include <glib/gi18n-lib.h>

#include "purpleoptions.h"

#include "debug.h"
#include "network.h"
#include "util.h"

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static gboolean
purple_options_force_online_cb(G_GNUC_UNUSED const char *option_name,
                               G_GNUC_UNUSED const char *value,
                               G_GNUC_UNUSED gpointer data,
                               G_GNUC_UNUSED GError **error)
{
	purple_network_force_online();

	return TRUE;
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GOptionGroup *
purple_get_option_group(void) {
	GOptionGroup *group = NULL;
	GOptionEntry entries[] = {
		{
			"force-online", 'f', G_OPTION_FLAG_NO_ARG,
			G_OPTION_ARG_CALLBACK, &purple_options_force_online_cb,
			_("force online, regardless of network status"),
			NULL
		}, {
			NULL
		},
	};

	group = g_option_group_new(
		"libpurple",
		_("LibPurple options"),
		_("Show LibPurple Options"),
		NULL,
		NULL
	);

	g_option_group_add_entries(group, entries);

	return group;
}
