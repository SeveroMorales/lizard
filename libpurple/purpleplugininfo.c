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

#include "core.h"
#include "debug.h"
#include "purpleenums.h"
#include "util.h"

#include "purpleplugininfo.h"

typedef struct {
	gchar *error;           /* Why a plugin is not loadable                 */

	PurplePluginInfoFlags flags; /* Flags for the plugin */

	/* Callback that returns a preferences frame for a plugin */
	PurplePluginPrefFrameCb pref_frame_cb;

	/* Callback that returns a preferences request handle for a plugin */
	PurplePluginPrefRequestCb pref_request_cb;

	/* TRUE if a plugin has been unloaded at least once. Auto-load
	 * plugins that have been unloaded once will not be auto-loaded again. */
	gboolean unloaded;

	GActionGroup *action_group;
	GMenuModel *menu_model;
} PurplePluginInfoPrivate;

enum {
	PROP_0,
	PROP_PREF_FRAME_CB,
	PROP_PREF_REQUEST_CB,
	PROP_FLAGS,
	PROP_ACTION_GROUP,
	PROP_ACTION_MENU,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE(PurplePluginInfo, purple_plugin_info,
                           GPLUGIN_TYPE_PLUGIN_INFO);

/**************************************************************************
 * Helpers
 **************************************************************************/
static void
purple_plugin_info_set_action_group(PurplePluginInfo *info,
                                    GActionGroup *group)
{
	PurplePluginInfoPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_PLUGIN_INFO(info));

	priv = purple_plugin_info_get_instance_private(info);

	if(g_set_object(&priv->action_group, group)) {
		g_object_notify_by_pspec(G_OBJECT(info), properties[PROP_ACTION_GROUP]);
	}
}

static void
purple_plugin_info_set_action_menu(PurplePluginInfo *info,
                                   GMenuModel *menu_model)
{
	PurplePluginInfoPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_PLUGIN_INFO(info));

	priv = purple_plugin_info_get_instance_private(info);

	if(g_set_object(&priv->menu_model, menu_model)) {
		g_object_notify_by_pspec(G_OBJECT(info), properties[PROP_ACTION_MENU]);
	}
}

/**************************************************************************
 * GObject Implementation
 **************************************************************************/
static void
purple_plugin_info_init(G_GNUC_UNUSED PurplePluginInfo *info) {
}

static void
purple_plugin_info_set_property(GObject *obj, guint param_id,
                                const GValue *value, GParamSpec *pspec)
{
	PurplePluginInfo *info = PURPLE_PLUGIN_INFO(obj);
	PurplePluginInfoPrivate *priv = NULL;

	priv = purple_plugin_info_get_instance_private(info);

	switch (param_id) {
		case PROP_PREF_FRAME_CB:
			priv->pref_frame_cb = g_value_get_pointer(value);
			break;
		case PROP_PREF_REQUEST_CB:
			priv->pref_request_cb = g_value_get_pointer(value);
			break;
		case PROP_FLAGS:
			priv->flags = g_value_get_flags(value);
			break;
		case PROP_ACTION_GROUP:
			purple_plugin_info_set_action_group(info,
			                                    g_value_get_object(value));
			break;
		case PROP_ACTION_MENU:
			purple_plugin_info_set_action_menu(info,
			                                   g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_plugin_info_get_property(GObject *obj, guint param_id, GValue *value,
                                GParamSpec *pspec)
{
	PurplePluginInfo *info = PURPLE_PLUGIN_INFO(obj);

	switch (param_id) {
		case PROP_PREF_FRAME_CB:
			g_value_set_pointer(value,
					purple_plugin_info_get_pref_frame_cb(info));
			break;
		case PROP_PREF_REQUEST_CB:
			g_value_set_pointer(value,
					purple_plugin_info_get_pref_request_cb(info));
			break;
		case PROP_FLAGS:
			g_value_set_flags(value, purple_plugin_info_get_flags(info));
			break;
		case PROP_ACTION_GROUP:
			g_value_take_object(value,
			                    purple_plugin_info_get_action_group(info));
			break;
		case PROP_ACTION_MENU:
			g_value_take_object(value,
			                    purple_plugin_info_get_action_menu(info));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_plugin_info_constructed(GObject *object) {
	PurplePluginInfo *info = PURPLE_PLUGIN_INFO(object);
	GPluginPluginInfo *ginfo = GPLUGIN_PLUGIN_INFO(info);
	PurplePluginInfoPrivate *priv = NULL;
	const gchar *id = gplugin_plugin_info_get_id(ginfo);
	guint32 version;

	priv = purple_plugin_info_get_instance_private(info);

	G_OBJECT_CLASS(purple_plugin_info_parent_class)->constructed(object);

	if(id == NULL || *id == '\0') {
		priv->error = g_strdup(_("This plugin has not defined an ID."));
	}

	version = gplugin_plugin_info_get_abi_version(ginfo);
	if (PURPLE_PLUGIN_ABI_MAJOR_VERSION(version) != PURPLE_MAJOR_VERSION ||
		PURPLE_PLUGIN_ABI_MINOR_VERSION(version) > PURPLE_MINOR_VERSION)
	{
		priv->error = g_strdup_printf(_("Your libpurple version is %d.%d.x "
		                                "(need %d.%d.x)"),
		                              PURPLE_MAJOR_VERSION,
		                              PURPLE_MINOR_VERSION,
		                              PURPLE_PLUGIN_ABI_MAJOR_VERSION(version),
		                              PURPLE_PLUGIN_ABI_MINOR_VERSION(version));
		purple_debug_error("plugins",
		                   "%s is not loadable: libpurple version is %d.%d.x "
		                   "(need %d.%d.x)\n",
		                   id, PURPLE_MAJOR_VERSION, PURPLE_MINOR_VERSION,
		                   PURPLE_PLUGIN_ABI_MAJOR_VERSION(version),
		                   PURPLE_PLUGIN_ABI_MINOR_VERSION(version));
	}
}

static void
purple_plugin_info_finalize(GObject *object) {
	PurplePluginInfoPrivate *priv = NULL;

	priv = purple_plugin_info_get_instance_private(PURPLE_PLUGIN_INFO(object));

	g_free(priv->error);

	G_OBJECT_CLASS(purple_plugin_info_parent_class)->finalize(object);
}

static void
purple_plugin_info_class_init(PurplePluginInfoClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->constructed = purple_plugin_info_constructed;
	obj_class->finalize    = purple_plugin_info_finalize;

	obj_class->get_property = purple_plugin_info_get_property;
	obj_class->set_property = purple_plugin_info_set_property;

	properties[PROP_PREF_FRAME_CB] = g_param_spec_pointer(
		"pref-frame-cb", "Preferences frame callback",
		"The callback that returns the preferences frame",
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	properties[PROP_PREF_REQUEST_CB] = g_param_spec_pointer(
		"pref-request-cb", "Preferences request callback",
		"Callback that returns preferences request handle",
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	properties[PROP_FLAGS] = g_param_spec_flags(
		"flags", "Plugin flags",
		"The flags for the plugin",
		PURPLE_TYPE_PLUGIN_INFO_FLAGS,
		0,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurplePluginInfo::action-group:
	 *
	 * A [class@Gio.ActionGroup] of actions that this plugin provides.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ACTION_GROUP] = g_param_spec_object(
		"action-group", "action-group",
		"The action group for this plugin",
		G_TYPE_ACTION_GROUP,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurplePluginInfo::action-menu:
	 *
	 * A [class@Gio.MenuModel] for activating actions.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ACTION_MENU] = g_param_spec_object(
		"action-menu", "action-menu",
		"The menu model for this plugin",
		G_TYPE_MENU_MODEL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/**************************************************************************
 * Public API
 **************************************************************************/
GPluginPluginInfo *
purple_plugin_info_new(const char *first_property, ...) {
	GObject *info;
	va_list var_args;

	/* at least ID is required */
	if (!first_property) {
		return NULL;
	}

	va_start(var_args, first_property);
	info = g_object_new_valist(PURPLE_TYPE_PLUGIN_INFO, first_property,
	                           var_args);
	va_end(var_args);

	return GPLUGIN_PLUGIN_INFO(info);
}

PurplePluginPrefFrameCb
purple_plugin_info_get_pref_frame_cb(PurplePluginInfo *info) {
	PurplePluginInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PLUGIN_INFO(info), NULL);

	priv = purple_plugin_info_get_instance_private(info);

	return priv->pref_frame_cb;
}

PurplePluginPrefRequestCb
purple_plugin_info_get_pref_request_cb(PurplePluginInfo *info) {
	PurplePluginInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PLUGIN_INFO(info), NULL);

	priv = purple_plugin_info_get_instance_private(info);

	return priv->pref_request_cb;
}

PurplePluginInfoFlags
purple_plugin_info_get_flags(PurplePluginInfo *info) {
	PurplePluginInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PLUGIN_INFO(info), 0);

	priv = purple_plugin_info_get_instance_private(info);

	return priv->flags;
}

const gchar *
purple_plugin_info_get_error(PurplePluginInfo *info) {
	PurplePluginInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PLUGIN_INFO(info), NULL);

	priv = purple_plugin_info_get_instance_private(info);

	return priv->error;
}

gboolean
purple_plugin_info_get_unloaded(PurplePluginInfo *info) {
	PurplePluginInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PLUGIN_INFO(info), FALSE);

	priv = purple_plugin_info_get_instance_private(info);

	return priv->unloaded;
}

void
purple_plugin_info_set_unloaded(PurplePluginInfo *info, gboolean unloaded) {
	PurplePluginInfoPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_PLUGIN_INFO(info));

	priv = purple_plugin_info_get_instance_private(info);

	priv->unloaded = unloaded;
}

GActionGroup *
purple_plugin_info_get_action_group(PurplePluginInfo *info) {
	PurplePluginInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PLUGIN_INFO(info), NULL);

	priv = purple_plugin_info_get_instance_private(info);

	if(G_IS_ACTION_GROUP(priv->action_group)) {
		return g_object_ref(priv->action_group);
	}

	return NULL;
}

GMenuModel *
purple_plugin_info_get_action_menu(PurplePluginInfo *info) {
	PurplePluginInfoPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_PLUGIN_INFO(info), NULL);

	priv = purple_plugin_info_get_instance_private(info);

	if(G_IS_MENU_MODEL(priv->menu_model)) {
		return g_object_ref(priv->menu_model);
	}

	return NULL;
}
