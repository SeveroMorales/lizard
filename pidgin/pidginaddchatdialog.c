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

#include "pidginaddchatdialog.h"

#include "gtkaccount.h"
#include "gtkroomlist.h"
#include "pidginaccountchooser.h"
#include "pidgincore.h"

struct _PidginAddChatDialog {
	GtkDialog parent;

	GtkCustomFilter *filter;

	const gchar *default_name;

	GtkWidget *account;
	GtkWidget *dynamic_group;
	GtkWidget *alias;
	GtkWidget *group;
	GtkWidget *autojoin;
	GtkWidget *persistent;

	GList *rows;
};

G_DEFINE_TYPE(PidginAddChatDialog, pidgin_add_chat_dialog, GTK_TYPE_DIALOG)

/* ugh, prototypes... */
static void pidgin_add_chat_dialog_input_changed_cb(GtkEditable *editable, gpointer data);

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_add_chat_dialog_validate_input(gpointer data, gpointer user_data) {
	gboolean *valid = user_data;
	gboolean required = FALSE;

	required = GPOINTER_TO_INT(g_object_get_data(data, "required"));
	if(required) {
		const gchar *value = gtk_editable_get_text(GTK_EDITABLE(data));

		if(value == NULL || *value == '\0') {
			*valid = FALSE;
		} else if(g_object_get_data(data, "integer")) {
			gint int_value = atoi(value);
			char *str_value = g_strdup_printf("%d", int_value);

			if(!purple_strequal(value, str_value)) {
				*valid = FALSE;
			}

			g_free(str_value);
		}
	}
}

static void
pidgin_add_chat_dialog_validate(PidginAddChatDialog *dialog) {
	gboolean valid = TRUE;

	/* The callback should only set valid to FALSE if a field is invalid and
	 * not set valid if the field is valid.
	 */
	g_list_foreach(dialog->rows,
	               pidgin_add_chat_dialog_validate_input,
	               &valid);

	gtk_dialog_set_response_sensitive(GTK_DIALOG(dialog), GTK_RESPONSE_OK,
	                                  valid);
}

static void
pidgin_add_chat_dialog_update_components(PidginAddChatDialog *dialog) {
	PurpleAccount *account = NULL;
	PurpleConnection *connection = NULL;
	PurpleProtocol *protocol = NULL;
	GList *info = NULL;
	GHashTable *defaults = NULL;
	gboolean focus_set = FALSE;

	/* Clean up the dynamic group and our list of rows. */
	while(dialog->rows != NULL) {
		adw_preferences_group_remove(ADW_PREFERENCES_GROUP(dialog->dynamic_group),
		                             dialog->rows->data);
		dialog->rows = g_list_delete_link(dialog->rows, dialog->rows);
	}

	account = pidgin_account_chooser_get_selected(PIDGIN_ACCOUNT_CHOOSER(dialog->account));
	if(!PURPLE_IS_ACCOUNT(account)) {
		return;
	}

	connection = purple_account_get_connection(account);
	protocol = purple_account_get_protocol(account);
	info = purple_protocol_chat_info(PURPLE_PROTOCOL_CHAT(protocol),
	                                 connection);
	defaults = purple_protocol_chat_info_defaults(PURPLE_PROTOCOL_CHAT(protocol),
	                                              connection,
	                                              dialog->default_name);

	while(info != NULL) {
		GtkWidget *row = NULL;
		PurpleProtocolChatEntry *pce = info->data;
		char *value = NULL;

		if(pce->is_int) {
			row = adw_entry_row_new();
			adw_entry_row_set_input_purpose(ADW_ENTRY_ROW(row),
			                                GTK_INPUT_PURPOSE_NUMBER);
		} else if(pce->secret) {
			row = adw_password_entry_row_new();
		} else {
			row = adw_entry_row_new();
		}

		value = g_hash_table_lookup(defaults, pce->identifier);
		if(value != NULL) {
			gtk_editable_set_text(GTK_EDITABLE(row), value);
		}

		g_signal_connect(row, "changed",
		                 G_CALLBACK(pidgin_add_chat_dialog_input_changed_cb),
		                 dialog);

		adw_preferences_row_set_title(ADW_PREFERENCES_ROW(row), pce->label);
		adw_preferences_row_set_use_underline(ADW_PREFERENCES_ROW(row), TRUE);

		adw_preferences_group_add(ADW_PREFERENCES_GROUP(dialog->dynamic_group),
		                          row);

		if(!focus_set) {
			gtk_widget_grab_focus(row);
			focus_set = TRUE;
		}

		g_object_set_data(G_OBJECT(row), "identifier",
		                  (gpointer)pce->identifier);
		g_object_set_data(G_OBJECT(row), "integer",
		                  GINT_TO_POINTER(pce->is_int));
		g_object_set_data(G_OBJECT(row), "required",
		                  GINT_TO_POINTER(pce->required));

		dialog->rows = g_list_append(dialog->rows, row);

		g_free(pce);

		info = g_list_delete_link(info, info);
	}

	g_hash_table_destroy(defaults);

	pidgin_add_chat_dialog_validate(dialog);
}

static gboolean
pidgin_add_chat_dialog_filter_accounts(gpointer item,
                                       G_GNUC_UNUSED gpointer data)
{
	gboolean ret = FALSE;

	if(PURPLE_IS_ACCOUNT(item)) {
		PurpleAccount *account = PURPLE_ACCOUNT(item);
		PurpleProtocol *protocol = purple_account_get_protocol(account);

		if(PURPLE_IS_PROTOCOL(protocol)) {
			ret = PURPLE_PROTOCOL_IMPLEMENTS(protocol, CHAT, info);
		}
	}

	return ret;
}

static void
pidgin_add_chat_dialog_add_input_to_components(gpointer data,
                                               gpointer user_data)
{
	GHashTable *components = user_data;
	gchar *identifier = NULL;
	gchar *value = NULL;

	identifier = g_strdup(g_object_get_data(data, "identifier"));

	if(g_object_get_data(data, "integer")) {
		const char *str_value = gtk_editable_get_text(GTK_EDITABLE(data));
		gint int_value = atoi(str_value);

		value = g_strdup_printf("%d", int_value);
	} else {
		const gchar *str_value = gtk_editable_get_text(GTK_EDITABLE(data));

		if(*str_value != '\0') {
			value = g_strdup(str_value);
		}
	}

	/* If the value was changed to an empty string, we should remove it from the
	 * components.
	 */
	if(value == NULL) {
		g_hash_table_remove(components, identifier);
		g_free(identifier);
	} else {
		g_hash_table_replace(components, identifier, value);
	}
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_add_chat_dialog_input_changed_cb(G_GNUC_UNUSED GtkEditable *editable,
                                        gpointer data)
{
	pidgin_add_chat_dialog_validate(data);
}

static void
pidgin_add_chat_dialog_response_cb(GtkDialog *dialog, gint response_id,
                                   G_GNUC_UNUSED gpointer data)
{
	PurpleAccount *account = NULL;
	PidginAddChatDialog *acdialog = PIDGIN_ADD_CHAT_DIALOG(dialog);
	gboolean close = TRUE;

	account = pidgin_account_chooser_get_selected(PIDGIN_ACCOUNT_CHOOSER(acdialog->account));

	if(response_id == 1) {
		pidgin_roomlist_dialog_show_with_account(account);

		close = FALSE;
	} else if(response_id == GTK_RESPONSE_OK) {
		PurpleChat *chat = NULL;
		GHashTable *components = NULL;
		const gchar *alias = NULL;

		components = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
		                                   g_free);
		g_list_foreach(acdialog->rows,
		               pidgin_add_chat_dialog_add_input_to_components,
		               components);

		alias = gtk_editable_get_text(GTK_EDITABLE(acdialog->alias));

		chat = purple_chat_new(account, alias, components);

		if(PURPLE_IS_CHAT(chat)) {
			PurpleGroup *group = NULL;
			gchar *group_name = NULL;

			group_name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(acdialog->group));
			if(group_name != NULL && *group_name != '\0') {
				group = purple_blist_find_group(group_name);
			}

			if(!PURPLE_IS_GROUP(group)) {
				group = purple_group_new(group_name);
				purple_blist_add_group(group, NULL);
			}

			purple_blist_add_chat(chat, group, NULL);

			if(gtk_switch_get_active(GTK_SWITCH(acdialog->autojoin))) {
				purple_blist_node_set_bool(PURPLE_BLIST_NODE(chat),
				                           "gtk-autojoin", TRUE);
			}
			if(gtk_switch_get_active(GTK_SWITCH(acdialog->persistent))) {
				purple_blist_node_set_bool(PURPLE_BLIST_NODE(chat),
				                           "gtk-persistent", TRUE);
			}

			g_free(group_name);
		} else {
			g_warning("failed to create chat");
		}
	}

	if(close) {
		gtk_window_destroy(GTK_WINDOW(dialog));
	}
}

static void
pidgin_add_chat_dialog_account_changed_cb(GObject *obj,
                                          G_GNUC_UNUSED GParamSpec *pspec,
                                          gpointer data)
{
	PidginAddChatDialog *dialog = data;
	PurpleAccount *account = NULL;

	account = pidgin_account_chooser_get_selected(PIDGIN_ACCOUNT_CHOOSER(obj));

	if(PURPLE_IS_ACCOUNT(account)) {
		PurpleProtocol *protocol = purple_account_get_protocol(account);

		if(PURPLE_IS_PROTOCOL(protocol)) {
			gboolean roomlist = FALSE;

			roomlist = PURPLE_PROTOCOL_IMPLEMENTS(protocol, ROOMLIST, get_list);

			gtk_dialog_set_response_sensitive(GTK_DIALOG(dialog), 1, roomlist);
		}
	}

	pidgin_add_chat_dialog_update_components(dialog);

	pidgin_add_chat_dialog_validate(dialog);
}

static void
pidgin_add_chat_dialog_username_changed_cb(G_GNUC_UNUSED GtkEditable *editable,
                                           gpointer data)
{
	pidgin_add_chat_dialog_validate(data);
}

static void
pidgin_add_chat_dialog_group_cb(PurpleBlistNode *node, gpointer data) {
	PidginAddChatDialog *dialog = data;
	PurpleGroup *group = PURPLE_GROUP(node);

	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(dialog->group),
	                               purple_group_get_name(group));
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_add_chat_dialog_finalize(GObject *obj) {
	PidginAddChatDialog *dialog = PIDGIN_ADD_CHAT_DIALOG(obj);

	g_list_free(dialog->rows);

	G_OBJECT_CLASS(pidgin_add_chat_dialog_parent_class)->finalize(obj);
}

static void
pidgin_add_chat_dialog_init(PidginAddChatDialog *dialog) {
	gtk_widget_init_template(GTK_WIDGET(dialog));

	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

	gtk_custom_filter_set_filter_func(dialog->filter,
	                                  pidgin_add_chat_dialog_filter_accounts,
	                                  NULL, NULL);

	purple_blist_walk(pidgin_add_chat_dialog_group_cb, NULL, NULL, NULL,
	                  dialog);
}

static void
pidgin_add_chat_dialog_class_init(PidginAddChatDialogClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->finalize = pidgin_add_chat_dialog_finalize;

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/im/pidgin/Pidgin3/Dialogs/addchat.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginAddChatDialog,
	                                     filter);

	gtk_widget_class_bind_template_child(widget_class, PidginAddChatDialog,
	                                     account);
	gtk_widget_class_bind_template_child(widget_class, PidginAddChatDialog,
	                                     dynamic_group);
	gtk_widget_class_bind_template_child(widget_class, PidginAddChatDialog,
	                                     alias);
	gtk_widget_class_bind_template_child(widget_class, PidginAddChatDialog,
	                                     group);
	gtk_widget_class_bind_template_child(widget_class, PidginAddChatDialog,
	                                     autojoin);
	gtk_widget_class_bind_template_child(widget_class, PidginAddChatDialog,
	                                     persistent);

	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_add_chat_dialog_response_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_add_chat_dialog_account_changed_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_add_chat_dialog_username_changed_cb);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_add_chat_dialog_new(PurpleAccount *account, PurpleGroup *group,
                           const gchar *alias, const gchar *name)
{
	GtkWidget *dialog = g_object_new(PIDGIN_TYPE_ADD_CHAT_DIALOG, NULL);
	PidginAddChatDialog *acdialog = PIDGIN_ADD_CHAT_DIALOG(dialog);

	if(PURPLE_IS_ACCOUNT(account)) {
		pidgin_account_chooser_set_selected(PIDGIN_ACCOUNT_CHOOSER(acdialog->account),
		                                    account);
	}

	if(alias != NULL) {
		gtk_editable_set_text(GTK_EDITABLE(acdialog->alias), alias);
	}

	if(PURPLE_IS_GROUP(group)) {
		GtkWidget *entry = NULL;

		entry = gtk_combo_box_get_child(GTK_COMBO_BOX(acdialog->group));
		gtk_editable_set_text(GTK_EDITABLE(entry),
		                      purple_group_get_name(group));
	}

	acdialog->default_name = name;

	pidgin_add_chat_dialog_update_components(acdialog);

	return dialog;
}
