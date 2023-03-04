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

/* TODO
 *
 * This keyring now works (at the time of this writing), but there are
 * some inconvenient edge cases. When looking up passwords, libsecret
 * doesn't error if the keyring is locked. Therefore, it appears to
 * this plugin that there's no stored password. libpurple seems to
 * handle this as gracefully as possible, but it's still inconvenient.
 * This plugin could possibly be ported to use libsecret's "Complete API"
 * to resolve this if desired.
 */

#include <glib/gi18n-lib.h>

#include <gplugin.h>
#include <gplugin-native.h>

#include <purple.h>

#include <libsecret/secret.h>

/* Translators: Secret Service is a service that runs on the user's computer.
   It is one option for where the user's passwords can be stored. It is a
   project name. It may not be appropriate to translate this string, but
   transliterating to your alphabet is reasonable. More info about the
   project can be found at https://wiki.gnome.org/Projects/Libsecret */
#define LIBSECRET_ID "libsecret"
#define LIBSECRET_NAME N_("libsecret")
#define LIBSECRET_DESCRIPTION N_("Credential provider for libsecret. Common " \
                                 "in GNOME and other desktop environments.")

/******************************************************************************
 * Globals
 *****************************************************************************/
static PurpleCredentialProvider *instance = NULL;

static const SecretSchema purple_libsecret_schema = {
	"im.pidgin.Purple3", SECRET_SCHEMA_NONE,
	{
		{"user", SECRET_SCHEMA_ATTRIBUTE_STRING},
		{"protocol", SECRET_SCHEMA_ATTRIBUTE_STRING},
		{"NULL", 0}
	},
	/* Reserved fields */
	0, 0, 0, 0, 0, 0, 0, 0
};

#define PURPLE_TYPE_LIBSECRET (purple_libsecret_get_type())
G_DECLARE_FINAL_TYPE(PurpleLibSecret, purple_libsecret,
                     PURPLE, LIBSECRET, PurpleCredentialProvider)

struct _PurpleLibSecret {
	PurpleCredentialProvider parent;
};

G_DEFINE_DYNAMIC_TYPE(PurpleLibSecret, purple_libsecret,
                      PURPLE_TYPE_CREDENTIAL_PROVIDER)

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
purple_libsecret_read_password_callback(G_GNUC_UNUSED GObject *obj,
                                        GAsyncResult *result, gpointer data)
{
	GTask *task = G_TASK(data);
	GError *error = NULL;
	gchar *password = NULL;

	password = secret_password_lookup_finish(result, &error);

	if(error != NULL) {
		g_task_return_error(task, error);
	} else {
		g_task_return_pointer(task, password, g_free);
	}

	g_object_unref(G_OBJECT(task));
}

static void
purple_libsecret_write_password_callback(G_GNUC_UNUSED GObject *obj,
                                         GAsyncResult *result, gpointer data)
{
	GTask *task = G_TASK(data);
	GError *error = NULL;
	gboolean ret = FALSE;

	ret = secret_password_store_finish(result, &error);

	if(error != NULL) {
		g_task_return_error(task, error);
	} else {
		g_task_return_boolean(task, ret);
	}

	g_object_unref(G_OBJECT(task));
}

static void
purple_libsecret_clear_password_callback(G_GNUC_UNUSED GObject *obj,
                                         GAsyncResult *result, gpointer data)
{
	GTask *task = G_TASK(data);
	GError *error = NULL;

	/* This returns whether a password was removed or not. Which means that it
	 * can return FALSE with error unset. This would complicate all of the other
	 * credential API and we don't need to make this distinction, so we just
	 * return TRUE unless error is set.
	 */
	secret_password_clear_finish(result, &error);

	if(error != NULL) {
		g_task_return_error(task, error);
	} else {
		g_task_return_boolean(task, TRUE);
	}

	g_object_unref(G_OBJECT(task));
}

/******************************************************************************
 * PurpleCredentialProvider Implementation
 *****************************************************************************/
static void
purple_libsecret_read_password_async(PurpleCredentialProvider *provider,
                                     PurpleAccount *account,
                                     GCancellable *cancellable,
                                     GAsyncReadyCallback callback,
                                     gpointer data)
{
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
	GTask *task = g_task_new(G_OBJECT(provider), cancellable, callback, data);

	secret_password_lookup(&purple_libsecret_schema, cancellable,
                           purple_libsecret_read_password_callback, task,
                           "user", purple_contact_info_get_username(info),
                           "protocol", purple_account_get_protocol_id(account),
                           NULL);
}

static gchar *
purple_libsecret_read_password_finish(PurpleCredentialProvider *provider,
                                      GAsyncResult *result,
                                      GError **error)
{
	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), FALSE);
	g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);

	return g_task_propagate_pointer(G_TASK(result), error);
}

static void
purple_libsecret_write_password_async(PurpleCredentialProvider *provider,
                                      PurpleAccount *account,
                                      const gchar *password,
                                      GCancellable *cancellable,
                                      GAsyncReadyCallback callback,
                                      gpointer data)
{
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
	GTask *task = NULL;
	gchar *label = NULL;
	const gchar *username = NULL;

	task = g_task_new(G_OBJECT(provider), cancellable, callback, data);
	username = purple_contact_info_get_username(info);

	label = g_strdup_printf(_("libpurple password for account %s"), username);
	secret_password_store(&purple_libsecret_schema,
	                      SECRET_COLLECTION_DEFAULT, label, password,
	                      cancellable,
	                      purple_libsecret_write_password_callback, task,
	                      "user", username,
	                      "protocol", purple_account_get_protocol_id(account),
	                      NULL);
	g_free(label);
}

static gboolean
purple_libsecret_write_password_finish(PurpleCredentialProvider *provider,
                                       GAsyncResult *result,
                                       GError **error)
{
	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), FALSE);
	g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);

	return g_task_propagate_boolean(G_TASK(result), error);
}

static void
purple_libsecret_clear_password_async(PurpleCredentialProvider *provider,
                                      PurpleAccount *account,
                                      GCancellable *cancellable,
                                      GAsyncReadyCallback callback,
                                      gpointer data)
{
	PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
	GTask *task = g_task_new(G_OBJECT(provider), cancellable, callback, data);

	secret_password_clear(&purple_libsecret_schema, cancellable,
	                      purple_libsecret_clear_password_callback, task,
	                      "user", purple_contact_info_get_username(info),
	                      "protocol", purple_account_get_protocol_id(account),
	                      NULL);
}

static gboolean
purple_libsecret_clear_password_finish(PurpleCredentialProvider *provider,
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
purple_libsecret_init(G_GNUC_UNUSED PurpleLibSecret *libsecret) {
}

static void
purple_libsecret_class_init(PurpleLibSecretClass *klass) {
	PurpleCredentialProviderClass *provider_class = NULL;

	provider_class = PURPLE_CREDENTIAL_PROVIDER_CLASS(klass);
	provider_class->read_password_async =
		purple_libsecret_read_password_async;
	provider_class->read_password_finish =
		purple_libsecret_read_password_finish;
	provider_class->write_password_async =
		purple_libsecret_write_password_async;
	provider_class->write_password_finish =
		purple_libsecret_write_password_finish;
	provider_class->clear_password_async =
		purple_libsecret_clear_password_async;
	provider_class->clear_password_finish =
		purple_libsecret_clear_password_finish;
}

static void
purple_libsecret_class_finalize(G_GNUC_UNUSED PurpleLibSecretClass *klass) {
}

/******************************************************************************
 * API
 *****************************************************************************/
static PurpleCredentialProvider *
purple_libsecret_new(void) {
	return g_object_new(
		PURPLE_TYPE_LIBSECRET,
		"id", LIBSECRET_ID,
		"name", _(LIBSECRET_NAME),
		"description", _(LIBSECRET_DESCRIPTION),
		NULL
	);
}

/******************************************************************************
 * Plugin Exports
 *****************************************************************************/
static GPluginPluginInfo *
libsecret_query(G_GNUC_UNUSED GError **error) {
	const gchar * const authors[] = {
		"Pidgin Developers <devel@pidgin.im>",
		NULL
	};

	return GPLUGIN_PLUGIN_INFO(purple_plugin_info_new(
		"id",           "credential-provider-" LIBSECRET_ID,
		"name",         LIBSECRET_NAME,
		"version",      DISPLAY_VERSION,
		"category",     N_("Credentials"),
		"summary",      "libsecret credential provider",
		"description",  N_("Adds support for using libsecret as a credential "
		                   "provider."),
		"authors",      authors,
		"website",      PURPLE_WEBSITE,
		"abi-version",  PURPLE_ABI_VERSION,
		"flags",        PURPLE_PLUGIN_INFO_FLAGS_INTERNAL |
		                PURPLE_PLUGIN_INFO_FLAGS_AUTO_LOAD,
		NULL
	));
}

static gboolean
libsecret_load(GPluginPlugin *plugin, GError **error) {
	PurpleCredentialManager *manager = NULL;

	purple_libsecret_register_type(G_TYPE_MODULE(plugin));

	manager = purple_credential_manager_get_default();

	instance = purple_libsecret_new();

	return purple_credential_manager_register(manager, instance, error);
}

static gboolean
libsecret_unload(G_GNUC_UNUSED GPluginPlugin *plugin,
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

GPLUGIN_NATIVE_PLUGIN_DECLARE(libsecret)
