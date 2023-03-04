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

#include <glib/gi18n-lib.h>

#include "purplecredentialmanager.h"

#include "core.h"
#include "debug.h"
#include "prefs.h"
#include "purplenoopcredentialprovider.h"
#include "purpleprivate.h"
#include "util.h"

enum {
	SIG_REGISTERED,
	SIG_UNREGISTERED,
	SIG_ACTIVE_CHANGED,
	N_SIGNALS,
};
static guint signals[N_SIGNALS] = {0, };

struct _PurpleCredentialManager {
	GObject parent;

	GHashTable *providers;

	PurpleCredentialProvider *active;
};

G_DEFINE_TYPE(PurpleCredentialManager, purple_credential_manager,
              G_TYPE_OBJECT);

static PurpleCredentialManager *default_manager = NULL;

/******************************************************************************
 * Async Callbacks
 *****************************************************************************/
static void
purple_credential_manager_read_password_callback(GObject *obj,
                                                 GAsyncResult *res,
                                                 gpointer data)
{
	PurpleCredentialProvider *provider = PURPLE_CREDENTIAL_PROVIDER(obj);
	GError *error = NULL;
	GTask *task = G_TASK(data);
	gchar *password = NULL;

	password = purple_credential_provider_read_password_finish(provider, res,
	                                                           &error);

	if(error != NULL) {
		g_task_return_error(task, error);
	} else {
		g_task_return_pointer(task, password, g_free);
	}

	/* Clean up our initial reference to the task. */
	g_object_unref(G_OBJECT(task));
}

static void
purple_credential_manager_write_password_callback(GObject *obj,
                                                  GAsyncResult *res,
                                                  gpointer data)
{
	PurpleCredentialProvider *provider = PURPLE_CREDENTIAL_PROVIDER(obj);
	GError *error = NULL;
	GTask *task = G_TASK(data);
	gboolean ret = FALSE;

	ret = purple_credential_provider_write_password_finish(provider, res,
	                                                       &error);

	if(error != NULL) {
		g_task_return_error(task, error);
	} else {
		g_task_return_boolean(task, ret);
	}

	/* Clean up our initial reference to the task. */
	g_object_unref(G_OBJECT(task));
}

static void
purple_credential_manager_clear_password_callback(GObject *obj,
                                                  GAsyncResult *res,
                                                  gpointer data)
{
	PurpleCredentialProvider *provider = PURPLE_CREDENTIAL_PROVIDER(obj);
	GError *error = NULL;
	GTask *task = G_TASK(data);
	gboolean ret = FALSE;

	ret = purple_credential_provider_clear_password_finish(provider, res,
	                                                       &error);

	if(error != NULL) {
		g_task_return_error(task, error);
	} else {
		g_task_return_boolean(task, ret);
	}

	/* Clean up our initial reference to the task. */
	g_object_unref(G_OBJECT(task));
}

/******************************************************************************
 * Purple Callbacks
 *****************************************************************************/
static void
purple_credential_manager_core_init_cb(gpointer data) {
	PurpleCredentialManager *manager = PURPLE_CREDENTIAL_MANAGER(data);

	if(!PURPLE_IS_CREDENTIAL_PROVIDER(manager->active)) {
		GSettings *settings = NULL;
		GError *error = NULL;
		gchar *id = NULL;

		settings = g_settings_new_with_backend("im.pidgin.Purple.Credentials",
		                                       purple_core_get_settings_backend());
		id = g_settings_get_string(settings, "active-provider");
		g_object_unref(settings);

		if(!purple_credential_manager_set_active(manager, id, &error)) {
			g_warning("Failed to make %s the active credential provider : %s",
			          id, error != NULL ? error->message : "unknown error");

			purple_notify_error(NULL, _("Credential Manager"),
			                    _("Failed to load the selected credential "
			                      "provider."),
			                    _("Check your system configuration or select "
			                      "another one in the preferences dialog."),
			                    NULL);
		}

		g_free(id);
		g_clear_error(&error);
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_credential_manager_finalize(GObject *obj) {
	PurpleCredentialManager *manager = NULL;

	manager = PURPLE_CREDENTIAL_MANAGER(obj);

	g_clear_pointer(&manager->providers, g_hash_table_destroy);
	g_clear_object(&manager->active);

	G_OBJECT_CLASS(purple_credential_manager_parent_class)->finalize(obj);
}

static void
purple_credential_manager_init(PurpleCredentialManager *manager) {
	manager->providers = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
	                                           g_object_unref);

	/* Connect to the core-initialized signal so we can alert the user if we
	 * were unable to find their credential provider.
	 */
	purple_signal_connect(purple_get_core(), "core-initialized", manager,
	                      G_CALLBACK(purple_credential_manager_core_init_cb),
	                      manager);
}

static void
purple_credential_manager_class_init(PurpleCredentialManagerClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = purple_credential_manager_finalize;

	/**
	 * PurpleCredentialManager::registered:
	 * @manager: The #PurpleCredentialManager instance.
	 * @provider: The #PurpleCredentialProvider that was registered.
	 *
	 * Emitted after @provider has been registered in @manager.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_REGISTERED] = g_signal_new_class_handler(
		"registered",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		PURPLE_TYPE_CREDENTIAL_PROVIDER);

	/**
	 * PurpleCredentialManager::unregistered:
	 * @manager: The #PurpleCredentialManager instance.
	 * @provider: The #PurpleCredentialProvider that was unregistered.
	 *
	 * Emitted after @provider has been unregistered from @manager.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_UNREGISTERED] = g_signal_new_class_handler(
		"unregistered",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		1,
		PURPLE_TYPE_CREDENTIAL_PROVIDER);

	/**
	 * PurpleCredentialManager::active-changed:
	 * @manager: The #PurpleCredentialManager instance.
	 * @previous: The #PurpleCredentialProvider that was previously active.
	 * @current: The #PurpleCredentialProvider that is now currently active.
	 *
	 * Emitted after @provider has become the active provider for @manager.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_ACTIVE_CHANGED] = g_signal_new_class_handler(
		"active-changed",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		2,
		PURPLE_TYPE_CREDENTIAL_PROVIDER,
		PURPLE_TYPE_CREDENTIAL_PROVIDER);
}

/******************************************************************************
 * Private API
 *****************************************************************************/

/* Currently we're auto-registering the noop provider on the default manager,
 * this may get moved to purple core later, so we just want to keep it all in
 * one place for now.
 */
static PurpleCredentialProvider *noop = NULL;

void
purple_credential_manager_startup(void) {
	if(default_manager == NULL) {
		GError *error = NULL;

		default_manager = g_object_new(PURPLE_TYPE_CREDENTIAL_MANAGER, NULL);

		noop = purple_noop_credential_provider_new();
		if(!purple_credential_manager_register(default_manager, noop, &error)) {
			g_warning("failed to register the noop credential manager: %s",
			          error ? error->message : "unknown");
			g_clear_error(&error);
			g_clear_object(&noop);
		}
	}
}

void
purple_credential_manager_shutdown(void) {
	if(PURPLE_IS_CREDENTIAL_MANAGER(default_manager)) {
		guint size = 0;

		/* If we have an instance of the noop provider we need to unregister
		 * it before continuing.
		 */
		if(PURPLE_IS_CREDENTIAL_PROVIDER(noop)) {
			GError *error = NULL;
			if(!purple_credential_manager_unregister(default_manager,
			                                         noop, &error))
			{
				g_warning("failed to unregister the noop provider: %s",
				          error ? error->message : "unknown");
				g_clear_error(&error);
			}
			g_clear_object(&noop);
		}

		size = g_hash_table_size(default_manager->providers);
		if(size > 0) {
			g_warning("purple_credential_manager_shutdown called while %d "
				      "providers were still registered. Skipping shutdown",
				      size);
		} else {
			g_clear_object(&default_manager);
		}
	}
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleCredentialManager *
purple_credential_manager_get_default(void) {
	return default_manager;
}

gboolean
purple_credential_manager_register(PurpleCredentialManager *manager,
                                   PurpleCredentialProvider *provider,
                                   GError **error)
{
	const gchar *id = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_MANAGER(manager), FALSE);
	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), FALSE);

	if(!purple_credential_provider_is_valid(provider, error)) {
		/* purple_credential_provider_is_valid sets the error on failure. */

		return FALSE;
	}

	id = purple_credential_provider_get_id(provider);
	if(g_hash_table_lookup(manager->providers, id) != NULL) {
		g_set_error(error, PURPLE_CREDENTIAL_MANAGER_DOMAIN, 0,
		            _("provider %s is already registered"), id);

		return FALSE;
	}

	g_hash_table_insert(manager->providers, g_strdup(id),
	                    g_object_ref(provider));

	g_signal_emit(G_OBJECT(manager), signals[SIG_REGISTERED], 0, provider);

	/* If we don't currently have an active provider, check if the newly
	 * registered provider has the id of the stored provider in preferences.
	 * If it is, go ahead and make it the active provider.
	 */
	if(!PURPLE_IS_CREDENTIAL_PROVIDER(manager->active)) {
		GSettings *settings = NULL;
		gchar *wanted = NULL;

		settings = g_settings_new_with_backend("im.pidgin.Purple.Credentials",
		                                       purple_core_get_settings_backend());
		wanted = g_settings_get_string(settings, "active-provider");

		if(purple_strequal(wanted, id)) {
			purple_credential_manager_set_active(manager, id, error);
		}

		g_free(wanted);
		g_object_unref(settings);
	}

	return TRUE;
}

gboolean
purple_credential_manager_unregister(PurpleCredentialManager *manager,
                                     PurpleCredentialProvider *provider,
                                     GError **error)
{
	const gchar *id = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_MANAGER(manager), FALSE);
	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), FALSE);

	id = purple_credential_provider_get_id(provider);

	if(provider == manager->active) {
		g_set_error(error, PURPLE_CREDENTIAL_MANAGER_DOMAIN, 0,
		            _("provider %s is currently in use"), id);

		return FALSE;
	}

	if(g_hash_table_remove(manager->providers, id)) {
		g_signal_emit(G_OBJECT(manager), signals[SIG_UNREGISTERED], 0,
		              provider);

		return TRUE;
	}

	g_set_error(error, PURPLE_CREDENTIAL_MANAGER_DOMAIN, 0,
	            _("provider %s is not registered"), id);

	return FALSE;
}

gboolean
purple_credential_manager_set_active(PurpleCredentialManager *manager,
                                     const gchar *id, GError **error)
{
	PurpleCredentialProvider *previous = NULL, *provider = NULL;
	GSettings *settings = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_MANAGER(manager), FALSE);

	/* First look up the new provider if we're given one. */
	if(id != NULL) {
		provider = g_hash_table_lookup(manager->providers, id);
		if(!PURPLE_IS_CREDENTIAL_PROVIDER(provider)) {
			g_set_error(error, PURPLE_CREDENTIAL_MANAGER_DOMAIN, 0,
			            "no credential provider found with id %s", id);

			return FALSE;
		}
	}

	if(PURPLE_IS_CREDENTIAL_PROVIDER(manager->active)) {
		previous = PURPLE_CREDENTIAL_PROVIDER(g_object_ref(manager->active));
	}

	if(g_set_object(&manager->active, provider)) {
		if(PURPLE_IS_CREDENTIAL_PROVIDER(previous)) {
			purple_credential_provider_deactivate(previous);
		}

		if(PURPLE_IS_CREDENTIAL_PROVIDER(provider)) {
			purple_credential_provider_activate(provider);
		}

		g_signal_emit(G_OBJECT(manager), signals[SIG_ACTIVE_CHANGED], 0,
		              previous, manager->active);
	}

	g_clear_object(&previous);

	/* Finally update the preference if we were given a new id. We assume, that
	 * a NULL id means we're shutting down and thus shouldn't update the
	 * setting.
	 */
	if(id != NULL) {
		settings = g_settings_new_with_backend("im.pidgin.Purple.Credentials",
		                                       purple_core_get_settings_backend());

		g_settings_set_string(settings, "active-provider", id);

		g_object_unref(settings);
	}

	purple_debug_info("credential-manager", "set active provider to '%s'", id);

	return TRUE;
}

PurpleCredentialProvider *
purple_credential_manager_get_active(PurpleCredentialManager *manager) {
	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_MANAGER(manager), NULL);

	return manager->active;
}

void
purple_credential_manager_read_password_async(PurpleCredentialManager *manager,
                                              PurpleAccount *account,
                                              GCancellable *cancellable,
                                              GAsyncReadyCallback callback,
                                              gpointer data)
{
	GTask *task = NULL;

	g_return_if_fail(PURPLE_IS_CREDENTIAL_MANAGER(manager));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	task = g_task_new(manager, cancellable, callback, data);

	if(manager->active != NULL) {
		purple_credential_provider_read_password_async(manager->active,
		                                               account,
		                                               cancellable,
		                                               purple_credential_manager_read_password_callback,
		                                               task);
	} else {
		g_task_return_new_error(task, PURPLE_CREDENTIAL_MANAGER_DOMAIN, 0,
		                        _("can not read password, no active "
		                          "credential provider"));
		g_object_unref(G_OBJECT(task));
	}
}

gchar *
purple_credential_manager_read_password_finish(PurpleCredentialManager *manager,
                                               GAsyncResult *result,
                                               GError **error)
{
	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_MANAGER(manager), NULL);

	return g_task_propagate_pointer(G_TASK(result), error);
}

void
purple_credential_manager_write_password_async(PurpleCredentialManager *manager,
                                               PurpleAccount *account,
                                               const gchar *password,
                                               GCancellable *cancellable,
                                               GAsyncReadyCallback callback,
                                               gpointer data)
{
	GTask *task = NULL;

	g_return_if_fail(PURPLE_IS_CREDENTIAL_MANAGER(manager));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	task = g_task_new(manager, cancellable, callback, data);

	if(!purple_account_get_remember_password(account)) {
		const gchar *name = NULL;

		name = purple_contact_info_get_username(PURPLE_CONTACT_INFO(account));

		g_task_return_new_error(task, PURPLE_CREDENTIAL_MANAGER_DOMAIN, 0,
		                        _("account \"%s\" is not marked to be stored"),
		                        name);
		g_object_unref(G_OBJECT(task));

		return;
	}

	if(manager->active != NULL) {
		purple_credential_provider_write_password_async(manager->active,
		                                                account,
		                                                password, cancellable,
		                                                purple_credential_manager_write_password_callback,
		                                                task);
	} else {
		g_task_return_new_error(task, PURPLE_CREDENTIAL_MANAGER_DOMAIN, 0,
		                        _("can not write password, no active "
		                          "credential provider"));

		g_object_unref(G_OBJECT(task));
	}

}

gboolean
purple_credential_manager_write_password_finish(PurpleCredentialManager *manager,
                                                GAsyncResult *result,
                                                GError **error)
{
	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_MANAGER(manager), FALSE);

	return g_task_propagate_boolean(G_TASK(result), error);
}

void
purple_credential_manager_clear_password_async(PurpleCredentialManager *manager,
                                               PurpleAccount *account,
                                               GCancellable *cancellable,
                                               GAsyncReadyCallback callback,
                                               gpointer data)
{
	GTask *task = NULL;

	g_return_if_fail(PURPLE_IS_CREDENTIAL_MANAGER(manager));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	task = g_task_new(manager, cancellable, callback, data);

	if(manager->active != NULL) {
		purple_credential_provider_clear_password_async(manager->active,
		                                                account,
		                                                cancellable,
		                                                purple_credential_manager_clear_password_callback,
		                                                task);
	} else {
		g_task_return_new_error(task, PURPLE_CREDENTIAL_MANAGER_DOMAIN, 0,
		                        _("can not clear password, no active "
		                          "credential provider"));

		g_object_unref(G_OBJECT(task));
	}
}

gboolean
purple_credential_manager_clear_password_finish(PurpleCredentialManager *manager,
                                                GAsyncResult *result,
                                                GError **error)
{
	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_MANAGER(manager), FALSE);

	return g_task_propagate_boolean(G_TASK(result), error);
}

void
purple_credential_manager_foreach(PurpleCredentialManager *manager,
                                  PurpleCredentialManagerForeachFunc func,
                                  gpointer data)
{
	GHashTableIter iter;
	gpointer value;

	g_return_if_fail(PURPLE_IS_CREDENTIAL_MANAGER(manager));
	g_return_if_fail(func != NULL);

	g_hash_table_iter_init(&iter, manager->providers);
	while(g_hash_table_iter_next(&iter, NULL, &value)) {
		func(PURPLE_CREDENTIAL_PROVIDER(value), data);
	}
}

