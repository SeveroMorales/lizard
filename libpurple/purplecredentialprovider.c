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

#include "purplecredentialprovider.h"

#include "purpleprivate.h"

typedef struct {
	gchar *id;
	gchar *name;
	gchar *description;

	GSettings *settings;
} PurpleCredentialProviderPrivate;

enum {
	PROP_0,
	PROP_ID,
	PROP_NAME,
	PROP_DESCRIPTION,
	PROP_SETTINGS,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PurpleCredentialProvider,
                                    purple_credential_provider, G_TYPE_OBJECT)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_credential_provider_set_id(PurpleCredentialProvider *provider,
                                  const gchar *id)
{
	PurpleCredentialProviderPrivate *priv = NULL;

	priv = purple_credential_provider_get_instance_private(provider);

	g_free(priv->id);
	priv->id = g_strdup(id);

	g_object_notify_by_pspec(G_OBJECT(provider), properties[PROP_ID]);
}

static void
purple_credential_provider_set_name(PurpleCredentialProvider *provider,
                                    const gchar *name)
{
	PurpleCredentialProviderPrivate *priv = NULL;

	priv = purple_credential_provider_get_instance_private(provider);

	g_free(priv->name);
	priv->name = g_strdup(name);

	g_object_notify_by_pspec(G_OBJECT(provider), properties[PROP_NAME]);
}

static void
purple_credential_provider_set_description(PurpleCredentialProvider *provider,
                                           const gchar *description)
{
	PurpleCredentialProviderPrivate *priv = NULL;

	priv = purple_credential_provider_get_instance_private(provider);

	g_free(priv->description);
	priv->description = g_strdup(description);

	g_object_notify_by_pspec(G_OBJECT(provider), properties[PROP_DESCRIPTION]);
}

static void
purple_credential_provider_set_settings(PurpleCredentialProvider *provider,
                                        GSettings *settings)
{
	PurpleCredentialProviderPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider));

	priv = purple_credential_provider_get_instance_private(provider);

	if(g_set_object(&priv->settings, settings)) {
		g_object_notify_by_pspec(G_OBJECT(provider),
		                         properties[PROP_SETTINGS]);
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_credential_provider_get_property(GObject *obj, guint param_id,
                                        GValue *value, GParamSpec *pspec)
{
	PurpleCredentialProvider *provider = PURPLE_CREDENTIAL_PROVIDER(obj);

	switch(param_id) {
		case PROP_ID:
			g_value_set_string(value,
			                   purple_credential_provider_get_id(provider));
			break;
		case PROP_NAME:
			g_value_set_string(value,
			                   purple_credential_provider_get_name(provider));
			break;
		case PROP_DESCRIPTION:
			g_value_set_string(value,
			                   purple_credential_provider_get_description(provider));
			break;
		case PROP_SETTINGS:
			g_value_set_object(value,
			                   purple_credential_provider_get_settings(provider));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_credential_provider_set_property(GObject *obj, guint param_id,
                                        const GValue *value, GParamSpec *pspec)
{
	PurpleCredentialProvider *provider = PURPLE_CREDENTIAL_PROVIDER(obj);

	switch(param_id) {
		case PROP_ID:
			purple_credential_provider_set_id(provider,
			                                  g_value_get_string(value));
			break;
		case PROP_NAME:
			purple_credential_provider_set_name(provider,
			                                    g_value_get_string(value));
			break;
		case PROP_DESCRIPTION:
			purple_credential_provider_set_description(provider,
			                                           g_value_get_string(value));
			break;
		case PROP_SETTINGS:
			purple_credential_provider_set_settings(provider,
			                                        g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_credential_provider_finalize(GObject *obj) {
	PurpleCredentialProvider *provider = NULL;
	PurpleCredentialProviderPrivate *priv = NULL;

	provider = PURPLE_CREDENTIAL_PROVIDER(obj);
	priv = purple_credential_provider_get_instance_private(provider);

	g_clear_pointer(&priv->id, g_free);
	g_clear_pointer(&priv->name, g_free);
	g_clear_pointer(&priv->description, g_free);

	g_clear_object(&priv->settings);

	G_OBJECT_CLASS(purple_credential_provider_parent_class)->finalize(obj);
}

static void
purple_credential_provider_init(G_GNUC_UNUSED PurpleCredentialProvider *provider)
{
}

static void
purple_credential_provider_class_init(PurpleCredentialProviderClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = purple_credential_provider_get_property;
	obj_class->set_property = purple_credential_provider_set_property;
	obj_class->finalize = purple_credential_provider_finalize;

	/**
	 * PurpleCredentialProvider::id:
	 *
	 * The ID of the provider.  Used for preferences and other things that need
	 * to address it.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ID] = g_param_spec_string(
		"id", "id", "The identifier of the provider",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS
	);

	/**
	 * PurpleCredentialProvider::name:
	 *
	 * The name of the provider which will be displayed to the user.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_NAME] = g_param_spec_string(
		"name", "name", "The name of the provider",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS
	);

	/**
	 * PurpleCredentialProvider::description:
	 *
	 * The description of the provider which will be displayed to the user.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_DESCRIPTION] = g_param_spec_string(
		"description", "description", "The description of the provider",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS
	);

	/**
	 * PurpleCredentialProvider::settings:
	 *
	 * The [class@Gio.Settings] used to configure the provider. This may be
	 * %NULL.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_SETTINGS] = g_param_spec_object(
		"settings", "setings",
		"The GSettings for the provider",
		G_TYPE_SETTINGS,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Private API
 *****************************************************************************/
void
purple_credential_provider_activate(PurpleCredentialProvider *provider) {
	PurpleCredentialProviderClass *klass = NULL;

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	if(klass && klass->activate) {
		klass->activate(provider);
	}
}

void
purple_credential_provider_deactivate(PurpleCredentialProvider *provider) {
	PurpleCredentialProviderClass *klass = NULL;

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	if(klass && klass->deactivate) {
		klass->deactivate(provider);
	}
}

/******************************************************************************
 * Public API
 *****************************************************************************/
const gchar *
purple_credential_provider_get_id(PurpleCredentialProvider *provider) {
	PurpleCredentialProviderPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), NULL);

	priv = purple_credential_provider_get_instance_private(provider);

	return priv->id;
}

const gchar *
purple_credential_provider_get_name(PurpleCredentialProvider *provider) {
	PurpleCredentialProviderPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), NULL);

	priv = purple_credential_provider_get_instance_private(provider);

	return priv->name;
}

const gchar *
purple_credential_provider_get_description(PurpleCredentialProvider *provider) {
	PurpleCredentialProviderPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), NULL);

	priv = purple_credential_provider_get_instance_private(provider);

	return priv->description;
}

gboolean
purple_credential_provider_is_valid(PurpleCredentialProvider *provider,
                                    GError **error)
{
	PurpleCredentialProviderClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), FALSE);

	if(purple_credential_provider_get_id(provider) == NULL) {
		g_set_error_literal(error, PURPLE_CREDENTIAL_PROVIDER_DOMAIN, 0,
		                    "provider has no id");

		return FALSE;
	}

	if(purple_credential_provider_get_name(provider) == NULL) {
		g_set_error_literal(error, PURPLE_CREDENTIAL_PROVIDER_DOMAIN, 1,
		                    "provider has no name");

		return FALSE;
	}

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);

	if(klass->read_password_async == NULL || klass->read_password_finish == NULL) {
		g_set_error_literal(error, PURPLE_CREDENTIAL_PROVIDER_DOMAIN, 2,
		                    "provider can not read passwords");

		return FALSE;
	}

	if(klass->write_password_async == NULL || klass->write_password_finish == NULL) {
		g_set_error_literal(error, PURPLE_CREDENTIAL_PROVIDER_DOMAIN, 3,
		                    "provider can not write passwords");

		return FALSE;
	}

	return TRUE;
}

void
purple_credential_provider_read_password_async(PurpleCredentialProvider *provider,
                                               PurpleAccount *account,
                                               GCancellable *cancellable,
                                               GAsyncReadyCallback callback,
                                               gpointer data)
{
	PurpleCredentialProviderClass *klass = NULL;

	g_return_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	if(klass && klass->read_password_async) {
		klass->read_password_async(provider, account, cancellable, callback,
		                           data);
	}
}

gchar *
purple_credential_provider_read_password_finish(PurpleCredentialProvider *provider,
                                                GAsyncResult *result,
                                                GError **error)
{
	PurpleCredentialProviderClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), NULL);
	g_return_val_if_fail(G_IS_ASYNC_RESULT(result), NULL);

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	if(klass && klass->read_password_finish) {
		return klass->read_password_finish(provider, result, error);
	}

	return NULL;
}

void
purple_credential_provider_write_password_async(PurpleCredentialProvider *provider,
                                                PurpleAccount *account,
                                                const gchar *password,
                                                GCancellable *cancellable,
                                                GAsyncReadyCallback callback,
                                                gpointer data)
{
	PurpleCredentialProviderClass *klass = NULL;

	g_return_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	if(klass && klass->write_password_async) {
		klass->write_password_async(provider, account, password, cancellable,
		                            callback, data);
	}
}

gboolean
purple_credential_provider_write_password_finish(PurpleCredentialProvider *provider,
                                                 GAsyncResult *result,
                                                 GError **error)
{
	PurpleCredentialProviderClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), FALSE);
	g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	if(klass && klass->write_password_finish) {
		return klass->write_password_finish(provider, result, error);
	}

	return FALSE;
}

void
purple_credential_provider_clear_password_async(PurpleCredentialProvider *provider,
                                                PurpleAccount *account,
                                                GCancellable *cancellable,
                                                GAsyncReadyCallback callback,
                                                gpointer data)
{
	PurpleCredentialProviderClass *klass = NULL;

	g_return_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	if(klass && klass->clear_password_async) {
		klass->clear_password_async(provider, account, cancellable, callback,
		                            data);
	}
}

gboolean
purple_credential_provider_clear_password_finish(PurpleCredentialProvider *provider,
                                                 GAsyncResult *result,
                                                 GError **error)
{
	PurpleCredentialProviderClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), FALSE);
	g_return_val_if_fail(G_IS_ASYNC_RESULT(result), FALSE);

	klass = PURPLE_CREDENTIAL_PROVIDER_GET_CLASS(provider);
	if(klass && klass->clear_password_finish) {
		return klass->clear_password_finish(provider, result, error);
	}

	return FALSE;
}

GSettings *
purple_credential_provider_get_settings(PurpleCredentialProvider *provider) {
	PurpleCredentialProviderPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CREDENTIAL_PROVIDER(provider), NULL);

	priv = purple_credential_provider_get_instance_private(provider);

	return priv->settings;
}
