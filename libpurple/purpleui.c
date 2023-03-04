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

#include "purpleui.h"

typedef struct {
	char *id;
	char *name;
	char *version;
	char *website;
	char *support_website;
	char *client_type;
} PurpleUiPrivate;

enum {
	PROP_0,
	PROP_ID,
	PROP_NAME,
	PROP_VERSION,
	PROP_WEBSITE,
	PROP_SUPPORT_WEBSITE,
	PROP_CLIENT_TYPE,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL,};

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PurpleUi, purple_ui, G_TYPE_OBJECT)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_ui_set_id(PurpleUi *ui, const char *id) {
	PurpleUiPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_UI(ui));

	priv = purple_ui_get_instance_private(ui);

	g_free(priv->id);
	priv->id = g_strdup(id);

	g_object_notify_by_pspec(G_OBJECT(ui), properties[PROP_ID]);
}

static void
purple_ui_set_name(PurpleUi *ui, const char *name) {
	PurpleUiPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_UI(ui));

	priv = purple_ui_get_instance_private(ui);

	g_free(priv->name);
	priv->name = g_strdup(name);

	g_object_notify_by_pspec(G_OBJECT(ui), properties[PROP_NAME]);
}

static void
purple_ui_set_version(PurpleUi *ui, const char *version) {
	PurpleUiPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_UI(ui));

	priv = purple_ui_get_instance_private(ui);

	g_free(priv->version);
	priv->version = g_strdup(version);

	g_object_notify_by_pspec(G_OBJECT(ui), properties[PROP_VERSION]);
}

static void
purple_ui_set_website(PurpleUi *ui, const char *website) {
	PurpleUiPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_UI(ui));

	priv = purple_ui_get_instance_private(ui);

	g_free(priv->website);
	priv->website = g_strdup(website);

	g_object_notify_by_pspec(G_OBJECT(ui), properties[PROP_WEBSITE]);
}

static void
purple_ui_set_support_website(PurpleUi *ui, const char *support_website) {
	PurpleUiPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_UI(ui));

	priv = purple_ui_get_instance_private(ui);

	g_free(priv->support_website);
	priv->support_website = g_strdup(support_website);

	g_object_notify_by_pspec(G_OBJECT(ui), properties[PROP_SUPPORT_WEBSITE]);
}

static void
purple_ui_set_client_type(PurpleUi *ui, const char *client_type) {
	PurpleUiPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_UI(ui));

	priv = purple_ui_get_instance_private(ui);

	g_free(priv->client_type);
	priv->client_type = g_strdup(client_type);

	g_object_notify_by_pspec(G_OBJECT(ui), properties[PROP_CLIENT_TYPE]);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_ui_get_property(GObject *obj, guint param_id, GValue *value,
                       GParamSpec *pspec)
{
	PurpleUi *ui = PURPLE_UI(obj);

	switch(param_id) {
		case PROP_ID:
			g_value_set_string(value, purple_ui_get_id(ui));
			break;
		case PROP_NAME:
			g_value_set_string(value, purple_ui_get_name(ui));
			break;
		case PROP_VERSION:
			g_value_set_string(value, purple_ui_get_version(ui));
			break;
		case PROP_WEBSITE:
			g_value_set_string(value, purple_ui_get_website(ui));
			break;
		case PROP_SUPPORT_WEBSITE:
			g_value_set_string(value, purple_ui_get_support_website(ui));
			break;
		case PROP_CLIENT_TYPE:
			g_value_set_string(value, purple_ui_get_client_type(ui));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_ui_set_property(GObject *obj, guint param_id, const GValue *value,
                       GParamSpec *pspec)
{
	PurpleUi *ui = PURPLE_UI(obj);

	switch(param_id) {
		case PROP_ID:
			purple_ui_set_id(ui, g_value_get_string(value));
			break;
		case PROP_NAME:
			purple_ui_set_name(ui, g_value_get_string(value));
			break;
		case PROP_VERSION:
			purple_ui_set_version(ui, g_value_get_string(value));
			break;
		case PROP_WEBSITE:
			purple_ui_set_website(ui, g_value_get_string(value));
			break;
		case PROP_SUPPORT_WEBSITE:
			purple_ui_set_support_website(ui, g_value_get_string(value));
			break;
		case PROP_CLIENT_TYPE:
			purple_ui_set_client_type(ui, g_value_get_string(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_ui_finalize(GObject *obj) {
	PurpleUiPrivate *priv = purple_ui_get_instance_private(PURPLE_UI(obj));

	g_clear_pointer(&priv->id, g_free);
	g_clear_pointer(&priv->name, g_free);
	g_clear_pointer(&priv->version, g_free);
	g_clear_pointer(&priv->website, g_free);
	g_clear_pointer(&priv->support_website, g_free);
	g_clear_pointer(&priv->client_type, g_free);

	G_OBJECT_CLASS(purple_ui_parent_class)->finalize(obj);
}

static void
purple_ui_init(G_GNUC_UNUSED PurpleUi *ui) {
}

static void
purple_ui_class_init(PurpleUiClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = purple_ui_get_property;
	obj_class->set_property = purple_ui_set_property;
	obj_class->finalize = purple_ui_finalize;

	/**
	 * PurpleUi:id:
	 *
	 * The identifier of the user interface. This is used in places where a
	 * constant string is need to represent the user interface.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ID] = g_param_spec_string(
		"id", "id",
		"The identifier of the user interface",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleUi:name:
	 *
	 * The name of the user interface. This is used in places where it will be
	 * displayed to users, so it should be translated.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_NAME] = g_param_spec_string(
		"name", "name",
		"The name of the user interface",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleUi:version:
	 *
	 * The version number of the user interface.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_VERSION] = g_param_spec_string(
		"version", "version",
		"The version of the user interface",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleUi:website:
	 *
	 * The website of the user interface. This should be the main website.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_WEBSITE] = g_param_spec_string(
		"website", "website",
		"The website of the user interface",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleUi:support-website:
	 *
	 * The support website of the user interface. This should link to a page
	 * that specifically directs users how to get support for your user
	 * interface.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_SUPPORT_WEBSITE] = g_param_spec_string(
		"support-website", "support-website",
		"The support website of the user interface",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleUi:client-type:
	 *
	 * The client type of the user interface. Common values include `bot`,
	 * `console`, `mobile`, `pc`, `web`, etc.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_CLIENT_TYPE] = g_param_spec_string(
		"client-type", "client-type",
		"The client type of the user interface",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
const char *
purple_ui_get_id(PurpleUi *ui) {
	PurpleUiPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_UI(ui), NULL);

	priv = purple_ui_get_instance_private(ui);

	return priv->id;
}

const char *
purple_ui_get_name(PurpleUi *ui) {
	PurpleUiPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_UI(ui), NULL);

	priv = purple_ui_get_instance_private(ui);

	return priv->name;
}

const char *
purple_ui_get_version(PurpleUi *ui) {
	PurpleUiPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_UI(ui), NULL);

	priv = purple_ui_get_instance_private(ui);

	return priv->version;
}

const char *
purple_ui_get_website(PurpleUi *ui) {
	PurpleUiPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_UI(ui), NULL);

	priv = purple_ui_get_instance_private(ui);

	return priv->website;
}

const char *
purple_ui_get_support_website(PurpleUi *ui) {
	PurpleUiPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_UI(ui), NULL);

	priv = purple_ui_get_instance_private(ui);

	return priv->support_website;
}

const char *
purple_ui_get_client_type(PurpleUi *ui) {
	PurpleUiPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_UI(ui), NULL);

	priv = purple_ui_get_instance_private(ui);

	return priv->client_type;
}

void
purple_ui_prefs_init(PurpleUi *ui) {
	PurpleUiClass *klass = NULL;

	g_return_if_fail(PURPLE_IS_UI(ui));

	klass = PURPLE_UI_GET_CLASS(ui);
	if(klass != NULL && klass->prefs_init != NULL) {
		klass->prefs_init(ui);
	}
}

gboolean
purple_ui_start(PurpleUi *ui, GError **error) {
	PurpleUiClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_UI(ui), FALSE);

	klass = PURPLE_UI_GET_CLASS(ui);
	if(klass != NULL && klass->start != NULL) {
		return klass->start(ui, error);
	}

	return FALSE;
}

void
purple_ui_stop(PurpleUi *ui) {
	PurpleUiClass *klass = NULL;

	g_return_if_fail(PURPLE_IS_UI(ui));

	klass = PURPLE_UI_GET_CLASS(ui);
	if(klass != NULL && klass->stop != NULL) {
		klass->stop(ui);
	}
}

gpointer
purple_ui_get_settings_backend(PurpleUi *ui) {
	PurpleUiClass *klass = NULL;

	g_return_val_if_fail(PURPLE_IS_UI(ui), NULL);

	klass = PURPLE_UI_GET_CLASS(ui);
	if(klass != NULL && klass->get_settings_backend != NULL) {
		return klass->get_settings_backend(ui);
	}

	return NULL;
}
