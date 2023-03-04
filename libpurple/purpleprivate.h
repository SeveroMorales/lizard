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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_PRIVATE_H
#define PURPLE_PRIVATE_H

#include <glib.h>
#include <glib/gstdio.h>

#include "accounts.h"
#include "connection.h"
#include "purplecredentialprovider.h"
#include "purplehistoryadapter.h"

G_BEGIN_DECLS

/**
 * _purple_account_to_xmlnode:
 * @account:  The account
 *
 * Get an XML description of an account.
 *
 * Returns:  The XML description of the account.
 */
PurpleXmlNode *_purple_account_to_xmlnode(PurpleAccount *account);

/**
 * _purple_blist_get_last_child:
 * @node:  The node whose last child is to be retrieved.
 *
 * Returns the last child of a particular node.
 *
 * Returns: The last child of the node.
 */
PurpleBlistNode *_purple_blist_get_last_child(PurpleBlistNode *node);

/* This is for the accounts code to notify the buddy icon code that
 * it's done loading.  We may want to replace this with a signal. */
void
_purple_buddy_icons_account_loaded_cb(void);

/* This is for the buddy list to notify the buddy icon code that
 * it's done loading.  We may want to replace this with a signal. */
void
_purple_buddy_icons_blist_loaded_cb(void);

/**
 * _purple_connection_wants_to_die:
 * @gc:  The connection to check
 *
 * Checks if a connection is disconnecting, and should not attempt to reconnect.
 *
 * Note: This function should only be called by purple_account_set_enabled()
 *       in account.c.
 */
gboolean _purple_connection_wants_to_die(PurpleConnection *gc);

/**
 * _purple_connection_add_active_chat:
 * @gc:    The connection
 * @chat:  The chat conversation to add
 *
 * Adds a chat to the active chats list of a connection
 *
 * Note: This function should only be called by purple_serv_got_joined_chat()
 *       in server.c.
 */
void _purple_connection_add_active_chat(PurpleConnection *gc,
                                        PurpleChatConversation *chat);
/**
 * _purple_connection_remove_active_chat:
 * @gc:    The connection
 * @chat:  The chat conversation to remove
 *
 * Removes a chat from the active chats list of a connection
 *
 * Note: This function should only be called by purple_serv_got_chat_left()
 *       in server.c.
 */
void _purple_connection_remove_active_chat(PurpleConnection *gc,
                                           PurpleChatConversation *chat);

/**
 * _purple_statuses_get_primitive_scores:
 *
 * Note: This function should only be called by
 *       purple_buddy_presence_compute_score() in presence.c.
 *
 * Returns: The primitive scores array from status.c.
 */
int *_purple_statuses_get_primitive_scores(void);

/**
 * _purple_conversation_write_common:
 * @conv:    The conversation.
 * @msg:     The message.
 *
 * Writes to a conversation window.
 *
 * This function should not be used to write IM or chat messages. Use
 * purple_conversation_write_message() instead. This function will
 * most likely call this anyway, but it may do it's own formatting,
 * sound playback, etc. depending on whether the conversation is a chat or an
 * IM.
 *
 * See purple_conversation_write_message().
 */
void
_purple_conversation_write_common(PurpleConversation *conv, PurpleMessage *msg);

/**
 * purple_account_manager_startup:
 *
 * Starts up the account manager by creating the default instance.
 *
 * Since: 3.0.0
 */
void purple_account_manager_startup(void);

/**
 * purple_account_manager_shutdown:
 *
 * Shuts down the account manager by destroying the default instance.
 *
 * Since: 3.0.0
 */
void purple_account_manager_shutdown(void);

/**
 * purple_contact_manager_startup:
 *
 * Starts up the contact manager by creating the default instance.
 *
 * Since: 3.0.0
 */
void purple_contact_manager_startup(void);

/**
 * purple_contact_manager_shutdown:
 *
 * Shuts down the contact manager by destroying the default instance.
 *
 * Since: 3.0.0
 */
void purple_contact_manager_shutdown(void);

/**
 * purple_conversation_manager_startup:
 *
 * Starts up the conversation manager by creating the default instance.
 *
 * Since: 3.0.0
 */
void purple_conversation_manager_startup(void);

/**
 * purple_conversation_manager_shutdown:
 *
 * Shuts down the conversation manager by destroying the default instance.
 *
 * Since: 3.0.0
 */
void purple_conversation_manager_shutdown(void);

/**
 * purple_credential_manager_startup:
 *
 * Starts up the credential manager by creating the default instance.
 *
 * Since: 3.0.0
 */
void purple_credential_manager_startup(void);

/**
 * purple_credential_manager_shutdown:
 *
 * Shuts down the credential manager by destroying the default instance.
 *
 * Since: 3.0.0
 */
void purple_credential_manager_shutdown(void);

/**
 * purple_protocol_manager_startup:
 *
 * Starts up the protocol manager by creating the default instance.
 *
 * Since: 3.0.0
 */
void purple_protocol_manager_startup(void);

/**
 * purple_protocol_manager_shutdown:
 *
 * Shuts down the protocol manager by destroying the default instance.
 *
 * Since: 3.0.0
 */
void purple_protocol_manager_shutdown(void);

/**
 * purple_credential_provider_activate:
 * @provider: The #PurpleCredentialProvider instance.
 *
 * Tells a @provider that it has become the active provider.
 *
 * Since: 3.0.0
 */
void purple_credential_provider_activate(PurpleCredentialProvider *provider);

/**
 * purple_credential_provider_deactivate:
 * @provider: The #PurpleCredentialProvider instance.
 *
 * Tells @provider that another #PurpleCredentialProvider has become the active
 * provider.
 *
 * Since: 3.0.0
 */
void purple_credential_provider_deactivate(PurpleCredentialProvider *provider);

/**
 * purple_history_adapter_activate:
 * @adapter: The #PurpleHistoryAdapter instance.
 * @error: A return address for a #GError.
 *
 * Asks @adapter to become the active adapter. If @adapter can not become active
 * it should return %FALSE and set @error.
 *
 * Returns: %TRUE on success otherwise %FALSE with @error set.
 *
 * Since: 3.0.0
 */
gboolean purple_history_adapter_activate(PurpleHistoryAdapter *adapter, GError **error);

/**
 * purple_history_adapter_deactivate:
 * @adapter: The #PurpleHistoryAdapter instance.
 * @error: A return address for a #GError.
 *
 * Asks @adapter to stop being the active adapter. If @adapter can not
 * deactivate it should return %FALSE and set @error.
 *
 * Returns: %TRUE on success otherwise %FALSE with @error set.
 *
 * Since: 3.0.0
 */
gboolean purple_history_adapter_deactivate(PurpleHistoryAdapter *adapter, GError **error);

/**
 * purple_history_manager_startup:
 *
 * Starts up the history manager by creating the default instance.
 *
 * Since: 3.0.0
 */
void purple_history_manager_startup(void);

/**
 * purple_history_manager_shutdown:
 *
 * Shuts down the history manager by destroying the default instance.
 *
 * Since: 3.0.0
 */
void purple_history_manager_shutdown(void);

/**
 * purple_notification_manager_startup:
 *
 * Starts up the notification manager by creating the default instance.
 *
 * Since: 3.0.0
 */
void purple_notification_manager_startup(void);

/**
 * purple_notification_manager_shutdown:
 *
 * Shuts down the notification manager by destroying the default instance.
 *
 * Since: 3.0.0
 */
void purple_notification_manager_shutdown(void);

/**
 * purple_whiteboard_manager_startup:
 *
 * Starts up the whiteboard manager by creating the default instance.
 *
 * Since: 3.0.0
 */
void purple_whiteboard_manager_startup(void);

/**
 * purple_whiteboard_manager_shutdown:
 *
 * Shuts down the whiteboard manager by destroying the default instance.
 *
 * Since: 3.0.0
 */
void purple_whiteboard_manager_shutdown(void);

/**
 * purple_account_set_enabled_plain:
 * @account: The instance.
 * @enabled: Whether or not the account is enabled.
 *
 * This is a temporary method until we overhaul serialization of accounts.
 *
 * This method sets the enabled state of an account without any side effects.
 * Its primary usage is when loading accounts from disk, as without this, the
 * account attempts to connect immediately.
 *
 * Since: 3.0.0
 */
G_GNUC_INTERNAL void purple_account_set_enabled_plain(PurpleAccount *account, gboolean enabled);

G_END_DECLS

#endif /* PURPLE_PRIVATE_H */

