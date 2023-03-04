/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PURPLE_ACCOUNT_MANAGER_H
#define PURPLE_ACCOUNT_MANAGER_H

#include <glib.h>
#include <glib-object.h>

#include <libpurple/account.h>

G_BEGIN_DECLS

#define PURPLE_TYPE_ACCOUNT_MANAGER (purple_account_manager_get_type())
G_DECLARE_FINAL_TYPE(PurpleAccountManager, purple_account_manager, PURPLE, ACCOUNT_MANAGER, GObject)

/**
 * PurpleAccountManagerForeachFunc:
 * @account: The account instance.
 * @data: User specified data.
 *
 * A function used as a callback with purple_account_manager_foreach().
 *
 * Since: 3.0.0
 */
typedef void (*PurpleAccountManagerForeachFunc)(PurpleAccount *account, gpointer data);

/**
 * PurpleAccountManager:
 *
 * A manager that keeps track of all [class@Purple.Account]s.
 *
 * Since: 3.0.0
 */

/**
 * purple_account_manager_get_default:
 *
 * Gets the default account manager for libpurple.
 *
 * Returns: (transfer none): The default account manager for libpurple.
 *
 * Since: 3.0.0
 */
PurpleAccountManager *purple_account_manager_get_default(void);

/**
 * purple_account_manager_get_default_as_model:
 *
 * Gets the default account manager for libpurple as a list model.
 *
 * Returns: (transfer none): The default account manager for libpurple.
 *
 * Since: 3.0.0
 */
GListModel *purple_account_manager_get_default_as_model(void);

/**
 * purple_account_manager_add:
 * @manager: The account manager instance.
 * @account: (transfer full): The account to add.
 *
 * Adds @account to @manager.
 *
 * Since: 3.0.0
 */
void purple_account_manager_add(PurpleAccountManager *manager, PurpleAccount *account);

/**
 * purple_account_manager_remove:
 * @manager: The account manager instance.
 * @account: The account to remove.
 *
 * Removes @account from @manager.
 *
 * Since: 3.0.0
 */
void purple_account_manager_remove(PurpleAccountManager *manager, PurpleAccount *account);

/**
 * purple_account_manager_reorder:
 * @manager: The account manager instance.
 * @account: The account instance.
 * @new_index: The numerical position to move @account to.
 *
 * Moves @account to @new_index in @manager.
 *
 * Since: 3.0.0
 */
void purple_account_manager_reorder(PurpleAccountManager *manager, PurpleAccount *account, guint new_index);

/**
 * purple_account_manager_get_enabled:
 * @manager: The account manager instance.
 *
 * Gets the list of all enabled accounts.
 *
 * Returns: (transfer container) (element-type PurpleAccount): The list of all
 *          enabled accounts.
 *
 * Since: 3.0.0
 */
GList *purple_account_manager_get_enabled(PurpleAccountManager *manager);

/**
 * purple_account_manager_get_disabled:
 * @manager: The account manager instance.
 *
 * Gets the list of all disabled accounts.
 *
 * Returns: (transfer container) (element-type PurpleAccount): The list of all
 *          disabled accounts.
 *
 * Since: 3.0.0
 */
GList *purple_account_manager_get_disabled(PurpleAccountManager *manager);

/**
 * purple_account_manager_get_connected:
 * @manager: The instance.
 *
 * Gets a list of all accounts that are currently connected.
 *
 * Returns: (transfer container) (element-type PurpleAccount): The list of all
 *          connected accounts.
 *
 * Since: 3.0.0
 */
GList *purple_account_manager_get_connected(PurpleAccountManager *manager);

/**
 * purple_account_manager_find_by_id:
 * @manager: The account manager instance.
 * @id: The id of the account.
 *
 * Looks up an account based on its id property.
 *
 * Returns: (transfer none): The account if found, otherwise %NULL.
 *
 * Since: 3.0.0
 */
PurpleAccount *purple_account_manager_find_by_id(PurpleAccountManager *manager, const gchar *id);

/**
 * purple_account_manager_find:
 * @manager: The account manager instance.
 * @username: The username of the account.
 * @protocol_id: The id of the protocol of the account.
 *
 * Attempts to find an account in @manager with the matching @username and
 * @protocol_id.
 *
 * Returns: (transfer none): The account if found, otherwise %NULL.
 *
 * Since: 3.0.0
 */
PurpleAccount *purple_account_manager_find(PurpleAccountManager *manager, const gchar *username, const gchar *protocol_id);

/**
 * purple_account_manager_find_custom:
 * @manager: The account manager instance.
 * @func: (scope call): The function to call for each account. It should return
 *        TRUE when the desired element is found
 * @data: The user data to pass to the function, as its second argument.
 *
 * Attempts to find an account in @manager with a custom matching function.
 *
 * Returns: (transfer none): The account if found, otherwise %NULL.
 *
 * Since: 3.0.0
 */
PurpleAccount * purple_account_manager_find_custom(PurpleAccountManager *manager, GEqualFunc func, gconstpointer data);

/**
 * purple_account_manager_foreach:
 * @manager: The account manager instance.
 * @callback: (scope call): The function to call.
 * @data: User data to pass to @callback.
 *
 * Calls @callback with @data for each account that @manager knows about.
 *
 * Since: 3.0.0
 */
void purple_account_manager_foreach(PurpleAccountManager *manager, PurpleAccountManagerForeachFunc callback, gpointer data);

G_END_DECLS

#endif /* PURPLE_ACCOUNT_MANAGER_H */
