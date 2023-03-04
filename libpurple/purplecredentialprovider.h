/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_CREDENTIAL_PROVIDER_H
#define PURPLE_CREDENTIAL_PROVIDER_H

#include <glib.h>
#include <glib-object.h>

#include "account.h"
#include "request.h"

G_BEGIN_DECLS

/**
 * PURPLE_CREDENTIAL_PROVIDER_DOMAIN:
 *
 * A #GError domain for errors from #PurpleCredentialProviders.
 *
 * Since: 3.0.0
 */
#define PURPLE_CREDENTIAL_PROVIDER_DOMAIN (g_quark_from_static_string("purple-credential-provider"))

/**
 * PurpleCredentialProvider:
 *
 * #PurpleCredentialProvider is an abstract base class for implementing support
 * for a specific password or keyring manager. At the time of this writing,
 * libpurple ships with plugins for libsecret, macOS Keychain Access, KWallet,
 * and the Windows Credentials store by subclassing #PurpleCredentialProvider.
 *
 * Since: 3.0.0
 */

#define PURPLE_TYPE_CREDENTIAL_PROVIDER (purple_credential_provider_get_type())
G_DECLARE_DERIVABLE_TYPE(PurpleCredentialProvider, purple_credential_provider,
                         PURPLE, CREDENTIAL_PROVIDER, GObject)

/**
 * PurpleCredentialProviderClass:
 * @activate: Called when the provider is made active.
 * @deactivate: Called when another provider has been made active.
 * @read_password_async: Reads a password from the provider.
 * @read_password_finish: Finishes reading a password.
 * @write_password_async: Writes a password to the provider.
 * @write_password_finish: Finishes writing a password.
 * @clear_password_async: Clears a password from the provider.
 * @clear_password_finish: Finishes clearing a password from the provider.
 *
 * #PurpleCredentialProviderClass defines the interface for interacting with
 * credential providers like libsecret, kwallet, etc.
 *
 * Since: 3.0.0
 */
struct _PurpleCredentialProviderClass {
	/*< private >*/
	GObjectClass parent;

	/*< public >*/
	void (*activate)(PurpleCredentialProvider *provider);
	void (*deactivate)(PurpleCredentialProvider *provider);

	void (*read_password_async)(PurpleCredentialProvider *provider, PurpleAccount *account, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
	gchar *(*read_password_finish)(PurpleCredentialProvider *provider, GAsyncResult *result, GError **error);

	void (*write_password_async)(PurpleCredentialProvider *provider, PurpleAccount *account, const gchar *password, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
	gboolean (*write_password_finish)(PurpleCredentialProvider *provider, GAsyncResult *result, GError **error);

	void (*clear_password_async)(PurpleCredentialProvider *provider, PurpleAccount *account, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
	gboolean (*clear_password_finish)(PurpleCredentialProvider *provider, GAsyncResult *result, GError **error);

	/*< private >*/

	/* Some extra padding to play it safe. */
	gpointer reserved[8];
};

/**
 * purple_credential_provider_get_id:
 * @provider: The #PurpleCredentialProvider instance.
 *
 * Gets the identifier of @provider.
 *
 * Returns: The identifier of @provider.
 *
 * Since: 3.0.0
 */
const gchar *purple_credential_provider_get_id(PurpleCredentialProvider *provider);

/**
 * purple_credential_provider_get_name:
 * @provider: The #PurpleCredentialProvider instance.
 *
 * Gets the name of @provider which can be show in user interfaces.
 *
 * Returns: The name of @provider.
 *
 * Since: 3.0.0
 */
const gchar *purple_credential_provider_get_name(PurpleCredentialProvider *provider);

/**
 * purple_credential_provider_get_description:
 * @provider: The #PurpleCredentialProvider instance.
 *
 * Gets the description of @provider which can be displayed in user interfaces
 * to help users figure out which provider to use.
 *
 * Returns: The description of @provider.
 *
 * Since: 3.0.0
 */
const gchar *purple_credential_provider_get_description(PurpleCredentialProvider *provider);

/**
 * purple_credential_provider_is_valid:
 * @provider: The #PurpleCredentialProvider instance.
 * @error: Return address for a #GError, or %NULL.
 *
 * Checks whether or not @provider is setup correctly.  This is primarily meant
 * for #purple_credential_provider_register_provider to call to avoid
 * programming errors, but can be used by anyone.
 *
 * Returns: %FALSE on error, otherwise %TRUE.
 *
 * Since: 3.0.0
 */
gboolean purple_credential_provider_is_valid(PurpleCredentialProvider *provider, GError **error);

/**
 * purple_credential_provider_read_password_async:
 * @provider: The #PurpleCredentialProvider instance.
 * @account: The #PurpleAccount whose password to read.
 * @cancellable: (nullable): optional GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *            satisfied.
 * @data: User data to pass to @callback.
 *
 * Reads the password for @account from @provider.
 *
 * Since: 3.0.0
 */
void purple_credential_provider_read_password_async(PurpleCredentialProvider *provider, PurpleAccount *account, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);

/**
 * purple_credential_provider_read_password_finish:
 * @provider: The #PurpleCredentialProvider instance.
 * @result: The #GAsyncResult from the previous
 *          purple_credential_provider_read_password_async() call.
 * @error: Return address for a #GError, or %NULL.
 *
 * Finishes a previous call to purple_credential_provider_read_password_async().
 *
 * Returns: (transfer full): The password or %NULL if successful, otherwise
 *                           %NULL with @error set on failure.
 *
 * Since: 3.0.0
 */
gchar *purple_credential_provider_read_password_finish(PurpleCredentialProvider *provider, GAsyncResult *result, GError **error);

/**
 * purple_credential_provider_write_password_async:
 * @provider: The #PurpleCredentialProvider instance.
 * @account: The #PurpleAccount whose password to write.
 * @password: The password to write.
 * @cancellable: (nullable): optional GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *            satisfied.
 * @data: User data to pass to @callback.
 *
 * Writes @password for @account to @provider.
 *
 * Since: 3.0.0
 */
void purple_credential_provider_write_password_async(PurpleCredentialProvider *provider, PurpleAccount *account, const gchar *password, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);

/**
 * purple_credential_provider_write_password_finish:
 * @provider: The #PurpleCredentialProvider instance.
 * @result: The #GAsyncResult from the previous
 *          purple_credential_provider_write_password_async() call.
 * @error: Return address for a #GError, or %NULL.
 *
 * Finishes a previous call to
 * purple_credential_provider_write_password_async().
 *
 * Returns: %TRUE if the password was written successfully, otherwise %FALSE
 *          with @error set.
 *
 * Since: 3.0.0
 */
gboolean purple_credential_provider_write_password_finish(PurpleCredentialProvider *provider, GAsyncResult *result, GError **error);

/**
 * purple_credential_provider_clear_password_async:
 * @provider: The #PurpleCredentialProvider instance.
 * @account: The #PurpleAccount whose password to clear.
 * @cancellable: (nullable): optional #GCancellable object, or %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *            satisfied.
 * @data: User data to pass to @callback.
 *
 * Clears the password for @account from @provider.
 *
 * Since: 3.0.0
 */
void purple_credential_provider_clear_password_async(PurpleCredentialProvider *provider, PurpleAccount *account, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);

/**
 * purple_credential_provider_clear_password_finish:
 * @provider: The #PurpleCredentialProvider instance.
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
gboolean purple_credential_provider_clear_password_finish(PurpleCredentialProvider *provider, GAsyncResult *result, GError **error);

/**
 * purple_credential_provider_get_settings:
 * @provider: The instance.
 *
 * Gets the [class@Gio.Settings] that @provider provides.
 *
 * Returns: (transfer none): The settings for @provider or %NULL if @provider
 *          doesn't have any settings.
 *
 * Since: 3.0.0
 */
GSettings *purple_credential_provider_get_settings(PurpleCredentialProvider *provider);

G_END_DECLS

#endif /* PURPLE_CREDENTIAL_PROVIDER_H */
