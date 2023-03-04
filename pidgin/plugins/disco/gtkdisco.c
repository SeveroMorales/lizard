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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib/gi18n-lib.h>

#include <purple.h>

#include <pidgin.h>

#include "gtkdisco.h"
#include "xmppdisco.h"

GList *dialogs = NULL;

enum {
	ICON_NAME_COLUMN = 0,
	NAME_COLUMN,
	DESCRIPTION_COLUMN,
	SERVICE_COLUMN,
	NUM_OF_COLUMNS
};

static PidginDiscoList *
pidgin_disco_list_new(void) {
	return g_rc_box_new0(PidginDiscoList);
}

static void
pidgin_disco_list_destroy(PidginDiscoList *list)
{
	if (list->dialog && list->dialog->discolist == list) {
		list->dialog->discolist = NULL;
	}

	g_free((gchar*)list->server);
}

PidginDiscoList *pidgin_disco_list_ref(PidginDiscoList *list)
{
	g_return_val_if_fail(list != NULL, NULL);

	purple_debug_misc("xmppdisco", "reffing list");
	return g_rc_box_acquire(list);
}

void pidgin_disco_list_unref(PidginDiscoList *list)
{
	g_return_if_fail(list != NULL);

	purple_debug_misc("xmppdisco", "unreffing list");
	g_rc_box_release_full(list, (GDestroyNotify)pidgin_disco_list_destroy);
}

void pidgin_disco_list_set_in_progress(PidginDiscoList *list, gboolean in_progress)
{
	PidginDiscoDialog *dialog = list->dialog;

	if (!dialog) {
		return;
	}

	list->in_progress = in_progress;

	if (in_progress) {
		gtk_widget_set_sensitive(dialog->account_chooser, FALSE);
		g_simple_action_set_enabled(dialog->stop_action, TRUE);
		g_simple_action_set_enabled(dialog->browse_action, FALSE);
	} else {
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(dialog->progress), 0.0);

		gtk_widget_set_sensitive(dialog->account_chooser, TRUE);

		g_simple_action_set_enabled(dialog->stop_action, FALSE);
		g_simple_action_set_enabled(dialog->browse_action, TRUE);
/*
		g_simple_action_set_enabled(dialog->register_action, FALSE);
		g_simple_action_set_enabled(dialog->add_action, FALSE);
*/
	}
}

static void
dialog_select_account_cb(GObject *obj, G_GNUC_UNUSED GParamSpec *pspec,
                         gpointer data)
{
	PidginDiscoDialog *dialog = data;
	PurpleAccount *account = pidgin_account_chooser_get_selected(PIDGIN_ACCOUNT_CHOOSER(obj));
	gboolean change = (account != dialog->account);
	dialog->account = account;
	g_simple_action_set_enabled(dialog->browse_action, account != NULL);

	if (change) {
		g_clear_pointer(&dialog->discolist, pidgin_disco_list_unref);
	}
}

static void
activate_register(G_GNUC_UNUSED GSimpleAction *action,
                  G_GNUC_UNUSED GVariant *parameter,
                  gpointer data)
{
	PidginDiscoDialog *dialog = data;
	XmppDiscoService *service = dialog->selected;

	g_return_if_fail(XMPP_DISCO_IS_SERVICE(service));

	xmpp_disco_register_service(service);
}

static void
discolist_cancel_cb(PidginDiscoList *pdl, G_GNUC_UNUSED const char *server) {
	pdl->dialog->prompt_handle = NULL;

	pidgin_disco_list_set_in_progress(pdl, FALSE);
	pidgin_disco_list_unref(pdl);
}

static void discolist_ok_cb(PidginDiscoList *pdl, const char *server)
{
	pdl->dialog->prompt_handle = NULL;
	g_simple_action_set_enabled(pdl->dialog->browse_action, TRUE);

	if (!server || !*server) {
		purple_notify_error(my_plugin, _("Invalid Server"), _("Invalid Server"),
			NULL, purple_request_cpar_from_connection(pdl->pc));

		pidgin_disco_list_set_in_progress(pdl, FALSE);
		pidgin_disco_list_unref(pdl);
		return;
	}

	pdl->server = g_strdup(server);
	pidgin_disco_list_set_in_progress(pdl, TRUE);
	xmpp_disco_start(pdl);
}

static void
activate_browse(G_GNUC_UNUSED GSimpleAction *action,
                G_GNUC_UNUSED GVariant *parameter,
                gpointer data)
{
	PidginDiscoDialog *dialog = data;
	PurpleConnection *pc;
	PurpleContactInfo *info = NULL;
	PidginDiscoList *pdl;
	const char *username;
	const char *at, *slash;
	char *server = NULL;

	pc = purple_account_get_connection(dialog->account);
	if (!pc) {
		return;
	}

	g_simple_action_set_enabled(dialog->browse_action, FALSE);
	g_simple_action_set_enabled(dialog->add_action, FALSE);
	g_simple_action_set_enabled(dialog->register_action, FALSE);

	g_clear_pointer(&dialog->discolist, pidgin_disco_list_unref);
	g_list_store_remove_all(dialog->root);

	pdl = dialog->discolist = pidgin_disco_list_new();
	pdl->pc = pc;
	/* We keep a copy... */
	pidgin_disco_list_ref(pdl);

	pdl->dialog = dialog;

	gtk_widget_set_sensitive(dialog->account_chooser, FALSE);

	info = PURPLE_CONTACT_INFO(dialog->account);
	username = purple_contact_info_get_username(info);
	at = strchr(username, '@');
	slash = strchr(username, '/');
	if (at && !slash) {
		server = g_strdup(at + 1);
	} else if (at && slash && at + 1 < slash) {
		server = g_strdup_printf("%.*s", (int)(slash - (at + 1)), at + 1);
	}

	if (server == NULL) {
		/* This shouldn't ever happen since the account is connected */
		server = g_strdup("jabber.org");
	}

	/* Translators: The string "Enter an XMPP Server" is asking the user to
	   type the name of an XMPP server which will then be queried */
	dialog->prompt_handle = purple_request_input(my_plugin, _("Server name request"), _("Enter an XMPP Server"),
			_("Select an XMPP server to query"),
			server, FALSE, FALSE, NULL,
			_("Find Services"), G_CALLBACK(discolist_ok_cb),
			_("Cancel"), G_CALLBACK(discolist_cancel_cb),
			purple_request_cpar_from_connection(pc), pdl);

	g_free(server);
}

static void
activate_add_to_blist(G_GNUC_UNUSED GSimpleAction *action,
                      G_GNUC_UNUSED GVariant *parameter,
                      gpointer data)
{
	PidginDiscoDialog *dialog = data;
	XmppDiscoService *service = dialog->selected;
	PidginDiscoList *list = NULL;
	PurpleAccount *account;
	const char *jid;

	g_return_if_fail(XMPP_DISCO_IS_SERVICE(service));

	list = xmpp_disco_service_get_list(service);
	account = purple_connection_get_account(list->pc);
	jid = xmpp_disco_service_get_jid(service);

	if(xmpp_disco_service_get_service_type(service) == XMPP_DISCO_SERVICE_TYPE_CHAT) {
		purple_blist_request_add_chat(account, NULL, NULL, jid);
	} else {
		purple_blist_request_add_buddy(account, jid, NULL, NULL);
	}
}

static void
selection_changed_cb(GtkSelectionModel *self, G_GNUC_UNUSED guint position,
                     G_GNUC_UNUSED guint n_items, gpointer data)
{
	PidginDiscoDialog *dialog = data;
	GtkTreeListRow *row = NULL;
	gboolean allow_add = FALSE, allow_register = FALSE;

	/* The passed in position and n_items gives the *range* of selections that
	 * have changed, so just re-query it since GtkSingleSelection has exactly
	 * one. */
	row = gtk_single_selection_get_selected_item(GTK_SINGLE_SELECTION(self));
	if(row != NULL) {
		dialog->selected = gtk_tree_list_row_get_item(row);
		if(XMPP_DISCO_IS_SERVICE(dialog->selected)) {
			XmppDiscoServiceFlags flags;

			flags = xmpp_disco_service_get_flags(dialog->selected);
			allow_add = (flags & XMPP_DISCO_ADD) != 0;
			allow_register = (flags & XMPP_DISCO_REGISTER) != 0;

			/* gtk_tree_list_row_get_item returns a ref, but this struct isn't
			 * supposed to hold one, as the model holds on to it. */
			g_object_unref(dialog->selected);
		}
	}

	g_simple_action_set_enabled(dialog->add_action, allow_add);
	g_simple_action_set_enabled(dialog->register_action, allow_register);
}

static GListModel *
service_create_child_model_cb(GObject *item, G_GNUC_UNUSED gpointer data) {
	XmppDiscoService *service = XMPP_DISCO_SERVICE(item);
	GListModel *model = NULL;

	model = xmpp_disco_service_get_child_model(service);
	if(G_IS_LIST_MODEL(model)) {
		/* Always return a new ref, as the caller takes ownership. */
		g_object_ref(model);
	}

	return model;
}

static void
row_expanded_cb(GObject *obj, G_GNUC_UNUSED GParamSpec *pspec,
                G_GNUC_UNUSED gpointer data)
{
	GtkTreeListRow *row = GTK_TREE_LIST_ROW(obj);

	if(gtk_tree_list_row_get_expanded(row)) {
		XmppDiscoService *service = gtk_tree_list_row_get_item(row);
		if(XMPP_DISCO_IS_SERVICE(service)) {
			xmpp_disco_service_expand(service);
		}
		g_clear_object(&service);
	}
}

static void
list_row_notify_cb(GObject *obj, G_GNUC_UNUSED GParamSpec *pspec,
                   G_GNUC_UNUSED gpointer data)
{
	GtkTreeListRow *row = NULL;

	row = gtk_tree_expander_get_list_row(GTK_TREE_EXPANDER(obj));
	if(GTK_IS_TREE_LIST_ROW(row)) {
		g_signal_connect(row, "notify::expanded", G_CALLBACK(row_expanded_cb),
		                 NULL);
	}
}

static void
row_activated_cb(GtkColumnView *self, guint position, gpointer user_data) {
	PidginDiscoDialog *dialog = user_data;
	GtkSelectionModel *model = NULL;
	GtkTreeListRow *row = NULL;
	XmppDiscoService *service = NULL;
	XmppDiscoServiceFlags flags;

	model = gtk_column_view_get_model(self);
	row = g_list_model_get_item(G_LIST_MODEL(model), position);
	service = gtk_tree_list_row_get_item(row);

	flags = xmpp_disco_service_get_flags(service);
	if((flags & XMPP_DISCO_BROWSE) != 0) {
		if(gtk_tree_list_row_get_expanded(row)) {
			gtk_tree_list_row_set_expanded(row, FALSE);
		} else {
			gtk_tree_list_row_set_expanded(row, TRUE);
		}
	} else if((flags & XMPP_DISCO_REGISTER) != 0) {
		g_action_activate(G_ACTION(dialog->register_action), NULL);
	} else if((flags & XMPP_DISCO_ADD) != 0) {
		g_action_activate(G_ACTION(dialog->add_action), NULL);
	}

	g_clear_object(&service);
	g_clear_object(&row);
}

static void
destroy_win_cb(GtkWidget *window, G_GNUC_UNUSED gpointer data)
{
	PidginDiscoDialog *dialog = PIDGIN_DISCO_DIALOG(window);
	PidginDiscoList *list = dialog->discolist;

	if (dialog->prompt_handle) {
		purple_request_close(PURPLE_REQUEST_INPUT, dialog->prompt_handle);
	}

	if (list) {
		list->dialog = NULL;

		if (list->in_progress) {
			list->in_progress = FALSE;
		}

		pidgin_disco_list_unref(list);
	}

	dialogs = g_list_remove(dialogs, dialog);
}

static void
activate_stop(G_GNUC_UNUSED GSimpleAction *action,
              G_GNUC_UNUSED GVariant *parameter,
              gpointer data)
{
	PidginDiscoDialog *dialog = data;

	pidgin_disco_list_set_in_progress(dialog->discolist, FALSE);
}

void pidgin_disco_signed_off_cb(PurpleConnection *pc)
{
	GList *node;

	for (node = dialogs; node; node = node->next) {
		PidginDiscoDialog *dialog = node->data;
		PidginDiscoList *list = dialog->discolist;

		if (list && list->pc == pc) {
			PurpleAccount *account = NULL;

			if (list->in_progress) {
				pidgin_disco_list_set_in_progress(list, FALSE);
			}

			g_list_store_remove_all(dialog->root);

			pidgin_disco_list_unref(list);
			dialog->discolist = NULL;

			account = pidgin_account_chooser_get_selected(
				PIDGIN_ACCOUNT_CHOOSER(dialog->account_chooser));

			g_simple_action_set_enabled(dialog->browse_action, account != NULL);
			g_simple_action_set_enabled(dialog->add_action, FALSE);
			g_simple_action_set_enabled(dialog->register_action, FALSE);
		}
	}
}

void pidgin_disco_dialogs_destroy_all(void)
{
	while (dialogs) {
		GtkWidget *dialog = dialogs->data;

		gtk_window_destroy(GTK_WINDOW(dialog));
		/* destroy_win_cb removes the dialog from the list */
	}
}

/******************************************************************************
 * GObject implementation
 *****************************************************************************/

G_DEFINE_DYNAMIC_TYPE(PidginDiscoDialog, pidgin_disco_dialog, GTK_TYPE_DIALOG)

static void
pidgin_disco_dialog_class_init(PidginDiscoDialogClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
	        widget_class, "/im/pidgin/Pidgin3/Plugin/XMPPDisco/disco.ui");

	gtk_widget_class_bind_template_child(widget_class, PidginDiscoDialog,
	                                     account_chooser);
	gtk_widget_class_bind_template_child(widget_class, PidginDiscoDialog,
	                                     progress);
	gtk_widget_class_bind_template_child(widget_class, PidginDiscoDialog,
	                                     sorter);

	gtk_widget_class_bind_template_callback(widget_class, destroy_win_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        dialog_select_account_cb);
	gtk_widget_class_bind_template_callback(widget_class, row_activated_cb);
	gtk_widget_class_bind_template_callback(widget_class, list_row_notify_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        selection_changed_cb);
}

static void
pidgin_disco_dialog_class_finalize(G_GNUC_UNUSED PidginDiscoDialogClass *klass)
{
}

static void
pidgin_disco_dialog_init(PidginDiscoDialog *dialog)
{
	GtkTreeListModel *model = NULL;
	GActionEntry entries[] = {
		{ .name = "stop", .activate = activate_stop },
		{ .name = "browse", .activate = activate_browse },
		{ .name = "register", .activate = activate_register },
		{ .name = "add", .activate = activate_add_to_blist },
	};
	GSimpleActionGroup *action_group = NULL;
	GActionMap *action_map = NULL;

	dialogs = g_list_prepend(dialogs, dialog);

	gtk_widget_init_template(GTK_WIDGET(dialog));

	/* accounts dropdown list */
	dialog->account = pidgin_account_chooser_get_selected(
		PIDGIN_ACCOUNT_CHOOSER(dialog->account_chooser));

	action_group = g_simple_action_group_new();
	action_map = G_ACTION_MAP(action_group);
	g_action_map_add_action_entries(action_map, entries, G_N_ELEMENTS(entries),
	                                dialog);

	dialog->stop_action = G_SIMPLE_ACTION(
			g_action_map_lookup_action(action_map, "stop"));
	g_simple_action_set_enabled(dialog->stop_action, FALSE);

	dialog->browse_action = G_SIMPLE_ACTION(
			g_action_map_lookup_action(action_map, "browse"));
	g_simple_action_set_enabled(dialog->browse_action, dialog->account != NULL);

	dialog->register_action = G_SIMPLE_ACTION(
			g_action_map_lookup_action(action_map, "register"));
	g_simple_action_set_enabled(dialog->register_action, FALSE);

	dialog->add_action = G_SIMPLE_ACTION(
			g_action_map_lookup_action(action_map, "add"));
	g_simple_action_set_enabled(dialog->add_action, FALSE);

	gtk_widget_insert_action_group(GTK_WIDGET(dialog), "disco",
	                               G_ACTION_GROUP(action_group));

	dialog->root = g_list_store_new(XMPP_DISCO_TYPE_SERVICE);
	model = gtk_tree_list_model_new(G_LIST_MODEL(dialog->root), FALSE, FALSE,
	                                (GtkTreeListModelCreateModelFunc)service_create_child_model_cb,
	                                NULL, NULL);
	gtk_sort_list_model_set_model(dialog->sorter, G_LIST_MODEL(model));
	g_object_unref(model);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

void
pidgin_disco_dialog_register(PurplePlugin *plugin)
{
	pidgin_disco_dialog_register_type(G_TYPE_MODULE(plugin));
}

PidginDiscoDialog *
pidgin_disco_dialog_new(void)
{
	PidginDiscoDialog *dialog = g_object_new(PIDGIN_TYPE_DISCO_DIALOG, NULL);
	gtk_widget_show(GTK_WIDGET(dialog));
	return dialog;
}

void
pidgin_disco_add_service(PidginDiscoList *pdl, XmppDiscoService *service,
                         XmppDiscoService *parent)
{
	PidginDiscoDialog *dialog;

	dialog = pdl->dialog;
	g_return_if_fail(dialog != NULL);

	purple_debug_info("xmppdisco", "Adding service \"%s\" to %p",
	                  xmpp_disco_service_get_name(service), parent);

	gtk_progress_bar_pulse(GTK_PROGRESS_BAR(dialog->progress));

	if(parent != NULL) {
		xmpp_disco_service_add_child(parent, service);
	} else {
		g_list_store_append(dialog->root, service);
	}
}
