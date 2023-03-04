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
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_NOTIFICATION_H
#define PURPLE_NOTIFICATION_H

#include <glib.h>
#include <glib-object.h>

#include "account.h"
#include "purpleauthorizationrequest.h"
#include "purpleaddcontactrequest.h"

G_BEGIN_DECLS

/**
 * PurpleNotificationType:
 *
 * Since: 3.0.0.
 */
typedef enum {
    PURPLE_NOTIFICATION_TYPE_UNKNOWN,
    PURPLE_NOTIFICATION_TYPE_GENERIC,
    PURPLE_NOTIFICATION_TYPE_CONNECTION_ERROR,
    PURPLE_NOTIFICATION_TYPE_AUTHORIZATION_REQUEST,
    PURPLE_NOTIFICATION_TYPE_ADD_CONTACT,
    PURPLE_NOTIFICATION_TYPE_FILE_TRANSFER,
    PURPLE_NOTIFICATION_TYPE_CHAT_INVITE,
    PURPLE_NOTIFICATION_TYPE_MENTION,
    PURPLE_NOTIFICATION_TYPE_REACTION,
} PurpleNotificationType;

/**
 * PurpleNotification:
 *
 * An object that represents a notification.
 *
 * Since: 3.0.0
 */

#define PURPLE_TYPE_NOTIFICATION (purple_notification_get_type())
G_DECLARE_FINAL_TYPE(PurpleNotification, purple_notification, PURPLE,
                     NOTIFICATION, GObject)

/**
 * purple_notification_new:
 * @type: The [enum@NotificationType] of the notification.
 * @account: (nullable): The [class@Account] that created the notification if
 *           applicable.
 * @data: The data for the notification.
 * @data_destroy_func: A GDestroyNotify to call to free @data.
 *
 * Creates a new notification with the given properties. @account is optional.
 *
 * Once the notification is prepared, it should be added to a
 * [class@NotificationManager] to be presented to the user.
 *
 * Returns: (transfer full): The new notification.
 *
 * Since: 3.0.0
 */
PurpleNotification *purple_notification_new(PurpleNotificationType type, PurpleAccount *account, gpointer data, GDestroyNotify data_destroy_func);

/**
 * purple_notification_new_from_add_contact_request:
 * @request: (transfer full): The [class@AddContactRequest] instance.
 *
 * Creates a new [class@Notification] for @request. This helper will
 * automatically fill out the notification according to the information in
 * @request.
 *
 * Returns: (transfer full): The new notification.
 *
 * Since: 3.0.0.
 */
PurpleNotification *purple_notification_new_from_add_contact_request(PurpleAddContactRequest *request);

/**
 * purple_notification_new_from_authorization_request:
 * @authorization_request: (transfer full): The [class@AuthorizationRequest]
 *                         instance.
 *
 * Creates a new [class@Notification] for the @authorization_request. This
 * helper will automatically fill out the notification according to the
 * information in @authorization_request.
 *
 * Returns: (transfer full): The new notification.
 *
 * Since: 3.0.0
 */
PurpleNotification *purple_notification_new_from_authorization_request(PurpleAuthorizationRequest *authorization_request);

/**
 * purple_notification_new_from_connection_error:
 * @account: The [class@Purple.Account] that had the connection error.
 * @info: The [struct@Purple.ConnectionErrorInfo] for the error.
 *
 * Creates a new [class@Purple.Notification] for @account with the @info for
 * the connection. This helper will automatically fill out the notification
 * according to the given parameters.
 *
 * Returns: (transfer full): The new notification.
 *
 * Since: 3.0.0
 */
PurpleNotification *purple_notification_new_from_connection_error(PurpleAccount *account, PurpleConnectionErrorInfo *info);

/**
 * purple_notification_get_id:
 * @notification: The instance.
 *
 * Gets the identifier of @notification.
 *
 * Returns: The identifier of @notification.
 *
 * Since: 3.0.0
 */
const gchar *purple_notification_get_id(PurpleNotification *notification);

/**
 * purple_notification_get_notification_type:
 * @notification: The instance.
 *
 * Gets the [enum@NotificationType] of @notification.
 *
 * Returns: The type of @notification.
 *
 * Since: 3.0.0
 */
PurpleNotificationType purple_notification_get_notification_type(PurpleNotification *notification);

/**
 * purple_notification_get_account:
 * @notification: The instance.
 *
 * Gets the [class@Account] of @notification.
 *
 * Returns: (transfer none): The account of @notification.
 *
 * Since: 3.0.0
 */
PurpleAccount *purple_notification_get_account(PurpleNotification *notification);

/**
 * purple_notification_get_created_timestamp:
 * @notification: The instance.
 *
 * Gets the created time of @notification.
 *
 * Returns: (transfer none): The creation time of @notification.
 *
 * Since: 3.0.0
 */
GDateTime *purple_notification_get_created_timestamp(PurpleNotification *notification);

/**
 * purple_notification_set_created_timestamp:
 * @notification: The instance.
 * @timestamp: (transfer none): The new timestamp.
 *
 * Sets the created timestamp of @notification to @timestamp.
 *
 * Timestamp is internally converted to UTC so you don't need to do that ahead
 * of time.
 *
 * If @timestamp is %NULL, the current time will be used.
 *
 * Since: 3.0.0
 */
void purple_notification_set_created_timestamp(PurpleNotification *notification, GDateTime *timestamp);

/**
 * purple_notification_get_title:
 * @notification: The instance.
 *
 * Gets the title of @notification.
 *
 * Returns: The title of @notification.
 *
 * Since: 3.0.0
 */
const gchar *purple_notification_get_title(PurpleNotification *notification);

/**
 * purple_notification_set_title:
 * @notification: The instance.
 * @title: (nullable): The new title.
 *
 * Sets the title of @notification to @title.
 *
 * Since: 3.0.0
 */
void purple_notification_set_title(PurpleNotification *notification, const gchar *title);

/**
 * purple_notification_get_icon_name:
 * @notification: The instance.
 *
 * Gets the named icon for @notification.
 *
 * Returns: The named icon for @notification.
 *
 * Since: 3.0.0
 */
const gchar *purple_notification_get_icon_name(PurpleNotification *notification);

/**
 * purple_notification_set_icon_name:
 * @notification: The instance.
 * @icon_name: (nullable): The icon name.
 *
 * Sets the named icon for @notification to @icon_name.
 *
 * Since: 3.0.0
 */
void purple_notification_set_icon_name(PurpleNotification *notification, const gchar *icon_name);

/**
 * purple_notification_get_read:
 * @notification: The instance.
 *
 * Gets whether or not @notification has been read.
 *
 * Returns: %TRUE if @notification has been read, %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_notification_get_read(PurpleNotification *notification);

/**
 * purple_notification_set_read:
 * @notification: The instance.
 * @read: Whether or not the notification has been read.
 *
 * Sets @notification's read state to @read.
 *
 * Since: 3.0.0
 */
void purple_notification_set_read(PurpleNotification *notification, gboolean read);

/**
 * purple_notification_get_interactive:
 * @notification: The instance.
 *
 * Gets whether or not @notification can be interacted with.
 *
 * Returns: %TRUE if @notification can be interacted with, %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_notification_get_interactive(PurpleNotification *notification);

/**
 * purple_notification_set_interactive:
 * @notification: The instance.
 * @interactive: Whether or not the notification can be interacted with.
 *
 * Sets @notification's interactive state to @interactive.
 *
 * Since: 3.0.0
 */
void purple_notification_set_interactive(PurpleNotification *notification, gboolean interactive);

/**
 * purple_notification_get_data:
 * @notification: The instance.
 *
 * Gets the data that @notification was created with.
 *
 * Returns: (transfer none): The data for @notification.
 *
 * Since: 3.0.0
 */
gpointer purple_notification_get_data(PurpleNotification *notification);

/**
 * purple_notification_compare:
 * @a: The first notification to compare.
 * @b: The second notification to compare.
 *
 * A comparison function for PurpleNotification that is suitable as a
 * GCompareFunc.
 *
 * Returns: -1 if @a's created timestamp occurred before @b, 0 if they were
 *          created at the same time, or 1 if @b was created before @a.
 *
 * Since: 3.0.0
 */
gint purple_notification_compare(gconstpointer a, gconstpointer b);

/**
 * purple_notification_delete:
 * @notification: The instance.
 *
 * Emits the [signal@PurpleNotification::deleted] signal. This is typically
 * called by a user interface when the user has deleted a notification.
 *
 * If this is called more than once for @notification, the signal will not be
 * emitted.
 *
 * Since: 3.0.0
 */
void purple_notification_delete(PurpleNotification *notification);

G_END_DECLS

#endif /* PURPLE_NOTIFICATION_H */
