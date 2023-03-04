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

#include "pidginaccountsdisabledmenu.h"

struct _PidginAccountsDisabledMenu {
	GMenuModel parent;

	GList *accounts;
};

G_DEFINE_TYPE(PidginAccountsDisabledMenu, pidgin_accounts_disabled_menu,
              G_TYPE_MENU_MODEL)

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_accounts_disabled_menu_refresh(PidginAccountsDisabledMenu *menu) {
	PurpleAccountManager *manager = NULL;
	gint removed = 0, added = 0;

	/* When refreshing we're always removing at least 1 item because of the
	 * "no disabled accounts" item that we put in place when all accounts
	 * are enabled.
	 */
	removed = MAX(1, g_list_length(menu->accounts));

	/* Grab the manager and get all the disabled accounts. */
	manager = purple_account_manager_get_default();
	g_list_free(menu->accounts);
	menu->accounts = purple_account_manager_get_disabled(manager);

	/* Similar to the aboved note about removed items, if every account is
	 * enabled, we add an item saying "no disabled accounts".
	 */
	added = MAX(1, g_list_length(menu->accounts));

	/* Tell any listeners that our menu has changed. */
	g_menu_model_items_changed(G_MENU_MODEL(menu), 0, removed, added);
}

static void
pidgin_accounts_disabled_menu_changed_cb(G_GNUC_UNUSED PurpleAccountManager *manager,
                                         G_GNUC_UNUSED PurpleAccount *account,
                                         G_GNUC_UNUSED GParamSpec *pspec,
                                         gpointer data)
{
	PidginAccountsDisabledMenu *menu = data;

	pidgin_accounts_disabled_menu_refresh(menu);
}

/******************************************************************************
 * GMenuModel Implementation
 *****************************************************************************/
static gboolean
pidgin_accounts_disabled_menu_is_mutable(G_GNUC_UNUSED GMenuModel *model) {
	return TRUE;
}

static gboolean
pidgin_accounts_disabled_menu_get_n_items(GMenuModel *model) {
	PidginAccountsDisabledMenu *menu = NULL;

	menu = PIDGIN_ACCOUNTS_DISABLED_MENU(model);

	if(menu->accounts == NULL) {
		return 1;
	}

	return g_list_length(menu->accounts);
}

static void
pidgin_accounts_disabled_menu_get_item_attributes(GMenuModel *model,
                                                  gint index,
                                                  GHashTable **attributes)
{
	PidginAccountsDisabledMenu *menu = NULL;
	PurpleAccount *account = NULL;
	PurpleContactInfo *info = NULL;
	PurpleProtocol *protocol = NULL;
	GVariant *value = NULL;
	const gchar *account_name = NULL, *protocol_name = NULL;

	menu = PIDGIN_ACCOUNTS_DISABLED_MENU(model);

	/* Create our hash table of attributes to return. */
	*attributes = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
	                                    (GDestroyNotify)g_variant_unref);

	/* If we don't have any disabled accounts, just return a single item,
	 * stating as much.
	 */
	if(menu->accounts == NULL) {
		value = g_variant_new_string(_("No disabled accounts"));
		g_hash_table_insert(*attributes, G_MENU_ATTRIBUTE_LABEL,
		                    g_variant_ref_sink(value));

		value = g_variant_new_string("disabled");
		g_hash_table_insert(*attributes, G_MENU_ATTRIBUTE_ACTION,
		                    g_variant_ref_sink(value));

		return;
	}

	account = g_list_nth_data(menu->accounts, index);
	if(account == NULL) {
		return;
	}

	info = PURPLE_CONTACT_INFO(account);
	account_name = purple_contact_info_get_username(info);
	protocol_name = purple_account_get_protocol_name(account);

	/* translators: This format string is intended to contain the account
	 * name followed by the protocol name to uniquely identify a specific
	 * account.
	 */
	value = g_variant_new_printf(_("%s (%s)"), account_name, protocol_name);
	g_hash_table_insert(*attributes, G_MENU_ATTRIBUTE_LABEL,
	                    g_variant_ref_sink(value));

	value = g_variant_new_string("app.enable-account");
	g_hash_table_insert(*attributes, G_MENU_ATTRIBUTE_ACTION,
	                    g_variant_ref_sink(value));

	value = g_variant_new_printf("%s", purple_contact_info_get_id(info));
	g_hash_table_insert(*attributes, G_MENU_ATTRIBUTE_TARGET,
	                    g_variant_ref_sink(value));

	protocol = purple_account_get_protocol(account);
	if(protocol != NULL) {
		value = g_variant_new_printf("%s", purple_protocol_get_icon_name(protocol));
		g_hash_table_insert(*attributes, G_MENU_ATTRIBUTE_ICON,
		                    g_variant_ref_sink(value));
	}
}

static void
pidgin_accounts_disabled_menu_get_item_links(G_GNUC_UNUSED GMenuModel *model,
                                             G_GNUC_UNUSED gint index,
                                             GHashTable **links)
{
	*links = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
	                               g_object_unref);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_accounts_disabled_menu_dispose(GObject *obj) {
	purple_signals_disconnect_by_handle(obj);

	G_OBJECT_CLASS(pidgin_accounts_disabled_menu_parent_class)->dispose(obj);
}

static void
pidgin_accounts_disabled_menu_constructed(GObject *obj) {
	G_OBJECT_CLASS(pidgin_accounts_disabled_menu_parent_class)->constructed(obj);

	pidgin_accounts_disabled_menu_refresh(PIDGIN_ACCOUNTS_DISABLED_MENU(obj));
}

static void
pidgin_accounts_disabled_menu_init(PidginAccountsDisabledMenu *menu) {
	PurpleAccountManager *manager = purple_account_manager_get_default();

	g_signal_connect_object(manager, "account-changed::enabled",
	                        G_CALLBACK(pidgin_accounts_disabled_menu_changed_cb),
	                        menu, 0);
}

static void
pidgin_accounts_disabled_menu_class_init(PidginAccountsDisabledMenuClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GMenuModelClass *model_class = G_MENU_MODEL_CLASS(klass);

	obj_class->constructed = pidgin_accounts_disabled_menu_constructed;
	obj_class->dispose = pidgin_accounts_disabled_menu_dispose;

	model_class->is_mutable = pidgin_accounts_disabled_menu_is_mutable;
	model_class->get_n_items = pidgin_accounts_disabled_menu_get_n_items;
	model_class->get_item_attributes = pidgin_accounts_disabled_menu_get_item_attributes;
	model_class->get_item_links = pidgin_accounts_disabled_menu_get_item_links;
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GMenuModel *
pidgin_accounts_disabled_menu_new(void) {
	return g_object_new(PIDGIN_TYPE_ACCOUNTS_DISABLED_MENU, NULL);
}
