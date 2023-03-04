/*
 * Pidgin - Internet Messenger
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

#include "pidgin/pidginaccountfilterconnected.h"

#include <purple.h>

struct _PidginAccountFilterConnected {
	GtkFilter parent;
};

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_account_filter_connected_changed(PurpleConnection *connection,
                                        gpointer data)
{
	PidginAccountFilterConnected *filter = NULL;
	PurpleConnectionState state;

	filter = PIDGIN_ACCOUNT_FILTER_CONNECTED(data);

	state = purple_connection_get_state(connection);
	gtk_filter_changed(GTK_FILTER(filter),
	                   (state == PURPLE_CONNECTION_STATE_CONNECTED) ?
	                   GTK_FILTER_CHANGE_LESS_STRICT :
	                   GTK_FILTER_CHANGE_MORE_STRICT);
}

/******************************************************************************
 * GtkFilter Implementation
 *****************************************************************************/
static GtkFilterMatch
pidgin_account_filter_connected_get_strictness(G_GNUC_UNUSED GtkFilter *self) {
	return GTK_FILTER_MATCH_SOME;
}

static gboolean
pidgin_account_filter_connected_match(G_GNUC_UNUSED GtkFilter *self,
                                      gpointer item)
{
	gboolean ret = FALSE;

	if(PURPLE_IS_ACCOUNT(item)) {
		ret = purple_account_is_connected(PURPLE_ACCOUNT(item));
	}

	return ret;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PidginAccountFilterConnected, pidgin_account_filter_connected,
              GTK_TYPE_FILTER)

static void
pidgin_account_filter_connected_init(PidginAccountFilterConnected *filter) {
	gpointer connections_handle = NULL;

	/* we connect to the connections signals to force a refresh of the filter */
	connections_handle = purple_connections_get_handle();
	purple_signal_connect(connections_handle, "signed-on", filter,
	                      G_CALLBACK(pidgin_account_filter_connected_changed),
	                      filter);
	purple_signal_connect(connections_handle, "signed-off", filter,
	                      G_CALLBACK(pidgin_account_filter_connected_changed),
	                      filter);
}

static void
pidgin_account_filter_connected_finalize(GObject *obj) {
	purple_signals_disconnect_by_handle(obj);

	G_OBJECT_CLASS(pidgin_account_filter_connected_parent_class)->finalize(obj);
}

static void
pidgin_account_filter_connected_class_init(PidginAccountFilterConnectedClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkFilterClass *filter_class = GTK_FILTER_CLASS(klass);

	obj_class->finalize = pidgin_account_filter_connected_finalize;

	filter_class->get_strictness = pidgin_account_filter_connected_get_strictness;
	filter_class->match = pidgin_account_filter_connected_match;
}

/******************************************************************************
 * API
 *****************************************************************************/
GtkFilter *
pidgin_account_filter_connected_new(void)
{
	return g_object_new(PIDGIN_TYPE_ACCOUNT_FILTER_CONNECTED, NULL);
}
