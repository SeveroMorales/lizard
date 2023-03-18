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

#include "pidgin/pidginnotificationaddcontact.h"

#include "pidgin/gtkdialogs.h"

struct _PidginNotificationAddContact {
	AdwActionRow parent;

	PurpleNotification *notification;

	GtkWidget *add;
	GtkWidget *message;
};

enum {
	PROP_0,
	PROP_NOTIFICATION,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE(PidginNotificationAddContact, pidgin_notification_add_contact,
              ADW_TYPE_ACTION_ROW)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_notification_add_contact_update(PidginNotificationAddContact *add_contact)
{
	PurpleAccount *account = NULL;
	PurpleAddContactRequest *request = NULL;
	const gchar *title = NULL;
	const gchar *icon_name = NULL, *message = NULL;

	g_return_if_fail(PIDGIN_IS_NOTIFICATION_ADD_CONTACT(add_contact));

	if(!PURPLE_IS_NOTIFICATION(add_contact->notification)) {
		adw_preferences_row_set_title(ADW_PREFERENCES_ROW(add_contact),
		                              _("Notification missing"));

		adw_action_row_set_icon_name(ADW_ACTION_ROW(add_contact), NULL);
		adw_action_row_set_subtitle(ADW_ACTION_ROW(add_contact), NULL);

		gtk_widget_hide(add_contact->add);
		gtk_widget_hide(add_contact->message);

		return;
	}

	account = purple_notification_get_account(add_contact->notification);
	if(!PURPLE_IS_ACCOUNT(account)) {
		adw_preferences_row_set_title(ADW_PREFERENCES_ROW(add_contact),
		                              _("Notification is missing an account"));

		adw_action_row_set_icon_name(ADW_ACTION_ROW(add_contact), NULL);
		adw_action_row_set_subtitle(ADW_ACTION_ROW(add_contact), NULL);

		gtk_widget_hide(add_contact->add);
		gtk_widget_hide(add_contact->message);

		return;
	}

	request = purple_notification_get_data(add_contact->notification);

	/* Set the icon name if one was specified. */
	icon_name = purple_notification_get_icon_name(add_contact->notification);
	if(icon_name == NULL) {
		PurpleProtocol *protocol = NULL;

		protocol = purple_account_get_protocol(account);
		icon_name = purple_protocol_get_icon_name(protocol);

		if(icon_name == NULL) {
			icon_name = "dialog-question";
		}
	}
	adw_action_row_set_icon_name(ADW_ACTION_ROW(add_contact), icon_name);

	title = purple_notification_get_title(add_contact->notification);
	adw_preferences_row_set_title(ADW_PREFERENCES_ROW(add_contact), title);

	message = purple_add_contact_request_get_message(request);
	adw_action_row_set_subtitle(ADW_ACTION_ROW(add_contact), message);

	gtk_widget_show(add_contact->add);
	gtk_widget_show(add_contact->message);
}

static void
pidgin_notification_add_contact_set_notification(PidginNotificationAddContact *add_contact,
                                                 PurpleNotification *notification)
{
	if(g_set_object(&add_contact->notification, notification)) {
		pidgin_notification_add_contact_update(add_contact);

		g_object_notify_by_pspec(G_OBJECT(add_contact),
		                         properties[PROP_NOTIFICATION]);
	}
}

static void
pidgin_notification_add_contact_close(PidginNotificationAddContact *add_contact)
{
	PurpleNotificationManager *manager = NULL;

	purple_notification_delete(add_contact->notification);

	manager = purple_notification_manager_get_default();
	purple_notification_manager_remove(manager, add_contact->notification);
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_notification_add_contact_add_cb(G_GNUC_UNUSED GtkButton *button,
                                       gpointer data)
{
	PidginNotificationAddContact *pidgin_request = data;
	PurpleAddContactRequest *request = NULL;

	request = purple_notification_get_data(pidgin_request->notification);

	purple_add_contact_request_add(request);

	pidgin_notification_add_contact_close(pidgin_request);
}

static void
pidgin_notification_add_contact_message_cb(G_GNUC_UNUSED GtkButton *button,
                                           gpointer data)
{
	PidginNotificationAddContact *pidgin_request = data;
	PurpleAddContactRequest *request = NULL;
	PurpleAccount *account = NULL;
	const gchar *username = NULL;

	request = purple_notification_get_data(pidgin_request->notification);

	account = purple_add_contact_request_get_account(request);
	username = purple_add_contact_request_get_username(request);

	pidgin_dialogs_im_with_user(account, username);
}

static void
pidgin_notification_add_contact_remove_cb(G_GNUC_UNUSED GtkButton *button,
                                          gpointer data)
{
	PidginNotificationAddContact *request = data;

	pidgin_notification_add_contact_close(request);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_notification_add_contact_get_property(GObject *obj, guint param_id,
                                             GValue *value, GParamSpec *pspec)
{
	PidginNotificationAddContact *request = NULL;

	request = PIDGIN_NOTIFICATION_ADD_CONTACT(obj);

	switch(param_id) {
		case PROP_NOTIFICATION:
			g_value_set_object(value,
			                   pidgin_notification_add_contact_get_notification(request));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_notification_add_contact_set_property(GObject *obj, guint param_id,
                                             const GValue *value,
                                             GParamSpec *pspec)
{
	PidginNotificationAddContact *request = NULL;

	request = PIDGIN_NOTIFICATION_ADD_CONTACT(obj);

	switch(param_id) {
		case PROP_NOTIFICATION:
			pidgin_notification_add_contact_set_notification(request,
			                                                 g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_notification_add_contact_dispose(GObject *obj) {
	PidginNotificationAddContact *request = NULL;

	request = PIDGIN_NOTIFICATION_ADD_CONTACT(obj);

	g_clear_object(&request->notification);

	G_OBJECT_CLASS(pidgin_notification_add_contact_parent_class)->dispose(obj);
}

static void
pidgin_notification_add_contact_init(PidginNotificationAddContact *list) {
	gtk_widget_init_template(GTK_WIDGET(list));
}

static void
pidgin_notification_add_contact_class_init(PidginNotificationAddContactClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->get_property = pidgin_notification_add_contact_get_property;
	obj_class->set_property = pidgin_notification_add_contact_set_property;
	obj_class->dispose = pidgin_notification_add_contact_dispose;

	/**
	 * PidginNotificationAddContact:notification:
	 *
	 * The [type@Purple.Notification] that is being displayed.
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
	    "/im/pidgin/Pidgin3/Notifications/addcontact.ui"
	);

	gtk_widget_class_bind_template_child(widget_class,
	                                     PidginNotificationAddContact,
	                                     add);
	gtk_widget_class_bind_template_child(widget_class,
	                                     PidginNotificationAddContact,
	                                     message);

	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_notification_add_contact_add_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_notification_add_contact_message_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_notification_add_contact_remove_cb);
}

/******************************************************************************
 * API
 *****************************************************************************/
GtkWidget *
pidgin_notification_add_contact_new(PurpleNotification *notification) {
	return g_object_new(
		PIDGIN_TYPE_NOTIFICATION_ADD_CONTACT,
		"notification", notification,
		NULL);
}

PurpleNotification *
pidgin_notification_add_contact_get_notification(PidginNotificationAddContact *add_contact)
{
	g_return_val_if_fail(PIDGIN_IS_NOTIFICATION_ADD_CONTACT(add_contact), NULL);

	return add_contact->notification;
}
