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

#include "pidgin/pidginnotificationauthorizationrequest.h"

#include "pidgin/gtkdialogs.h"

struct _PidginNotificationAuthorizationRequest {
	AdwActionRow parent;

	PurpleNotification *notification;

	GtkWidget *accept;
	GtkWidget *deny;
	GtkWidget *message;
};

enum {
	PROP_0,
	PROP_NOTIFICATION,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE(PidginNotificationAuthorizationRequest,
              pidgin_notification_authorization_request, ADW_TYPE_ACTION_ROW)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_notification_authorization_request_update(PidginNotificationAuthorizationRequest *request) {
	PurpleAccount *account = NULL;
	PurpleAuthorizationRequest *purple_request = NULL;
	const gchar *title = NULL;
	const gchar *icon_name = NULL, *message = NULL;

	g_return_if_fail(PIDGIN_IS_NOTIFICATION_AUTHORIZATION_REQUEST(request));

	if(!PURPLE_IS_NOTIFICATION(request->notification)) {
		adw_preferences_row_set_title(ADW_PREFERENCES_ROW(request),
		                              _("Notification missing"));

		adw_action_row_set_icon_name(ADW_ACTION_ROW(request), NULL);
		adw_action_row_set_subtitle(ADW_ACTION_ROW(request), NULL);

		gtk_widget_hide(request->accept);
		gtk_widget_hide(request->deny);
		gtk_widget_hide(request->message);

		return;
	}

	account = purple_notification_get_account(request->notification);
	if(!PURPLE_IS_ACCOUNT(account)) {
		adw_preferences_row_set_title(ADW_PREFERENCES_ROW(request),
		                              _("Notification is missing an account"));

		adw_action_row_set_icon_name(ADW_ACTION_ROW(request), NULL);
		adw_action_row_set_subtitle(ADW_ACTION_ROW(request), NULL);

		gtk_widget_hide(request->accept);
		gtk_widget_hide(request->deny);
		gtk_widget_hide(request->message);

		return;
	}

	purple_request = purple_notification_get_data(request->notification);

	/* Set the icon name if one was specified. */
	icon_name = purple_notification_get_icon_name(request->notification);
	if(icon_name == NULL) {
		PurpleProtocol *protocol = NULL;

		protocol = purple_account_get_protocol(account);
		icon_name = purple_protocol_get_icon_name(protocol);

		if(icon_name == NULL) {
			icon_name = "dialog-question";
		}
	}
	adw_action_row_set_icon_name(ADW_ACTION_ROW(request), icon_name);

	title = purple_notification_get_title(request->notification);
	adw_preferences_row_set_title(ADW_PREFERENCES_ROW(request), title);

	message = purple_authorization_request_get_message(purple_request);
	adw_action_row_set_subtitle(ADW_ACTION_ROW(request), message);

	gtk_widget_show(request->accept);
	gtk_widget_show(request->deny);
	gtk_widget_show(request->message);
}

static void
pidgin_notification_authorization_request_set_notification(PidginNotificationAuthorizationRequest *request,
                                                           PurpleNotification *notification)
{
	if(g_set_object(&request->notification, notification)) {
		pidgin_notification_authorization_request_update(request);

		g_object_notify_by_pspec(G_OBJECT(request), properties[PROP_NOTIFICATION]);
	}
}

static void
pidgin_notification_authorization_request_close(PidginNotificationAuthorizationRequest *request)
{
	PurpleNotificationManager *manager = NULL;

	purple_notification_delete(request->notification);

	manager = purple_notification_manager_get_default();
	purple_notification_manager_remove(manager, request->notification);
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_notification_authorization_request_accept_cb(G_GNUC_UNUSED GtkButton *button,
                                                    gpointer data)
{
	PidginNotificationAuthorizationRequest *pidgin_request = data;
	PurpleAuthorizationRequest *request = NULL;

	request = purple_notification_get_data(pidgin_request->notification);

	purple_authorization_request_accept(request);

	if(purple_authorization_request_get_add(request)) {
		PurpleAccount *account = NULL;
		const gchar *username = NULL, *alias = NULL;

		account = purple_authorization_request_get_account(request);
		username = purple_authorization_request_get_username(request);
		alias = purple_authorization_request_get_alias(request);

		purple_blist_request_add_buddy(account, username, NULL, alias);
	}

	pidgin_notification_authorization_request_close(pidgin_request);
}

static void
pidgin_notification_authorization_request_deny_cb(G_GNUC_UNUSED GtkButton *button,
                                                  gpointer data)
{
	PidginNotificationAuthorizationRequest *pidgin_request = data;
	PurpleAuthorizationRequest *request = NULL;

	request = purple_notification_get_data(pidgin_request->notification);

	purple_authorization_request_deny(request, NULL);

	pidgin_notification_authorization_request_close(pidgin_request);
}

static void
pidgin_notification_authorization_request_message_cb(G_GNUC_UNUSED GtkButton *button,
                                                     gpointer data)
{
	PidginNotificationAuthorizationRequest *pidgin_request = data;
	PurpleAuthorizationRequest *request = NULL;
	PurpleAccount *account = NULL;
	const gchar *username = NULL;

	request = purple_notification_get_data(pidgin_request->notification);

	account = purple_authorization_request_get_account(request);
	username = purple_authorization_request_get_username(request);

	pidgin_dialogs_im_with_user(account, username);
}

static void
pidgin_notification_authorization_request_remove_cb(G_GNUC_UNUSED GtkButton *button,
                                                    gpointer data)
{
	PidginNotificationAuthorizationRequest *request = data;

	pidgin_notification_authorization_request_close(request);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_notification_authorization_request_get_property(GObject *obj,
                                                       guint param_id,
                                                       GValue *value,
                                                       GParamSpec *pspec)
{
	PidginNotificationAuthorizationRequest *request = NULL;

	request = PIDGIN_NOTIFICATION_AUTHORIZATION_REQUEST(obj);

	switch(param_id) {
		case PROP_NOTIFICATION:
			g_value_set_object(value,
			                   pidgin_notification_authorization_request_get_notification(request));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_notification_authorization_request_set_property(GObject *obj,
                                                       guint param_id,
                                                       const GValue *value,
                                                       GParamSpec *pspec)
{
	PidginNotificationAuthorizationRequest *request = NULL;

	request = PIDGIN_NOTIFICATION_AUTHORIZATION_REQUEST(obj);

	switch(param_id) {
		case PROP_NOTIFICATION:
			pidgin_notification_authorization_request_set_notification(request,
			                                                           g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_notification_authorization_request_dispose(GObject *obj) {
	PidginNotificationAuthorizationRequest *request = NULL;

	request = PIDGIN_NOTIFICATION_AUTHORIZATION_REQUEST(obj);

	g_clear_object(&request->notification);

	G_OBJECT_CLASS(pidgin_notification_authorization_request_parent_class)->dispose(obj);
}

static void
pidgin_notification_authorization_request_init(PidginNotificationAuthorizationRequest *list)
{
	gtk_widget_init_template(GTK_WIDGET(list));
}

static void
pidgin_notification_authorization_request_class_init(PidginNotificationAuthorizationRequestClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->get_property = pidgin_notification_authorization_request_get_property;
	obj_class->set_property = pidgin_notification_authorization_request_set_property;
	obj_class->dispose = pidgin_notification_authorization_request_dispose;

	/**
	 * PidginNotificationAuthorizationRequest:info:
	 *
	 * The [type@Purple.AuthorizationRequestInfo] that this notification is for.
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
	    "/im/pidgin/Pidgin3/Notifications/authorizationrequest.ui"
	);

	gtk_widget_class_bind_template_child(widget_class,
	                                     PidginNotificationAuthorizationRequest,
	                                     accept);
	gtk_widget_class_bind_template_child(widget_class,
	                                     PidginNotificationAuthorizationRequest,
	                                     deny);
	gtk_widget_class_bind_template_child(widget_class,
	                                     PidginNotificationAuthorizationRequest,
	                                     message);

	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_notification_authorization_request_accept_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_notification_authorization_request_deny_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_notification_authorization_request_message_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_notification_authorization_request_remove_cb);
}

/******************************************************************************
 * API
 *****************************************************************************/
GtkWidget *
pidgin_notification_authorization_request_new(PurpleNotification *notification) {
	return g_object_new(
		PIDGIN_TYPE_NOTIFICATION_AUTHORIZATION_REQUEST,
		"notification", notification,
		NULL);
}

PurpleNotification *
pidgin_notification_authorization_request_get_notification(PidginNotificationAuthorizationRequest *request)
{
	g_return_val_if_fail(PIDGIN_IS_NOTIFICATION_AUTHORIZATION_REQUEST(request),
	                     NULL);

	return request->notification;
}
