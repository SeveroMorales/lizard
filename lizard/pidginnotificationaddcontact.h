/*
 * Pidgin - Internet Messenger
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
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

#if !defined(PIDGIN_GLOBAL_HEADER_INSIDE) && !defined(PIDGIN_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PIDGIN_NOTIFICATION_ADD_CONTACT_H
#define PIDGIN_NOTIFICATION_ADD_CONTACT_H

#include <glib.h>

#include <gtk/gtk.h>

#include <adwaita.h>

#include <purple.h>

G_BEGIN_DECLS

/**
 * PidginNotificationAddContact:
 *
 * #PidginNotificationAddContact is a widget that displays
 * notifications from [class@Purple.NotificationManager] for
 * [class@Purple.AddContactRequest].
 *
 * Since: 3.0.0
 */

#define PIDGIN_TYPE_NOTIFICATION_ADD_CONTACT (pidgin_notification_add_contact_get_type())
G_DECLARE_FINAL_TYPE(PidginNotificationAddContact, pidgin_notification_add_contact,
                     PIDGIN, NOTIFICATION_ADD_CONTACT, AdwActionRow)

/**
 * pidgin_notification_add_contact_new:
 * @notification: A [class@Purple.Notification] to display.
 *
 * Creates a new #PidginNotificationAddContact instance that will display
 * @notification.
 *
 * Returns: (transfer full): The new #PidginNotificationAddContact
 *          instance.
 */
GtkWidget *pidgin_notification_add_contact_new(PurpleNotification *notification);

/**
 * pidgin_notification_add_contact_get_notification:
 * @request: The instance.
 *
 * Gets the [class@Purple.Notification] that @request is displaying.
 *
 * Returns: (transfer none): The notification.
 *
 * Since: 3.0.0
 */
PurpleNotification *pidgin_notification_add_contact_get_notification(PidginNotificationAddContact *request);

G_END_DECLS

#endif /* PIDGIN_NOTIFICATION_ADD_CONTACT_H */
