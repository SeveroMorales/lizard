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

#include <purple.h>

#include "pidginaccountmanager.h"

#include "gtkaccount.h"
#include "pidgincore.h"
#include "pidginaccounteditor.h"
#include "pidginaccountrow.h"

struct _PidginAccountManager {
	GtkDialog parent;

	GtkListBox *list_box;
	GtkWidget *add;
};

enum {
	RESPONSE_ADD,
};

G_DEFINE_TYPE(PidginAccountManager, pidgin_account_manager, GTK_TYPE_DIALOG)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static GtkWidget *
pidgin_account_manager_create_widget(gpointer item,
                                     G_GNUC_UNUSED gpointer data)
{
	if(!PURPLE_IS_ACCOUNT(item)) {
		return NULL;
	}

	return pidgin_account_row_new(PURPLE_ACCOUNT(item));
}

static void
pidgin_account_manager_create_account(PidginAccountManager *manager) {
	GtkWidget *editor = pidgin_account_editor_new(NULL);
	gtk_window_set_transient_for(GTK_WINDOW(editor),
	                             GTK_WINDOW(manager));
	gtk_window_present_with_time(GTK_WINDOW(editor), GDK_CURRENT_TIME);
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/

static void
pidgin_account_manager_refresh_add_cb(GListModel *list,
                                      G_GNUC_UNUSED guint position,
                                      G_GNUC_UNUSED guint removed,
                                      G_GNUC_UNUSED guint added,
                                      gpointer data)
{
	PidginAccountManager *manager = data;

	/* If there are no accounts, the placeholder is shown, which includes an
	 * Add button. So hide the one in the button box if that's the case. */
	gtk_widget_set_visible(manager->add, g_list_model_get_n_items(list) != 0);
}

static void
pidgin_account_manager_response_cb(GtkDialog *dialog, gint response_id,
                                   G_GNUC_UNUSED gpointer data)
{
	PidginAccountManager *manager = PIDGIN_ACCOUNT_MANAGER(dialog);

	switch(response_id) {
		case RESPONSE_ADD:
			pidgin_account_manager_create_account(manager);
			break;
		case GTK_RESPONSE_CLOSE:
		case GTK_RESPONSE_DELETE_EVENT:
			gtk_window_destroy(GTK_WINDOW(dialog));
			break;
		default:
			g_warning("not sure how you got here...");
	}
}

static void
pidgin_account_manager_row_activated_cb(G_GNUC_UNUSED GtkListBox *box,
                                        GtkListBoxRow *row,
                                        G_GNUC_UNUSED gpointer data)
{
	GtkWidget *editor = NULL;
	PurpleAccount *account = NULL;

	account = pidgin_account_row_get_account(PIDGIN_ACCOUNT_ROW(row));
	editor = pidgin_account_editor_new(account);
	gtk_widget_show(editor);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_account_manager_init(PidginAccountManager *manager) {
	GListModel *purple_manager = NULL;

	gtk_widget_init_template(GTK_WIDGET(manager));

	purple_manager = purple_account_manager_get_default_as_model();
	gtk_list_box_bind_model(manager->list_box, purple_manager,
	                        pidgin_account_manager_create_widget, NULL, NULL);
	g_signal_connect_object(purple_manager, "items-changed",
	                        G_CALLBACK(pidgin_account_manager_refresh_add_cb),
	                        manager, 0);
	pidgin_account_manager_refresh_add_cb(purple_manager, 0, 0, 0, manager);
}

static void
pidgin_account_manager_class_init(PidginAccountManagerClass *klass) {
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/im/pidgin/Pidgin3/Accounts/manager.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginAccountManager,
	                                     list_box);
	gtk_widget_class_bind_template_child(widget_class, PidginAccountManager,
	                                     add);

	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_account_manager_response_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_account_manager_row_activated_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_account_manager_create_account);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_account_manager_new(void) {
	return g_object_new(PIDGIN_TYPE_ACCOUNT_MANAGER, NULL);
}
