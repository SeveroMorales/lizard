/*
 * purple
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

#include "purpleaccountpresence.h"

#include "purpleprotocolserver.h"

struct _PurpleAccountPresence {
	PurplePresence parent;

	PurpleAccount *account;
	GList *statuses;
};

enum {
	PROP_0,
	PROP_ACCOUNT,
	N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES];

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_account_presence_set_account(PurpleAccountPresence *presence,
                                    PurpleAccount *account)
{
	if(g_set_object(&presence->account, account)) {
		g_object_notify_by_pspec(G_OBJECT(presence), properties[PROP_ACCOUNT]);
	}
}

/******************************************************************************
 * PurplePresence Implementation
 *****************************************************************************/
static void
purple_account_presence_update_idle(PurplePresence *presence,
                                    G_GNUC_UNUSED gboolean old_idle)
{
	PurpleAccountPresence *account_presence = PURPLE_ACCOUNT_PRESENCE(presence);
	PurpleConnection *gc = NULL;
	PurpleProtocol *protocol = NULL;
	gboolean idle = purple_presence_is_idle(presence);

	gc = purple_account_get_connection(account_presence->account);

	if(PURPLE_CONNECTION_IS_CONNECTED(gc)) {
		protocol = purple_connection_get_protocol(gc);
	}

	if(PURPLE_IS_PROTOCOL_SERVER(protocol)) {
		gint idle_time = 0;

		if(idle) {
			GDateTime *idle_since = purple_presence_get_idle_time(presence);

			if(idle_since != NULL) {
				GDateTime *now = g_date_time_new_now_local();

				idle_time = g_date_time_difference(now, idle_since);
				idle_time = idle_time / G_TIME_SPAN_SECOND;

				g_date_time_unref(now);
				g_date_time_unref(idle_since);
			}
		}

		purple_protocol_server_set_idle(PURPLE_PROTOCOL_SERVER(protocol), gc,
		                                idle_time);
	}
}

static GList *
purple_account_presence_get_statuses(PurplePresence *presence) {
	PurpleAccountPresence *account_presence = PURPLE_ACCOUNT_PRESENCE(presence);

	/* We cache purple_protocol_get_statuses because it creates all new
	 * statuses which loses at least the active attribute, which breaks all
	 * sorts of things.
	 */
	if(account_presence->statuses == NULL) {
		account_presence->statuses =
			purple_protocol_get_statuses(account_presence->account, presence);
	}

	return account_presence->statuses;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PurpleAccountPresence, purple_account_presence,
              PURPLE_TYPE_PRESENCE)

static void
purple_account_presence_set_property(GObject *obj, guint param_id,
                                     const GValue *value, GParamSpec *pspec)
{
	PurpleAccountPresence *presence = PURPLE_ACCOUNT_PRESENCE(obj);

	switch (param_id) {
		case PROP_ACCOUNT:
			purple_account_presence_set_account(presence,
			                                    g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_account_presence_get_property(GObject *obj, guint param_id,
                                     GValue *value, GParamSpec *pspec)
{
	PurpleAccountPresence *presence = PURPLE_ACCOUNT_PRESENCE(obj);

	switch (param_id) {
		case PROP_ACCOUNT:
			g_value_set_object(value,
			                   purple_account_presence_get_account(presence));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_account_presence_finalize(GObject *obj) {
	PurpleAccountPresence *presence = PURPLE_ACCOUNT_PRESENCE(obj);

	g_clear_object(&presence->account);
	g_list_free_full(presence->statuses, g_object_unref);

	G_OBJECT_CLASS(purple_account_presence_parent_class)->finalize(obj);
}

static void
purple_account_presence_init(G_GNUC_UNUSED PurpleAccountPresence *presence) {
}

static void
purple_account_presence_class_init(PurpleAccountPresenceClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	PurplePresenceClass *presence_class = PURPLE_PRESENCE_CLASS(klass);

	obj_class->get_property = purple_account_presence_get_property;
	obj_class->set_property = purple_account_presence_set_property;
	obj_class->finalize = purple_account_presence_finalize;

	presence_class->update_idle = purple_account_presence_update_idle;
	presence_class->get_statuses = purple_account_presence_get_statuses;

	properties[PROP_ACCOUNT] = g_param_spec_object(
		"account", "Account",
		"The account for this presence.",
		PURPLE_TYPE_ACCOUNT,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleAccountPresence *
purple_account_presence_new(PurpleAccount *account) {
	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);

	return g_object_new(
		PURPLE_TYPE_ACCOUNT_PRESENCE,
		"account", account,
		NULL);
}

PurpleAccount *
purple_account_presence_get_account(PurpleAccountPresence *presence)
{
	g_return_val_if_fail(PURPLE_IS_ACCOUNT_PRESENCE(presence), NULL);

	return presence->account;
}
