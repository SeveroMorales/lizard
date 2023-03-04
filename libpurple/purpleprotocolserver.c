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

#include "purpleprotocolserver.h"

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_INTERFACE(PurpleProtocolServer, purple_protocol_server,
                   PURPLE_TYPE_PROTOCOL)

static void
purple_protocol_server_default_init(G_GNUC_UNUSED PurpleProtocolServerInterface *iface)
{
}

/******************************************************************************
 * Public API
 *****************************************************************************/
void
purple_protocol_server_set_info(PurpleProtocolServer *protocol_server,
                                PurpleConnection *connection,
                                const gchar *info)
{
	PurpleProtocolServerInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_SERVER(protocol_server));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));

	iface = PURPLE_PROTOCOL_SERVER_GET_IFACE(protocol_server);
	if(iface != NULL && iface->set_info != NULL) {
		iface->set_info(protocol_server, connection, info);
	}
}

void
purple_protocol_server_get_info(PurpleProtocolServer *protocol_server,
                                PurpleConnection *connection, const gchar *who)
{
	PurpleProtocolServerInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_SERVER(protocol_server));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));
	g_return_if_fail(who != NULL);

	iface = PURPLE_PROTOCOL_SERVER_GET_IFACE(protocol_server);
	if(iface != NULL && iface->get_info != NULL) {
		iface->get_info(protocol_server, connection, who);
	}
}

void
purple_protocol_server_set_status(PurpleProtocolServer *protocol_server,
                                  PurpleAccount *account, PurpleStatus *status)
{
	PurpleProtocolServerInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_SERVER(protocol_server));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));
	g_return_if_fail(PURPLE_IS_STATUS(status));

	iface = PURPLE_PROTOCOL_SERVER_GET_IFACE(protocol_server);
	if(iface != NULL && iface->set_status != NULL) {
		iface->set_status(protocol_server, account, status);
	}
}

void
purple_protocol_server_set_idle(PurpleProtocolServer *protocol_server,
                                PurpleConnection *connection, gint idletime)
{
	PurpleProtocolServerInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_SERVER(protocol_server));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));

	iface = PURPLE_PROTOCOL_SERVER_GET_IFACE(protocol_server);
	if(iface != NULL && iface->set_idle != NULL) {
		iface->set_idle(protocol_server, connection, idletime);
	}
}

void
purple_protocol_server_change_passwd(PurpleProtocolServer *protocol_server,
                                     PurpleConnection *connection,
                                     const gchar *old_pass,
                                     const gchar *new_pass)
{
	PurpleProtocolServerInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_SERVER(protocol_server));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));
	g_return_if_fail(old_pass != NULL);
	g_return_if_fail(new_pass != NULL);

	iface = PURPLE_PROTOCOL_SERVER_GET_IFACE(protocol_server);
	if(iface != NULL && iface->change_passwd != NULL) {
		iface->change_passwd(protocol_server, connection, old_pass, new_pass);
	}
}

void
purple_protocol_server_add_buddy(PurpleProtocolServer *protocol_server,
                                 PurpleConnection *connection,
                                 PurpleBuddy *buddy, PurpleGroup *group,
                                 const gchar *message)
{
	PurpleProtocolServerInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_SERVER(protocol_server));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));
	g_return_if_fail(PURPLE_IS_BUDDY(buddy));
	g_return_if_fail(PURPLE_IS_GROUP(group));

	iface = PURPLE_PROTOCOL_SERVER_GET_IFACE(protocol_server);
	if(iface != NULL && iface->add_buddy != NULL) {
		iface->add_buddy(protocol_server, connection, buddy, group, message);
	}
}

void
purple_protocol_server_add_buddies(PurpleProtocolServer *protocol_server,
                                   PurpleConnection *connection,
                                   GList *buddies, GList *groups,
                                   const gchar *message)
{
	PurpleProtocolServerInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_SERVER(protocol_server));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));
	g_return_if_fail(g_list_length(buddies) != g_list_length(groups));

	iface = PURPLE_PROTOCOL_SERVER_GET_IFACE(protocol_server);
	if(iface != NULL && iface->add_buddies != NULL) {
		iface->add_buddies(protocol_server, connection, buddies, groups,
		                   message);
	} else {
		while(buddies != NULL && groups != NULL) {
			purple_protocol_server_add_buddy(protocol_server, connection,
			                                 PURPLE_BUDDY(buddies->data),
			                                 PURPLE_GROUP(groups->data),
			                                 message);

			buddies = g_list_next(buddies);
			groups = g_list_next(groups);
		}
	}
}

void
purple_protocol_server_remove_buddy(PurpleProtocolServer *protocol_server,
                                    PurpleConnection *connection,
                                    PurpleBuddy *buddy, PurpleGroup *group)
{
	PurpleProtocolServerInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_SERVER(protocol_server));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));
	g_return_if_fail(PURPLE_IS_BUDDY(buddy));

	iface = PURPLE_PROTOCOL_SERVER_GET_IFACE(protocol_server);
	if(iface != NULL && iface->remove_buddy != NULL) {
		iface->remove_buddy(protocol_server, connection, buddy, group);
	}
}

void
purple_protocol_server_remove_buddies(PurpleProtocolServer *protocol_server,
                                      PurpleConnection *connection,
                                      GList *buddies, GList *groups)
{
	PurpleProtocolServerInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_SERVER(protocol_server));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));

	iface = PURPLE_PROTOCOL_SERVER_GET_IFACE(protocol_server);
	if(iface != NULL && iface->remove_buddies != NULL) {
		iface->remove_buddies(protocol_server, connection, buddies, groups);
	} else {
		while(buddies != NULL && groups != NULL) {
			purple_protocol_server_remove_buddy(protocol_server, connection,
			                                    PURPLE_BUDDY(buddies->data),
			                                    PURPLE_GROUP(groups->data));

			buddies = g_list_next(buddies);
			groups = g_list_next(groups);
		}
	}
}

void
purple_protocol_server_keepalive(PurpleProtocolServer *protocol_server,
                                 PurpleConnection *connection)
{
	PurpleProtocolServerInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_SERVER(protocol_server));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));

	iface = PURPLE_PROTOCOL_SERVER_GET_IFACE(protocol_server);
	if(iface != NULL && iface->keepalive != NULL) {
		iface->keepalive(protocol_server, connection);
	}
}

gint
purple_protocol_server_get_keepalive_interval(PurpleProtocolServer *protocol_server)
{
	PurpleProtocolServerInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_SERVER(protocol_server), -1);

	iface = PURPLE_PROTOCOL_SERVER_GET_IFACE(protocol_server);
	if(iface != NULL && iface->get_keepalive_interval != NULL) {
		return iface->get_keepalive_interval(protocol_server);
	}

	return -1;
}

void
purple_protocol_server_alias_buddy(PurpleProtocolServer *protocol_server,
                                   PurpleConnection *connection,
                                   const gchar *who, const gchar *alias)
{
	PurpleProtocolServerInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_SERVER(protocol_server));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));
	g_return_if_fail(who != NULL);

	iface = PURPLE_PROTOCOL_SERVER_GET_IFACE(protocol_server);
	if(iface != NULL && iface->alias_buddy != NULL) {
		iface->alias_buddy(protocol_server, connection, who, alias);
	}
}

void
purple_protocol_server_group_buddy(PurpleProtocolServer *protocol_server,
                                   PurpleConnection *connection,
                                   const gchar *who, const gchar *old_group,
                                   const gchar *new_group)
{
	PurpleProtocolServerInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_SERVER(protocol_server));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));
	g_return_if_fail(who != NULL);

	iface = PURPLE_PROTOCOL_SERVER_GET_IFACE(protocol_server);
	if(iface != NULL && iface->group_buddy != NULL) {
		iface->group_buddy(protocol_server, connection, who, old_group,
		                   new_group);
	}
}

void
purple_protocol_server_rename_group(PurpleProtocolServer *protocol_server,
                                    PurpleConnection *connection,
                                    const gchar *old_name, PurpleGroup *group,
                                    GList *moved_buddies)
{
	PurpleProtocolServerInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_SERVER(protocol_server));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));
	g_return_if_fail(PURPLE_IS_GROUP(group));

	iface = PURPLE_PROTOCOL_SERVER_GET_IFACE(protocol_server);
	if(iface != NULL && iface->rename_group != NULL) {
		iface->rename_group(protocol_server, connection, old_name, group,
		                    moved_buddies);
	}
}

void
purple_protocol_server_set_buddy_icon(PurpleProtocolServer *protocol_server,
                                      PurpleConnection *connection,
                                      PurpleImage *img)
{
	PurpleProtocolServerInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_SERVER(protocol_server));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));

	iface = PURPLE_PROTOCOL_SERVER_GET_IFACE(protocol_server);
	if(iface != NULL && iface->set_buddy_icon != NULL) {
		iface->set_buddy_icon(protocol_server, connection, img);
	}
}

void
purple_protocol_server_remove_group(PurpleProtocolServer *protocol_server,
                                    PurpleConnection *connection,
                                    PurpleGroup *group)
{
	PurpleProtocolServerInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_SERVER(protocol_server));
	g_return_if_fail(PURPLE_IS_CONNECTION(connection));
	g_return_if_fail(PURPLE_IS_GROUP(group));

	iface = PURPLE_PROTOCOL_SERVER_GET_IFACE(protocol_server);
	if(iface != NULL && iface->remove_group != NULL) {
		iface->remove_group(protocol_server, connection, group);
	}
}

gint
purple_protocol_server_send_raw(PurpleProtocolServer *protocol_server,
                                PurpleConnection *connection,
                                const gchar *buf, gint len)
{
	PurpleProtocolServerInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_SERVER(protocol_server), -1);
	g_return_val_if_fail(PURPLE_IS_CONNECTION(connection), -1);

	iface = PURPLE_PROTOCOL_SERVER_GET_IFACE(protocol_server);
	if(iface != NULL && iface->send_raw != NULL) {
		return iface->send_raw(protocol_server, connection, buf, len);
	}

	return -1;
}
