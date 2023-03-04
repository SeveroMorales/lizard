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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_CONVERSATION_MANAGER_H
#define PURPLE_CONVERSATION_MANAGER_H

#include <glib.h>

#include <purpleconversation.h>

G_BEGIN_DECLS

#define PURPLE_TYPE_CONVERSATION_MANAGER (purple_conversation_manager_get_type())

/**
 * PurpleConversationManager:
 *
 * #PurpleConversationManager keeps track of all #PurpleConversation's inside
 * of libpurple and allows searching of them.
 *
 * Since: 3.0.0
 */
G_DECLARE_FINAL_TYPE(PurpleConversationManager, purple_conversation_manager,
                     PURPLE, CONVERSATION_MANAGER, GObject)

/**
 * PurpleConversationManagerForeachFunc:
 * @conversation: The #PurpleConversation instance.
 * @data: User supplied data.
 *
 * A function to be used as a callback with
 * purple_conversation_manager_foreach().
 *
 * Since: 3.0.0
 */
typedef void (*PurpleConversationManagerForeachFunc)(PurpleConversation *conversation, gpointer data);

/**
 * purple_conversation_manager_get_default:
 *
 * Gets the default instance of #PurpleConversationManager.  This instance
 * can be used for any of the API including connecting to signals.
 *
 * Returns: (transfer none): The default #PurpleConversationManager instance.
 *
 * Since: 3.0.0
 */
PurpleConversationManager *purple_conversation_manager_get_default(void);

/**
 * purple_conversation_manager_register:
 * @manager: The #PurpleConversationManager instance.
 * @conversation: The #PurpleConversation to register.
 *
 * Registers @conversation with @manager.
 *
 * Returns: %TRUE if @conversation was not yet registered.
 *
 * Since: 3.0.0
 */
gboolean purple_conversation_manager_register(PurpleConversationManager *manager, PurpleConversation *conversation);

/**
 * purple_conversation_manager_unregister:
 * @manager: The #PurpleConversationManager instance.
 * @conversation: The #PurpleConversation to unregister.
 *
 * Unregisters @conversation with @manager.
 *
 * Returns: %TRUE if @conversation was found and unregistered.
 *
 * Since: 3.0.0
 */
gboolean purple_conversation_manager_unregister(PurpleConversationManager *manager, PurpleConversation *conversation);

/**
 * purple_conversation_manager_is_registered:
 * @manager: The #PurpleConversationManager instance.
 * @conversation: The #PurpleConversation instance.
 *
 * Checks if @conversation is registered with @manager.
 *
 * Returns: %TRUE if @conversation is registered with @manager, %FALSE
 *          otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_conversation_manager_is_registered(PurpleConversationManager *manager, PurpleConversation *conversation);

/**
 * purple_conversation_manager_foreach:
 * @manager: The #PurpleConversationManager instance.
 * @func: (scope call): The #PurpleConversationManagerForeachFunc to call.
 * @data: User data to pass to @func.
 *
 * Calls @func for each #PurpleConversation that @manager knows about.
 *
 * Since: 3.0.0
 */
void purple_conversation_manager_foreach(PurpleConversationManager *manager, PurpleConversationManagerForeachFunc func, gpointer data);

/**
 * purple_conversation_manager_get_all:
 * @manager: The #PurpleConversationManager instance.
 *
 * Gets a list of all conversations that are registered with @manager.
 *
 * Returns: (transfer container) (element-type PurpleConversation): A list of
 *          all of the registered conversations.
 *
 * Since: 3.0.0
 */
GList *purple_conversation_manager_get_all(PurpleConversationManager *manager);

/**
 * purple_conversation_manager_find:
 * @manager: The #PurpleConversationManager instance.
 * @account: The #PurpleAccount instance whose conversation to find.
 * @name: The name of the conversation.
 *
 * Looks for a registered conversation belonging to @account and named @named.
 * This function will return the first one matching the given criteria. If you
 * specifically need an im or chat see purple_conversation_manager_find_im()
 * or purple_conversation_manager_find_chat().
 *
 * Returns: (transfer none): The #PurpleConversation if found, otherwise %NULL.
 *
 * Since: 3.0.0
 */
PurpleConversation *purple_conversation_manager_find(PurpleConversationManager *manager, PurpleAccount *account, const gchar *name);

/**
 * purple_conversation_manager_find_im:
 * @manager: The #PurpleConversationManager instance.
 * @account: The #PurpleAccount instance whose conversation to find.
 * @name: The name of the conversation.
 *
 * Looks for a registered im conversation belonging to @account and named
 * @name.
 *
 * Returns: (transfer none): The #PurpleConversation if found, otherwise %NULL.
 *
 * Since: 3.0.0
 */
PurpleConversation *purple_conversation_manager_find_im(PurpleConversationManager *manager, PurpleAccount *account, const gchar *name);

/**
 * purple_conversation_manager_find_chat:
 * @manager: The #PurpleConversationManager instance.
 * @account: The #PurpleAccount instance whose conversation to find.
 * @name: The name of the conversation.
 *
 * Looks for a registered chat conversation belonging to @account and named
 * @name.
 *
 * Returns: (transfer none): The #PurpleConversation if found, otherwise %NULL.
 *
 * Since: 3.0.0
 */
PurpleConversation *purple_conversation_manager_find_chat(PurpleConversationManager *manager, PurpleAccount *account, const gchar *name);

/**
 * purple_conversation_manager_find_chat_by_id:
 * @manager: The #PurpleConversationManager instance.
 * @account: The #PurpleAccount instance whose conversation to find.
 * @id: The id of the conversation.
 *
 * Looks for a registered chat conversation belonging to @account with an id of
 * @id.
 *
 * This is typically only called by protocols.
 *
 * Returns: (transfer none): The #PurpleConversation if found, otherwise %NULL.
 *
 * Since: 3.0.0
 */
PurpleConversation *purple_conversation_manager_find_chat_by_id(PurpleConversationManager *manager, PurpleAccount *account, gint id);

G_END_DECLS

#endif /* PURPLE_CONVERSATION_MANAGER_H */
