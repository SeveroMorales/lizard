/*
 * Finch - Universal Text Chat Client
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Finch is the legal property of its developers, whose names are too numerous
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

#include <config.h>

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <glib/gstdio.h>

#include <purple.h>

#include <gnt.h>

#include "finchnotifications.h"

#include "gntaccount.h"

static struct {
	GntWidget *window;
	GntWidget *list;
} notifications;

/*******************************************************************************
 * Helpers
 ******************************************************************************/
static void
finch_notifications_update(GntTree *list, GListModel *model) {
	guint index = 0;

	gnt_tree_remove_all(GNT_TREE(list));

	for(index = 0; index < g_list_model_get_n_items(model); index++) {
		PurpleNotification *notification = g_list_model_get_item(model, index);
		GntTreeRow *row = NULL;

		row = gnt_tree_create_row(list,
		                          purple_notification_get_title(notification));
		gnt_tree_add_row_last(list, notification, row, NULL);
	}
}

static void
finch_notification_delete_notification(PurpleNotification *notification) {
	if(PURPLE_IS_NOTIFICATION(notification)) {
		PurpleNotificationManager *manager = NULL;

		purple_notification_delete(notification);

		manager = purple_notification_manager_get_default();
		purple_notification_manager_remove(manager, notification);
	}
}

/*******************************************************************************
 * Finch Notification Callbacks
 ******************************************************************************/
static void
finch_notification_delete(G_GNUC_UNUSED GntWidget *widget, gpointer data) {
	PurpleNotification *notification = g_object_get_data(data, "notification");

	finch_notification_delete_notification(notification);

	gnt_widget_destroy(GNT_WIDGET(data));
}

static void
finch_notification_reconnect_account(G_GNUC_UNUSED GntWidget *widget,
                                     gpointer data)
{
	PurpleNotification *notification = g_object_get_data(data, "notification");
	PurpleAccount *account = purple_notification_get_account(notification);

	if(PURPLE_IS_ACCOUNT(account)) {
		purple_account_connect(account);
	}

	gnt_widget_destroy(GNT_WIDGET(data));
}

static void
finch_notification_reenable_account(G_GNUC_UNUSED GntWidget *widget,
                                    gpointer data)
{
	PurpleNotification *notification = g_object_get_data(data, "notification");
	PurpleAccount *account = purple_notification_get_account(notification);

	if(PURPLE_IS_ACCOUNT(account)) {
		purple_account_set_enabled(account, TRUE);
	}

	gnt_widget_destroy(GNT_WIDGET(data));
}

static void
finch_notification_modify_account(G_GNUC_UNUSED GntWidget *widget,
                                  gpointer data)
{
	PurpleNotification *notification = g_object_get_data(data, "notification");
	PurpleAccount *account = purple_notification_get_account(notification);

	if(PURPLE_IS_ACCOUNT(account)) {
		finch_account_dialog_show(account);
	}

	gnt_widget_destroy(GNT_WIDGET(data));
}

static void
finch_notification_contact_authorize(G_GNUC_UNUSED GntWidget *widget,
                                     gpointer data)
{
	PurpleAccount *account = NULL;
	PurpleNotification *notification = NULL;
	PurpleNotificationManager *manager = NULL;
	PurpleAuthorizationRequest *auth_request = NULL;
	const gchar *alias = NULL, *username = NULL;

	/* Get the notification and authorization request from the data. */
	notification = g_object_get_data(data, "notification");
	auth_request = purple_notification_get_data(notification);

	/* Accept the authorization request. */
	purple_authorization_request_accept(auth_request);

	/* Remove the notification from the manager. */
	manager = purple_notification_manager_get_default();
	purple_notification_manager_remove(manager, notification);

	/* Request the user to add the person they just authorized. */
	account = purple_authorization_request_get_account(auth_request);
	alias = purple_authorization_request_get_alias(auth_request);
	username = purple_authorization_request_get_username(auth_request);
	purple_blist_request_add_buddy(account, username, NULL, alias);

	/* Destroy the dialog. */
	gnt_widget_destroy(GNT_WIDGET(data));
}

static void
finch_notification_contact_deny(G_GNUC_UNUSED GntWidget *widget, gpointer data)
{
	PurpleNotification *notification = NULL;
	PurpleNotificationManager *manager = NULL;
	PurpleAuthorizationRequest *auth_request = NULL;

	/* Get the notification and authorization request from the data. */
	notification = g_object_get_data(data, "notification");
	auth_request = purple_notification_get_data(notification);

	/* Deny the request. */
	purple_authorization_request_deny(auth_request, NULL);

	/* Remove the notification from the manager. */
	manager = purple_notification_manager_get_default();
	purple_notification_manager_remove(manager, notification);

	/* Destroy the dialog. */
	gnt_widget_destroy(GNT_WIDGET(data));
}

/*******************************************************************************
 * Finch Notification API
 ******************************************************************************/
static void
finch_notification_show(PurpleNotification *notification) {
	GntWidget *dialog = NULL, *label = NULL, *hbox = NULL, *button = NULL;
	PurpleAccount *account = NULL;
	PurpleNotificationType type;
	gpointer data = NULL;

	account = purple_notification_get_account(notification);

	dialog = gnt_box_new(FALSE, TRUE);
	gnt_box_set_toplevel(GNT_BOX(dialog), TRUE);
	gnt_box_set_alignment(GNT_BOX(dialog), GNT_ALIGN_MID);
	g_object_set_data(G_OBJECT(dialog), "notification", notification);

	label = gnt_label_new(purple_notification_get_title(notification));
	gnt_box_add_widget(GNT_BOX(dialog), label);

	hbox = gnt_box_new(FALSE, FALSE);

	type = purple_notification_get_notification_type(notification);
	data = purple_notification_get_data(notification);

	if(type == PURPLE_NOTIFICATION_TYPE_GENERIC) {
		gnt_box_set_title(GNT_BOX(dialog),
		                  purple_notification_get_title(notification));
		label = gnt_label_new(purple_notification_get_data(notification));
		gnt_box_add_widget(GNT_BOX(dialog), label);
	} else if(type == PURPLE_NOTIFICATION_TYPE_CONNECTION_ERROR) {
		PurpleConnectionErrorInfo *info = data;

		/* Set the title. */
		gnt_box_set_title(GNT_BOX(dialog), _("Connection Error"));

		/* Add the connection error reason. */
		label = gnt_label_new(info->description);
		gnt_box_add_widget(GNT_BOX(dialog), label);

		/* Add the buttons. */
		if(purple_account_get_enabled(account)) {
			button = gnt_button_new(_("Reconnect"));
			g_signal_connect(button, "activate",
			                 G_CALLBACK(finch_notification_reconnect_account),
			                 dialog);
		} else {
			button = gnt_button_new(_("Re-enable"));
			g_signal_connect(button, "activate",
			                 G_CALLBACK(finch_notification_reenable_account),
			                 dialog);
		}
		gnt_box_add_widget(GNT_BOX(hbox), button);

		button = gnt_button_new(_("Modify Account"));
		g_signal_connect(button, "activate",
		                 G_CALLBACK(finch_notification_modify_account),
		                 dialog);
		gnt_box_add_widget(GNT_BOX(hbox), button);
	} else if(type == PURPLE_NOTIFICATION_TYPE_AUTHORIZATION_REQUEST) {
		PurpleAuthorizationRequest *auth_request = NULL;
		const gchar *message = NULL;

		/* Set the title. */
		gnt_box_set_title(GNT_BOX(dialog), _("Authorization Request"));

		auth_request = purple_notification_get_data(notification);
		message = purple_authorization_request_get_message(auth_request);

		/* Add the message if we have one. */
		if(message != NULL && *message != '\0') {
			label = gnt_label_new(message);
			gnt_box_add_widget(GNT_BOX(dialog), label);
		}

		/* Add the buttons. */
		button = gnt_button_new(_("Authorize"));
		g_signal_connect(button, "activate",
		                 G_CALLBACK(finch_notification_contact_authorize),
		                 dialog);
		gnt_box_add_widget(GNT_BOX(hbox), button);

		button = gnt_button_new(_("Deny"));
		g_signal_connect(button, "activate",
		                 G_CALLBACK(finch_notification_contact_deny), dialog);
		gnt_box_add_widget(GNT_BOX(hbox), button);
	}

	gnt_box_add_widget(GNT_BOX(dialog), hbox);

	button = gnt_button_new(_("Delete"));
	g_signal_connect(button, "activate",
	                 G_CALLBACK(finch_notification_delete), dialog);
	gnt_box_add_widget(GNT_BOX(hbox), button);

	gnt_widget_show(dialog);
}

/*******************************************************************************
 * Callbacks
 ******************************************************************************/
static void
finch_notifications_changed_cb(GListModel *model,
                               G_GNUC_UNUSED guint position,
                               G_GNUC_UNUSED guint removed,
                               guint added,
                               gpointer data)
{
	finch_notifications_update(data, model);

	if(added > 0) {
		gnt_widget_set_urgent(notifications.window);
	}
}

static void
finch_notifications_open_cb(G_GNUC_UNUSED GntWidget *w,
                            G_GNUC_UNUSED gpointer data)
{
	PurpleNotification *notification = NULL;

	notification = gnt_tree_get_selection_data(GNT_TREE(notifications.list));
	if(!PURPLE_IS_NOTIFICATION(notification)) {
		return;
	}

	finch_notification_show(notification);
}

static void
finch_notifications_delete_cb(G_GNUC_UNUSED GntWidget *widget,
                              G_GNUC_UNUSED gpointer data)
{
	PurpleNotification *notification = NULL;

	notification = gnt_tree_get_selection_data(GNT_TREE(notifications.list));

	finch_notification_delete_notification(notification);
}


static void
finch_notifications_activate_cb(G_GNUC_UNUSED GntWidget *w,
                                G_GNUC_UNUSED gpointer data)
{
	PurpleNotification *notification = NULL;

	notification = gnt_tree_get_selection_data(GNT_TREE(notifications.list));

	finch_notification_show(notification);
}

/*******************************************************************************
 * Public API
 ******************************************************************************/
void
finch_notifications_window_show(void) {
	GntWidget *wid, *box;
	GListModel *model = NULL;

	if(notifications.window) {
		gnt_window_present(notifications.window);

		return;
	}

	notifications.window = gnt_vbox_new(FALSE);
	gnt_box_set_toplevel(GNT_BOX(notifications.window), TRUE);
	gnt_box_set_fill(GNT_BOX(notifications.window), TRUE);
	gnt_box_set_title(GNT_BOX(notifications.window), _("Notifications"));
	gnt_box_set_alignment(GNT_BOX(notifications.window), GNT_ALIGN_MID);
	gnt_box_set_pad(GNT_BOX(notifications.window), 0);
	gnt_widget_set_name(notifications.window, "notifications");

	/* Create the box that lists all of the notifications. */
	notifications.list = gnt_tree_new_with_columns(1);
	gnt_tree_set_compare_func(GNT_TREE(notifications.list),
	                          purple_notification_compare);
	gnt_widget_set_has_border(notifications.list, FALSE);
	gnt_box_add_widget(GNT_BOX(notifications.window), notifications.list);
	g_signal_connect(notifications.list, "activate",
	                 G_CALLBACK(finch_notifications_activate_cb), NULL);

	/* Get the notification manager to get the model and populate the list. */
	model = purple_notification_manager_get_default_as_model();
	finch_notifications_update(GNT_TREE(notifications.list), model);
	g_signal_connect_object(model, "items-changed",
	                        G_CALLBACK(finch_notifications_changed_cb),
	                        notifications.list, 0);

	gnt_box_add_widget(GNT_BOX(notifications.window), gnt_line_new(FALSE));

	box = gnt_hbox_new(FALSE);
	gnt_box_set_alignment(GNT_BOX(box), GNT_ALIGN_MID);
	gnt_box_set_fill(GNT_BOX(box), FALSE);

	wid = gnt_button_new(_("Open"));
	g_signal_connect(wid, "activate", G_CALLBACK(finch_notifications_open_cb),
	                 NULL);
	gnt_box_add_widget(GNT_BOX(box), wid);

	wid = gnt_button_new(_("Delete"));
	g_signal_connect(wid, "activate", G_CALLBACK(finch_notifications_delete_cb),
	                 NULL);
	gnt_box_add_widget(GNT_BOX(box), wid);

	gnt_box_add_widget(GNT_BOX(notifications.window), box);

	gnt_widget_show(notifications.window);
}

void
finch_notifications_init(void) {
}

void
finch_notifications_uninit(void) {
	g_clear_pointer(&notifications.window, gnt_widget_destroy);
}
