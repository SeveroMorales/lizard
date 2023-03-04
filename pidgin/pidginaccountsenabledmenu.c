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

#include <gtk/gtk.h>

#include "pidginaccountsenabledmenu.h"

#include "pidginapplication.h"

struct _PidginAccountsEnabledMenu {
	GMenuModel parent;

	GQueue *accounts;
};

G_DEFINE_TYPE(PidginAccountsEnabledMenu, pidgin_accounts_enabled_menu,
              G_TYPE_MENU_MODEL)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_accounts_enabled_menu_update(PidginAccountsEnabledMenu *menu,
                                    PurpleAccount *account)
{
	PurpleProtocol *protocol = NULL;
	gint index = -1;

	index = g_queue_index(menu->accounts, account);
	if(index >= 0) {
		/* Tell the model that the account needs to be updated. */
		g_menu_model_items_changed(G_MENU_MODEL(menu), index, 0, 0);
	}

	/* If the protocol has actions add them to the application windows. */
	protocol = purple_account_get_protocol(account);
	if(PURPLE_IS_PROTOCOL_ACTIONS(protocol)) {
		PurpleProtocolActions *actions = PURPLE_PROTOCOL_ACTIONS(protocol);
		PurpleConnection *connection = NULL;
		GActionGroup *action_group = NULL;

		connection = purple_account_get_connection(account);
		action_group = purple_protocol_actions_get_action_group(actions,
		                                                        connection);
		if(G_IS_ACTION_GROUP(action_group)) {
			GApplication *application = g_application_get_default();
			const gchar *prefix = purple_protocol_actions_get_prefix(actions);

			pidgin_application_add_action_group(PIDGIN_APPLICATION(application),
			                                    prefix, action_group);
			g_object_unref(action_group);
		}
	}
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_accounts_enabled_menu_changed_cb(G_GNUC_UNUSED PurpleAccountManager *manager,
                                        PurpleAccount *account,
                                        G_GNUC_UNUSED GParamSpec *pspec,
                                        gpointer data)
{
	PidginAccountsEnabledMenu *menu = data;

	if(purple_account_get_enabled(account)) {
		/* Add the account to the start of the list. */
		g_queue_push_head(menu->accounts, g_object_ref(account));

		/* Tell everyone our model added a new item at position 0. */
		g_menu_model_items_changed(G_MENU_MODEL(menu), 0, 0, 1);
	} else {
		gint index = g_queue_index(menu->accounts, account);

		if(index >= 0) {
			g_queue_pop_nth(menu->accounts, index);

			/* Tell the model that we removed one item at the given index. */
			g_menu_model_items_changed(G_MENU_MODEL(menu), index, 1, 0);

			/* Remove the reference to the account that we were holding. */
			g_object_unref(account);
		}
	}
}

static void
pidgin_accounts_enabled_menu_connected_cb(PurpleAccount *account, gpointer data)
{
	pidgin_accounts_enabled_menu_update(data, account);
}

static void
pidgin_accounts_enabled_menu_disconnected_cb(PurpleAccount *account,
                                             gpointer data)
{
	PidginAccountsEnabledMenu *menu = data;
	PurpleProtocol *protocol = NULL;
	gint index = -1;

	index = g_queue_index(menu->accounts, account);
	if(index >= 0) {
		/* Tell the model that the account needs to be updated. */
		g_menu_model_items_changed(G_MENU_MODEL(menu), index, 0, 0);
	}

	/* Figure out if this is the last connected account for this protocol, and
	 * if so, remove the action group from the application windows.
	 */
	protocol = purple_account_get_protocol(account);
	if(PURPLE_IS_PROTOCOL_ACTIONS(protocol)) {
		PurpleAccountManager *manager = NULL;
		PurpleProtocolActions *actions = PURPLE_PROTOCOL_ACTIONS(protocol);
		GList *enabled_accounts = NULL;
		gboolean found = FALSE;

		manager = purple_account_manager_get_default();
		enabled_accounts = purple_account_manager_get_enabled(manager);

		while(enabled_accounts != NULL) {
			PurpleAccount *account2 = PURPLE_ACCOUNT(enabled_accounts->data);
			PurpleProtocol *protocol2 = purple_account_get_protocol(account2);

			if(!found && protocol2 == protocol) {
				found = TRUE;
			}

			enabled_accounts = g_list_delete_link(enabled_accounts,
			                                      enabled_accounts);
		}

		if(!found) {
			GApplication *application = g_application_get_default();
			const gchar *prefix = purple_protocol_actions_get_prefix(actions);

			pidgin_application_add_action_group(PIDGIN_APPLICATION(application),
			                                    prefix, NULL);
		}
	}
}

static void
pidgin_accounts_enabled_menu_actions_changed_cb(G_GNUC_UNUSED PurpleProtocolManager *manager,
                                                G_GNUC_UNUSED PurpleProtocol *protocol,
                                                PurpleAccount *account,
                                                gpointer data)
{
	pidgin_accounts_enabled_menu_update(data, account);
}

/******************************************************************************
 * GMenuModel Implementation
 *****************************************************************************/
static gboolean
pidgin_accounts_enabled_menu_is_mutable(G_GNUC_UNUSED GMenuModel *model) {
	return TRUE;
}

static gint
pidgin_accounts_enabled_menu_get_n_items(GMenuModel *model) {
	PidginAccountsEnabledMenu *menu = PIDGIN_ACCOUNTS_ENABLED_MENU(model);

	return g_queue_get_length(menu->accounts);
}

static void
pidgin_accounts_enabled_menu_get_item_attributes(GMenuModel *model, gint index,
                                                 GHashTable **attributes)
{
	PidginAccountsEnabledMenu *menu = PIDGIN_ACCOUNTS_ENABLED_MENU(model);
	PurpleAccount *account = NULL;
	PurpleProtocol *protocol = NULL;
	GVariant *value = NULL;
	gchar *label = NULL;
	const gchar *account_name = NULL, *protocol_name = NULL, *icon_name = NULL;

	/* Create our hash table of attributes to return. This must always be
	 * populated.
	 */
	*attributes = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
	                                    (GDestroyNotify)g_variant_unref);

	/* Get the account the caller is interested in. */
	account = g_queue_peek_nth(menu->accounts, index);
	if(!PURPLE_IS_ACCOUNT(account)) {
		return;
	}

	account_name = purple_contact_info_get_username(PURPLE_CONTACT_INFO(account));

	/* Get the protocol from the account. */
	protocol = purple_account_get_protocol(account);
	if(PURPLE_IS_PROTOCOL(protocol)) {
		protocol_name = purple_protocol_get_name(protocol);
		icon_name = purple_protocol_get_icon_name(protocol);
	}

	/* Add the label. */

	/* translators: This format string is intended to contain the account
	 * name followed by the protocol name to uniquely identify a specific
	 * account.
	 */
	label = g_strdup_printf(_("%s (%s)"), account_name, protocol_name);
	value = g_variant_new_string(label);
	g_free(label);
	g_hash_table_insert(*attributes, G_MENU_ATTRIBUTE_LABEL,
	                    g_variant_ref_sink(value));

	/* Add the icon if we have one. */
	if(icon_name != NULL) {
		value = g_variant_new_string(icon_name);
		g_hash_table_insert(*attributes, G_MENU_ATTRIBUTE_ICON,
		                    g_variant_ref_sink(value));
	}
}

static void
pidgin_accounts_enabled_menu_get_item_links(GMenuModel *model, gint index,
                                            GHashTable **links)
{
	PidginAccountsEnabledMenu *menu = PIDGIN_ACCOUNTS_ENABLED_MENU(model);
	PurpleAccount *account = NULL;
	PurpleConnection *connection = NULL;
	PurpleProtocol *protocol = NULL;
	GApplication *application = g_application_get_default();
	GMenu *submenu = NULL, *template = NULL;
	const gchar *account_id = NULL, *connection_id = NULL;

	/* Create our hash table for links, this must always be populated. */
	*links = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
	                               g_object_unref);

	account = g_queue_peek_nth(menu->accounts, index);
	if(!PURPLE_IS_ACCOUNT(account)) {
		return;
	}

	account_id = purple_contact_info_get_id(PURPLE_CONTACT_INFO(account));

	connection = purple_account_get_connection(account);
	if(PURPLE_IS_CONNECTION(connection)) {
		connection_id = purple_connection_get_id(connection);
	}

	/* Create a copy of our template menu. */
	template = gtk_application_get_menu_by_id(GTK_APPLICATION(application),
	                                          "enabled-account");
	submenu = purple_menu_copy(G_MENU_MODEL(template));

	/* Add the account actions if we have any. */
	protocol = purple_account_get_protocol(account);
	if(PURPLE_IS_PROTOCOL_ACTIONS(protocol)) {
		PurpleProtocolActions *actions = PURPLE_PROTOCOL_ACTIONS(protocol);
		GMenu *protocol_menu = NULL;

		protocol_menu = purple_protocol_actions_get_menu(actions, connection);
		if(G_IS_MENU(protocol_menu)) {
			g_menu_insert_section(submenu, 1, NULL,
			                      G_MENU_MODEL(protocol_menu));
			g_object_unref(protocol_menu);
		}
	}

	purple_menu_populate_dynamic_targets(submenu,
	                                     "account", account_id,
	                                     "connection", connection_id,
	                                     NULL);

	g_hash_table_insert(*links, G_MENU_LINK_SUBMENU, submenu);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_accounts_enabled_menu_dispose(GObject *obj) {
	PidginAccountsEnabledMenu *menu = PIDGIN_ACCOUNTS_ENABLED_MENU(obj);

	if(menu->accounts != NULL) {
		g_queue_free_full(menu->accounts, g_object_unref);
		menu->accounts = NULL;
	}

	G_OBJECT_CLASS(pidgin_accounts_enabled_menu_parent_class)->dispose(obj);

}

static void
pidgin_accounts_enabled_menu_constructed(GObject *obj) {
	PidginAccountsEnabledMenu *menu = PIDGIN_ACCOUNTS_ENABLED_MENU(obj);
	PurpleAccountManager *manager = NULL;
	GList *enabled = NULL, *l = NULL;
	gint count = 0;

	G_OBJECT_CLASS(pidgin_accounts_enabled_menu_parent_class)->constructed(obj);

	manager = purple_account_manager_get_default();
	enabled = purple_account_manager_get_enabled(manager);

	for(l = enabled; l != NULL; l = l->next) {
		g_queue_push_head(menu->accounts, g_object_ref(l->data));
		count++;
	}
	g_list_free(enabled);

	g_menu_model_items_changed(G_MENU_MODEL(obj), 0, 0, count);
}

static void
pidgin_accounts_enabled_menu_init(PidginAccountsEnabledMenu *menu) {
	PurpleAccountManager *manager = NULL;
	PurpleProtocolManager *protocol_manager = NULL;
	gpointer handle = NULL;

	menu->accounts = g_queue_new();

	manager = purple_account_manager_get_default();
	g_signal_connect_object(manager, "account-changed::enabled",
	                        G_CALLBACK(pidgin_accounts_enabled_menu_changed_cb),
	                        menu, 0);

	/* Wire up the purple signals we care about. */
	handle = purple_accounts_get_handle();

	/* For the account actions, we also need to know when an account is online
	 * or offline.
	 */
	purple_signal_connect(handle, "account-signed-on", menu,
	                      G_CALLBACK(pidgin_accounts_enabled_menu_connected_cb),
	                      menu);
	purple_signal_connect(handle, "account-signed-off", menu,
	                      G_CALLBACK(pidgin_accounts_enabled_menu_disconnected_cb),
	                      menu);

	/* We also need to know when the protocol actions have changed. */
	protocol_manager = purple_protocol_manager_get_default();
	g_signal_connect_object(protocol_manager, "account-actions-changed",
	                        G_CALLBACK(pidgin_accounts_enabled_menu_actions_changed_cb),
	                        menu, 0);
}

static void
pidgin_accounts_enabled_menu_class_init(PidginAccountsEnabledMenuClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GMenuModelClass *model_class = G_MENU_MODEL_CLASS(klass);

	obj_class->constructed = pidgin_accounts_enabled_menu_constructed;
	obj_class->dispose = pidgin_accounts_enabled_menu_dispose;

	model_class->is_mutable = pidgin_accounts_enabled_menu_is_mutable;
	model_class->get_n_items = pidgin_accounts_enabled_menu_get_n_items;
	model_class->get_item_attributes = pidgin_accounts_enabled_menu_get_item_attributes;
	model_class->get_item_links = pidgin_accounts_enabled_menu_get_item_links;
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GMenuModel *
pidgin_accounts_enabled_menu_new(void) {
	return g_object_new(PIDGIN_TYPE_ACCOUNTS_ENABLED_MENU, NULL);
}
