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
 *
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib/gi18n-lib.h>

#include <purple.h>

#include "gtkblist.h"
#include "gtkconv.h"
#include "gtkroomlist.h"
#include "gtkutils.h"
#include "pidgin/pidginaccountchooser.h"
#include "pidgin/pidginaccountfilterconnected.h"
#include "pidgin/pidginaddbuddydialog.h"
#include "pidgin/pidginaddchatdialog.h"

typedef struct
{
	PurpleAccount *account;
	GtkWidget *window;
	GtkBox *vbox;
	GtkWidget *account_menu;
	GtkSizeGroup *sg;
} PidginBlistRequestData;

typedef struct
{
	PidginBlistRequestData rq_data;
	gchar *default_chat_name;
	GList *entries;
} PidginChatData;

G_DEFINE_TYPE(PidginBuddyList, pidgin_buddy_list, PURPLE_TYPE_BUDDY_LIST)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void gtk_blist_join_chat(PurpleChat *chat)
{
	PurpleAccount *account;
	PurpleConversation *conv;
	PurpleConversationManager *manager;
	PurpleProtocol *protocol;
	GHashTable *components;
	const char *name;
	char *chat_name = NULL;

	account = purple_chat_get_account(chat);
	protocol = purple_account_get_protocol(account);

	components = purple_chat_get_components(chat);

	if(PURPLE_IS_PROTOCOL_CHAT(protocol)) {
		chat_name = purple_protocol_chat_get_name(PURPLE_PROTOCOL_CHAT(protocol),
		                                          components);
	}

	if(chat_name != NULL) {
		name = chat_name;
	} else {
		name = purple_chat_get_name(chat);
	}

	manager = purple_conversation_manager_get_default();
	conv = purple_conversation_manager_find(manager, account, name);

	if(PURPLE_IS_CONVERSATION(conv)) {
		pidgin_conv_attach_to_conversation(conv);
		purple_conversation_present(conv);
	}

	purple_serv_join_chat(purple_account_get_connection(account), components);
	g_free(chat_name);
}

/******************************************************************************
 * Actions
 *****************************************************************************/
static void
do_join_chat(PidginChatData *data)
{
	if (data)
	{
		GHashTable *components =
			g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
		GList *tmp;
		PurpleChat *chat;

		for (tmp = data->entries; tmp != NULL; tmp = tmp->next)
		{
			if (g_object_get_data(tmp->data, "is_spin"))
			{
				g_hash_table_replace(components,
					g_strdup(g_object_get_data(tmp->data, "identifier")),
					g_strdup_printf("%d",
							gtk_spin_button_get_value_as_int(tmp->data)));
			}
			else
			{
				g_hash_table_replace(components,
					g_strdup(g_object_get_data(tmp->data, "identifier")),
					g_strdup(gtk_editable_get_text(GTK_EDITABLE(tmp->data))));
			}
		}

		chat = purple_chat_new(data->rq_data.account, NULL, components);
		gtk_blist_join_chat(chat);
		purple_blist_remove_chat(chat);
	}
}

static void
do_joinchat(GtkWidget *dialog, int id, PidginChatData *info)
{
	switch(id)
	{
		case GTK_RESPONSE_OK:
			do_join_chat(info);
			break;

		case 1:
			pidgin_roomlist_dialog_show_with_account(info->rq_data.account);
			return;

		break;
	}

	gtk_window_destroy(GTK_WINDOW(dialog));
	g_list_free(info->entries);
	g_free(info);
}

/*
 * Check the values of all the text entry boxes.  If any required input
 * strings are empty then don't allow the user to click on "OK."
 */
static void
set_sensitive_if_input_chat_cb(G_GNUC_UNUSED GtkWidget *entry,
                               gpointer user_data)
{
	PurpleProtocol *protocol;
	PurpleConnection *gc;
	PidginChatData *data;
	GList *tmp;
	const char *text;
	gboolean required;
	gboolean sensitive = TRUE;

	data = user_data;

	for (tmp = data->entries; tmp != NULL; tmp = tmp->next)
	{
		if (!g_object_get_data(tmp->data, "is_spin"))
		{
			required = GPOINTER_TO_INT(g_object_get_data(tmp->data, "required"));
			text = gtk_editable_get_text(GTK_EDITABLE(tmp->data));
			if (required && (*text == '\0'))
				sensitive = FALSE;
		}
	}

	gtk_dialog_set_response_sensitive(GTK_DIALOG(data->rq_data.window), GTK_RESPONSE_OK, sensitive);

	gc = purple_account_get_connection(data->rq_data.account);
	protocol = (gc != NULL) ? purple_connection_get_protocol(gc) : NULL;
	sensitive = (protocol != NULL && PURPLE_PROTOCOL_IMPLEMENTS(protocol, ROOMLIST, get_list));

	gtk_dialog_set_response_sensitive(GTK_DIALOG(data->rq_data.window), 1, sensitive);
}

static gboolean
chat_account_filter_func(gpointer item, G_GNUC_UNUSED gpointer data) {
	PurpleConnection *gc = NULL;
	PurpleProtocol *protocol = NULL;

	if(!PURPLE_IS_ACCOUNT(item)) {
		return FALSE;
	}

	gc = purple_account_get_connection(PURPLE_ACCOUNT(item));
	if(gc == NULL) {
		return FALSE;
	}

	protocol = purple_connection_get_protocol(gc);

	return (PURPLE_PROTOCOL_IMPLEMENTS(protocol, CHAT, info));
}

gboolean
pidgin_blist_joinchat_is_showable(void)
{
	GList *c;
	PurpleConnection *gc;

	for (c = purple_connections_get_all(); c != NULL; c = c->next) {
		gc = c->data;

		if(chat_account_filter_func(purple_connection_get_account(gc), NULL)) {
			return TRUE;
		}
	}

	return FALSE;
}

static void
make_blist_request_dialog(PidginBlistRequestData *data, PurpleAccount *account,
                          const char *title, const char *label_text,
                          GCallback callback_func,
                          GtkCustomFilterFunc filter_func,
                          GCallback response_cb)
{
	GtkWidget *label;
	GtkWidget *img;
	GtkWidget *content_area;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkEveryFilter *every = NULL;
	GtkFilter *filter = NULL;

	data->account = account;

	data->window = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(data->window), title);
	gtk_dialog_set_default_response(GTK_DIALOG(data->window), GTK_RESPONSE_OK);
	gtk_window_set_resizable(GTK_WINDOW(data->window), FALSE);
	content_area = gtk_dialog_get_content_area(GTK_DIALOG(data->window));
	gtk_box_set_spacing(GTK_BOX(content_area), 12);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
	gtk_box_append(GTK_BOX(content_area), hbox);

	img = gtk_image_new_from_icon_name("dialog-question");
	gtk_image_set_icon_size(GTK_IMAGE(img), GTK_ICON_SIZE_LARGE);
	gtk_box_append(GTK_BOX(hbox), img);
	gtk_widget_set_halign(img, GTK_ALIGN_START);
	gtk_widget_set_valign(img, GTK_ALIGN_START);

	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	gtk_box_append(GTK_BOX(hbox), vbox);

	label = gtk_label_new(label_text);

	gtk_widget_set_size_request(label, 400, -1);
	gtk_label_set_wrap(GTK_LABEL(label), TRUE);
	gtk_label_set_xalign(GTK_LABEL(label), 0);
	gtk_label_set_yalign(GTK_LABEL(label), 0);
	gtk_box_append(GTK_BOX(vbox), label);

	data->sg = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	data->account_menu = pidgin_account_chooser_new();
	every = gtk_every_filter_new();
	filter = pidgin_account_filter_connected_new();
	gtk_multi_filter_append(GTK_MULTI_FILTER(every), filter);
	filter = GTK_FILTER(gtk_custom_filter_new(filter_func, NULL, NULL));
	gtk_multi_filter_append(GTK_MULTI_FILTER(every), filter);
	pidgin_account_chooser_set_filter(
	        PIDGIN_ACCOUNT_CHOOSER(data->account_menu),
	        GTK_FILTER(every));
	g_object_unref(every);

	if(PURPLE_IS_ACCOUNT(account)) {
		pidgin_account_chooser_set_selected(PIDGIN_ACCOUNT_CHOOSER(
			data->account_menu), account);
	}

	pidgin_add_widget_to_vbox(GTK_BOX(vbox), _("A_ccount"), data->sg, data->account_menu, TRUE, NULL);
	g_signal_connect(data->account_menu, "notify::account",
	                 G_CALLBACK(callback_func), data);

	data->vbox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 5));
	gtk_box_append(GTK_BOX(vbox), GTK_WIDGET(data->vbox));

	g_signal_connect(G_OBJECT(data->window), "response", response_cb, data);

	g_object_unref(data->sg);
}

static void
rebuild_chat_entries(PidginChatData *data, const char *default_chat_name)
{
	PurpleConnection *gc;
	PurpleProtocol *protocol;
	GtkWidget *child = NULL;
	GList *list = NULL, *tmp;
	GHashTable *defaults = NULL;
	PurpleProtocolChatEntry *pce;
	gboolean focus = TRUE;

	g_return_if_fail(data->rq_data.account != NULL);

	gc = purple_account_get_connection(data->rq_data.account);
	protocol = purple_connection_get_protocol(gc);

	child = gtk_widget_get_first_child(GTK_WIDGET(data->rq_data.vbox));
	while (child != NULL) {
		gtk_widget_unparent(child);
		child = gtk_widget_get_first_child(GTK_WIDGET(data->rq_data.vbox));
	}

	g_list_free(data->entries);
	data->entries = NULL;

	if(!PURPLE_IS_PROTOCOL_CHAT(protocol)) {
		return;
	}

	list = purple_protocol_chat_info(PURPLE_PROTOCOL_CHAT(protocol), gc);
	defaults = purple_protocol_chat_info_defaults(PURPLE_PROTOCOL_CHAT(protocol),
	                                              gc, default_chat_name);

	for (tmp = list; tmp; tmp = tmp->next)
	{
		GtkWidget *input;

		pce = tmp->data;

		if (pce->is_int)
		{
			GtkAdjustment *adjust;
			adjust = GTK_ADJUSTMENT(gtk_adjustment_new(pce->min,
			                                           pce->min, pce->max,
			                                           1, 10, 10));
			input = gtk_spin_button_new(adjust, 1, 0);
			gtk_widget_set_size_request(input, 50, -1);
			pidgin_add_widget_to_vbox(GTK_BOX(data->rq_data.vbox), pce->label, data->rq_data.sg, input, FALSE, NULL);
		}
		else
		{
			char *value;
			input = gtk_entry_new();
			gtk_entry_set_activates_default(GTK_ENTRY(input), TRUE);
			value = g_hash_table_lookup(defaults, pce->identifier);
			if(value != NULL) {
				gtk_editable_set_text(GTK_EDITABLE(input), value);
			}
			if (pce->secret)
			{
				gtk_entry_set_visibility(GTK_ENTRY(input), FALSE);
			}
			pidgin_add_widget_to_vbox(data->rq_data.vbox, pce->label, data->rq_data.sg, input, TRUE, NULL);
			g_signal_connect(G_OBJECT(input), "changed",
							 G_CALLBACK(set_sensitive_if_input_chat_cb), data);
		}

		/* Do the following for any type of input widget */
		if (focus)
		{
			gtk_widget_grab_focus(input);
			focus = FALSE;
		}
		g_object_set_data(G_OBJECT(input), "identifier", (gpointer)pce->identifier);
		g_object_set_data(G_OBJECT(input), "is_spin", GINT_TO_POINTER(pce->is_int));
		g_object_set_data(G_OBJECT(input), "required", GINT_TO_POINTER(pce->required));
		data->entries = g_list_append(data->entries, input);

		g_free(pce);
	}

	g_list_free(list);
	g_hash_table_destroy(defaults);

	/* Set whether the "OK" button should be clickable initially */
	set_sensitive_if_input_chat_cb(NULL, data);
}

static void
chat_select_account_cb(GObject *obj, G_GNUC_UNUSED GParamSpec *pspec,
                       gpointer user_data)
{
	PidginChatData *data = user_data;
	PidginAccountChooser *chooser = PIDGIN_ACCOUNT_CHOOSER(obj);
	PurpleAccount *account = pidgin_account_chooser_get_selected(chooser);

	g_return_if_fail(data != NULL);
	if(account == NULL) {
		return;
	}

	if (purple_strequal(purple_account_get_protocol_id(data->rq_data.account),
	                    purple_account_get_protocol_id(account)))
	{
		data->rq_data.account = account;
	}
	else
	{
		data->rq_data.account = account;
		rebuild_chat_entries(data, data->default_chat_name);
	}
}

void
pidgin_blist_joinchat_show(void)
{
	PidginChatData *data = NULL;
	PidginAccountChooser *chooser = NULL;

	data = g_new0(PidginChatData, 1);

	make_blist_request_dialog((PidginBlistRequestData *)data, NULL,
		_("Join a Chat"),
		_("Please enter the appropriate information about the chat "
			"you would like to join.\n"),
		G_CALLBACK(chat_select_account_cb),
		chat_account_filter_func, (GCallback)do_joinchat);
	gtk_dialog_add_buttons(GTK_DIALOG(data->rq_data.window),
		_("Room _List"), 1,
		_("Cancel"), GTK_RESPONSE_CANCEL,
		_("_Join"), GTK_RESPONSE_OK, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(data->rq_data.window),
		GTK_RESPONSE_OK);
	data->default_chat_name = NULL;

	chooser = PIDGIN_ACCOUNT_CHOOSER(data->rq_data.account_menu);
	data->rq_data.account = pidgin_account_chooser_get_selected(chooser);

	rebuild_chat_entries(data, NULL);

	gtk_widget_show(data->rq_data.window);
}

static void
pidgin_blist_request_add_buddy(G_GNUC_UNUSED PurpleBuddyList *list,
                               PurpleAccount *account, const char *username,
                               const char *group, const char *alias)
{
	GtkWidget *dialog = NULL;

	dialog = pidgin_add_buddy_dialog_new(account, username, alias, NULL, group);

	gtk_widget_show(dialog);
}

static void
pidgin_blist_request_add_chat(G_GNUC_UNUSED PurpleBuddyList *list,
                              PurpleAccount *account, PurpleGroup *group,
                              const char *alias, const char *name)
{
	GtkWidget *dialog = NULL;

	dialog = pidgin_add_chat_dialog_new(account, group, alias, name);

	gtk_widget_show(dialog);
}

static void
add_group_cb(G_GNUC_UNUSED PurpleConnection *connection,
             const char *group_name)
{
	PurpleGroup *group;

	if ((group_name == NULL) || (*group_name == '\0'))
		return;

	group = purple_group_new(group_name);
	purple_blist_add_group(group, NULL);
}

static void
pidgin_blist_request_add_group(G_GNUC_UNUSED PurpleBuddyList *list)
{
	purple_request_input(NULL, _("Add Group"), NULL,
					   _("Please enter the name of the group to be added."),
					   NULL, FALSE, FALSE, NULL,
					   _("Add"), G_CALLBACK(add_group_cb),
					   _("Cancel"), NULL,
					   NULL, NULL);
}

/**************************************************************************
 * GTK Buddy list GObject code
 **************************************************************************/
static void
pidgin_buddy_list_init(G_GNUC_UNUSED PidginBuddyList *self)
{
}

static void
pidgin_buddy_list_class_init(PidginBuddyListClass *klass) {
	PurpleBuddyListClass *purple_blist_class = PURPLE_BUDDY_LIST_CLASS(klass);

	purple_blist_class->request_add_buddy = pidgin_blist_request_add_buddy;
	purple_blist_class->request_add_chat = pidgin_blist_request_add_chat;
	purple_blist_class->request_add_group = pidgin_blist_request_add_group;
}
