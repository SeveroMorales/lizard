/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#ifndef PURPLE_HISTORY_MANAGER_H
#define PURPLE_HISTORY_MANAGER_H

#include <glib.h>
#include <glib-object.h>

#include "purplehistoryadapter.h"

G_BEGIN_DECLS

/**
 * PURPLE_HISTORY_MANAGER_DOMAIN:
 *
 * A #GError domain for errors from #PurpleHistoryManager.
 *
 * Since: 3.0.0
 */
#define PURPLE_HISTORY_MANAGER_DOMAIN (g_quark_from_static_string("purple-history-manager"))

#define PURPLE_TYPE_HISTORY_MANAGER (purple_history_manager_get_type())
G_DECLARE_FINAL_TYPE(PurpleHistoryManager, purple_history_manager, PURPLE,
                     HISTORY_MANAGER, GObject)

/**
 * PurpleHistoryManager:
 *
 * #PurpleHistoryManager keeps track of all adapters and emits signals when
 * adapters are registered and unregistered.
 *
 * Since: 3.0.0
 */

/**
 * PurpleHistoryManagerForeachFunc:
 * @adapter: The #PurpleHistoryAdapter instance.
 * @data: User supplied data.
 *
 * A function to be used as a callback with
 * purple_history_manager_foreach().
 *
 * Since: 3.0.0
 */
typedef void (*PurpleHistoryManagerForeachFunc)(PurpleHistoryAdapter *adapter, gpointer data);

/**
 * purple_history_manager_get_default:
 *
 * Gets the default #PurpleHistoryManager instance.
 *
 * Returns: (transfer none): The default #PurpleHistoryManager instance.
 *
 * Since: 3.0.0
 */
PurpleHistoryManager *purple_history_manager_get_default(void);

/**
 * purple_history_manager_get_active:
 * @manager: The #PurpleHistoryManager instance.
 *
 * Gets the active #PurpleHistoryAdapter instance.
 *
 * Returns: (transfer none): The active @adapter
 *
 * Since: 3.0.0
 */
 PurpleHistoryAdapter *purple_history_manager_get_active(PurpleHistoryManager *manager);

 /**
 * purple_history_manager_set_active:
 * @manager: The #PurpleHistoryManager instance.
 * @id: The id of the #PurpleHistoryAdapter to set active.
 * @error: A return address for a #GError.
 *
 * Sets the active #PurpleHistoryAdapter instance.
 *
 * Returns: %TRUE if setting the @adapter was successful with @manager
 *          %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_history_manager_set_active(PurpleHistoryManager *manager, const gchar *id, GError **error);

/**
 * purple_history_manager_register:
 * @manager: The #PurpleHistoryManager instance.
 * @adapter: The #PurpleHistoryAdapter to register.
 * @error: A return address for a #GError.
 *
 * Registers @adapter with @manager.
 *
 * Returns: %TRUE if @adapter was successfully registered with @manager,
 *          %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_history_manager_register(PurpleHistoryManager *manager, PurpleHistoryAdapter *adapter, GError **error);

/**
 * purple_history_manager_unregister:
 * @manager: The #PurpleHistoryManager instance.
 * @adapter: The #PurpleHistoryAdapter to unregister.
 * @error: A return address for a #GError.
 *
 * Unregisters @adapter from @manager.
 *
 * Returns: %TRUE if @adapter was successfully unregistered from @manager,
 *          %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_history_manager_unregister(PurpleHistoryManager *manager, PurpleHistoryAdapter *adapter, GError **error);

/**
 * purple_history_manager_find:
 * @manager: The #PurpleHistoryManager instance.
 * @id: The id of the #PurpleHistoryAdapter to find.
 *
 * Gets the #PurpleHistoryAdapter identified by @id if found, otherwise %NULL.
 *
 * Returns: (transfer none): The #PurpleHistoryAdapter identified by @id or %NULL.
 *
 * Since: 3.0.0
 */
PurpleHistoryAdapter *purple_history_manager_find(PurpleHistoryManager *manager, const gchar *id);

/**
 * purple_history_manager_get_all:
 * @manager: The #PurpleHistoryManager instance.
 *
 * Gets a list of all #PurpleHistoryAdapter's that are currently registered in
 * @manager.
 *
 * Returns: (transfer container) (element-type PurpleHistoryAdapter): The list
 *          containing all of the #PurpleHistoryAdapter's registered with @manager.
 *
 * Since: 3.0.0
 */
GList *purple_history_manager_get_all(PurpleHistoryManager *manager);

/**
 * purple_history_manager_query:
 * @manager: The #PurpleHistoryManager instance.
 * @query: A query to send to the @manager instance.
 * @error: A return address for a #GError.
 *
 * Sends a query to the #PurpleHistoryAdapter @manager instance.
 *
 * Returns: (transfer full) (element-type PurpleHistoryAdapter): The list
 *          containing all of the #PurpleMessage's that matched the query
 *          with @manager.
 *
 * Since: 3.0.0
 */
GList *purple_history_manager_query(PurpleHistoryManager *manager, const gchar *query, GError **error);

/**
 * purple_history_manager_remove:
 * @manager: The #PurpleHistoryManager instance.
 * @query: A query to send to the @manager instance.
 * @error: A return address for a #GError.
 *
 * Removes messages from the active #PurpleHistoryAdapter of @manager that match @query.
 *
 * Returns: %TRUE if messages matching @query were successfully removed from
 *          the active adapter of @manager, %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_history_manager_remove(PurpleHistoryManager *manager, const gchar *query, GError **error);

/**
 * purple_history_manager_write:
 * @manager: The #PurpleHistoryManager instance.
 * @conversation: The #PurpleConversation.
 * @message: The #PurpleMessage to pass to the @manager.
 * @error: A return address for a #GError.
 *
 * Writes @message to the active adapter of @manager.
 *
 * Returns: %TRUE if @message was successfully written, %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_history_manager_write(PurpleHistoryManager *manager, PurpleConversation *conversation, PurpleMessage *message, GError **error);

/**
 * purple_history_manager_foreach:
 * @manager: The #PurpleHistoryManager instance.
 * @func: (scope call): The #PurpleHistoryManagerForeachFunc to call.
 * @data: User data to pass to @func.
 *
 * Calls @func for each #PurpleHistoryAdapter that @manager knows about.
 *
 * Since: 3.0.0
 */
void purple_history_manager_foreach(PurpleHistoryManager *manager, PurpleHistoryManagerForeachFunc func, gpointer data);

G_END_DECLS

#endif /* PURPLE_HISTORY_MANAGER_H */
