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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_PROTOCOL_SERVER_H
#define PURPLE_PROTOCOL_SERVER_H

#include <glib.h>
#include <glib-object.h>

#include <libpurple/account.h>
#include <libpurple/buddy.h>
#include <libpurple/connection.h>
#include <libpurple/group.h>
#include <libpurple/purplemessage.h>
#include <libpurple/purpleprotocol.h>

#define PURPLE_TYPE_PROTOCOL_SERVER (purple_protocol_server_get_type())
G_DECLARE_INTERFACE(PurpleProtocolServer, purple_protocol_server, PURPLE,
                    PROTOCOL_SERVER, PurpleProtocol)

G_BEGIN_DECLS

/**
 * PurpleProtocolServer:
 *
 * #PurpleProtocolServer describes the API for protocols that have a central
 * server.
 *
 * Since: 3.0.0
 */

/**
 * PurpleProtocolServerInterface:
 * @set_info: Sets the user's profile.
 * @get_info: Should arrange for purple_notify_userinfo() to be called with the
 *            requested user's profile.
 * @set_status: Sets the active status for the given account.
 * @set_idle: Set the idle time for the given account.
 * @change_passwd: Changes the users password.
 * @add_buddy: Add a buddy to a group on the server.
 * @add_buddies: Add multiple buddies on the server at once.
 * @remove_buddy: Removes the given buddy from the user's buddy list.
 * @remove_buddies: Removes multiple buddies from the user's buddy list.
 * @keepalive: If implemented, this will be called regularly for this
 *             protocol's active connections. You'd want to do this if you need
 *             to repeatedly send some kind of keepalive packet to the server
 *             to avoid being disconnected. ("Regularly" is defined to be 30
 *             unless @get_keepalive_interval is implemented to override it).
 * @get_keepalive_interval: If implemented, this will override the default
 *                          keepalive interval.
 * @alias_buddy: Save/store buddy's alias on server list/roster
 * @group_buddy: Change a buddy's group on a server list/roster
 * @rename_group: Rename a group on a server list/roster
 * @set_buddy_icon: Set the buddy icon for the given connection to @img. The
 *                  protocol does <emphasis>NOT</emphasis> own a reference to
 *                  @img; if it needs one, it must #g_object_ref(@img) itself.
 * @remove_group: Removes the given group from the users buddy list.
 * @send_raw: For use in plugins that may understand the underlying protocol.
 *
 * The protocol server interface.
 *
 * This interface provides a gateway between purple and the protocol's server.
 */
struct _PurpleProtocolServerInterface {
	/*< private >*/
	GTypeInterface parent;

	/*< public >*/
	void (*set_info)(PurpleProtocolServer *protocol_server, PurpleConnection *connection, const gchar *info);
	void (*get_info)(PurpleProtocolServer *protocol_server, PurpleConnection *connection, const gchar *who);

	void (*set_status)(PurpleProtocolServer *protocol_server, PurpleAccount *account, PurpleStatus *status);
	void (*set_idle)(PurpleProtocolServer *protocol_server, PurpleConnection *connection, gint idletime);

	void (*change_passwd)(PurpleProtocolServer *protocol_server, PurpleConnection *connection, const gchar *old_pass, const gchar *new_pass);

	void (*add_buddy)(PurpleProtocolServer *protocol_server, PurpleConnection *connection, PurpleBuddy *buddy, PurpleGroup *group, const gchar *message);
	void (*add_buddies)(PurpleProtocolServer *protocol_server, PurpleConnection *connection, GList *buddies, GList *groups, const gchar *message);
	void (*remove_buddy)(PurpleProtocolServer *protocol_server, PurpleConnection *connection, PurpleBuddy *buddy, PurpleGroup *group);
	void (*remove_buddies)(PurpleProtocolServer *protocol_server, PurpleConnection *connection, GList *buddies, GList *groups);

	void (*keepalive)(PurpleProtocolServer *protocol_server, PurpleConnection *connection);
	gint (*get_keepalive_interval)(PurpleProtocolServer *protocol_server);

	void (*alias_buddy)(PurpleProtocolServer *protocol_server, PurpleConnection *connection, const gchar *who, const gchar *alias);

	void (*group_buddy)(PurpleProtocolServer *protocol_server, PurpleConnection *connection, const gchar *who, const gchar *old_group, const gchar *new_group);

	void (*rename_group)(PurpleProtocolServer *protocol_server, PurpleConnection *connection, const gchar *old_name, PurpleGroup *group, GList *moved_buddies);

	void (*set_buddy_icon)(PurpleProtocolServer *protocol_server, PurpleConnection *connection, PurpleImage *img);

	void (*remove_group)(PurpleProtocolServer *protocol_server, PurpleConnection *connection, PurpleGroup *group);

	gint (*send_raw)(PurpleProtocolServer *protocol_server, PurpleConnection *connection, const gchar *buf, gint len);

	/*< private >*/
	gpointer reserved[8];
};

/**
 * purple_protocol_server_set_info:
 * @protocol_server: The #PurpleProtocolServer instance.
 * @connection: The #PurpleConnection instance.
 * @info: The user info to set.
 *
 * Sets the user info, sometimes referred to as a user profile to @info.
 *
 * Since: 3.0.0
 */
void purple_protocol_server_set_info(PurpleProtocolServer *protocol_server, PurpleConnection *connection, const gchar *info);

/**
 * purple_protocol_server_get_info:
 * @protocol_server: The #PurpleProtocolServer instance.
 * @connection: The #PurpleConnection instance.
 * @who: The name of the user whose information you're asking for.
 *
 * Gets the user info or profile for @who and displays it in a protocol
 * specific way.
 *
 * Since: 3.0.0
 */
void purple_protocol_server_get_info(PurpleProtocolServer *protocol_server, PurpleConnection *connection, const gchar *who);

/**
 * purple_protocol_server_set_status:
 * @protocol_server: The #PurpleProtocolServer instance.
 * @account: The #PurpleAccount instance.
 * @status: The #PurpleStatus instance.
 *
 * Sets the status for account @account to @status.
 *
 * Since: 3.0.0
 */
void purple_protocol_server_set_status(PurpleProtocolServer *protocol_server, PurpleAccount *account, PurpleStatus *status);

/**
 * purple_protocol_server_set_idle:
 * @protocol_server: The #PurpleProtocolServer instance.
 * @connection: The #PurpleConnection instance.
 * @idletime: The number of seconds that the user has been idle.
 *
 * Tells @protocol_server to set the user's idle time to @idletime.
 *
 * Since: 3.0.0
 */
void purple_protocol_server_set_idle(PurpleProtocolServer *protocol_server, PurpleConnection *connection, gint idletime);

/**
 * purple_protocol_server_change_passwd:
 * @protocol_server: The #PurpleProtocolServer instance.
 * @connection: The #PurpleConnection instance.
 * @old_pass: The user's old password.
 * @new_pass: The new password for the user.
 *
 * Changes the user's password from @old_pass to @new_pass.
 *
 * Since: 3.0.0
 */
void purple_protocol_server_change_passwd(PurpleProtocolServer *protocol_server, PurpleConnection *connection, const gchar *old_pass, const gchar *new_pass);

/**
 * purple_protocol_server_add_buddy:
 * @protocol_server: The #PurpleProtocolServer instance.
 * @connection: The #PurpleConnection instance.
 * @buddy: The #PurpleBuddy to add.
 * @group: The #PurpleGroup for @buddy.
 * @message: An optional invite message.
 *
 * This protocol function may be called in situations in which the buddy is
 * already in the specified group. If the protocol supports authorization and
 * the user is not already authorized to see the status of @buddy, this
 * function will request authorization. If authorization is required, then
 * @message will be used as an invite message.
 *
 * Since: 3.0.0
 */
void purple_protocol_server_add_buddy(PurpleProtocolServer *protocol_server, PurpleConnection *connection, PurpleBuddy *buddy, PurpleGroup *group, const gchar *message);

/**
 * purple_protocol_server_add_buddies:
 * @protocol_server: The #PurpleProtocolServer instance.
 * @connection: The #PurpleConnection instance.
 * @buddies: (element-type PurpleBuddy): A #GList of #PurpleBuddy's to add.
 * @groups: (element-type PurpleGroup): A #GList of #PurpleGroup's that
 *          correspond to the @buddies parameter.
 * @message: An optional invite message to send.
 *
 * Similar to purple_protocol_server_add_buddy() but can add multiple buddies
 * at a time. If @protocol_server does not implement this function, this will
 * call purple_protocol_server_add_buddy() for each buddy/group pair in
 * @buddies/@groups.
 *
 * Since: 3.0.0
 */
void purple_protocol_server_add_buddies(PurpleProtocolServer *protocol_server, PurpleConnection *connection, GList *buddies, GList *groups, const gchar *message);

/**
 * purple_protocol_server_remove_buddy:
 * @protocol_server: The #PurpleProtocolServer instance.
 * @connection: The #PurpleConnection instance.
 * @buddy: The #PurpleBuddy instance.
 * @group: (nullable): The #PurpleGroup instance.
 *
 * Removes @buddy and potentially @group from the server side list of contacts.
 *
 * Since: 3.0.0
 */
void purple_protocol_server_remove_buddy(PurpleProtocolServer *protocol_server, PurpleConnection *connection, PurpleBuddy *buddy, PurpleGroup *group);

/**
 * purple_protocol_server_remove_buddies:
 * @protocol_server: The #PurpleProtocolServer instance.
 * @connection: The #PurpleConnection instance.
 * @buddies: (element-type PurpleBuddy): A #GList of #PurpleBuddy's to remove.
 * @groups: (element-type PurpleGroup): A #GList of #PurpleGroup's
 *          corresponding to @buddies.
 *
 * Similar to purple_protocol_server_remove_buddy() but allows you to remove
 * multiple at a time.
 *
 * If @protocol_server doesn't implement this function directly,
 * purple_protocol_server_remove_buddy() will be called for each buddy/group
 * pair in @buddies/@groups.
 *
 * Since: 3.0.0
 */
void purple_protocol_server_remove_buddies(PurpleProtocolServer *protocol_server, PurpleConnection *connection, GList *buddies, GList *groups);

/**
 * purple_protocol_server_keepalive:
 * @protocol_server: The #PurpleProtocolServer instance.
 * @connection: The #PurpleConnection instance.
 *
 * Tell @protocol_server to send its keep alive to the server.
 *
 * Since: 3.0.0
 */
void purple_protocol_server_keepalive(PurpleProtocolServer *protocol_server, PurpleConnection *connection);

/**
 * purple_protocol_server_get_keepalive_interval:
 * @protocol_server: The #PurpleProtocolServer instance.
 *
 * Returns a custom interval, in seconds, that libpurple should tell
 * @protocol_server to send its keepalive.
 *
 * Returns: The interval, in seconds, that the keep-alive function should be
 *          called.
 *
 * Since: 3.0.0
 */
gint purple_protocol_server_get_keepalive_interval(PurpleProtocolServer *protocol_server);

/**
 * purple_protocol_server_alias_buddy:
 * @protocol_server: The #PurpleProtocolServer instance.
 * @connection: The #PurpleConnection instance.
 * @who: The name of the user to alias.
 * @alias: The new alias for @who.
 *
 * Sets the server side alias for @who to @alias.
 *
 * Since: 3.0.0
 */
void purple_protocol_server_alias_buddy(PurpleProtocolServer *protocol_server, PurpleConnection *connection, const gchar *who, const gchar *alias);

/**
 * purple_protocol_server_group_buddy:
 * @protocol_server: The #PurpleProtocolServer instance.
 * @connection: The #PurpleConnection instance.
 * @who: The name of the user whose group to switch.
 * @old_group: The name of @who's old group.
 * @new_group: The name of the new group to add @who to.
 *
 * Moves @who from group @old_group to a new group of @new_group.
 *
 * Since: 3.0.0
 */
void purple_protocol_server_group_buddy(PurpleProtocolServer *protocol_server, PurpleConnection *connection, const gchar *who, const gchar *old_group, const gchar *new_group);

/**
 * purple_protocol_server_rename_group:
 * @protocol_server: The #PurpleProtocolServer instance.
 * @connection:  The #PurpleConnection instance.
 * @old_name: The old name of the group.
 * @group: The new #PurpleGroup instance.
 * @moved_buddies: (element-type PurpleBuddy): A list of #PurpleBuddy's being
 *                 moved as part of this rename.
 *
 * Renames the group named @old_name to the new @group.
 *
 * Since: 3.0.0
 */
void purple_protocol_server_rename_group(PurpleProtocolServer *protocol_server, PurpleConnection *connection, const gchar *old_name, PurpleGroup *group, GList *moved_buddies);

/**
 * purple_protocol_server_set_buddy_icon:
 * @protocol_server: The #PurpleProtocolServer instance.
 * @connection: The #PurpleConnection instance.
 * @img: (nullable): The #PurpleImage instance, or %NULL to unset the icon.
 *
 * Sets the user's buddy icon to @img.
 *
 * Since: 3.0.0
 */
void purple_protocol_server_set_buddy_icon(PurpleProtocolServer *protocol_server, PurpleConnection *connection, PurpleImage *img);

/**
 * purple_protocol_server_remove_group:
 * @protocol_server: The #PurpleProtocolServer instance.
 * @connection: The #PurpleConnection instance.
 * @group: The #PurpleGroup instance.
 *
 * Removes @group from the server side contact list.
 *
 * Since: 3.0.0
 */
void purple_protocol_server_remove_group(PurpleProtocolServer *protocol_server, PurpleConnection *connection, PurpleGroup *group);

/**
 * purple_protocol_server_send_raw:
 * @protocol_server: The #PurpleProtocolServer instance.
 * @connection: The #PurpleConnection instance.
 * @buf: The raw protocol data to send.
 * @len: The length of @buf in bytes.
 *
 * Sends raw data over the protocol. This should only be called when you know
 * the exact underlying protocol.
 *
 * Returns: The number of bytes that was sent.
 *
 * Since: 3.0.0
 */
gint purple_protocol_server_send_raw(PurpleProtocolServer *protocol_server, PurpleConnection *connection, const gchar *buf, gint len);

G_END_DECLS

#endif /* PURPLE_PROTOCOL_SERVER_H */

