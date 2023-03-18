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

#include <adwaita.h>

#include "pidgin/pidginaccountrow.h"

struct _PidginAccountRow {
	GtkListBoxRow parent;

	PurpleAccount *account;

	GtkSwitch *enabled;
	AdwAvatar *avatar;
	GtkLabel *name;
	GtkLabel *status;
};

enum {
	PROP_0,
	PROP_ACCOUNT,
	N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES] = { NULL, };

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_account_row_refresh_buddy_icon(PidginAccountRow *row) {
	PurpleImage *image = NULL;

#warning FIX call this in the right place when buddy icons are better and can autorefresh
	if(!PURPLE_IS_ACCOUNT(row->account)) {
		return;
	}

	image = purple_buddy_icons_find_account_icon(row->account);
	if(PURPLE_IS_IMAGE(image)) {
		GdkTexture *texture = NULL;
		GBytes *bytes = NULL;

		bytes = purple_image_get_contents(image);
		texture = gdk_texture_new_from_bytes(bytes, NULL);
		g_bytes_unref(bytes);

		if(GDK_IS_TEXTURE(texture)) {
			adw_avatar_set_custom_image(row->avatar, GDK_PAINTABLE(texture));
			g_object_unref(texture);
		}
	}
}

static void
pidgin_account_row_refresh_status(PidginAccountRow *row) {
	const char *status = NULL;
	gboolean connected = FALSE;
	gboolean error = FALSE;

	if(PURPLE_IS_ACCOUNT(row->account)) {
		if(!purple_account_get_enabled(row->account)) {
			status = _("Disabled");
		} else {
			const PurpleConnectionErrorInfo *error_info = NULL;

			error_info = purple_account_get_error(row->account);
			if(error_info != NULL) {
				status = error_info->description;
				error = TRUE;
			} else {
				PurpleConnection *connection = NULL;

				connection = purple_account_get_connection(row->account);
				if(PURPLE_IS_CONNECTION(connection)) {
					switch(purple_connection_get_state(connection)) {
						case PURPLE_CONNECTION_STATE_DISCONNECTED:
							status = _("Disconnected");
							break;
						case PURPLE_CONNECTION_STATE_DISCONNECTING:
							status = _("Disconnecting...");
							break;
						case PURPLE_CONNECTION_STATE_CONNECTED:
							status = _("Connected");
							connected = TRUE;
							break;
						case PURPLE_CONNECTION_STATE_CONNECTING:
							status = _("Connecting...");
							break;
					}
				}
			}
		}
	}

	gtk_switch_set_state(row->enabled, connected);
	gtk_label_set_text(row->status, status);
	if(error) {
		gtk_widget_add_css_class(GTK_WIDGET(row->status), "error");
	} else {
		gtk_widget_remove_css_class(GTK_WIDGET(row->status), "error");
	}
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_account_row_state_changed_cb(G_GNUC_UNUSED GObject *obj,
                                    G_GNUC_UNUSED GParamSpec *pspec,
                                    gpointer data)
{
	PidginAccountRow *row = data;

	pidgin_account_row_refresh_status(row);
}

static void
pidgin_account_row_connection_changed_cb(G_GNUC_UNUSED GObject *obj,
                                         G_GNUC_UNUSED GParamSpec *pspec,
                                         gpointer data)
{
	PidginAccountRow *row = data;
	PurpleConnection *connection = NULL;

	connection = purple_account_get_connection(row->account);
	if(PURPLE_IS_CONNECTION(connection)) {
		g_signal_connect_object(connection, "notify::state",
		                        G_CALLBACK(pidgin_account_row_state_changed_cb),
		                        row, 0);
	}

	pidgin_account_row_refresh_status(row);
}

static void
pidgin_account_row_enable_state_set_cb(G_GNUC_UNUSED GtkSwitch *sw,
                                       gboolean state, gpointer data)
{
	PidginAccountRow *row = data;
	PurpleAccount *account = row->account;

	if(purple_account_get_enabled(account) == state) {
		return;
	}

	/* The account was just enabled, so set its status. */
	if(state) {
		PurpleSavedStatus *status = purple_savedstatus_get_current();
		purple_savedstatus_activate_for_account(status, account);
	}

	purple_account_set_enabled(account, state);
}

static char *
pidgin_account_row_buddyicon_cb(G_GNUC_UNUSED GObject *self,
                                PurpleAccount *account,
                                G_GNUC_UNUSED gpointer data)
{
	const char *buddy_icon_path = NULL;
	char *path = NULL;

	if(!PURPLE_IS_ACCOUNT(account)) {
		return NULL;
	}

	buddy_icon_path = purple_account_get_buddy_icon_path(account);
	if(buddy_icon_path != NULL) {
		path = g_strdup_printf("file://%s", buddy_icon_path);
	}

	return path;
}

static char *
pidgin_account_row_protocol_name_cb(G_GNUC_UNUSED GObject *self,
                                    PurpleAccount *account,
                                    G_GNUC_UNUSED gpointer data)
{
	const char *name = _("Unknown");

	if(PURPLE_IS_ACCOUNT(account)) {
		PurpleProtocol *protocol = purple_account_get_protocol(account);
		if(PURPLE_IS_PROTOCOL(protocol)) {
			name = purple_protocol_get_name(protocol);
		}
	}

	return g_strdup(name);
}

static char *
pidgin_account_row_protocol_icon_cb(G_GNUC_UNUSED GObject *self,
                                    PurpleAccount *account,
                                    G_GNUC_UNUSED gpointer data)
{
	const char *icon_name = NULL;

	if(PURPLE_IS_ACCOUNT(account)) {
		PurpleProtocol *protocol = purple_account_get_protocol(account);
		if(PURPLE_IS_PROTOCOL(protocol)) {
			icon_name = purple_protocol_get_icon_name(protocol);
		}
	}

	return g_strdup(icon_name);
}

static void
pidgin_account_manager_remove_account_cb(G_GNUC_UNUSED AdwMessageDialog *self,
                                         char *response, gpointer data)
{
	PurpleAccount *account = data;
	PurpleNotificationManager *notification_manager = NULL;

	if(!purple_strequal(response, "remove")) {
		return;
	}

	/* Remove all notifications including connection errors for the account. */
	notification_manager = purple_notification_manager_get_default();
	purple_notification_manager_remove_with_account(notification_manager,
	                                                account, TRUE);

	/* Delete the account. */
	purple_accounts_delete(account);
}

static void
pidgin_account_row_remove_cb(G_GNUC_UNUSED GtkButton *self, gpointer data) {
	PidginAccountRow *row = data;
	GtkRoot *root = NULL;
	AdwMessageDialog *dialog = NULL;
	PurpleContactInfo *info = NULL;
	const char *name = NULL;
	char *protocol_name = NULL;

	info = PURPLE_CONTACT_INFO(row->account);
	name = purple_contact_info_get_name_for_display(info);
	protocol_name = pidgin_account_row_protocol_name_cb(NULL, row->account,
	                                                    NULL);

	root = gtk_widget_get_root(GTK_WIDGET(row));
	dialog = ADW_MESSAGE_DIALOG(adw_message_dialog_new(GTK_WINDOW(root),
	                                                   _("Remove account?"),
	                                                   NULL));
	adw_message_dialog_format_body(dialog,
	                               _("Do you want to remove the %s (%s) "
	                                 "account from Pidgin?"),
	                               name, protocol_name);
	adw_message_dialog_add_responses(dialog, "cancel", _("Cancel"),
	                                 "remove", _("Remove"), NULL);
	adw_message_dialog_set_response_appearance(dialog, "remove",
	                                           ADW_RESPONSE_DESTRUCTIVE);
	adw_message_dialog_set_default_response(dialog, "cancel");
	adw_message_dialog_set_close_response(dialog, "cancel");

	g_signal_connect_object(dialog, "response",
	                        G_CALLBACK(pidgin_account_manager_remove_account_cb),
	                        row->account, 0);

	gtk_window_present(GTK_WINDOW(dialog));

	g_free(protocol_name);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PidginAccountRow, pidgin_account_row, GTK_TYPE_LIST_BOX_ROW)

static void
pidgin_account_row_get_property(GObject *obj, guint param_id, GValue *value,
                                GParamSpec *pspec)
{
	PidginAccountRow *row = PIDGIN_ACCOUNT_ROW(obj);

	switch(param_id) {
		case PROP_ACCOUNT:
			g_value_set_object(value, pidgin_account_row_get_account(row));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_account_row_set_property(GObject *obj, guint param_id,
                                const GValue *value, GParamSpec *pspec)
{
	PidginAccountRow *row = PIDGIN_ACCOUNT_ROW(obj);

	switch(param_id) {
		case PROP_ACCOUNT:
			pidgin_account_row_set_account(row, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_account_row_finalize(GObject *obj) {
	PidginAccountRow *row = PIDGIN_ACCOUNT_ROW(obj);

	g_clear_object(&row->account);

	G_OBJECT_CLASS(pidgin_account_row_parent_class)->finalize(obj);
}

static void
pidgin_account_row_init(PidginAccountRow *row) {
	gtk_widget_init_template(GTK_WIDGET(row));

	pidgin_account_row_refresh_buddy_icon(row);
	pidgin_account_row_refresh_status(row);
}

static void
pidgin_account_row_class_init(PidginAccountRowClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->finalize = pidgin_account_row_finalize;
	obj_class->get_property = pidgin_account_row_get_property;
	obj_class->set_property = pidgin_account_row_set_property;

	/* properties */

	/**
	 * PidginAccountRow:account:
	 *
	 * The account that this row will be representing.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ACCOUNT] = g_param_spec_object(
		"account", "account",
		"The account that this row is representing",
		PURPLE_TYPE_ACCOUNT,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(
	    widget_class,
	    "/im/pidgin/Pidgin3/Accounts/account-row.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginAccountRow,
	                                     enabled);
	gtk_widget_class_bind_template_child(widget_class, PidginAccountRow,
	                                     avatar);
	gtk_widget_class_bind_template_child(widget_class, PidginAccountRow,
	                                     name);
	gtk_widget_class_bind_template_child(widget_class, PidginAccountRow,
	                                     status);

	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_account_row_enable_state_set_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_account_row_buddyicon_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_account_row_protocol_name_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_account_row_protocol_icon_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_account_row_remove_cb);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_account_row_new(PurpleAccount *account) {
	return g_object_new(PIDGIN_TYPE_ACCOUNT_ROW, "account", account, NULL);
}

PurpleAccount *
pidgin_account_row_get_account(PidginAccountRow *row) {
	g_return_val_if_fail(PIDGIN_IS_ACCOUNT_ROW(row), NULL);

	return row->account;
}

void
pidgin_account_row_set_account(PidginAccountRow *row, PurpleAccount *account) {
	PurpleAccount *old = NULL;

	g_return_if_fail(PIDGIN_IS_ACCOUNT_ROW(row));

	old = (row->account != NULL) ? g_object_ref(row->account) : NULL;

	if(g_set_object(&row->account, account)) {
		if(PURPLE_IS_ACCOUNT(old)) {
			PurpleConnection *connection = purple_account_get_connection(old);

			if(PURPLE_IS_CONNECTION(connection)) {
				g_signal_handlers_disconnect_by_func(connection,
				                                     pidgin_account_row_state_changed_cb,
				                                     row);
			}

			g_signal_handlers_disconnect_by_func(old,
			                                     pidgin_account_row_connection_changed_cb,
			                                     row);
		}

		if(PURPLE_IS_ACCOUNT(account)) {
			g_signal_connect_object(account, "notify::connection",
			                        G_CALLBACK(pidgin_account_row_connection_changed_cb),
			                        row, 0);
			g_signal_connect_object(account, "notify::error",
			                        G_CALLBACK(pidgin_account_row_state_changed_cb),
			                        row, 0);
			pidgin_account_row_refresh_buddy_icon(row);
			pidgin_account_row_refresh_status(row);
		}

		g_object_notify_by_pspec(G_OBJECT(row), properties[PROP_ACCOUNT]);
	}

	g_clear_object(&old);
}
