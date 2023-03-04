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

#ifndef PURPLE_NOTIFICATION_MANAGER_H
#define PURPLE_NOTIFICATION_MANAGER_H

#include <glib.h>
#include <glib-object.h>

#include "account.h"
#include <purplenotification.h>

G_BEGIN_DECLS

#define PURPLE_TYPE_NOTIFICATION_MANAGER (purple_notification_manager_get_type())
G_DECLARE_FINAL_TYPE(PurpleNotificationManager, purple_notification_manager,
                     PURPLE, NOTIFICATION_MANAGER, GObject)

/**
 * PurpleNotificationManager:
 *
 * Purple Notification Manager manages all notifications between protocols and
 * plugins and how the user interface interacts with them.
 *
 * Since: 3.0.0
 */

/**
 * purple_notification_manager_get_default:
 *
 * Gets the default [class@NotificationManager] instance.
 *
 * Returns: (transfer none): The default instance.
 *
 * Since: 3.0.0
 */
PurpleNotificationManager *purple_notification_manager_get_default(void);

/**
 * purple_notification_manager_get_default_as_model:
 *
 * Gets the default manager instance type casted to [iface@Gio.ListModel].
 *
 * Returns: (transfer none): The model.
 *
 * Since: 3.0.0
 */
GListModel *purple_notification_manager_get_default_as_model(void);

/**
 * purple_notification_manager_add:
 * @manager: The instance.
 * @notification: (transfer full): The [class@Notification] to add.
 *
 * Adds @notification into @manager.
 *
 * Since: 3.0.0
 */
void purple_notification_manager_add(PurpleNotificationManager *manager, PurpleNotification *notification);

/**
 * purple_notification_manager_remove:
 * @manager: The instance.
 * @notification: The notification to remove.
 *
 * Removes @notification from @manager.
 *
 * Since: 3.0.0
 */
void purple_notification_manager_remove(PurpleNotificationManager *manager, PurpleNotification *notification);

/**
 * purple_notification_manager_remove_with_account:
 * @manager: The instance.
 * @account: The [class@Account] whose notifications to remove.
 * @all: Whether or not to clear connection error notifications as well.
 *
 * Removes all notifications with @account from @manager.
 *
 * If @all is set to %TRUE, notifications of type
 * PURPLE_NOTIFICATION_TYPE_CONNECTION_ERROR will be removed as well. These are
 * treated differently from other notifications tied to accounts, as those are
 * transient and depend on the account being connected to be valid.
 *
 * Since: 3.0.0
 */
void purple_notification_manager_remove_with_account(PurpleNotificationManager *manager, PurpleAccount *account, gboolean all);

/**
 * purple_notification_manager_get_unread_count:
 * @manager: The instance.
 *
 * Gets the number of currently unread notifications.
 *
 * Returns: The number of unread notifications.
 *
 * Since: 3.0.0
 */
guint purple_notification_manager_get_unread_count(PurpleNotificationManager *manager);

/**
 * purple_notification_manager_clear:
 * @manager: The instance.
 *
 * Removes all notifications from @manager.
 *
 * Since: 3.0.0
 */
void purple_notification_manager_clear(PurpleNotificationManager *manager);

G_END_DECLS

#endif /* PURPLE_NOTIFICATION_MANAGER_H */
