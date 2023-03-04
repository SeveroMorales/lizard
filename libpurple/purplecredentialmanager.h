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
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_CREDENTIAL_MANAGER_H
#define PURPLE_CREDENTIAL_MANAGER_H

#include <glib.h>
#include <glib-object.h>

#include "account.h"
#include <purplecredentialprovider.h>

G_BEGIN_DECLS

/**
 * PURPLE_CREDENTIAL_MANAGER_DOMAIN:
 *
 * A #GError domain for errors from #PurpleCredentialManager.
 *
 * Since: 3.0.0
 */
#define PURPLE_CREDENTIAL_MANAGER_DOMAIN (g_quark_from_static_string("purple-credential-manager"))

#define PURPLE_TYPE_CREDENTIAL_MANAGER (purple_credential_manager_get_type())
G_DECLARE_FINAL_TYPE(PurpleCredentialManager, purple_credential_manager,
                     PURPLE, CREDENTIAL_MANAGER, GObject)

/**
 * PurpleCredentialManager:
 *
 * Purple Credential Manager is the main API access to different credential
 * providers. Providers register themselves with the manager and then the user
 * can choose which provider to use.
 *
 * Once a provider is selected, all credential access will be directed to that
 * provider.
 *
 * Since: 3.0.0
 */

/**
 * PurpleCredentialManagerForeachFunc:
 * @provider: The #PurpleCredentialProvider instance.
 * @data: User supplied data.
 *
 * A function to be used as a callback with purple_credential_manager_foreach().
 *
 * Since: 3.0.0
 */
typedef void (*PurpleCredentialManagerForeachFunc)(PurpleCredentialProvider *provider, gpointer data);

/**
 * purple_credential_manager_get_default:
 *
 * Gets the default #PurpleCredentialManager instance.
 *
 * Returns: (transfer none): The default #PurpleCredentialManager instance.
 *
 * Since: 3.0.0
 */
PurpleCredentialManager *purple_credential_manager_get_default(void);

/**
 * purple_credential_manager_register:
 * @manager: The #PurpleCredentialManager instance.
 * @provider: The #PurpleCredentialProvider to register.
 * @error: Return address for a #GError, or %NULL.
 *
 * Registers @provider with @manager.
 *
 * Returns: %TRUE if @provider was successfully registered with @manager, %FALSE
 *          otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_credential_manager_register(PurpleCredentialManager *manager, PurpleCredentialProvider *provider, GError **error);

/**
 * purple_credential_manager_unregister:
 * @manager: The #PurpleCredentialManager instance.
 * @provider: The #PurpleCredentialProvider to unregister.
 * @error: Return address for a #GError, or %NULL.
 *
 * Unregisters @provider from @manager.
 *
 * Returns: %TRUE if @provider was successfully unregistered from @provider,
 *          %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_credential_manager_unregister(PurpleCredentialManager *manager, PurpleCredentialProvider *provider, GError **error);

/**
 * purple_credential_manager_set_active:
 * @manager: The #PurpleCredentialManager instance.
 * @id: The id of the #PurpleCredentialProvider to use or %NULL to disable the
 *      active provider.
 * @error: Return address for a #GError, or %NULL.
 *
 * Changes the active #PurpleCredentialProvider of @manager to provider with an
 * id of @id.
 *
 * If @id is %NULL, it is assumed that the process is shutting down and that
 * the active provider setting will be left intact. If you want to disable the
 * active provider and keep running, you should set the active provider to
 * [class@Purple.NoopCredentialProvider].
 *
 * Returns: %TRUE on success or %FALSE with @error set on failure.
 *
 * Since: 3.0.0
 */
gboolean purple_credential_manager_set_active(PurpleCredentialManager *manager, const gchar *id, GError **error);

/**
 * purple_credential_manager_get_active:
 * @manager: The #PurpleCredentialManager instance.
 *
 * Gets the currently active #PurpleCredentialProvider or %NULL if there is no
 * active provider.
 *
 * Returns: (transfer none): The active #PurpleCredentialProvider.
 *
 * Since: 3.0.0
 */
PurpleCredentialProvider *purple_credential_manager_get_active(PurpleCredentialManager *manager);

/**
 * purple_credential_manager_read_password_async:
 * @manager: The #PurpleCredentialManager instance.
 * @account: The #PurpleAccount whose password to read.
 * @cancellable: (nullable): optional GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *            satisfied.
 * @data: User data to pass to @callback.
 *
 * Reads the password for @account using the active #PurpleCredentialProvider of
 * @manager.
 *
 * Since: 3.0.0
 */
void purple_credential_manager_read_password_async(PurpleCredentialManager *manager, PurpleAccount *account, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);

/**
 * purple_credential_manager_read_password_finish:
 * @manager: The #PurpleCredentialManager instance.
 * @result: The #GAsyncResult from the previous
 *          purple_credential_manager_read_password_async() call.
 * @error: Return address for a #GError, or %NULL.
 *
 * Finishes a previous call to purple_credential_manager_read_password_async().
 *
 * Returns: (transfer full): The password or %NULL if successful, otherwise
 *                           %NULL with @error set on failure.
 *
 * Since: 3.0.0
 */
gchar *purple_credential_manager_read_password_finish(PurpleCredentialManager *manager, GAsyncResult *result, GError **error);

/**
 * purple_credential_manager_write_password_async:
 * @manager: The #PurpleCredentialManager instance.
 * @account: The #PurpleAccount whose password to write.
 * @password: The password to write.
 * @cancellable: (nullable): optional GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *            satisfied.
 * @data: User data to pass to @callback.
 *
 * Writes @password for @account to the active #PurpleCredentialProvider of
 * @manager.
 *
 * Since: 3.0.0
 */
void purple_credential_manager_write_password_async(PurpleCredentialManager *manager, PurpleAccount *account, const gchar *password, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);

/**
 * purple_credential_manager_write_password_finish:
 * @manager: The #PurpleCredentialManager instance.
 * @result: The #GAsyncResult from the previous
 *          purple_credential_provider_write_password_async() call.
 * @error: Return address for a #GError, or %NULL.
 *
 * Finishes a previous call to purple_credential_manager_write_password_async().
 *
 * Returns: %TRUE if the password was written successfully, otherwise %FALSE
 *          with @error set.
 *
 * Since: 3.0.0
 */
gboolean purple_credential_manager_write_password_finish(PurpleCredentialManager *manager, GAsyncResult *result, GError **error);

/**
 * purple_credential_manager_clear_password_async:
 * @manager: The #PurpleCredentialManager instance.
 * @account: The #PurpleAccount whose password to clear.
 * @cancellable: (nullable): optional #GCancellable object, or %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *            satisfied.
 * @data: User data to pass to @callback.
 *
 * Clears the password for @account from the active #PurpleCredentialProvider
 * of @manager.
 *
 * Since: 3.0.0
 */
void purple_credential_manager_clear_password_async(PurpleCredentialManager *manager, PurpleAccount *account, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);

/**
 * purple_credential_manager_clear_password_finish:
 * @manager: The #PurpleCredentialManager instance.
 * @result: The #GAsyncResult from the previous
 *          purple_credential_provider_clear_password_async() call.
 * @error: Return address for a #GError, or %NULL.
 *
 * Finishes a previous call to
 * purple_credential_provider_clear_password_async().
 *
 * Returns: %TRUE if the password didn't exist or was cleared successfully,
 *          otherwise %FALSE with @error set.
 *
 * Since: 3.0.0
 */
gboolean purple_credential_manager_clear_password_finish(PurpleCredentialManager *manager, GAsyncResult *result, GError **error);

/**
 * purple_credential_manager_foreach:
 * @manager: The #PurpleCredentialManager instance.
 * @func: (scope call): The #PurpleCredentialManagerForeachFunc to call.
 * @data: User data to pass to @func.
 *
 * Calls @func for each #PurpleCredentialProvider that @manager knows about.
 *
 * Since: 3.0.0
 */
void purple_credential_manager_foreach(PurpleCredentialManager *manager, PurpleCredentialManagerForeachFunc func, gpointer data);

G_END_DECLS

#endif /* PURPLE_CREDENTIAL_MANAGER_H */
