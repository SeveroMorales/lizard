/* purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
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
 * along with this program ; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1301 USA
 */

#include <glib/gi18n-lib.h>

#include <gplugin.h>
#include <gplugin-native.h>

#include <purple.h>

#include <wincred.h>

#define WINCRED_ID          "keyring-wincred"
#define WINCRED_NAME        N_("Windows credentials")
#define WINCRED_DESCRIPTION N_("The built-in credential manager for Windows.")

#define WINCRED_MAX_TARGET_NAME 256

/******************************************************************************
 * Globals
 *****************************************************************************/
static PurpleCredentialProvider *instance = NULL;

#define PURPLE_TYPE_WINCRED (purple_wincred_get_type())
G_DECLARE_FINAL_TYPE(PurpleWinCred, purple_wincred, PURPLE, SECRET_SERVICE,
                     PurpleCredentialProvider)

struct _PurpleWinCred {
	PurpleCredentialProvider parent;
};

#define PURPLE_WINCRED_ERROR (g_quark_from_static_string("wincred"))

G_DEFINE_DYNAMIC_TYPE(PurpleWinCred, purple_wincred,
                      PURPLE_TYPE_CREDENTIAL_PROVIDER)

/******************************************************************************
 * PurpleCredentialProvider Implementation
 *****************************************************************************/

static gunichar2 *
wincred_get_target_name(PurpleAccount *account, GError **error)
{
	PurpleContactInfo *info = NULL;
	gchar target_name_utf8[WINCRED_MAX_TARGET_NAME];
	gunichar2 *target_name_utf16;

	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), NULL);

	info = PURPLE_CONTACT_INFO(account);

	g_snprintf(target_name_utf8, WINCRED_MAX_TARGET_NAME, "libpurple_%s_%s",
		purple_account_get_protocol_id(account),
		purple_contact_info_get_username(info));

	target_name_utf16 =
	        g_utf8_to_utf16(target_name_utf8, -1, NULL, NULL, error);

	if (target_name_utf16 == NULL) {
		purple_debug_error("keyring-wincred", "Couldn't convert target name");
		return NULL;
	}

	return target_name_utf16;
}

static void
purple_wincred_read_password_async(PurpleCredentialProvider *provider,
                                   PurpleAccount *account,
                                   GCancellable *cancellable,
                                   GAsyncReadyCallback callback, gpointer data)
{
	GTask *task = NULL;
	GError *error = NULL;
	gunichar2 *target_name = NULL;
	gchar *password = NULL;
	PCREDENTIALW credential = NULL;

	task = g_task_new(G_OBJECT(provider), cancellable, callback, data);
	target_name = wincred_get_target_name(account, &error);
	if (target_name == NULL) {
		g_task_return_error(task, error);
		g_object_unref(G_OBJECT(task));
		return;
	}

	if (!CredReadW(target_name, CRED_TYPE_GENERIC, 0, &credential)) {
		DWORD error_code = GetLastError();

		if (error_code == ERROR_NOT_FOUND) {
			if (purple_debug_is_verbose()) {
				PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
				purple_debug_misc("keyring-wincred",
				                  "No password found for account %s\n",
				                  purple_contact_info_get_username(info));
			}
			error = g_error_new(PURPLE_WINCRED_ERROR,
				error_code,
				_("Password not found."));
		} else if (error_code == ERROR_NO_SUCH_LOGON_SESSION) {
			purple_debug_error("keyring-wincred",
				"Cannot read password, no valid logon "
				"session\n");
			error = g_error_new(PURPLE_WINCRED_ERROR,
				error_code,
				_("Cannot read password, no valid logon "
				"session."));
		} else {
			purple_debug_error("keyring-wincred",
				"Cannot read password, error %lx\n",
				error_code);
			error = g_error_new(PURPLE_WINCRED_ERROR,
				error_code,
				_("Cannot read password (error %lx)."), error_code);
		}

		g_task_return_error(task, error);
		g_object_unref(G_OBJECT(task));
		return;
	}

	password = g_utf16_to_utf8((gunichar2*)credential->CredentialBlob,
		credential->CredentialBlobSize / sizeof(gunichar2),
		NULL, NULL, NULL);

	memset(credential->CredentialBlob, 0, credential->CredentialBlobSize);
	CredFree(credential);

	if (password == NULL) {
		purple_debug_error("keyring-wincred",
			"Cannot convert password\n");
		error = g_error_new(PURPLE_WINCRED_ERROR,
			0,
			_("Cannot read password (unicode error)."));
		g_task_return_error(task, error);
		g_object_unref(G_OBJECT(task));
		return;
	} else {
		PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
		purple_debug_misc("keyring-wincred",
		                  _("Got password for account %s.\n"),
		                  purple_contact_info_get_username(info));
	}

	g_task_return_pointer(task, password, g_free);
	g_object_unref(G_OBJECT(task));
}

static gchar *
purple_wincred_read_password_finish(PurpleCredentialProvider *provider,
                                    GAsyncResult *result, GError **error)
{
	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), FALSE);
	g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);

	return g_task_propagate_pointer(G_TASK(result), error);
}

static void
purple_wincred_write_password_async(PurpleCredentialProvider *provider,
                                    PurpleAccount *account,
                                    const gchar *password,
                                    GCancellable *cancellable,
                                    GAsyncReadyCallback callback, gpointer data)
{
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
	GTask *task = NULL;
	GError *error = NULL;
	const char *username_utf8 = NULL;
	gunichar2 *target_name = NULL;
	gunichar2 *username_utf16 = NULL;
	gunichar2 *password_utf16 = NULL;
	glong password_len = 0;
	CREDENTIALW credential;

	task = g_task_new(G_OBJECT(provider), cancellable, callback, data);

	target_name = wincred_get_target_name(account, &error);
	if (target_name == NULL) {
		g_task_return_error(task, error);
		g_object_unref(G_OBJECT(task));
		return;
	}

	username_utf8 = purple_contact_info_get_username(info);
	username_utf16 = g_utf8_to_utf16(username_utf8, -1, NULL, NULL, &error);
	if (username_utf16 == NULL) {
		g_free(target_name);
		purple_debug_error("keyring-wincred", "Couldn't convert username");
		g_task_return_error(task, error);
		g_object_unref(G_OBJECT(task));
		return;
	}

	password_utf16 = g_utf8_to_utf16(password, -1, NULL, &password_len, &error);
	if (password_utf16 == NULL) {
		g_free(username_utf16);
		g_free(target_name);
		purple_debug_error("keyring-wincred", "Couldn't convert password");
		g_task_return_error(task, error);
		g_object_unref(G_OBJECT(task));
		return;
	}

	memset(&credential, 0, sizeof(CREDENTIALW));
	credential.Type = CRED_TYPE_GENERIC;
	credential.TargetName = target_name;
	credential.CredentialBlobSize = password_len * sizeof(gunichar2);
	credential.CredentialBlob = (LPBYTE)password_utf16;
	credential.Persist = CRED_PERSIST_LOCAL_MACHINE;
	credential.UserName = username_utf16;

	if (!CredWriteW(&credential, 0)) {
		DWORD error_code = GetLastError();

		if (error_code == ERROR_NO_SUCH_LOGON_SESSION) {
			purple_debug_error("keyring-wincred",
			                   "Cannot store password, no valid logon session");
			error = g_error_new(
			        PURPLE_WINCRED_ERROR, error_code,
			        _("Cannot remove password, no valid logon session."));
		} else {
			purple_debug_error("keyring-wincred",
				"Cannot store password, error %lx\n",
				error_code);
			error = g_error_new(PURPLE_WINCRED_ERROR, error_code,
				_("Cannot store password (error %lx)."), error_code);
		}
	} else {
		purple_debug_misc("keyring-wincred", "Password updated for account %s.",
		                  purple_contact_info_get_username(info));
	}

	g_free(target_name);
	g_free(username_utf16);
	g_free(password_utf16);

	if (error != NULL) {
		g_task_return_error(task, error);
	} else {
		g_task_return_boolean(task, TRUE);
	}

	g_object_unref(G_OBJECT(task));
}

static gboolean
purple_wincred_write_password_finish(PurpleCredentialProvider *provider,
                                     GAsyncResult *result, GError **error)
{
	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), FALSE);
	g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);

	return g_task_propagate_boolean(G_TASK(result), error);
}

static void
purple_wincred_clear_password_async(PurpleCredentialProvider *provider,
                                    PurpleAccount *account,
                                    GCancellable *cancellable,
                                    GAsyncReadyCallback callback, gpointer data)
{
	GTask *task = NULL;
	GError *error = NULL;
	gunichar2 *target_name = NULL;

	task = g_task_new(G_OBJECT(provider), cancellable, callback, data);

	target_name = wincred_get_target_name(account, &error);
	if (target_name == NULL) {
		g_task_return_error(task, error);
		g_object_unref(G_OBJECT(task));
		return;
	}

	if (CredDeleteW(target_name, CRED_TYPE_GENERIC, 0)) {
		PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);

		purple_debug_misc("keyring-wincred", "Password for account %s removed",
		                  purple_contact_info_get_username(info));
		g_task_return_boolean(task, TRUE);
	} else {
		DWORD error_code = GetLastError();

		if (error_code == ERROR_NOT_FOUND) {
			/* If there was no password we just return TRUE. */
			g_task_return_boolean(task, TRUE);
		} else {
			if (error_code == ERROR_NO_SUCH_LOGON_SESSION) {
				purple_debug_error(
				        "keyring-wincred",
				        "Cannot remove password, no valid logon session");
				error = g_error_new(
				        PURPLE_WINCRED_ERROR, error_code,
				        _("Cannot remove password, no valid logon session."));
			} else {
				purple_debug_error("keyring-wincred",
				                   "Cannot remove password, error %lx", error_code);
				error = g_error_new(
				        PURPLE_WINCRED_ERROR, error_code,
				        _("Cannot remove password (error %lx)."), error_code);
			}

			g_task_return_error(task, error);
		}
	}

	g_object_unref(G_OBJECT(task));
}

static gboolean
purple_wincred_clear_password_finish(PurpleCredentialProvider *provider,
                                     GAsyncResult *result, GError **error)
{
	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), FALSE);
	g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);

	return g_task_propagate_boolean(G_TASK(result), error);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_wincred_init(PurpleWinCred *wincred)
{
}

static void
purple_wincred_class_init(PurpleWinCredClass *klass)
{
	PurpleCredentialProviderClass *provider_class = NULL;

	provider_class = PURPLE_CREDENTIAL_PROVIDER_CLASS(klass);
	provider_class->read_password_async = purple_wincred_read_password_async;
	provider_class->read_password_finish = purple_wincred_read_password_finish;
	provider_class->write_password_async = purple_wincred_write_password_async;
	provider_class->write_password_finish =
	        purple_wincred_write_password_finish;
	provider_class->clear_password_async = purple_wincred_clear_password_async;
	provider_class->clear_password_finish =
	        purple_wincred_clear_password_finish;
}

static void
purple_wincred_class_finalize(PurpleWinCredClass *klass)
{
}

/******************************************************************************
 * API
 *****************************************************************************/
static PurpleCredentialProvider *
purple_wincred_new(void)
{
	return PURPLE_CREDENTIAL_PROVIDER(g_object_new(
		PURPLE_TYPE_WINCRED,
		"id", WINCRED_ID,
		"name", _(WINCRED_NAME),
		"description", _(WINCRED_DESCRIPTION),
		NULL
	));
}

/******************************************************************************
 * Plugin Exports
 *****************************************************************************/
static GPluginPluginInfo *
wincred_query(G_GNUC_UNUSED GError **error) {
	const gchar * const authors[] = {
		"Pidgin Developers <devel@pidgin.im>",
		NULL
	};

	return GPLUGIN_PLUGIN_INFO(purple_plugin_info_new(
		"id",           WINCRED_ID,
		"name",         WINCRED_NAME,
		"version",      DISPLAY_VERSION,
		"category",     _("Keyring"),
		"summary",      _("Store passwords using Windows credentials"),
		"description",  _("This plugin stores passwords using Windows credentials."),
		"authors",      authors,
		"website",      PURPLE_WEBSITE,
		"abi-version",  PURPLE_ABI_VERSION,
		"flags",        PURPLE_PLUGIN_INFO_FLAGS_INTERNAL |
		                PURPLE_PLUGIN_INFO_FLAGS_AUTO_LOAD,
		NULL
	));
}

static gboolean
wincred_load(GPluginPlugin *plugin, GError **error) {
	PurpleCredentialManager *manager = NULL;

	purple_wincred_register_type(G_TYPE_MODULE(plugin));

	manager = purple_credential_manager_get_default();

	instance = purple_wincred_new();

	return purple_credential_manager_register(manager, instance, error);
}

static gboolean
wincred_unload(G_GNUC_UNUSED GPluginPlugin *plugin,
               G_GNUC_UNUSED gboolean shutdown,
               GError **error)
{
	PurpleCredentialManager *manager = NULL;
	gboolean ret = FALSE;

	manager = purple_credential_manager_get_default();
	ret = purple_credential_manager_unregister(manager, instance, error);
	if (!ret) {
		return ret;
	}

	g_clear_object(&instance);

	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(wincred)
