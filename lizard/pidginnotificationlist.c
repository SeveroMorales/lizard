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

#include <glib/gi18n-lib.h>

#include <purple.h>

#include "pidgin/pidginnotificationlist.h"

#include "pidgin/pidginnotificationaddcontact.h"
#include "pidgin/pidginnotificationauthorizationrequest.h"
#include "pidgin/pidginnotificationconnectionerror.h"

struct _PidginNotificationList {
	GtkBox parent;

	GtkWidget *list_box;
};

G_DEFINE_TYPE(PidginNotificationList, pidgin_notification_list, GTK_TYPE_BOX)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static GtkWidget *
pidgin_notification_list_unknown_notification(PurpleNotification *notification) {
	GtkWidget *widget = NULL;
	gchar *label = NULL;
	const gchar *title = NULL;

	title = purple_notification_get_title(notification);
	if(title != NULL) {
		label = g_strdup_printf(_("Unknown notification type %d: %s"),
		                        purple_notification_get_notification_type(notification),
		                        title);
	} else {
		label = g_strdup_printf(_("Unknown notification type %d"),
		                        purple_notification_get_notification_type(notification));
	}

	widget = gtk_label_new(label);

	g_free(label);

	return widget;
}

static GtkWidget *
pidgin_notification_list_create_widget_func(gpointer item,
                                            G_GNUC_UNUSED gpointer data)
{
	PurpleNotification *notification = item;
	GtkWidget *widget = NULL;

	switch(purple_notification_get_notification_type(notification)) {
		case PURPLE_NOTIFICATION_TYPE_CONNECTION_ERROR:
			widget = pidgin_notification_connection_error_new(notification);
			break;
		case PURPLE_NOTIFICATION_TYPE_AUTHORIZATION_REQUEST:
			widget = pidgin_notification_authorization_request_new(notification);
			break;
		case PURPLE_NOTIFICATION_TYPE_ADD_CONTACT:
			widget = pidgin_notification_add_contact_new(notification);
			break;
		default:
			widget = pidgin_notification_list_unknown_notification(notification);
			break;
	}

	if(!GTK_IS_WIDGET(widget)) {
		widget = pidgin_notification_list_unknown_notification(notification);
	}

	return widget;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_notification_list_init(PidginNotificationList *list) {
	gtk_widget_init_template(GTK_WIDGET(list));

	gtk_list_box_bind_model(GTK_LIST_BOX(list->list_box),
	                        purple_notification_manager_get_default_as_model(),
	                        pidgin_notification_list_create_widget_func,
	                        list, NULL);
}

static void
pidgin_notification_list_class_init(PidginNotificationListClass *klass) {
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
	    widget_class,
	    "/im/pidgin/Pidgin3/Notifications/list.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginNotificationList,
	                                     list_box);
}

/******************************************************************************
 * API
 *****************************************************************************/
GtkWidget *
pidgin_notification_list_new(void) {
	return g_object_new(PIDGIN_TYPE_NOTIFICATION_LIST, NULL);
}
