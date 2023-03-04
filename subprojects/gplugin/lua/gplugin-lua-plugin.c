/*
 * Copyright (C) 2011-2020 Gary Kramlich <grim@reaperworld.com>
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

#include "gplugin-lua-plugin.h"

#include <lua.h>

/******************************************************************************
 * Typedefs
 *****************************************************************************/
struct _GPluginLuaPlugin {
	GObject parent;

	lua_State *L;

	gchar *filename;
	GPluginLoader *loader;
	GPluginPluginInfo *info;
	GPluginPluginState state;
	GPluginPluginState desired_state;
	GError *error;
};

/******************************************************************************
 * Enums
 *****************************************************************************/
enum {
	PROP_ZERO,
	PROP_LUA_STATE,
	N_PROPERTIES,
	/* overrides */
	PROP_FILENAME = N_PROPERTIES,
	PROP_LOADER,
	PROP_INFO,
	PROP_STATE,
	PROP_DESIRED_STATE,
	PROP_ERROR,
};
static GParamSpec *properties[N_PROPERTIES] = {
	NULL,
};

/* I hate forward declarations... */
static void gplugin_lua_plugin_iface_init(GPluginPluginInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED(
	GPluginLuaPlugin,
	gplugin_lua_plugin,
	G_TYPE_OBJECT,
	0,
	G_IMPLEMENT_INTERFACE(GPLUGIN_TYPE_PLUGIN, gplugin_lua_plugin_iface_init))

/******************************************************************************
 * GPluginPlugin Implementation
 *****************************************************************************/
static void
gplugin_lua_plugin_iface_init(G_GNUC_UNUSED GPluginPluginInterface *iface)
{
}

/******************************************************************************
 * Object Stuff
 *****************************************************************************/
static void
gplugin_lua_plugin_get_property(
	GObject *obj,
	guint param_id,
	GValue *value,
	GParamSpec *pspec)
{
	GPluginLuaPlugin *plugin = GPLUGIN_LUA_PLUGIN(obj);

	switch(param_id) {
		case PROP_LUA_STATE:
			g_value_set_pointer(value, plugin->L);
			break;

		/* overrides */
		case PROP_FILENAME:
			g_value_set_string(value, plugin->filename);
			break;
		case PROP_LOADER:
			g_value_set_object(value, plugin->loader);
			break;
		case PROP_INFO:
			g_value_set_object(value, plugin->info);
			break;
		case PROP_STATE:
			g_value_set_enum(value, plugin->state);
			break;
		case PROP_DESIRED_STATE:
			g_value_set_enum(value, plugin->desired_state);
			break;
		case PROP_ERROR:
			g_value_set_boxed(value, plugin->error);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
gplugin_lua_plugin_set_property(
	GObject *obj,
	guint param_id,
	const GValue *value,
	GParamSpec *pspec)
{
	GPluginLuaPlugin *plugin = GPLUGIN_LUA_PLUGIN(obj);

	switch(param_id) {
		case PROP_LUA_STATE:
			plugin->L = g_value_get_pointer(value);
			break;

		/* overrides */
		case PROP_FILENAME:
			plugin->filename = g_value_dup_string(value);
			break;
		case PROP_LOADER:
			plugin->loader = g_value_dup_object(value);
			break;
		case PROP_INFO:
			plugin->info = g_value_dup_object(value);
			break;
		case PROP_STATE:
			plugin->state = g_value_get_enum(value);
			break;
		case PROP_DESIRED_STATE:
			plugin->desired_state = g_value_get_enum(value);
			break;
		case PROP_ERROR:
			plugin->error = g_value_dup_boxed(value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
gplugin_lua_plugin_finalize(GObject *obj)
{
	GPluginLuaPlugin *plugin = GPLUGIN_LUA_PLUGIN(obj);

	g_clear_pointer(&plugin->L, lua_close);
	g_clear_pointer(&plugin->filename, g_free);
	g_clear_object(&plugin->loader);
	g_clear_object(&plugin->info);
	g_clear_error(&plugin->error);

	G_OBJECT_CLASS(gplugin_lua_plugin_parent_class)->finalize(obj);
}

static void
gplugin_lua_plugin_init(G_GNUC_UNUSED GPluginLuaPlugin *plugin)
{
}

static void
gplugin_lua_plugin_class_finalize(G_GNUC_UNUSED GPluginLuaPluginClass *klass)
{
}

static void
gplugin_lua_plugin_class_init(GPluginLuaPluginClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = gplugin_lua_plugin_get_property;
	obj_class->set_property = gplugin_lua_plugin_set_property;
	obj_class->finalize = gplugin_lua_plugin_finalize;

	properties[PROP_LUA_STATE] = g_param_spec_pointer(
		"lua-state",
		"lua-state",
		"The lua state for the plugin",
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	/* add our overrides */
	g_object_class_override_property(obj_class, PROP_FILENAME, "filename");
	g_object_class_override_property(obj_class, PROP_LOADER, "loader");
	g_object_class_override_property(obj_class, PROP_INFO, "info");
	g_object_class_override_property(obj_class, PROP_STATE, "state");
	g_object_class_override_property(
		obj_class,
		PROP_DESIRED_STATE,
		"desired-state");
	g_object_class_override_property(obj_class, PROP_ERROR, "error");
}

/******************************************************************************
 * API
 *****************************************************************************/
void
gplugin_lua_plugin_register(GTypeModule *module)
{
	gplugin_lua_plugin_register_type(module);
}

lua_State *
gplugin_lua_plugin_get_state(GPluginLuaPlugin *plugin)
{
	g_return_val_if_fail(GPLUGIN_LUA_IS_PLUGIN(plugin), NULL);

	return plugin->L;
}
