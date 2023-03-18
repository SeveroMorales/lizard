/* pidgin
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include <glib/gi18n-lib.h>

#include <gtk/gtk.h>

#include <purple.h>

#include "gtkroomlist.h"

#include "pidginaccountchooser.h"

#define PIDGIN_TYPE_ROOMLIST_DIALOG (pidgin_roomlist_dialog_get_type())
G_DECLARE_FINAL_TYPE(PidginRoomlistDialog, pidgin_roomlist_dialog, PIDGIN,
                     ROOMLIST_DIALOG, GtkDialog)

#define PIDGIN_ROOMLIST_UI_DATA "pidgin-ui"

enum {
	RESPONSE_STOP = 0,
	RESPONSE_LIST,
	RESPONSE_ADD,
	RESPONSE_JOIN,
};

struct _PidginRoomlistDialog {
	GtkDialog parent;

	GtkWidget *account_widget;
	GtkWidget *progress;
	GtkWidget *view;
	GtkSingleSelection *selection;
	GtkFilterListModel *filter;

	GtkWidget *stop_button;
	GtkWidget *list_button;
	GtkWidget *add_button;
	GtkWidget *join_button;
	GtkWidget *close_button;

	PurpleAccount *account;
	PurpleRoomlist *roomlist;

	gboolean pg_needs_pulse;
	guint pg_update_to;
};

G_DEFINE_TYPE(PidginRoomlistDialog, pidgin_roomlist_dialog, GTK_TYPE_DIALOG)

typedef struct {
	PidginRoomlistDialog *dialog;
	GListStore *model;
} PidginRoomlist;

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_roomlist_close(PidginRoomlistDialog *dialog)
{
	if (dialog->roomlist && purple_roomlist_get_in_progress(dialog->roomlist))
		purple_roomlist_cancel_get_list(dialog->roomlist);

	if (dialog->pg_update_to > 0)
		g_source_remove(dialog->pg_update_to);

	if (dialog->roomlist) {
		PidginRoomlist *rl = NULL;

		rl = g_object_get_data(G_OBJECT(dialog->roomlist),
		                       PIDGIN_ROOMLIST_UI_DATA);

		if (dialog->pg_update_to > 0)
			/* yes, that's right, unref it twice. */
			g_object_unref(dialog->roomlist);

		if (rl)
			rl->dialog = NULL;
		g_object_unref(dialog->roomlist);
	}

	dialog->progress = NULL;
}

static void
pidgin_roomlist_start_listing(PidginRoomlistDialog *dialog)
{
	PurpleConnection *gc;
	PidginRoomlist *rl;

	gc = purple_account_get_connection(dialog->account);
	if (!gc)
		return;

	if (dialog->roomlist != NULL) {
		rl = g_object_get_data(G_OBJECT(dialog->roomlist),
		                       PIDGIN_ROOMLIST_UI_DATA);

		g_clear_object(&rl->model);
		g_object_unref(dialog->roomlist);
	}

	dialog->roomlist = purple_roomlist_get_list(gc);
	if (!dialog->roomlist)
		return;
	g_object_ref(dialog->roomlist);

	rl = g_object_get_data(G_OBJECT(dialog->roomlist),
	                       PIDGIN_ROOMLIST_UI_DATA);
	rl->dialog = dialog;

	gtk_widget_set_sensitive(dialog->account_widget, FALSE);

	gtk_filter_list_model_set_model(dialog->filter, G_LIST_MODEL(rl->model));

	/* some protocols (not bundled with libpurple) finish getting their
	 * room list immediately */
	if(purple_roomlist_get_in_progress(dialog->roomlist)) {
		gtk_widget_set_sensitive(dialog->stop_button, TRUE);
		gtk_widget_set_sensitive(dialog->list_button, FALSE);
	} else {
		gtk_widget_set_sensitive(dialog->stop_button, FALSE);
		gtk_widget_set_sensitive(dialog->list_button, TRUE);
	}
	gtk_widget_set_sensitive(dialog->add_button, FALSE);
	gtk_widget_set_sensitive(dialog->join_button, FALSE);
}

static void
pidgin_roomlist_stop_listing(PidginRoomlistDialog *dialog)
{
	purple_roomlist_cancel_get_list(dialog->roomlist);

	gtk_widget_set_sensitive(dialog->account_widget, TRUE);

	gtk_widget_set_sensitive(dialog->stop_button, FALSE);
	gtk_widget_set_sensitive(dialog->list_button, TRUE);
	gtk_widget_set_sensitive(dialog->add_button, FALSE);
	gtk_widget_set_sensitive(dialog->join_button, FALSE);
}

static void
pidgin_roomlist_add_to_blist(PidginRoomlistDialog *dialog)
{
	char *name = NULL;
	PurpleAccount *account = NULL;
	PurpleConnection *gc = NULL;
	PurpleProtocol *protocol = NULL;
	PurpleRoomlistRoom *room = NULL;

	account = purple_roomlist_get_account(dialog->roomlist);
	gc = purple_account_get_connection(account);

	room = gtk_single_selection_get_selected_item(dialog->selection);
	g_return_if_fail(PURPLE_IS_ROOMLIST_ROOM(room));

	if(gc != NULL) {
		protocol = purple_connection_get_protocol(gc);
	}

	if(protocol != NULL && PURPLE_PROTOCOL_IMPLEMENTS(protocol, ROOMLIST, room_serialize)) {
		name = purple_protocol_roomlist_room_serialize(PURPLE_PROTOCOL_ROOMLIST(protocol),
		                                               room);
	} else {
		name = g_strdup(purple_roomlist_room_get_name(room));
	}

	purple_blist_request_add_chat(account, NULL, NULL, name);

	g_free(name);
}

static void
pidgin_roomlist_join(PidginRoomlistDialog *dialog)
{
	PurpleRoomlistRoom *room = NULL;

	room = gtk_single_selection_get_selected_item(dialog->selection);
	if(PURPLE_IS_ROOMLIST_ROOM(room)) {
		purple_roomlist_join_room(dialog->roomlist, room);
	}
}

/******************************************************************************
 * Actions
 *****************************************************************************/
static void
pidgin_roomlist_add_to_blist_cb(G_GNUC_UNUSED GSimpleAction *action,
                                G_GNUC_UNUSED GVariant *parameter,
                                gpointer data)
{
	pidgin_roomlist_add_to_blist(data);
}


static void
pidgin_roomlist_join_cb(G_GNUC_UNUSED GSimpleAction *action,
                        G_GNUC_UNUSED GVariant *parameter,
                        gpointer data)
{
	pidgin_roomlist_join(data);
}

static GActionEntry actions[] = {
	{
		.name = "add",
		.activate = pidgin_roomlist_add_to_blist_cb,
	}, {
		.name = "join",
		.activate = pidgin_roomlist_join_cb,
	},
};

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_roomlist_response_cb(GtkDialog *gtk_dialog, gint response_id,
                            G_GNUC_UNUSED gpointer data)
{
	PidginRoomlistDialog *dialog = PIDGIN_ROOMLIST_DIALOG(gtk_dialog);

	switch(response_id) {
	case RESPONSE_STOP:
		pidgin_roomlist_stop_listing(dialog);
		break;
	case RESPONSE_LIST:
		pidgin_roomlist_start_listing(dialog);
		break;
	case RESPONSE_ADD:
		pidgin_roomlist_add_to_blist(dialog);
		break;
	case RESPONSE_JOIN:
		pidgin_roomlist_join(dialog);
		break;
	case GTK_RESPONSE_CLOSE:
	case GTK_RESPONSE_DELETE_EVENT:
		gtk_window_destroy(GTK_WINDOW(gtk_dialog));
		break;
	}
}

static gboolean
close_request_cb(GtkWidget *w, G_GNUC_UNUSED gpointer d)
{
	pidgin_roomlist_close(PIDGIN_ROOMLIST_DIALOG(w));

	gtk_window_destroy(GTK_WINDOW(w));

	return TRUE;
}

static void
dialog_select_account_cb(GObject *obj, G_GNUC_UNUSED GParamSpec *pspec,
                         gpointer data)
{
	PidginRoomlistDialog *dialog = data;
	PidginAccountChooser *chooser = PIDGIN_ACCOUNT_CHOOSER(obj);
	PurpleAccount *account = pidgin_account_chooser_get_selected(chooser);
	gboolean change = (account != dialog->account);
	dialog->account = account;

	if (change && dialog->roomlist) {
		PidginRoomlist *rl = NULL;

		rl = g_object_get_data(G_OBJECT(dialog->roomlist),
		                       PIDGIN_ROOMLIST_UI_DATA);

		g_clear_object(&rl->model);
		g_clear_object(&dialog->roomlist);
	}
}

static void
selection_changed_cb(GtkSelectionModel *self, G_GNUC_UNUSED guint position,
                     G_GNUC_UNUSED guint n_items, gpointer data)
{
	PidginRoomlistDialog *dialog = data;
	guint index = GTK_INVALID_LIST_POSITION;
	gboolean found = FALSE;

	/* The passed in position and n_items gives the *range* of selections that
	 * have changed, so just re-query it exactly since GtkSingleSelection has
	 * only one. */
	index = gtk_single_selection_get_selected(GTK_SINGLE_SELECTION(self));
	found = index != GTK_INVALID_LIST_POSITION;

	gtk_widget_set_sensitive(dialog->add_button, found);
	gtk_widget_set_sensitive(dialog->join_button, found);
}

static void
row_activated_cb(GtkColumnView *self, guint position, gpointer data)
{
	PidginRoomlistDialog *dialog = data;
	PurpleRoomlistRoom *room = NULL;
	GtkSelectionModel *model = NULL;

	model = gtk_column_view_get_model(self);
	room = g_list_model_get_item(G_LIST_MODEL(model), position);

	if(PURPLE_IS_ROOMLIST_ROOM(room)) {
		purple_roomlist_join_room(dialog->roomlist, room);
	}

	g_clear_object(&room);
}

static gboolean
account_filter_func(gpointer item, G_GNUC_UNUSED gpointer data) {
	PurpleProtocol *protocol = NULL;

	if(PURPLE_IS_ACCOUNT(item)) {
		PurpleAccount *account = PURPLE_ACCOUNT(item);
		PurpleConnection *conn = purple_account_get_connection(account);
		if(conn && PURPLE_CONNECTION_IS_CONNECTED(conn)) {
			protocol = purple_connection_get_protocol(conn);
		}
	}

	return (protocol && PURPLE_PROTOCOL_IMPLEMENTS(protocol, ROOMLIST, get_list));
}

gboolean
pidgin_roomlist_is_showable(void)
{
	GList *c;
	PurpleConnection *gc;

	for (c = purple_connections_get_all(); c != NULL; c = c->next) {
		gc = c->data;

		if(account_filter_func(purple_connection_get_account(gc), NULL)) {
			return TRUE;
		}
	}

	return FALSE;
}

static void
pidgin_roomlist_dialog_class_init(PidginRoomlistDialogClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
	        widget_class, "/im/pidgin/Pidgin3/Roomlist/roomlist.ui");

	gtk_widget_class_bind_template_child(widget_class, PidginRoomlistDialog,
	                                     account_widget);
	gtk_widget_class_bind_template_child(widget_class, PidginRoomlistDialog,
	                                     view);
	gtk_widget_class_bind_template_child(widget_class, PidginRoomlistDialog,
	                                     selection);
	gtk_widget_class_bind_template_child(widget_class, PidginRoomlistDialog,
	                                     filter);
	gtk_widget_class_bind_template_child(widget_class, PidginRoomlistDialog,
	                                     add_button);
	gtk_widget_class_bind_template_child(widget_class, PidginRoomlistDialog,
	                                     close_button);
	gtk_widget_class_bind_template_child(widget_class, PidginRoomlistDialog,
	                                     join_button);
	gtk_widget_class_bind_template_child(widget_class, PidginRoomlistDialog,
	                                     list_button);
	gtk_widget_class_bind_template_child(widget_class, PidginRoomlistDialog,
	                                     progress);
	gtk_widget_class_bind_template_child(widget_class, PidginRoomlistDialog,
	                                     stop_button);

	gtk_widget_class_bind_template_callback(widget_class, close_request_cb);
	gtk_widget_class_bind_template_callback(widget_class, row_activated_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        dialog_select_account_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        selection_changed_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_roomlist_response_cb);
}

static void
pidgin_roomlist_dialog_init(PidginRoomlistDialog *self)
{
	GSimpleActionGroup *group = NULL;
	GtkCustomFilter *filter = NULL;

	gtk_widget_init_template(GTK_WIDGET(self));

	filter = gtk_custom_filter_new(account_filter_func, NULL, NULL);
	pidgin_account_chooser_set_filter(
	        PIDGIN_ACCOUNT_CHOOSER(self->account_widget),
	        GTK_FILTER(filter));
	g_object_unref(filter);

	/* Now setup our actions. */
	group = g_simple_action_group_new();
	g_action_map_add_action_entries(G_ACTION_MAP(group), actions,
	                                G_N_ELEMENTS(actions), self);
	gtk_widget_insert_action_group(GTK_WIDGET(self), "roomlist",
	                               G_ACTION_GROUP(group));
}

static PidginRoomlistDialog *
pidgin_roomlist_dialog_new_with_account(PurpleAccount *account)
{
	PidginRoomlistDialog *dialog = NULL;
	PidginAccountChooser *chooser = NULL;

	dialog = g_object_new(PIDGIN_TYPE_ROOMLIST_DIALOG, NULL);
	dialog->account = account;

	chooser = PIDGIN_ACCOUNT_CHOOSER(dialog->account_widget);

	if (!account) {
		/* This is normally NULL, and we normally don't care what the
		 * first selected item is */
		dialog->account = pidgin_account_chooser_get_selected(chooser);
	} else {
		pidgin_account_chooser_set_selected(chooser, account);
	}

	/* show the dialog window and return the dialog */
	gtk_widget_show(GTK_WIDGET(dialog));

	return dialog;
}

void pidgin_roomlist_dialog_show_with_account(PurpleAccount *account)
{
	PidginRoomlistDialog *dialog = pidgin_roomlist_dialog_new_with_account(account);

	pidgin_roomlist_start_listing(dialog);
}

void pidgin_roomlist_dialog_show(void)
{
	pidgin_roomlist_dialog_new_with_account(NULL);
}

static gboolean pidgin_progress_bar_pulse(gpointer data)
{
	PurpleRoomlist *list = data;
	PidginRoomlist *rl = NULL;

	rl = g_object_get_data(G_OBJECT(list), PIDGIN_ROOMLIST_UI_DATA);
	if (!rl || !rl->dialog || !rl->dialog->pg_needs_pulse) {
		if (rl && rl->dialog)
			rl->dialog->pg_update_to = 0;
		g_object_unref(list);
		return FALSE;
	}

	gtk_progress_bar_pulse(GTK_PROGRESS_BAR(rl->dialog->progress));
	rl->dialog->pg_needs_pulse = FALSE;
	return TRUE;
}

static void
pidgin_roomlist_add_room(PurpleRoomlist *list, PurpleRoomlistRoom *room) {
	PidginRoomlist *rl = NULL;

	rl = g_object_get_data(G_OBJECT(list), PIDGIN_ROOMLIST_UI_DATA);

	if (rl->dialog) {
		if (rl->dialog->pg_update_to == 0) {
			g_object_ref(list);
			rl->dialog->pg_update_to = g_timeout_add(100, pidgin_progress_bar_pulse, list);
			gtk_progress_bar_pulse(GTK_PROGRESS_BAR(rl->dialog->progress));
		} else
			rl->dialog->pg_needs_pulse = TRUE;
	}

	g_list_store_append(rl->model, room);
}

static void
pidgin_roomlist_in_progress(GObject *obj, G_GNUC_UNUSED GParamSpec *pspec,
                            gpointer data)
{
	PurpleRoomlist *list = PURPLE_ROOMLIST(obj);
	PidginRoomlist *rl = data;

	if (purple_roomlist_get_in_progress(list)) {
		if (rl->dialog->account_widget) {
			gtk_widget_set_sensitive(rl->dialog->account_widget, FALSE);
		}
		gtk_widget_set_sensitive(rl->dialog->stop_button, TRUE);
		gtk_widget_set_sensitive(rl->dialog->list_button, FALSE);
	} else {
		rl->dialog->pg_needs_pulse = FALSE;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(rl->dialog->progress), 0.0);
		if (rl->dialog->account_widget) {
			gtk_widget_set_sensitive(rl->dialog->account_widget, TRUE);
		}
		gtk_widget_set_sensitive(rl->dialog->stop_button, FALSE);
		gtk_widget_set_sensitive(rl->dialog->list_button, TRUE);
	}
}

static void
pidgin_roomlist_new(PurpleRoomlist *list)
{
	PidginRoomlist *rl = g_new0(PidginRoomlist, 1);

	g_object_set_data_full(G_OBJECT(list), PIDGIN_ROOMLIST_UI_DATA, rl,
	                       (GDestroyNotify)g_free);

	rl->model = g_list_store_new(PURPLE_TYPE_ROOMLIST_ROOM);

	g_signal_connect(list, "notify::in-progress",
	                 G_CALLBACK(pidgin_roomlist_in_progress), rl);
}

static PurpleRoomlistUiOps ops = {
	.show_with_account = pidgin_roomlist_dialog_show_with_account,
	.create = pidgin_roomlist_new,
	.add_room = pidgin_roomlist_add_room,
};


void pidgin_roomlist_init(void)
{
	purple_roomlist_set_ui_ops(&ops);
}
