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

#ifndef PURPLE_PROTOCOL_CLIENT_H
#define PURPLE_PROTOCOL_CLIENT_H

#include <glib.h>
#include <glib-object.h>

#include <libpurple/account.h>
#include <libpurple/connection.h>
#include <libpurple/purpleprotocol.h>

#define PURPLE_TYPE_PROTOCOL_CLIENT (purple_protocol_client_get_type())
G_DECLARE_INTERFACE(PurpleProtocolClient, purple_protocol_client, PURPLE,
                    PROTOCOL_CLIENT, PurpleProtocol)

/**
 * PurpleProtocolClient:
 *
 * #PurpleProtocolClient interface defines the behavior of a typical chat
 * service's client interface.
 */

/**
 * PurpleProtocolClientInterface:
 * @get_actions: Returns the actions the protocol can perform. These will show
 *               up in the Accounts menu, under a submenu with the name of the
 *               account.
 * @list_emblem: Fills the four <type>char**</type>'s with string identifiers
 *               for "emblems" that the UI will interpret and display as
 *               relevant.
 * @status_text: Gets a short string representing this buddy's status. This
 *               will be shown on the buddy list.
 * @tooltip_text: Allows the protocol to add text to a buddy's tooltip.
 * @blist_node_menu: Returns a list of #PurpleActionMenu structs, which
 *                   represent extra actions to be shown in (for example) the
 *                   right-click menu for @node.
 * @buddy_free: Allows the protocol to clean up any additional data for the
 *              given buddy.
 * @convo_closed: Allows the protocol to do any necessary cleanup when a
 *                conversation is closed.
 * @normalize: Convert the username @who to its canonical form. Also checks for
 *             validity.
 *             <sbr/>For example, AIM treats "fOo BaR" and "foobar" as the same
 *             user; this function should return the same normalized string for
 *             both of those. On the other hand, both of these are invalid for
 *             protocols with number-based usernames, so function should return
 *             %NULL in such case.
 *             <sbr/>@account: The account the username is related to. Can be
 *                             %NULL.
 *             <sbr/>@who:     The username to convert.
 *             <sbr/>Returns:  Normalized username, or %NULL, if it's invalid.
 * @find_blist_chat: Attempts to find a chat with the given name in the contact
 *                   list.
 * @offline_message: Checks whether offline messages to @buddy are supported.
 *                   <sbr/>Returns: %TRUE if @buddy can be sent messages while
 *                                  they are offline, or %FALSE if not.
 * @get_account_text_table: This allows protocols to specify additional strings
 *                          to be used for various purposes. The idea is to
 *                          stuff a bunch of strings in this hash table instead
 *                          of expanding the struct for every addition. This
 *                          hash table is allocated every call and
 *                          <emphasis>MUST</emphasis> be unrefed by the caller.
 *                          <sbr/>@account: The account to specify.  This can be
 *                                          %NULL.
 *                          <sbr/>Returns:  The protocol's string hash table.
 *                                          The hash table should be destroyed
 *                                          by the caller when it's no longer
 *                                          needed.
 * @get_moods: Returns an array of #PurpleMood's, with the last one having
 *             "mood" set to %NULL.
 * @get_max_message_size: Gets the maximum message size in bytes for the
 *                        conversation.
 *                        <sbr/>It may depend on connection-specific or
 *                        conversation-specific variables, like channel or
 *                        buddy's name length.
 *                        <sbr/>This value is intended for plaintext message,
 *                              the exact value may be lower because of:
 *                        <sbr/> - used newlines (some protocols count them as
 *                                 more than one byte),
 *                        <sbr/> - formatting,
 *                        <sbr/> - used special characters.
 *                        <sbr/>@conv:   The conversation to query, or NULL to
 *                                       get safe minimum for the protocol.
 *                        <sbr/>Returns: Maximum message size, 0 if unspecified,
 *                                       -1 for infinite.
 *
 * The protocol client interface.
 *
 * This interface provides a gateway between purple and the protocol.
 */
struct _PurpleProtocolClientInterface {
	/*< private >*/
	GTypeInterface parent;

	/*< public >*/
	const gchar *(*list_emblem)(PurpleProtocolClient *client, PurpleBuddy *buddy);

	gchar *(*status_text)(PurpleProtocolClient *client, PurpleBuddy *buddy);

	void (*tooltip_text)(PurpleProtocolClient *client, PurpleBuddy *buddy, PurpleNotifyUserInfo *user_info,
						 gboolean full);

	GList *(*blist_node_menu)(PurpleProtocolClient *client, PurpleBlistNode *node);

	void (*buddy_free)(PurpleProtocolClient *client, PurpleBuddy *buddy);

	void (*convo_closed)(PurpleProtocolClient *client, PurpleConnection *connection, const gchar *who);

	const gchar *(*normalize)(PurpleProtocolClient *client, PurpleAccount *account, const gchar *who);

	PurpleChat *(*find_blist_chat)(PurpleProtocolClient *client, PurpleAccount *account, const gchar *name);

	gboolean (*offline_message)(PurpleProtocolClient *client, PurpleBuddy *buddy);

	GHashTable *(*get_account_text_table)(PurpleProtocolClient *client, PurpleAccount *account);

	PurpleMood *(*get_moods)(PurpleProtocolClient *client, PurpleAccount *account);

	gssize (*get_max_message_size)(PurpleProtocolClient *client, PurpleConversation *conv);

	/*< private >*/
	gpointer reserved[4];
};

G_BEGIN_DECLS

/**
 * purple_protocol_client_list_emblem:
 * @client: The #PurpleProtocolClient instance.
 * @buddy: The #PurpleBuddy instance.
 *
 * Gets the icon name of the emblem that should be used for @buddy.
 *
 * Returns: The icon name of the emblem or %NULL.
 *
 * Since: 3.0.0
 */
const gchar *purple_protocol_client_list_emblem(PurpleProtocolClient *client, PurpleBuddy *buddy);

/**
 * purple_protocol_client_status_text:
 * @client: The #PurpleProtocolClient instance.
 * @buddy: The #PurpleBuddy instance.
 *
 * Gets the status text for @buddy.
 *
 * Returns: (transfer full): The status text for @buddy or %NULL.
 *
 * Since: 3.0.0
 */
gchar *purple_protocol_client_status_text(PurpleProtocolClient *client, PurpleBuddy *buddy);

/**
 * purple_protocol_client_tooltip_text:
 * @client: The #PurpleProtocolClient instance.
 * @buddy: The #PurpleBuddy instance.
 * @user_info: The #PurpleNotifyUserInfo instance.
 * @full: Whether or not additional info should be added.
 *
 * Asks @client to update @user_info for @buddy.  If @full is %TRUE then
 * more detailed information will added.
 *
 * Since: 3.0.0
 */
void purple_protocol_client_tooltip_text(PurpleProtocolClient *client, PurpleBuddy *buddy, PurpleNotifyUserInfo *user_info, gboolean full);

/**
 * purple_protocol_client_blist_node_menu:
 * @client: The #PurpleProtocolClient instance.
 * @node: The #PurpleBlistNode instance.
 *
 * Gets a list of #PurpleActionMenu structs, which represent extra actions to
 * be shown in (for example) the right-click menu for @node.
 *
 * Returns: (transfer full) (element-type PurpleActionMenu): The list of
 *          #PurpleActionMenu structs to display for @node.
 *
 * Since: 3.0.0
 */
GList *purple_protocol_client_blist_node_menu(PurpleProtocolClient *client, PurpleBlistNode *node);

/**
 * purple_protocol_client_buddy_free:
 * @client: The #PurpleProtocolClient instance.
 * @buddy: A #PurpleBuddy instance.
 *
 * Cleans up any protocol specific data for @buddy.
 *
 * Since: 3.0.0
 */
void purple_protocol_client_buddy_free(PurpleProtocolClient *client, PurpleBuddy *buddy);

/**
 * purple_protocol_client_convo_closed:
 * @client: The #PurpleProtocolClient instance.
 * @connection: A #PurpleConnection instance.
 * @who: The name of the conversation to close.
 *
 * Closes the conversation named @who on connection @connection.
 *
 * Since: 3.0.0
 */
void purple_protocol_client_convo_closed(PurpleProtocolClient *client, PurpleConnection *connection, const gchar *who);

/**
 * purple_protocol_client_normalize:
 * @client: The #PurpleProtocolClient instance.
 * @account: (nullable): A #PurpleAccount instance.
 * @who: The name to normalize.
 *
 * Normalizes a @who to the canonical form for the protocol.  For example, many
 * protocols only support all lower case, but might have display version where
 * there are capital letters.
 *
 * Returns: The normalized version of @who for @account.
 *
 * Since: 3.0.0
 *
 * Deprecated: 3.0.0: This should use purple_protcol_client_normalize_name when
 *             it is created which will return an allocated value.
 */
const gchar *purple_protocol_client_normalize(PurpleProtocolClient *client, PurpleAccount *account, const gchar *who);

/**
 * purple_protocol_client_find_blist_chat:
 * @client: The #PurpleProtocolClient instance.
 * @account: A #PurpleAccount instance.
 * @name: The name of the chat to find.
 *
 * Looks for a chat named @name in the contact list of @account.
 *
 * Returns: (transfer none): The #PurpleChat instance or %NULL if no chat could
 *          be found.
 *
 * Since: 3.0.0
 */
PurpleChat *purple_protocol_client_find_blist_chat(PurpleProtocolClient *client, PurpleAccount *account, const gchar *name);

/**
 * purple_protocol_client_offline_message:
 * @client: The #PurpleProtocolClient instance.
 * @buddy: A #PurpleBuddy instance.
 *
 * Checks whether offline messages to @buddy are supported.
 *
 * Returns: %TRUE if @buddy supports offline messages, otherwise %FALSE.
 *
 * Since: 3.0.0
 */
gboolean purple_protocol_client_offline_message(PurpleProtocolClient *client, PurpleBuddy *buddy);

/**
 * purple_protocol_client_get_account_text_table:
 * @client: The #PurpleProtocolClient instance.
 * @account: (nullable): A #PurpleAccount instance.
 *
 * Gets the account text table which allows protocols to specify additional
 * strings to be used for various purposes. The idea is to stuff a bunch of
 * strings in this hash table instead of expanding the struct for every
 * addition.
 *
 * Returns: (transfer full): The newly allocated text table.
 *
 * Since: 3.0.0
 *
 * Deprecated: 3.0.0: This is a premature optimization. Right now this is only
 *             used by GaduGadu for a single item and should be replaced.
 */
GHashTable *purple_protocol_client_get_account_text_table(PurpleProtocolClient *client, PurpleAccount *account);

/**
 * purple_protocol_client_get_moods:
 * @client: The #PurpleProtocolClient instance.
 * @account: A #PurpleAccount instance.
 *
 * Gets the mood's for @account.
 *
 * Returns: (transfer none): A %NULL terminated array of #PurpleMood's.
 *
 * Since: 3.0.0
 */
PurpleMood *purple_protocol_client_get_moods(PurpleProtocolClient *client, PurpleAccount *account);

/**
 * purple_protocol_client_get_max_message_size:
 * @client: The #PurpleProtocolClient instance.
 * @conv: A #PurpleConversation instance.
 *
 * Gets the maximum number of characters per message for @conv.
 *
 * Returns: The maximum number of characters per message for @conv or -1 for no
 *          limit.
 *
 * Since: 3.0.0
 */
gssize purple_protocol_client_get_max_message_size(PurpleProtocolClient *client, PurpleConversation *conv);

G_END_DECLS

#endif /* PURPLE_PROTOCOL_CLIENT_H */
