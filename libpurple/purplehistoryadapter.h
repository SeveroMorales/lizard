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

#ifndef PURPLE_HISTORY_ADAPTER_H
#define PURPLE_HISTORY_ADAPTER_H

#include <glib.h>
#include <glib-object.h>

#include <purplemessage.h>
#include <purpleconversation.h>

G_BEGIN_DECLS

/**
 * PURPLE_HISTORY_ADAPTER_DOMAIN:
 *
 * A #GError domain for errors.
 *
 * Since: 3.0.0
 */
#define PURPLE_HISTORY_ADAPTER_DOMAIN (g_quark_from_static_string("purple-history-adapter"))

/**
 * PurpleHistoryAdapter:
 *
 * #PurpleHistoryAdapter is a base class that should be sub classed by
 * history adapters. It defines the behavior of all history adapters
 * and implements some shared properties.
 *
 * Since: 3.0.0
 */

#define PURPLE_TYPE_HISTORY_ADAPTER (purple_history_adapter_get_type())
G_DECLARE_DERIVABLE_TYPE(PurpleHistoryAdapter, purple_history_adapter,
                         PURPLE, HISTORY_ADAPTER, GObject)

/**
 * PurpleHistoryAdapterClass:
 *
 * #PurpleHistoryAdapterClass defines the interface for interacting with
 * history adapters like sqlite, and so on.
 *
 * Since: 3.0.0
 */
struct _PurpleHistoryAdapterClass {
	/*< private >*/
	GObjectClass parent;

	/*< public >*/
	gboolean (*activate)(PurpleHistoryAdapter *adapter, GError **error);
	gboolean (*deactivate)(PurpleHistoryAdapter *adapter, GError **error);
	GList* (*query)(PurpleHistoryAdapter *adapter, const gchar *query, GError **error);
	gboolean (*remove)(PurpleHistoryAdapter *adapter, const gchar *query, GError **error);
	gboolean (*write)(PurpleHistoryAdapter *adapter, PurpleConversation *conversation, PurpleMessage *message, GError **error);

	/*< private >*/

	/* Some extra padding to play it safe. */
	gpointer reserved[8];
};

/**
 * purple_history_adapter_get_id:
 * @adapter: The #PurpleHistoryAdapter instance.
 *
 * Gets the identifier of @adapter.
 *
 * Returns: The identifier of @adapter.
 *
 * Since: 3.0.0
 */
const gchar *purple_history_adapter_get_id(PurpleHistoryAdapter *adapter);

/**
 * purple_history_adapter_get_name:
 * @adapter: The #PurpleHistoryAdapter instance.
 *
 * Gets the name of @adapter.
 *
 * Returns: The name of @adapter.
 *
 * Since: 3.0.0
 */
const gchar *purple_history_adapter_get_name(PurpleHistoryAdapter *adapter);

/**
 * purple_history_adapter_write:
 * @adapter: The #PurpleHistoryAdapter instance.
 * @conversation: The #PurpleConversation to send to the adapter.
 * @message: The #PurpleMessage to send to the adapter.
 * @error: A return address for a #GError.
 *
 * Writes a message to the @adapter.
 *
 * Returns: If the write was successful to the @adapter.
 *
 * Since: 3.0.0
 */
gboolean purple_history_adapter_write(PurpleHistoryAdapter *adapter,
                                      PurpleConversation *conversation,
                                      PurpleMessage *message,
                                      GError **error);

/**
 * purple_history_adapter_query:
 * @adapter: The #PurpleHistoryAdapter instance.
 * @query: The query to send to the @adapter.
 * @error: A return address for a #GError.
 *
 * Runs @query against @adapter.
 *
 * Returns: (element-type PurpleMessage) (transfer container): A list of messages that match @query.
 *
 * Since: 3.0.0
 */
GList *purple_history_adapter_query(PurpleHistoryAdapter *adapter,
                                    const gchar *query,
                                    GError **error);

/**
 * purple_history_adapter_remove:
 * @adapter: The #PurpleHistoryAdapter instance.
 * @query: Tells @adapter to remove messages that match @query.
 * @error: A return address for a #GError.
 *
 * Tells @adapter to remove messages that match @query
 *
 * Returns: If removing the messages was successful.
 *
 * Since: 3.0.0
 */
gboolean purple_history_adapter_remove(PurpleHistoryAdapter *adapter,
                                       const gchar *query,
                                       GError **error);

G_END_DECLS

#endif /* PURPLE_HISTORY_ADAPTER */
