/*
 * Pidgin - Internet Messenger
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "pidginpluginsmenu.h"

#include "pidginapplication.h"

#include <gplugin.h>

#include <purple.h>

struct _PidginPluginsMenu {
	GMenuModel parent;

	GQueue *plugins;
};

G_DEFINE_TYPE(PidginPluginsMenu, pidgin_plugins_menu, G_TYPE_MENU_MODEL)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_plugins_menu_add_item(gpointer data, gpointer user_data) {
	PidginPluginsMenu *menu = user_data;
	GPluginPlugin *plugin = data;
	GPluginPluginInfo *info = NULL;

	info = gplugin_plugin_get_info(plugin);
	if(PURPLE_IS_PLUGIN_INFO(info)) {
		GActionGroup *group = NULL;

		group = purple_plugin_info_get_action_group(PURPLE_PLUGIN_INFO(info));
		if(G_IS_ACTION_GROUP(group)) {
			GApplication *application = g_application_get_default();
			const gchar *prefix;

			prefix = gplugin_plugin_info_get_id(info);
			pidgin_application_add_action_group(PIDGIN_APPLICATION(application),
			                                    prefix, group);
			g_object_unref(group);

			g_queue_push_tail(menu->plugins, g_object_ref(plugin));
		}
	}

	g_clear_object(&info);
}

static void
pidgin_plugins_menu_refresh(PidginPluginsMenu *menu) {
	GPluginManager *manager = NULL;
	GSList *loaded = NULL;
	gint removed = 0, added = 0;

	removed = g_queue_get_length(menu->plugins);
	g_queue_clear_full(menu->plugins, g_object_unref);

	manager = gplugin_manager_get_default();
	loaded = gplugin_manager_find_plugins_with_state(manager,
	                                                 GPLUGIN_PLUGIN_STATE_LOADED);

	g_slist_foreach(loaded, pidgin_plugins_menu_add_item, menu);
	g_slist_free_full(loaded, g_object_unref);

	added = g_queue_get_length(menu->plugins);

	/* Tell our listeners that our menu has changed. */
	g_menu_model_items_changed(G_MENU_MODEL(menu), 0, removed, added);
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_plugins_menu_plugin_loaded_cb(G_GNUC_UNUSED GObject *manager,
                                     G_GNUC_UNUSED GPluginPlugin *plugin,
                                     gpointer data)
{
	pidgin_plugins_menu_refresh(PIDGIN_PLUGINS_MENU(data));
}

static void
pidgin_plugins_menu_plugin_unloaded_cb(G_GNUC_UNUSED GObject *manager,
                                       GPluginPlugin *plugin,
                                       gpointer data)
{
	GApplication *application = NULL;
	GPluginPluginInfo *info = NULL;
	const gchar *prefix;

	/* Remove the action group that the plugin added. */
	info = gplugin_plugin_get_info(plugin);
	if(GPLUGIN_IS_PLUGIN_INFO(info)) {
		prefix = gplugin_plugin_info_get_id(info);

		if(prefix != NULL) {
			application = g_application_get_default();
			pidgin_application_add_action_group(PIDGIN_APPLICATION(application),
			                                    prefix, NULL);
		}
	}

	/* Refresh the list */
	pidgin_plugins_menu_refresh(PIDGIN_PLUGINS_MENU(data));
}

/******************************************************************************
 * GMenuModel Implementation
 *****************************************************************************/
static gboolean
pidgin_plugins_menu_is_mutable(G_GNUC_UNUSED GMenuModel *model) {
	return TRUE;
}

static gint
pidgin_plugins_menu_get_n_items(GMenuModel *model) {
	PidginPluginsMenu *menu = PIDGIN_PLUGINS_MENU(model);

	return g_queue_get_length(menu->plugins);
}

static void
pidgin_plugins_menu_get_item_attributes(GMenuModel *model, gint index,
                                        GHashTable **attributes)
{
	PidginPluginsMenu *menu = PIDGIN_PLUGINS_MENU(model);
	GPluginPlugin *plugin = NULL;
	GPluginPluginInfo *info = NULL;
	GVariant *value = NULL;

	/* Create our hash table of attributes to return. */
	*attributes = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
	                                    (GDestroyNotify)g_variant_unref);

	/* Get the plugin the caller is interested in. */
	plugin = g_queue_peek_nth(menu->plugins, index);
	if(plugin == NULL) {
		return;
	}

	/* Grab the plugin info and set the label attribute to the name of the
	 * plugin and set the action name space to the plugin's id.
	 */
	info = gplugin_plugin_get_info(plugin);
	value = g_variant_new_string(gplugin_plugin_info_get_name(info));
	g_hash_table_insert(*attributes, G_MENU_ATTRIBUTE_LABEL,
	                    g_variant_ref_sink(value));

	g_object_unref(info);
}

static void
pidgin_plugins_menu_get_item_links(GMenuModel *model, gint index,
                                   GHashTable **links)
{
	PidginPluginsMenu *menu = PIDGIN_PLUGINS_MENU(model);
	PurplePluginInfo *purple_info = NULL;
	GPluginPlugin *plugin = NULL;
	GPluginPluginInfo *info = NULL;
	GMenuModel *actions_model = NULL;

	*links = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
	                               g_object_unref);

	plugin = g_queue_peek_nth(menu->plugins, index);
	if(!GPLUGIN_IS_PLUGIN(plugin)) {
		return;
	}

	info = gplugin_plugin_get_info(plugin);
	purple_info = PURPLE_PLUGIN_INFO(info);

	/* We need to use a section for the ACTION_NAMESPACE attribute to work, so
	 * create a menu, and add a new section item to it with the menu from the
	 * plugin, then set the ACTION_NAMESPACE attribute. Finally return the menu
	 * as the submenu.
	 */
	actions_model = purple_plugin_info_get_action_menu(purple_info);
	if(G_IS_MENU_MODEL(actions_model)) {
		GMenu *menu = NULL;
		GMenuItem *section = NULL;

		menu = g_menu_new();
		section = g_menu_item_new_section(NULL, actions_model);
		g_menu_item_set_attribute(section, G_MENU_ATTRIBUTE_ACTION_NAMESPACE,
		                          "s", gplugin_plugin_info_get_id(info));
		g_menu_append_item(menu, section);
		g_object_unref(section);

		g_hash_table_insert(*links, G_MENU_LINK_SUBMENU, menu);
	}

	g_object_unref(info);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_plugins_menu_finalize(GObject *obj) {
	PidginPluginsMenu *menu = PIDGIN_PLUGINS_MENU(obj);

	g_queue_free_full(menu->plugins, g_object_unref);

	G_OBJECT_CLASS(pidgin_plugins_menu_parent_class)->finalize(obj);
}

static void
pidgin_plugins_menu_constructed(GObject *obj) {
	G_OBJECT_CLASS(pidgin_plugins_menu_parent_class)->constructed(obj);

	pidgin_plugins_menu_refresh(PIDGIN_PLUGINS_MENU(obj));
}

static void
pidgin_plugins_menu_init(PidginPluginsMenu *menu) {
	GPluginManager *manager = NULL;

	menu->plugins = g_queue_new();

	/* Connect to the plugin manager's signals so we can stay up to date. */
	manager = gplugin_manager_get_default();

	g_signal_connect_object(manager, "loaded-plugin",
	                        G_CALLBACK(pidgin_plugins_menu_plugin_loaded_cb),
	                        menu, 0);
	g_signal_connect_object(manager, "unloaded-plugin",
	                        G_CALLBACK(pidgin_plugins_menu_plugin_unloaded_cb),
	                        menu, 0);
};

static void
pidgin_plugins_menu_class_init(PidginPluginsMenuClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GMenuModelClass *model_class = G_MENU_MODEL_CLASS(klass);

	obj_class->finalize = pidgin_plugins_menu_finalize;
	obj_class->constructed = pidgin_plugins_menu_constructed;

	model_class->is_mutable = pidgin_plugins_menu_is_mutable;
	model_class->get_n_items = pidgin_plugins_menu_get_n_items;
	model_class->get_item_attributes = pidgin_plugins_menu_get_item_attributes;
	model_class->get_item_links = pidgin_plugins_menu_get_item_links;
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GMenuModel *
pidgin_plugins_menu_new(void) {
	return g_object_new(PIDGIN_TYPE_PLUGINS_MENU, NULL);
}
