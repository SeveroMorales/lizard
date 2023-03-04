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

#include <glib/gi18n-lib.h>

#include <gplugin.h>
#include <gplugin-native.h>

#include <purple.h>

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>

#define KEYCHAIN_ACCESS_ID "purple/keychain-access"
#define KEYCHAIN_ACCESS_NAME N_("Keychain Access")
#define KEYCHAIN_ACCESS_SUMMARY N_("Keychain Access credential provider")
#define KEYCHAIN_ACCESS_DESCRIPTION N_("This plugin will store passwords in " \
                                       "Keychain Access on macOS.")

#define KEYCHAIN_ACCESS_DOMAIN (g_quark_from_static_string("keychain-access"))

#define PURPLE_TYPE_KEYCHAIN_ACCESS (purple_keychain_access_get_type())
G_DECLARE_FINAL_TYPE(PurpleKeychainAccess, purple_keychain_access,
                     PURPLE, KEYCHAIN_ACCESS, PurpleCredentialProvider)

struct _PurpleKeychainAccess {
	PurpleCredentialProvider parent;
};

G_DEFINE_DYNAMIC_TYPE(PurpleKeychainAccess, purple_keychain_access,
                      PURPLE_TYPE_CREDENTIAL_PROVIDER)

/* Most of this work is heavily based off of
 * https://stackoverflow.com/a/58850099.
 */

/******************************************************************************
 * Globals
 *****************************************************************************/
static PurpleCredentialProvider *instance = NULL;

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_keychain_access_set_task_error_from_osstatus(GTask *task,
                                                    OSStatus result)
{
	CFStringRef error_msg;
	gchar buff[255];
	gboolean converted = FALSE;

	error_msg = SecCopyErrorMessageString(result, NULL);
	converted = CFStringGetCString(error_msg, buff, sizeof(buff),
	                               kCFStringEncodingUTF8);

	if(converted) {
		g_task_return_new_error(task, KEYCHAIN_ACCESS_DOMAIN, result,
		                        "%s", buff);
	} else {
		g_task_return_new_error(task, KEYCHAIN_ACCESS_DOMAIN, result,
		                        "unknown error");
	}

	CFRelease(error_msg);
}

/******************************************************************************
 * PurpleCredentialProvider Implementation
 *****************************************************************************/
static void
purple_keychain_access_read_password_async(PurpleCredentialProvider *provider,
                                           PurpleAccount *account,
                                           GCancellable *cancellable,
                                           GAsyncReadyCallback callback,
                                           gpointer data)
{
	PurpleContactInfo *info = NULL;
	GTask *task = NULL;
	const gchar *account_name = NULL;
	CFStringRef keys[4];
	CFStringRef cf_account_name;
	CFTypeRef values[4];
	CFTypeRef item;
	CFDictionaryRef query;
	OSStatus result;

	g_return_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	info = PURPLE_CONTACT_INFO(account);
	account_name = purple_contact_info_get_username(info);
	cf_account_name = CFStringCreateWithCString(kCFAllocatorDefault,
	                                            account_name,
	                                            kCFStringEncodingUTF8);

	/* Set our security class. */
	keys[0] = kSecClass;
	values[0] = kSecClassGenericPassword;

	/* Set the username. */
	keys[1] = kSecAttrAccount;
	values[1] = cf_account_name;

	/* Make sure the password is decrypted. */
	keys[2] = kSecReturnData;
	values[2] = kCFBooleanTrue;

	/* Limit to a single result. */
	keys[3] = kSecMatchLimit;
	values[3] = kSecMatchLimitOne;

	/* Build our query. */
	query = CFDictionaryCreate(kCFAllocatorDefault, (const void **)keys,
	                           (const void **)values, 4, NULL, NULL);

	/* Finally query the item. */
	result = SecItemCopyMatching(query, &item);

	/* Cleanup */
	CFRelease(cf_account_name);
	CFRelease(query);

	/* Create the task and return our results. */
	task = g_task_new(provider, cancellable, callback, data);

	if(result == errSecSuccess) {
		if(CFGetTypeID(item) != CFDataGetTypeID()) {
			g_task_return_new_error(task, KEYCHAIN_ACCESS_DOMAIN, 0,
			                        "unexpected item found");
		} else {
			CFStringRef cf_password;
			gchar buff[255];

			cf_password = CFStringCreateFromExternalRepresentation(kCFAllocatorDefault,
			                                                       (CFDataRef)item,
			                                                       kCFStringEncodingUTF8);

			if(CFStringGetCString(cf_password, buff, sizeof(buff),
			                      kCFStringEncodingUTF8))
			{
				g_task_return_pointer(task, g_strdup(buff), g_free);
			} else {
				g_task_return_new_error(task, KEYCHAIN_ACCESS_DOMAIN, 0,
				                        "failed to read password");
			}

			CFRelease(cf_password);
		}
	} else {
		purple_keychain_access_set_task_error_from_osstatus(task, result);
	}

	g_object_unref(task);
}

static gchar *
purple_keychain_access_read_password_finish(PurpleCredentialProvider *provider,
                                            GAsyncResult *result,
                                            GError **error)
{
	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), NULL);
	g_return_val_if_fail(G_IS_ASYNC_RESULT(result), NULL);

	return g_task_propagate_pointer(G_TASK(result), error);
}

static void
purple_keychain_access_write_password_async(PurpleCredentialProvider *provider,
                                            PurpleAccount *account,
                                            const gchar *password,
                                            GCancellable *cancellable,
                                            GAsyncReadyCallback callback,
                                            gpointer data)
{
	PurpleContactInfo *info = NULL;
	GTask *task = NULL;
	const gchar *account_name = NULL;
	CFStringRef keys[3];
	CFTypeRef values[3];
	CFDictionaryRef query;
	CFStringRef cf_account_name, cf_password;
	OSStatus result;

	g_return_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));
	g_return_if_fail(password != NULL);

	info = PURPLE_CONTACT_INFO(account);
	account_name = purple_contact_info_get_username(info);
	cf_account_name = CFStringCreateWithCString(kCFAllocatorDefault,
	                                            account_name,
	                                            kCFStringEncodingUTF8);
	cf_password = CFStringCreateWithCString(kCFAllocatorDefault, password,
	                                        kCFStringEncodingUTF8);

	/* set our security class */
	keys[0] = kSecClass;
	values[0] = kSecClassGenericPassword;

	/* set the username */
	keys[1] = kSecAttrAccount;
	values[1] = cf_account_name;

	/* set the password */
	keys[2] = kSecValueData;
	values[2] = cf_password;

	/* build our query */
	query = CFDictionaryCreate(kCFAllocatorDefault, (const void **)keys,
	                           (const void **)values, 3, NULL, NULL);

	/* Finally add the item */
	result = SecItemAdd(query, NULL);

	/* Cleanup */
	CFRelease(cf_account_name);
	CFRelease(cf_password);
	CFRelease(query);

	/* Now create the GTask and set the result. */
	task = g_task_new(provider, cancellable, callback, data);

	if(result == errSecSuccess) {
		g_task_return_boolean(task, TRUE);
	} else {
		purple_keychain_access_set_task_error_from_osstatus(task, result);
	}

	g_object_unref(task);
}

static gboolean
purple_keychain_access_write_password_finish(PurpleCredentialProvider *provider,
                                             GAsyncResult *result,
                                             GError **error)
{
	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), FALSE);
	g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);

	return g_task_propagate_boolean(G_TASK(result), error);
}

static void
purple_keychain_access_clear_password_async(PurpleCredentialProvider *provider,
                                            PurpleAccount *account,
                                            GCancellable *cancellable,
                                            GAsyncReadyCallback callback,
                                            gpointer data)
{
	PurpleContactInfo *info = NULL;
	GTask *task = NULL;
	const gchar *account_name = NULL;
	CFStringRef keys[2];
	CFTypeRef values[2];
	CFDictionaryRef query;
	CFStringRef cf_account_name;
	OSStatus result;

	g_return_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	info = PURPLE_CONTACT_INFO(account);
	account_name = purple_contact_info_get_username(info);
	cf_account_name = CFStringCreateWithCString(kCFAllocatorDefault,
	                                            account_name,
	                                            kCFStringEncodingUTF8);

	/* set our security class */
	keys[0] = kSecClass;
	values[0] = kSecClassGenericPassword;

	/* set the username */
	keys[1] = kSecAttrAccount;
	values[1] = cf_account_name;

	/* build our query */
	query = CFDictionaryCreate(kCFAllocatorDefault, (const void **)keys,
	                           (const void **)values, 2, NULL, NULL);

	/* Finally add the item */
	result = SecItemDelete(query);

	/* Cleanup */
	CFRelease(cf_account_name);
	CFRelease(query);

	/* Now create the GTask and set the result. */
	task = g_task_new(provider, cancellable, callback, data);

	if(result == errSecSuccess) {
		g_task_return_boolean(task, TRUE);
	} else {
		purple_keychain_access_set_task_error_from_osstatus(task, result);
	}

	g_object_unref(task);
}

static gboolean
purple_keychain_access_clear_password_finish(PurpleCredentialProvider *provider,
                                             GAsyncResult *result,
                                             GError **error)
{
	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), FALSE);
	g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);

	return g_task_propagate_boolean(G_TASK(result), error);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_keychain_access_init(PurpleKeychainAccess *keychain_access) {
}

static void
purple_keychain_access_class_init(PurpleKeychainAccessClass *klass) {
	PurpleCredentialProviderClass *provider_class = NULL;

	provider_class = PURPLE_CREDENTIAL_PROVIDER_CLASS(klass);
	provider_class->read_password_async =
		purple_keychain_access_read_password_async;
	provider_class->read_password_finish =
		purple_keychain_access_read_password_finish;
	provider_class->write_password_async =
		purple_keychain_access_write_password_async;
	provider_class->write_password_finish =
		purple_keychain_access_write_password_finish;
	provider_class->clear_password_async =
		purple_keychain_access_clear_password_async;
	provider_class->clear_password_finish =
		purple_keychain_access_clear_password_finish;
}

static void
purple_keychain_access_class_finalize(PurpleKeychainAccessClass *klass) {
}

/******************************************************************************
 * API
 *****************************************************************************/
static PurpleCredentialProvider *
purple_keychain_access_new(void) {
	return g_object_new(
		PURPLE_TYPE_KEYCHAIN_ACCESS,
		"id", KEYCHAIN_ACCESS_ID,
		"name", KEYCHAIN_ACCESS_NAME,
		"description", _(KEYCHAIN_ACCESS_DESCRIPTION),
		NULL
	);
}

/******************************************************************************
 * Plugin Exports
 *****************************************************************************/
static GPluginPluginInfo *
keychain_access_query(G_GNUC_UNUSED GError **error) {
	const gchar * const authors[] = {
		"Pidgin Developers <devel@pidgin.im>",
		NULL
	};

	return GPLUGIN_PLUGIN_INFO(purple_plugin_info_new(
		"id",           KEYCHAIN_ACCESS_ID,
		"name",         KEYCHAIN_ACCESS_NAME,
		"version",      DISPLAY_VERSION,
		"category",     N_("Credentials"),
		"summary",      KEYCHAIN_ACCESS_SUMMARY,
		"description",  KEYCHAIN_ACCESS_DESCRIPTION,
		"authors",      authors,
		"website",      PURPLE_WEBSITE,
		"abi-version",  PURPLE_ABI_VERSION,
		"flags",        PURPLE_PLUGIN_INFO_FLAGS_INTERNAL |
		                PURPLE_PLUGIN_INFO_FLAGS_AUTO_LOAD,
		NULL
	));
}

static gboolean
keychain_access_load(GPluginPlugin *plugin, GError **error) {
	PurpleCredentialManager *manager = NULL;
	gboolean ret = FALSE;

	purple_keychain_access_register_type(G_TYPE_MODULE(plugin));

	manager = purple_credential_manager_get_default();

	instance = purple_keychain_access_new();

	ret = purple_credential_manager_register(manager, instance, error);
	if(!ret) {
		g_clear_object(&instance);
	}

	return ret;
}

static gboolean
keychain_access_unload(G_GNUC_UNUSED GPluginPlugin *plugin,
                       G_GNUC_UNUSED gboolean shutdown,
                       GError **error)
{
	PurpleCredentialManager *manager = NULL;
	gboolean ret = FALSE;

	manager = purple_credential_manager_get_default();
	ret = purple_credential_manager_unregister(manager, instance, error);
	if(!ret) {
		return ret;
	}

	g_clear_object(&instance);

	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(keychain_access)
