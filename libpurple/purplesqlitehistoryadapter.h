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

#ifndef PURPLE_SQLITE_HISTORY_ADAPTER_H
#define PURPLE_SQLITE_HISTORY_ADAPTER_H

#include <glib.h>
#include <glib-object.h>

#include <purplehistoryadapter.h>
#include <purplemessage.h>

G_BEGIN_DECLS

/**
 * PurpleSqliteHistoryAdapter:
 *
 * #PurpleSqliteHistoryAdapter is a class that allows interfacing with an
 * SQLite database to store history. It is a subclass of @PurpleHistoryAdapter.
 *
 * Since: 3.0.0
 */

#define PURPLE_TYPE_SQLITE_HISTORY_ADAPTER (purple_sqlite_history_adapter_get_type())
G_DECLARE_FINAL_TYPE(PurpleSqliteHistoryAdapter, purple_sqlite_history_adapter,
                     PURPLE, SQLITE_HISTORY_ADAPTER, PurpleHistoryAdapter)

/**
 * purple_sqlite_history_adapter_new:
 * @filename: The filename of the sqlite database.
 *
 * Creates a new #PurpleHistoryAdapter.
 *
 * Returns: (transfer full): The new #PurpleSqliteHistoryAdapter instance.
 *
 * Since: 3.0.0
 */
PurpleHistoryAdapter *purple_sqlite_history_adapter_new(const gchar *filename);

/**
 * purple_sqlite_history_adapter_get_filename
 * @adapter: The #PurpleSqliteHistoryAdapter instance
 *
 * Gets the filename of the sqlite database.
 *
 * Returns: The filename that the @adapter reads and writes to
 *
 * Since: 3.0.0
 */
const gchar *purple_sqlite_history_adapter_get_filename(PurpleSqliteHistoryAdapter *adapter);

G_END_DECLS

#endif /* PURPLE_SQLITE_HISTORY_ADAPTER */
