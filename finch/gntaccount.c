/*
 * finch
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include <glib/gi18n-lib.h>

#include <purple.h>

#include <gnt.h>

#include "gntaccount.h"
#include "gntblist.h"
#include "libfinch.h"

#include <string.h>

typedef struct
{
	GntWidget *window;
	GntWidget *tree;
} FinchAccountList;

static FinchAccountList accounts;

typedef struct
{
	PurpleAccount *account;          /* NULL for a new account */

	GntWidget *window;

	GntWidget *protocol;
	GntWidget *username;
	GntWidget *require_password;
	GntWidget *alias;

	GntWidget *splits;
	GList *split_entries;

	GList *protocol_entries;
	GntWidget *protocols;

	GntWidget *remember;
} AccountEditDialog;

/* This is necessary to close an edit-dialog when an account is deleted */
static GList *accountdialogs;

static void
account_add(PurpleAccount *account) {
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
	GntTreeRow *row = NULL;

	row = gnt_tree_create_row(GNT_TREE(accounts.tree),
	                          purple_contact_info_get_username(info),
	                          purple_account_get_protocol_name(account));

	gnt_tree_add_choice(GNT_TREE(accounts.tree), account, row, NULL, NULL);
	gnt_tree_set_choice(GNT_TREE(accounts.tree), account,
	                    purple_account_get_enabled(account));
}

static void
edit_dialog_destroy(AccountEditDialog *dialog)
{
	accountdialogs = g_list_remove(accountdialogs, dialog);
	g_list_free(dialog->protocol_entries);
	g_list_free(dialog->split_entries);
	g_free(dialog);
}

static void
save_account_cb(AccountEditDialog *dialog)
{
	PurpleAccount *account;
	PurpleContactInfo *info = NULL;
	PurpleProtocol *protocol;
	const char *value;
	GString *username;

	/* XXX: Do some error checking first. */

	protocol = gnt_combo_box_get_selected_data(GNT_COMBO_BOX(dialog->protocol));

	/* Username && user-splits */
	value = gnt_entry_get_text(GNT_ENTRY(dialog->username));

	if (value == NULL || *value == '\0')
	{
		purple_notify_error(NULL, _("Error"),
			dialog->account ? _("Account was not modified") :
				_("Account was not added"),
			_("Username of an account must be non-empty."),
			purple_request_cpar_from_account(dialog->account));
		return;
	}

	username = g_string_new(value);

	if (protocol != NULL)
	{
		GList *iter, *entries, *splits;
		splits = purple_protocol_get_user_splits(protocol);
		for (iter = splits, entries = dialog->split_entries;
				iter && entries; iter = iter->next, entries = entries->next)
		{
			PurpleAccountUserSplit *split = iter->data;
			GntWidget *entry = entries->data;

			value = entry ? gnt_entry_get_text(GNT_ENTRY(entry)) : NULL;
			if (value == NULL || *value == '\0')
				value = purple_account_user_split_get_default_value(split);
			g_string_append_printf(username, "%c%s",
					purple_account_user_split_get_separator(split),
					value);
		}
		g_list_free_full(splits,
		                 (GDestroyNotify)purple_account_user_split_destroy);
	}

	if(dialog->account == NULL) {
		PurpleAccountManager *manager = purple_account_manager_get_default();

		account = purple_account_new(username->str, purple_protocol_get_id(protocol));
		info = PURPLE_CONTACT_INFO(account);
		purple_account_manager_add(manager, account);
	} else {
		account = dialog->account;
		info = PURPLE_CONTACT_INFO(account);

		/* Protocol */
		if (purple_account_is_disconnected(account)) {
			purple_account_set_protocol_id(account,
			                               purple_protocol_get_id(protocol));
			purple_contact_info_set_username(info, username->str);
		} else {
			const char *old = purple_account_get_protocol_id(account);
			char *oldproto;
			if (!purple_strequal(old, purple_protocol_get_id(protocol))) {
				purple_notify_error(NULL, _("Error"),
					_("Account was not modified"),
					_("The account's protocol cannot be "
					"changed while it is connected to the "
					"server."),
					purple_request_cpar_from_account(
						account));
				g_string_free(username, TRUE);
				return;
			}

			oldproto = g_strdup(purple_normalize(account, purple_contact_info_get_username(info)));
			if (g_utf8_collate(oldproto, purple_normalize(account, username->str))) {
				purple_notify_error(NULL, _("Error"),
					_("Account was not modified"),
					_("The account's username cannot be "
					"changed while it is connected to the "
					"server."),
					purple_request_cpar_from_account(
						account));
				g_free(oldproto);
				g_string_free(username, TRUE);
				return;
			}
			g_free(oldproto);
			purple_contact_info_set_username(info, username->str);
		}
	}
	g_string_free(username, TRUE);

	/* Alias */
	value = gnt_entry_get_text(GNT_ENTRY(dialog->alias));
	purple_contact_info_set_alias(info, value);

	/* Remember password and require password */
	purple_account_set_remember_password(account,
			gnt_check_box_get_checked(GNT_CHECK_BOX(dialog->remember)));
	purple_account_set_require_password(account,
			gnt_check_box_get_checked(GNT_CHECK_BOX(dialog->require_password)));

	/* Protocol options */
	if (protocol)
	{
		GList *iter, *entries, *opts;

		opts = purple_protocol_get_account_options(protocol);
		for (iter = opts, entries = dialog->protocol_entries;
				iter && entries; iter = iter->next, entries = entries->next)
		{
			PurpleAccountOption *option = iter->data;
			GntWidget *entry = entries->data;
			PurplePrefType type = purple_account_option_get_pref_type(option);
			const char *setting = purple_account_option_get_setting(option);

			if (type == PURPLE_PREF_STRING)
			{
				const char *value = gnt_entry_get_text(GNT_ENTRY(entry));
				purple_account_set_string(account, setting, value);
			}
			else if (type == PURPLE_PREF_INT)
			{
				const char *str = gnt_entry_get_text(GNT_ENTRY(entry));
				int value = 0;
				if (str)
					value = atoi(str);
				purple_account_set_int(account, setting, value);
			}
			else if (type == PURPLE_PREF_BOOLEAN)
			{
				gboolean value = gnt_check_box_get_checked(GNT_CHECK_BOX(entry));
				purple_account_set_bool(account, setting, value);
			}
			else if (type == PURPLE_PREF_STRING_LIST)
			{
				gchar *value = gnt_combo_box_get_selected_data(GNT_COMBO_BOX(entry));
				purple_account_set_string(account, setting, value);
			}
			else
			{
				g_assert_not_reached();
			}
		}
		g_list_free_full(opts, (GDestroyNotify)purple_account_option_destroy);
	}

	/* XXX: Proxy options */

	if (accounts.window && accounts.tree) {
		gnt_tree_set_selected(GNT_TREE(accounts.tree), account);
		gnt_box_give_focus_to_child(GNT_BOX(accounts.window), accounts.tree);
	}

	if (dialog->account == NULL) {
		/* This is a new account. Set it to the current status. */
		/* Xerox from gtkaccount.c :D */
		const PurpleSavedStatus *saved_status;
		saved_status = purple_savedstatus_get_current();
		if (saved_status != NULL) {
			purple_savedstatus_activate_for_account(saved_status, account);
			purple_account_set_enabled(account, TRUE);
		}
	}

	/* In case of a new account, the 'Accounts' window is updated from the
	 * 'added' callback. In case of changes in an existing account, we need to
	 * explicitly do it here.
	 */
	if (dialog->account != NULL && accounts.window) {
		gnt_tree_change_text(GNT_TREE(accounts.tree), dialog->account,
				0, purple_contact_info_get_username(info));
		gnt_tree_change_text(GNT_TREE(accounts.tree), dialog->account,
				1, purple_account_get_protocol_name(dialog->account));
	}

	gnt_widget_destroy(dialog->window);
}

static void
update_user_splits(AccountEditDialog *dialog)
{
	GntWidget *hbox;
	PurpleProtocol *protocol;
	GList *iter, *entries, *splits;
	char *username = NULL;

	if (dialog->splits)
	{
		gnt_box_remove_all(GNT_BOX(dialog->splits));
		g_list_free(dialog->split_entries);
	}
	else
	{
		dialog->splits = gnt_vbox_new(FALSE);
		gnt_box_set_pad(GNT_BOX(dialog->splits), 0);
		gnt_box_set_fill(GNT_BOX(dialog->splits), TRUE);
	}

	dialog->split_entries = NULL;

	protocol = gnt_combo_box_get_selected_data(GNT_COMBO_BOX(dialog->protocol));
	if (!protocol)
		return;

	if(PURPLE_IS_ACCOUNT(dialog->account)) {
		PurpleContactInfo *info = PURPLE_CONTACT_INFO(dialog->account);

		username = g_strdup(purple_contact_info_get_username(info));
	}

	splits = purple_protocol_get_user_splits(protocol);
	for (iter = splits; iter; iter = iter->next)
	{
		PurpleAccountUserSplit *split = iter->data;
		GntWidget *entry = NULL;
		char *buf = NULL;

		if (!purple_account_user_split_is_constant(split)) {
			hbox = gnt_hbox_new(TRUE);
			gnt_box_add_widget(GNT_BOX(dialog->splits), hbox);

			buf = g_strdup_printf("%s:", purple_account_user_split_get_text(split));
			gnt_box_add_widget(GNT_BOX(hbox), gnt_label_new(buf));

			entry = gnt_entry_new(NULL);
			gnt_box_add_widget(GNT_BOX(hbox), entry);
		}

		dialog->split_entries = g_list_append(dialog->split_entries, entry);
		g_free(buf);
	}

	for (iter = g_list_last(splits), entries = g_list_last(dialog->split_entries);
			iter && entries; iter = iter->prev, entries = entries->prev)
	{
		GntWidget *entry = entries->data;
		PurpleAccountUserSplit *split = iter->data;
		const char *value = NULL;
		char *s;

		if (dialog->account)
		{
			if(purple_account_user_split_get_reverse(split))
				s = strrchr(username, purple_account_user_split_get_separator(split));
			else
				s = strchr(username, purple_account_user_split_get_separator(split));

			if (s != NULL)
			{
				*s = '\0';
				s++;
				value = s;
			}
		}
		if (value == NULL)
			value = purple_account_user_split_get_default_value(split);

		if (value != NULL && entry != NULL)
			gnt_entry_set_text(GNT_ENTRY(entry), value);
	}

	g_list_free_full(splits, (GDestroyNotify)purple_account_user_split_destroy);

	if (username != NULL)
		gnt_entry_set_text(GNT_ENTRY(dialog->username), username);

	g_free(username);
}

static void
add_account_options(AccountEditDialog *dialog)
{
	PurpleProtocol *protocol;
	GList *iter, *opts;
	GntWidget *vbox, *box;
	PurpleAccount *account;

	if (dialog->protocols)
		gnt_box_remove_all(GNT_BOX(dialog->protocols));
	else
	{
		dialog->protocols = vbox = gnt_vbox_new(FALSE);
		gnt_box_set_pad(GNT_BOX(vbox), 0);
		gnt_box_set_alignment(GNT_BOX(vbox), GNT_ALIGN_LEFT);
		gnt_box_set_fill(GNT_BOX(vbox), TRUE);
	}

	if (dialog->protocol_entries)
	{
		g_list_free(dialog->protocol_entries);
		dialog->protocol_entries = NULL;
	}

	vbox = dialog->protocols;

	protocol = gnt_combo_box_get_selected_data(GNT_COMBO_BOX(dialog->protocol));
	if (!protocol)
		return;

	account = dialog->account;

	opts = purple_protocol_get_account_options(protocol);
	for (iter = opts; iter; iter = iter->next)
	{
		PurpleAccountOption *option = iter->data;
		PurplePrefType type = purple_account_option_get_pref_type(option);

		box = gnt_hbox_new(TRUE);
		gnt_box_set_pad(GNT_BOX(box), 0);
		gnt_box_add_widget(GNT_BOX(vbox), box);

		if (type == PURPLE_PREF_BOOLEAN)
		{
			GntWidget *widget = gnt_check_box_new(purple_account_option_get_text(option));
			gnt_box_add_widget(GNT_BOX(box), widget);
			dialog->protocol_entries = g_list_append(dialog->protocol_entries, widget);

			if (account)
				gnt_check_box_set_checked(GNT_CHECK_BOX(widget),
						purple_account_get_bool(account,
							purple_account_option_get_setting(option),
							purple_account_option_get_default_bool(option)));
			else
				gnt_check_box_set_checked(GNT_CHECK_BOX(widget),
						purple_account_option_get_default_bool(option));
		}
		else
		{
			gnt_box_add_widget(GNT_BOX(box),
					gnt_label_new(purple_account_option_get_text(option)));

			if (type == PURPLE_PREF_STRING_LIST)
			{
				GntWidget *combo = gnt_combo_box_new();
				GList *opt_iter = purple_account_option_get_list(option);
				const char *dv = purple_account_option_get_default_list_value(option);
				const char *active = dv;

				if (account)
					active = purple_account_get_string(account,
						purple_account_option_get_setting(option), dv);

				gnt_box_add_widget(GNT_BOX(box), combo);
				dialog->protocol_entries = g_list_append(dialog->protocol_entries, combo);

				for ( ; opt_iter; opt_iter = opt_iter->next)
				{
					PurpleKeyValuePair *kvp = opt_iter->data;
					gnt_combo_box_add_data(GNT_COMBO_BOX(combo), kvp->value, kvp->key);

					if (purple_strequal(kvp->value, active))
						gnt_combo_box_set_selected(GNT_COMBO_BOX(combo), kvp->value);
				}
			}
			else
			{
				GntWidget *entry = gnt_entry_new(NULL);
				gnt_box_add_widget(GNT_BOX(box), entry);
				dialog->protocol_entries = g_list_append(dialog->protocol_entries, entry);

				if (type == PURPLE_PREF_STRING)
				{
					const char *dv = purple_account_option_get_default_string(option);

					if (account)
						gnt_entry_set_text(GNT_ENTRY(entry),
								purple_account_get_string(account,
									purple_account_option_get_setting(option), dv));
					else
						gnt_entry_set_text(GNT_ENTRY(entry), dv);
				}
				else if (type == PURPLE_PREF_INT)
				{
					char str[32];
					int value = purple_account_option_get_default_int(option);
					if (account)
						value = purple_account_get_int(account,
								purple_account_option_get_setting(option), value);
					g_snprintf(str, sizeof(str), "%d", value);
					gnt_entry_set_flag(GNT_ENTRY(entry), GNT_ENTRY_FLAG_INT);
					gnt_entry_set_text(GNT_ENTRY(entry), str);
				}
				else
				{
					g_assert_not_reached();
				}
			}
		}
	}
	g_list_free_full(opts, (GDestroyNotify)purple_account_option_destroy);
}

static void
update_user_options(AccountEditDialog *dialog)
{
	PurpleProtocol *protocol = NULL;
	PurpleProtocolOptions options;

	protocol = gnt_combo_box_get_selected_data(GNT_COMBO_BOX(dialog->protocol));
	if(!protocol) {
		return;
	}

	options = purple_protocol_get_options(protocol);

	if(dialog->remember == NULL) {
		dialog->remember = gnt_check_box_new(_("Remember password"));
	}

	if(dialog->require_password == NULL) {
		dialog->require_password = gnt_check_box_new(_("Require a password "
		                                               "for this account"));
	}

	gnt_widget_set_visible(dialog->require_password,
	                       options & OPT_PROTO_PASSWORD_OPTIONAL);

	if (dialog->account) {
		gboolean remember_password = FALSE;
		gboolean require_password = FALSE;

		remember_password =
			purple_account_get_remember_password(dialog->account);
		gnt_check_box_set_checked(GNT_CHECK_BOX(dialog->remember),
		                          remember_password);

		require_password = purple_account_get_require_password(dialog->account);
		gnt_check_box_set_checked(GNT_CHECK_BOX(dialog->require_password),
		                          require_password);
	}

}

static void
protocol_changed_cb(G_GNUC_UNUSED GntWidget *combo,
                    G_GNUC_UNUSED PurpleProtocol *old,
                    G_GNUC_UNUSED PurpleProtocol *new,
                    AccountEditDialog *dialog)
{
	update_user_splits(dialog);
	add_account_options(dialog);
	update_user_options(dialog);  /* This may not be necessary here */
	gnt_box_readjust(GNT_BOX(dialog->window));
	gnt_widget_draw(dialog->window);
}

static void
edit_account(PurpleAccount *account)
{
	GntWidget *window, *hbox;
	GntWidget *combo, *button, *entry;
	GList *list, *iter;
	AccountEditDialog *dialog;
	PurpleProtocol *protocol;
	PurpleProtocolManager *protocol_manager = NULL;

	if (account)
	{
		GList *iter;
		for (iter = accountdialogs; iter; iter = iter->next)
		{
			AccountEditDialog *dlg = iter->data;
			if (dlg->account == account)
				return;
		}
	}

	protocol_manager = purple_protocol_manager_get_default();
	list = purple_protocol_manager_get_all(protocol_manager);
	if (list == NULL) {
		purple_notify_error(NULL, _("Error"),
			_("There are no protocols installed."),
			_("(You probably forgot to 'make install'.)"),
			purple_request_cpar_from_account(account));
		return;
	}

	dialog = g_new0(AccountEditDialog, 1);
	accountdialogs = g_list_prepend(accountdialogs, dialog);

	dialog->window = window = gnt_vbox_new(FALSE);
	dialog->account = account;
	gnt_box_set_toplevel(GNT_BOX(window), TRUE);
	gnt_box_set_title(GNT_BOX(window), account ? _("Modify Account") : _("New Account"));
	gnt_box_set_alignment(GNT_BOX(window), GNT_ALIGN_MID);
	gnt_box_set_pad(GNT_BOX(window), 0);
	gnt_widget_set_name(window, "edit-account");
	gnt_box_set_fill(GNT_BOX(window), TRUE);

	hbox = gnt_hbox_new(TRUE);
	gnt_box_set_pad(GNT_BOX(hbox), 0);
	gnt_box_add_widget(GNT_BOX(window), hbox);

	dialog->protocol = combo = gnt_combo_box_new();
	for (iter = list; iter; iter = iter->next)
	{
		gnt_combo_box_add_data(GNT_COMBO_BOX(combo), iter->data,
				purple_protocol_get_name(PURPLE_PROTOCOL(iter->data)));
	}

	protocol = purple_account_get_protocol(account);

	if (account && protocol)
		gnt_combo_box_set_selected(GNT_COMBO_BOX(combo), protocol);
	else
		gnt_combo_box_set_selected(GNT_COMBO_BOX(combo), list->data);

	g_signal_connect(G_OBJECT(combo), "selection-changed", G_CALLBACK(protocol_changed_cb), dialog);

	gnt_box_add_widget(GNT_BOX(hbox), gnt_label_new(_("Protocol:")));
	gnt_box_add_widget(GNT_BOX(hbox), combo);

	hbox = gnt_hbox_new(TRUE);
	gnt_box_set_pad(GNT_BOX(hbox), 0);
	gnt_box_add_widget(GNT_BOX(window), hbox);

	dialog->username = entry = gnt_entry_new(NULL);
	gnt_box_add_widget(GNT_BOX(hbox), gnt_label_new(_("Username:")));
	gnt_box_add_widget(GNT_BOX(hbox), entry);

	/* User splits */
	update_user_splits(dialog);
	gnt_box_add_widget(GNT_BOX(window), dialog->splits);

	hbox = gnt_hbox_new(TRUE);
	gnt_box_set_pad(GNT_BOX(hbox), 0);
	gnt_box_add_widget(GNT_BOX(window), hbox);

	dialog->alias = entry = gnt_entry_new(NULL);
	gnt_box_add_widget(GNT_BOX(hbox), gnt_label_new(_("Alias:")));
	gnt_box_add_widget(GNT_BOX(hbox), entry);
	if(PURPLE_IS_ACCOUNT(account)) {
		PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
		const char *alias = purple_contact_info_get_alias(info);

		gnt_entry_set_text(GNT_ENTRY(entry), alias);
	}

	/* User options */
	update_user_options(dialog);
	gnt_box_add_widget(GNT_BOX(window), dialog->remember);
	gnt_box_add_widget(GNT_BOX(window), dialog->require_password);

	gnt_box_add_widget(GNT_BOX(window), gnt_line_new(FALSE));

	/* The advanced box */
	add_account_options(dialog);
	gnt_box_add_widget(GNT_BOX(window), dialog->protocols);

	/* TODO: Add proxy options */

	/* The button box */
	hbox = gnt_hbox_new(FALSE);
	gnt_box_add_widget(GNT_BOX(window), hbox);
	gnt_box_set_alignment(GNT_BOX(hbox), GNT_ALIGN_MID);

	button = gnt_button_new(_("Cancel"));
	gnt_box_add_widget(GNT_BOX(hbox), button);
	g_signal_connect_swapped(G_OBJECT(button), "activate", G_CALLBACK(gnt_widget_destroy), window);

	button = gnt_button_new(_("Save"));
	gnt_box_add_widget(GNT_BOX(hbox), button);
	g_signal_connect_swapped(G_OBJECT(button), "activate", G_CALLBACK(save_account_cb), dialog);

	g_signal_connect_swapped(G_OBJECT(window), "destroy", G_CALLBACK(edit_dialog_destroy), dialog);

	gnt_widget_show(window);
	gnt_box_readjust(GNT_BOX(window));
	gnt_widget_draw(window);

	g_list_free(list);
}

static void
add_account_cb(G_GNUC_UNUSED GntWidget *widget, G_GNUC_UNUSED gpointer data)
{
	edit_account(NULL);
}

static void
modify_account_cb(G_GNUC_UNUSED GntWidget *widget, GntTree *tree)
{
	PurpleAccount *account = gnt_tree_get_selection_data(tree);
	if (!account)
		return;
	edit_account(account);
}

static void
really_delete_account(PurpleAccount *account)
{
	PurpleNotificationManager *manager = NULL;
	GList *iter;
	for (iter = accountdialogs; iter; iter = iter->next)
	{
		AccountEditDialog *dlg = iter->data;
		if (dlg->account == account)
		{
			gnt_widget_destroy(dlg->window);
			break;
		}
	}

	manager = purple_notification_manager_get_default();
	purple_notification_manager_remove_with_account(manager, account, TRUE);

	purple_accounts_delete(account);
}

static void
delete_account_cb(G_GNUC_UNUSED GntWidget *widget, GntTree *tree)
{
	PurpleAccount *account = NULL;
	PurpleContactInfo *info = NULL;
	char *prompt = NULL;

	account = gnt_tree_get_selection_data(tree);
	if(!PURPLE_IS_ACCOUNT(account)) {
		return;
	}

	info = PURPLE_CONTACT_INFO(account);

	prompt = g_strdup_printf(_("Are you sure you want to delete %s?"),
	                         purple_contact_info_get_username(info));

	purple_request_action(account, _("Delete Account"), prompt, NULL,
		PURPLE_DEFAULT_ACTION_NONE,
		purple_request_cpar_from_account(account), account, 2,
		_("Delete"), really_delete_account, _("Cancel"), NULL);
	g_free(prompt);
}

static void
account_toggled(GntWidget *widget, void *key, G_GNUC_UNUSED gpointer data)
{
	PurpleAccount *account = key;
	gboolean enabled = gnt_tree_get_choice(GNT_TREE(widget), key);

	if (enabled)
		purple_savedstatus_activate_for_account(purple_savedstatus_get_current(),
												account);

	purple_account_set_enabled(account, enabled);
}

static gboolean
account_list_key_pressed_cb(GntWidget *widget, const char *text,
                            G_GNUC_UNUSED gpointer data)
{
	GntTree *tree = GNT_TREE(widget);
	PurpleAccountManager *manager = NULL;
	GListModel *manager_model = NULL;
	PurpleAccount *account = gnt_tree_get_selection_data(tree);
	int move, pos, count;

	if (!account)
		return FALSE;

	switch (text[0]) {
		case '-':
			move = -1;
			break;
		case '=':
			move = 2;  /* XXX: This seems to be a bug in libpurple */
			break;
		default:
			return FALSE;
	}

	manager = purple_account_manager_get_default();
	manager_model = G_LIST_MODEL(manager);

	count = g_list_model_get_n_items(manager_model);
	for(pos = 0; pos < count; pos++) {
		PurpleAccount *acct = g_list_model_get_item(manager_model, pos);
		if(account == acct) {
			pos = (move + pos + count + 1) % (count + 1);
			purple_account_manager_reorder(manager, account, pos);
			g_object_unref(acct);
			break;
		}
		g_object_unref(acct);
	}

	/* I don't like this, but recreating the entire list seems to be
	 * the easiest way of doing it */
	gnt_tree_remove_all(tree);
	for(pos = 0; pos < count; pos++) {
		PurpleAccount *account = g_list_model_get_item(manager_model, pos);
		account_add(account);
		g_object_unref(account);
	}
	gnt_tree_set_selected(tree, account);

	return TRUE;
}

static void
reset_accounts_win(G_GNUC_UNUSED GntWidget *widget,
                   G_GNUC_UNUSED gpointer data)
{
	accounts.window = NULL;
	accounts.tree = NULL;
}

void
finch_accounts_show_all(void)
{
	GListModel *manager_model = NULL;
	guint n_items = 0;
	GntWidget *box, *button;

	if (accounts.window) {
		gnt_window_present(accounts.window);
		return;
	}

	accounts.window = gnt_vbox_new(FALSE);
	gnt_box_set_toplevel(GNT_BOX(accounts.window), TRUE);
	gnt_box_set_title(GNT_BOX(accounts.window), _("Accounts"));
	gnt_box_set_pad(GNT_BOX(accounts.window), 0);
	gnt_box_set_alignment(GNT_BOX(accounts.window), GNT_ALIGN_MID);
	gnt_widget_set_name(accounts.window, "accounts");

	gnt_box_add_widget(GNT_BOX(accounts.window),
			gnt_label_new(_("You can enable/disable accounts from the following list.")));

	gnt_box_add_widget(GNT_BOX(accounts.window), gnt_line_new(FALSE));

	accounts.tree = gnt_tree_new_with_columns(2);
	gnt_widget_set_has_border(accounts.tree, FALSE);

	manager_model = purple_account_manager_get_default_as_model();
	n_items = g_list_model_get_n_items(manager_model);
	for(guint index = 0; index < n_items; index++) {
		PurpleAccount *account = g_list_model_get_item(manager_model, index);
		account_add(account);
		g_object_unref(account);
	}

	g_signal_connect(G_OBJECT(accounts.tree), "toggled", G_CALLBACK(account_toggled), NULL);
	g_signal_connect(G_OBJECT(accounts.tree), "key_pressed", G_CALLBACK(account_list_key_pressed_cb), NULL);

	gnt_tree_set_col_width(GNT_TREE(accounts.tree), 0, 40);
	gnt_tree_set_col_width(GNT_TREE(accounts.tree), 1, 10);
	gnt_box_add_widget(GNT_BOX(accounts.window), accounts.tree);

	gnt_box_add_widget(GNT_BOX(accounts.window), gnt_line_new(FALSE));

	box = gnt_hbox_new(FALSE);

	button = gnt_button_new(_("Add"));
	gnt_box_add_widget(GNT_BOX(box), button);
	gnt_util_set_trigger_widget(GNT_WIDGET(accounts.tree), GNT_KEY_INS, button);
	g_signal_connect(G_OBJECT(button), "activate", G_CALLBACK(add_account_cb), NULL);

	button = gnt_button_new(_("Modify"));
	gnt_box_add_widget(GNT_BOX(box), button);
	g_signal_connect(G_OBJECT(button), "activate", G_CALLBACK(modify_account_cb), accounts.tree);

	button = gnt_button_new(_("Delete"));
	gnt_box_add_widget(GNT_BOX(box), button);
	gnt_util_set_trigger_widget(GNT_WIDGET(accounts.tree), GNT_KEY_DEL, button);
	g_signal_connect(G_OBJECT(button), "activate", G_CALLBACK(delete_account_cb), accounts.tree);

	gnt_box_add_widget(GNT_BOX(accounts.window), box);

	g_signal_connect(G_OBJECT(accounts.window), "destroy", G_CALLBACK(reset_accounts_win), NULL);

	gnt_widget_show(accounts.window);
}

void finch_account_dialog_show(PurpleAccount *account)
{
	edit_account(account);
}

static void
account_added_callback(G_GNUC_UNUSED PurpleAccountManager *manager,
                       PurpleAccount *account, G_GNUC_UNUSED gpointer data)
{
	if (accounts.window == NULL)
		return;
	account_add(account);
	gnt_widget_draw(accounts.tree);
}

static void
account_removed_callback(G_GNUC_UNUSED PurpleAccountManager *manager,
                         PurpleAccount *account, G_GNUC_UNUSED gpointer data)
{
	if (accounts.window == NULL)
		return;

	gnt_tree_remove(GNT_TREE(accounts.tree), account);
}

static void
account_abled_cb(G_GNUC_UNUSED PurpleAccountManager *manager,
                 PurpleAccount *account,
                 G_GNUC_UNUSED GParamSpec *pspec,
                 G_GNUC_UNUSED gpointer data)
{
	if(accounts.window == NULL) {
		return;
	}

	gnt_tree_set_choice(GNT_TREE(accounts.tree), account,
	                    purple_account_get_enabled(account));
}

void
finch_accounts_init(void)
{
	PurpleAccountManager *manager = NULL;
	GListModel *manager_model = NULL;
	guint n_items = 0;

	manager = purple_account_manager_get_default();
	manager_model = G_LIST_MODEL(manager);

	g_signal_connect(manager, "added", G_CALLBACK(account_added_callback),
	                 NULL);
	g_signal_connect(manager, "removed", G_CALLBACK(account_removed_callback),
	                 NULL);
	g_signal_connect(manager, "account-changed::enabled",
	                 G_CALLBACK(account_abled_cb), NULL);

	n_items = g_list_model_get_n_items(manager_model);
	if(n_items != 0) {
		gboolean has_enabled_account = FALSE;

		for(guint index = 0; index < n_items; index++) {
			PurpleAccount *account = NULL;
			account = g_list_model_get_item(manager_model, index);

			if(purple_account_get_enabled(account)) {
				has_enabled_account = TRUE;
				g_object_unref(account);
				break;
			}

			g_object_unref(account);
		}

		if(!has_enabled_account) {
			finch_accounts_show_all();
		}
	} else {
		edit_account(NULL);
		finch_accounts_show_all();
	}
}

void
finch_accounts_uninit(void)
{
	if (accounts.window)
		gnt_widget_destroy(accounts.window);
}
