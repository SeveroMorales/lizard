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

#include "pidgin/pidginnotificationconnectionerror.h"

struct _PidginNotificationConnectionError {
	AdwActionRow parent;

	PurpleNotification *notification;

	GtkWidget *reconnect;
	GtkWidget *reenable;
	GtkWidget *modify;
};

enum {
	PROP_0,
	PROP_NOTIFICATION,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE(PidginNotificationConnectionError,
              pidgin_notification_connection_error, ADW_TYPE_ACTION_ROW)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_notification_connection_error_update(PidginNotificationConnectionError *error) {
	PurpleAccount *account = NULL;
	PurpleConnectionErrorInfo *info = NULL;
	const gchar *icon_name = NULL, *account_id = NULL, *title = NULL;
	gboolean enabled = FALSE;

	g_return_if_fail(PIDGIN_IS_NOTIFICATION_CONNECTION_ERROR(error));

	if(!PURPLE_IS_NOTIFICATION(error->notification)) {
		adw_preferences_row_set_title(ADW_PREFERENCES_ROW(error),
		                              _("Notification missing"));

		adw_action_row_set_icon_name(ADW_ACTION_ROW(error), NULL);
		adw_action_row_set_subtitle(ADW_ACTION_ROW(error), NULL);

		gtk_widget_hide(error->reconnect);
		gtk_widget_hide(error->reenable);
		gtk_widget_hide(error->modify);

		return;
	}

	account = purple_notification_get_account(error->notification);
	if(!PURPLE_IS_ACCOUNT(account)) {
		adw_preferences_row_set_title(ADW_PREFERENCES_ROW(error),
		                              _("Notification is missing an account"));

		adw_action_row_set_icon_name(ADW_ACTION_ROW(error), NULL);
		adw_action_row_set_subtitle(ADW_ACTION_ROW(error), NULL);

		gtk_widget_hide(error->reconnect);
		gtk_widget_hide(error->reenable);
		gtk_widget_hide(error->modify);

		return;
	}

	/* Set the target for our actions. */
	account_id = purple_contact_info_get_id(PURPLE_CONTACT_INFO(account));
	gtk_actionable_set_action_target(GTK_ACTIONABLE(error->reconnect), "s",
	                                 account_id);
	gtk_actionable_set_action_target(GTK_ACTIONABLE(error->reenable), "s",
	                                 account_id);
	gtk_actionable_set_action_target(GTK_ACTIONABLE(error->modify), "s",
	                                 account_id);

	/* Set the icon name if one was specified. */
	icon_name = purple_notification_get_icon_name(error->notification);
	if(icon_name != NULL) {
		adw_action_row_set_icon_name(ADW_ACTION_ROW(error), icon_name);
	}

	enabled = purple_account_get_enabled(account);

	title = purple_notification_get_title(error->notification);
	adw_preferences_row_set_title(ADW_PREFERENCES_ROW(error), title);

	info = purple_notification_get_data(error->notification);
	if(info != NULL) {
		adw_action_row_set_subtitle(ADW_ACTION_ROW(error), info->description);
	} else {
		adw_action_row_set_subtitle(ADW_ACTION_ROW(error), NULL);
	}

	/* If the account is still enabled, reconnect should be visible, otherwise
	 * re-enable should be visible.
	 */
	gtk_widget_set_visible(error->reconnect, enabled);
	gtk_widget_set_visible(error->reenable, !enabled);
}

static void
pidgin_notification_connection_error_set_notification(PidginNotificationConnectionError *error,
                                                      PurpleNotification *notification)
{
	if(g_set_object(&error->notification, notification)) {
		pidgin_notification_connection_error_update(error);

		g_object_notify_by_pspec(G_OBJECT(error), properties[PROP_NOTIFICATION]);
	}
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_notification_connection_error_remove_cb(G_GNUC_UNUSED GtkButton *button,
                                               gpointer data)
{
	PidginNotificationConnectionError *error = data;
	PurpleNotificationManager *manager = NULL;

	purple_notification_delete(error->notification);

	manager = purple_notification_manager_get_default();
	purple_notification_manager_remove(manager, error->notification);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_notification_connection_error_get_property(GObject *obj, guint param_id,
                                                  GValue *value,
                                                  GParamSpec *pspec)
{
	PidginNotificationConnectionError *error = NULL;

	error = PIDGIN_NOTIFICATION_CONNECTION_ERROR(obj);

	switch(param_id) {
		case PROP_NOTIFICATION:
			g_value_set_object(value,
			                   pidgin_notification_connection_error_get_notification(error));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_notification_connection_error_set_property(GObject *obj, guint param_id,
                                                  const GValue *value,
                                                  GParamSpec *pspec)
{
	PidginNotificationConnectionError *error = NULL;

	error = PIDGIN_NOTIFICATION_CONNECTION_ERROR(obj);

	switch(param_id) {
		case PROP_NOTIFICATION:
			pidgin_notification_connection_error_set_notification(error,
			                                                      g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_notification_connection_error_dispose(GObject *obj) {
	PidginNotificationConnectionError *error = NULL;

	error = PIDGIN_NOTIFICATION_CONNECTION_ERROR(obj);

	g_clear_object(&error->notification);

	G_OBJECT_CLASS(pidgin_notification_connection_error_parent_class)->dispose(obj);
}

static void
pidgin_notification_connection_error_init(PidginNotificationConnectionError *list)
{
	gtk_widget_init_template(GTK_WIDGET(list));
}

static void
pidgin_notification_connection_error_class_init(PidginNotificationConnectionErrorClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->get_property = pidgin_notification_connection_error_get_property;
	obj_class->set_property = pidgin_notification_connection_error_set_property;
	obj_class->dispose = pidgin_notification_connection_error_dispose;

	/**
	 * PidginNotificationConnectionError:notification:
	 *
	 * The [type@Purple.ConnectionErrorInfo] that this notification is for.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_NOTIFICATION] = g_param_spec_object(
		"notification", "notification",
		"The notification to display",
		PURPLE_TYPE_NOTIFICATION,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(
	    widget_class,
	    "/im/pidgin/Pidgin3/Notifications/connectionerror.ui"
	);

	gtk_widget_class_bind_template_child(widget_class,
	                                     PidginNotificationConnectionError,
	                                     reconnect);
	gtk_widget_class_bind_template_child(widget_class,
	                                     PidginNotificationConnectionError,
	                                     reenable);
	gtk_widget_class_bind_template_child(widget_class,
	                                     PidginNotificationConnectionError,
	                                     modify);

	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_notification_connection_error_remove_cb);
}

/******************************************************************************
 * API
 *****************************************************************************/
GtkWidget *
pidgin_notification_connection_error_new(PurpleNotification *notification) {
	return g_object_new(
		PIDGIN_TYPE_NOTIFICATION_CONNECTION_ERROR,
		"notification", notification,
		NULL);
}

PurpleNotification *
pidgin_notification_connection_error_get_notification(PidginNotificationConnectionError *error)
{
	g_return_val_if_fail(PIDGIN_IS_NOTIFICATION_CONNECTION_ERROR(error), NULL);

	return error->notification;
}
