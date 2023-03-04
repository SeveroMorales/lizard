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

#include "pidgin/pidginaccountfilterprotocol.h"

#include <purple.h>

struct _PidginAccountFilterProtocol {
	GtkFilter parent;

	gchar *protocol_id;
};

enum {
	PROP_0,
	PROP_PROTOCOL_ID,
	N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES] = { NULL, };

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_account_filter_protocol_set_protocol_id(PidginAccountFilterProtocol *filter,
                                               const gchar *protocol_id)
{
	g_free(filter->protocol_id);
	filter->protocol_id = g_strdup(protocol_id);

	gtk_filter_changed(GTK_FILTER(filter), GTK_FILTER_CHANGE_DIFFERENT);
}

/******************************************************************************
 * GtkFilter Implementation
 *****************************************************************************/
static GtkFilterMatch
pidgin_account_filter_protocol_get_strictness(G_GNUC_UNUSED GtkFilter *self) {
	return GTK_FILTER_MATCH_SOME;
}

static gboolean
pidgin_account_filter_protocol_match(G_GNUC_UNUSED GtkFilter *self,
                                     gpointer item)
{
	gboolean ret = FALSE;

	g_return_val_if_fail(PIDGIN_IS_ACCOUNT_FILTER_PROTOCOL(self), FALSE);

	if(PURPLE_IS_ACCOUNT(item)) {
		PidginAccountFilterProtocol *filter = NULL;
		PurpleAccount *account = NULL;

		filter = PIDGIN_ACCOUNT_FILTER_PROTOCOL(self);
		account = PURPLE_ACCOUNT(item);
		ret = purple_strequal(purple_account_get_protocol_id(account),
		                      filter->protocol_id);
	}

	return ret;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PidginAccountFilterProtocol, pidgin_account_filter_protocol,
              GTK_TYPE_FILTER)

static void
pidgin_account_filter_protocol_get_property(GObject *obj, guint param_id,
                                            GValue *value, GParamSpec *pspec)
{
	PidginAccountFilterProtocol *filter = PIDGIN_ACCOUNT_FILTER_PROTOCOL(obj);

	switch(param_id) {
		case PROP_PROTOCOL_ID:
			g_value_set_string(
				value, pidgin_account_filter_protocol_get_protocol_id(filter));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_account_filter_protocol_set_property(GObject *obj, guint param_id,
                                            const GValue *value,
                                            GParamSpec *pspec)
{
	PidginAccountFilterProtocol *filter = PIDGIN_ACCOUNT_FILTER_PROTOCOL(obj);

	switch(param_id) {
		case PROP_PROTOCOL_ID:
			pidgin_account_filter_protocol_set_protocol_id(
				filter, g_value_get_string(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_account_filter_protocol_finalize(GObject *obj) {
	PidginAccountFilterProtocol *filter = PIDGIN_ACCOUNT_FILTER_PROTOCOL(obj);

	g_free(filter->protocol_id);

	G_OBJECT_CLASS(pidgin_account_filter_protocol_parent_class)->finalize(obj);
}

static void
pidgin_account_filter_protocol_init(G_GNUC_UNUSED PidginAccountFilterProtocol *filter)
{
}

static void
pidgin_account_filter_protocol_class_init(PidginAccountFilterProtocolClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkFilterClass *filter_class = GTK_FILTER_CLASS(klass);

	obj_class->get_property = pidgin_account_filter_protocol_get_property;
	obj_class->set_property = pidgin_account_filter_protocol_set_property;
	obj_class->finalize = pidgin_account_filter_protocol_finalize;

	filter_class->get_strictness = pidgin_account_filter_protocol_get_strictness;
	filter_class->match = pidgin_account_filter_protocol_match;

	/**
	 * PidginAccountFilterProtocol:protocol-id:
	 *
	 * The protocol id that will be filtered for.
	 */
	properties[PROP_PROTOCOL_ID] = g_param_spec_string(
		"protocol-id", "protocol-id", "The id of the protocol to filter",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * API
 *****************************************************************************/
GtkFilter *
pidgin_account_filter_protocol_new(const gchar *protocol_id) {
	return g_object_new(
		PIDGIN_TYPE_ACCOUNT_FILTER_PROTOCOL,
		"protocol-id", protocol_id,
		NULL);
}

const gchar *
pidgin_account_filter_protocol_get_protocol_id(PidginAccountFilterProtocol *filter) {
	g_return_val_if_fail(PIDGIN_IS_ACCOUNT_FILTER_PROTOCOL(filter), NULL);

	return filter->protocol_id;
}
