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

#include <glib/gi18n.h>

#include "pidginaddbuddydialog.h"

#include "gtkaccount.h"
#include "pidginaccountchooser.h"
#include "pidgincore.h"

struct _PidginAddBuddyDialog {
	GtkDialog parent;

	GtkCustomFilter *filter;

	GtkWidget *account;
	GtkWidget *username;
	GtkWidget *alias;
	GtkWidget *message;
	GtkWidget *group;
};

G_DEFINE_TYPE(PidginAddBuddyDialog, pidgin_add_buddy_dialog, GTK_TYPE_DIALOG)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static gboolean
pidgin_add_buddy_dialog_filter_accounts(gpointer item,
                                        G_GNUC_UNUSED gpointer data)
{
	gboolean ret = FALSE;

	if(PURPLE_IS_ACCOUNT(item)) {
		PurpleAccount *account = PURPLE_ACCOUNT(item);
		PurpleProtocol *protocol = purple_account_get_protocol(account);

		if(PURPLE_IS_PROTOCOL(protocol)) {
			ret = PURPLE_PROTOCOL_IMPLEMENTS(protocol, SERVER, add_buddy);
		}
	}

	return ret;
}

static void
pidgin_add_buddy_dialog_validate(PidginAddBuddyDialog *dialog) {
	PurpleAccount *account = NULL;
	gboolean valid = FALSE;

	account = pidgin_account_chooser_get_selected(PIDGIN_ACCOUNT_CHOOSER(dialog->account));
	if(PURPLE_IS_ACCOUNT(account)) {
		PurpleProtocol *protocol = NULL;

		protocol = purple_account_get_protocol(account);
		if(PURPLE_IS_PROTOCOL(protocol)) {
			const gchar *username = gtk_editable_get_text(GTK_EDITABLE(dialog->username));

			valid = purple_validate(protocol, username);
		}
	}

	gtk_dialog_set_response_sensitive(GTK_DIALOG(dialog), GTK_RESPONSE_OK,
	                                  valid);
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_add_buddy_dialog_response_cb(GtkDialog *dialog, gint response_id,
                                    G_GNUC_UNUSED gpointer data)
{
	PidginAddBuddyDialog *abdialog = PIDGIN_ADD_BUDDY_DIALOG(dialog);

	if(response_id == GTK_RESPONSE_OK) {
		PurpleAccount *account = NULL;
		PurpleBuddy *buddy = NULL;
		PurpleGroup *group = NULL;
		const gchar *username = NULL, *alias = NULL, *message = NULL;
		gchar *groupname = NULL;

		/* Grab all of the values that the user entered. */
		account = pidgin_account_chooser_get_selected(PIDGIN_ACCOUNT_CHOOSER(abdialog->account));
		username = gtk_editable_get_text(GTK_EDITABLE(abdialog->username));
		alias = gtk_editable_get_text(GTK_EDITABLE(abdialog->alias));
		message = gtk_editable_get_text(GTK_EDITABLE(abdialog->message));
		groupname = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(abdialog->group));

		/* Make anything that is an empty string NULL. */
		if(alias != NULL && *alias == '\0') {
			alias = NULL;
		}

		if(message != NULL && *message == '\0') {
			message = NULL;
		}

		if(groupname != NULL && *groupname == '\0') {
			g_clear_pointer(&groupname, g_free);
		}

		/* Find the PurpleGroup that the user requested. */
		if(groupname != NULL) {
			group = purple_blist_find_group(groupname);

			if(group == NULL) {
				/* Create the group if it's new. */
				group = purple_group_new(groupname);
				purple_blist_add_group(group, NULL);
			} else {
				/* Otherwise see if we already have a buddy in the existing
				 * group.
				 */
				buddy = purple_blist_find_buddy_in_group(account, username,
				                                         group);
			}
		} else {
			/* If a group name was not provided, look if the buddy exists, and
			 * use its group.
			 */
			buddy = purple_blist_find_buddy(account, username);
			if(PURPLE_IS_BUDDY(buddy)) {
				group = purple_buddy_get_group(buddy);
			}
		}

		/* If we don't a buddy yet, create it now. */
		if(!PURPLE_IS_BUDDY(buddy)) {
			buddy = purple_buddy_new(account, username, alias);
			purple_blist_add_buddy(buddy, NULL, group, NULL);
		}

		/* Add the buddy to the account. */
		purple_account_add_buddy(account, buddy, message);

#if 0
		/* This is disabled for now because gtk_blist_auto_personize is static
		 * and we're going to be changing the data structure of the contact list
		 * in the near future, so this probably completely change.
		 */

		/* Offer to merge people with the same alias. */
		if(alias != NULL && PURPLE_IS_GROUP(group)) {
			gtk_blist_auto_personize(PURPLE_BLIST_NODE(group), alias);
		}
#endif

		g_free(groupname);
	}

	gtk_window_destroy(GTK_WINDOW(abdialog));
}

static void
pidgin_add_buddy_dialog_account_changed_cb(GObject *obj,
                                           G_GNUC_UNUSED GParamSpec *pspec,
                                           gpointer data)
{
	PidginAddBuddyDialog *dialog = data;
	PurpleAccount *account = NULL;
	gboolean message_sensitive = TRUE;

	account = pidgin_account_chooser_get_selected(PIDGIN_ACCOUNT_CHOOSER(obj));

	if(PURPLE_IS_ACCOUNT(account)) {
		PurpleProtocol *protocol = purple_account_get_protocol(account);

		if(PURPLE_IS_PROTOCOL(protocol)) {
			PurpleProtocolOptions opts = purple_protocol_get_options(protocol);

			message_sensitive = (opts & OPT_PROTO_INVITE_MESSAGE);
		}
	}

	gtk_widget_set_sensitive(dialog->message, message_sensitive);

	pidgin_add_buddy_dialog_validate(dialog);
}

static void
pidgin_add_buddy_dialog_username_changed_cb(G_GNUC_UNUSED GtkEditable *editable,
                                            gpointer data)
{
	pidgin_add_buddy_dialog_validate(data);
}

static void
pidgin_add_buddy_dialog_group_cb(PurpleBlistNode *node, gpointer data) {
	PidginAddBuddyDialog *dialog = data;
	PurpleGroup *group = PURPLE_GROUP(node);

	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(dialog->group),
	                               purple_group_get_name(group));
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_add_buddy_dialog_init(PidginAddBuddyDialog *dialog) {
	gtk_widget_init_template(GTK_WIDGET(dialog));

	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	gtk_custom_filter_set_filter_func(dialog->filter,
	                                  pidgin_add_buddy_dialog_filter_accounts,
	                                  NULL, NULL);

	purple_blist_walk(pidgin_add_buddy_dialog_group_cb, NULL, NULL, NULL,
	                  dialog);
}

static void
pidgin_add_buddy_dialog_class_init(PidginAddBuddyDialogClass *klass) {
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/im/pidgin/Pidgin3/Dialogs/addbuddy.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginAddBuddyDialog,
	                                     filter);

	gtk_widget_class_bind_template_child(widget_class, PidginAddBuddyDialog,
	                                     account);
	gtk_widget_class_bind_template_child(widget_class, PidginAddBuddyDialog,
	                                     username);
	gtk_widget_class_bind_template_child(widget_class, PidginAddBuddyDialog,
	                                     alias);
	gtk_widget_class_bind_template_child(widget_class, PidginAddBuddyDialog,
	                                     message);
	gtk_widget_class_bind_template_child(widget_class, PidginAddBuddyDialog,
	                                     group);

	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_add_buddy_dialog_response_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_add_buddy_dialog_account_changed_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_add_buddy_dialog_username_changed_cb);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_add_buddy_dialog_new(PurpleAccount *account, const gchar *username,
                            const gchar *alias, const gchar *message,
                            const gchar *group)
{
	GtkWidget *dialog = g_object_new(PIDGIN_TYPE_ADD_BUDDY_DIALOG, NULL);
	PidginAddBuddyDialog *abdialog = PIDGIN_ADD_BUDDY_DIALOG(dialog);

	if(PURPLE_IS_ACCOUNT(account)) {
		pidgin_account_chooser_set_selected(PIDGIN_ACCOUNT_CHOOSER(abdialog->account),
		                                    account);
	}

	if(username != NULL) {
		gtk_editable_set_text(GTK_EDITABLE(abdialog->username), username);
	}

	if(alias != NULL) {
		gtk_editable_set_text(GTK_EDITABLE(abdialog->alias), alias);
	}

	if(message != NULL) {
		gtk_editable_set_text(GTK_EDITABLE(abdialog->message), message);
	}

	if(group != NULL) {
		GtkWidget *entry = NULL;

		entry = gtk_combo_box_get_child(GTK_COMBO_BOX(abdialog->group));
		gtk_editable_set_text(GTK_EDITABLE(entry), group);
	}

	return dialog;
}
