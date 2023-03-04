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

#include "gplugin-lua-loader.h"

#include <glib/gi18n-lib.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "gplugin-lua-plugin.h"

struct _GPluginLuaLoader {
	GPluginLoader parent;
};

G_DEFINE_DYNAMIC_TYPE(GPluginLuaLoader, gplugin_lua_loader, GPLUGIN_TYPE_LOADER)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
_gplugin_lua_error_to_gerror(lua_State *L, GError **error)
{
	const gchar *msg = NULL;

	if(error == NULL)
		return;

	msg = lua_tostring(L, -1);

	g_set_error_literal(error, GPLUGIN_DOMAIN, 0, (msg) ? msg : "Unknown");
}

/******************************************************************************
 * GPluginLoaderInterface API
 *****************************************************************************/
static GSList *
gplugin_lua_loader_supported_extensions(G_GNUC_UNUSED GPluginLoader *l)
{
	return g_slist_append(NULL, "lua");
}

static GPluginPlugin *
gplugin_lua_loader_query(
	GPluginLoader *loader,
	const gchar *filename,
	GError **error)
{
	GPluginPlugin *plugin = NULL;
	GPluginPluginInfo *info = NULL;
	lua_State *L = NULL;

	L = luaL_newstate();
	luaL_openlibs(L);

	if(luaL_loadfile(L, filename) != 0) {
		_gplugin_lua_error_to_gerror(L, error);

		return NULL;
	}

	/* run the script */
	if(lua_pcall(L, 0, 0, 0) != 0) {
		_gplugin_lua_error_to_gerror(L, error);

		return NULL;
	}

	lua_getglobal(L, "gplugin_query");
	if(lua_isnil(L, -1)) {
		g_set_error(
			error,
			GPLUGIN_DOMAIN,
			0,
			"no gplugin_query function found");

		return NULL;
	}
	if(lua_pcall(L, 0, 1, 0) != 0) {
		_gplugin_lua_error_to_gerror(L, error);

		return NULL;
	}

	if(!lua_isuserdata(L, -1)) {
		_gplugin_lua_error_to_gerror(L, error);

		return NULL;
	}

	lua_getfield(L, -1, "_native");
	info = lua_touserdata(L, -1);
	lua_pop(L, 1);

	/* clang-format off */
	plugin = g_object_new(
		GPLUGIN_LUA_TYPE_PLUGIN,
		"filename", filename,
		"loader", loader,
		"lua-state", L,
		"info", info,
		NULL);
	/* clang-format on */

	return plugin;
}

static gboolean
gplugin_lua_loader_load(
	G_GNUC_UNUSED GPluginLoader *loader,
	GPluginPlugin *plugin,
	GError **error)
{
	gboolean ret = TRUE;
	lua_State *L = gplugin_lua_plugin_get_state(GPLUGIN_LUA_PLUGIN(plugin));

	lua_getglobal(L, "gplugin_load");
	lua_pushlightuserdata(L, plugin);
	if(lua_pcall(L, 1, 1, 0) != 0) {
		_gplugin_lua_error_to_gerror(L, error);

		return FALSE;
	}

	if(!lua_isboolean(L, -1)) {
		_gplugin_lua_error_to_gerror(L, error);

		return FALSE;
	}

	if(!lua_toboolean(L, -1)) {
		ret = FALSE;
		_gplugin_lua_error_to_gerror(L, error);
	}

	return ret;
}

static gboolean
gplugin_lua_loader_unload(
	G_GNUC_UNUSED GPluginLoader *loader,
	GPluginPlugin *plugin,
	gboolean shutdown,
	GError **error)
{
	gboolean ret = TRUE;
	lua_State *L = gplugin_lua_plugin_get_state(GPLUGIN_LUA_PLUGIN(plugin));

	lua_getglobal(L, "gplugin_unload");
	lua_pushlightuserdata(L, plugin);
	lua_pushboolean(L, shutdown);
	if(lua_pcall(L, 2, 1, 0) != 0) {
		_gplugin_lua_error_to_gerror(L, error);

		return FALSE;
	}

	if(!lua_isboolean(L, -1)) {
		_gplugin_lua_error_to_gerror(L, error);

		return FALSE;
	}

	if(!lua_toboolean(L, -1)) {
		ret = FALSE;
		_gplugin_lua_error_to_gerror(L, error);
	}

	return ret;
}

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
gplugin_lua_loader_init(G_GNUC_UNUSED GPluginLuaLoader *loader)
{
}

static void
gplugin_lua_loader_class_finalize(G_GNUC_UNUSED GPluginLuaLoaderClass *klass)
{
}

static void
gplugin_lua_loader_class_init(GPluginLuaLoaderClass *klass)
{
	GPluginLoaderClass *loader_class = GPLUGIN_LOADER_CLASS(klass);

	loader_class->supported_extensions =
		gplugin_lua_loader_supported_extensions;
	loader_class->query = gplugin_lua_loader_query;
	loader_class->load = gplugin_lua_loader_load;
	loader_class->unload = gplugin_lua_loader_unload;
}

/******************************************************************************
 * API
 *****************************************************************************/
void
gplugin_lua_loader_register(GTypeModule *module)
{
	gplugin_lua_loader_register_type(module);
}

GPluginLoader *
gplugin_lua_loader_new(void)
{
	/* clang-format off */
	return GPLUGIN_LOADER(g_object_new(
		GPLUGIN_LUA_TYPE_LOADER,
		"id", "gplugin-lua5",
		NULL));
	/* clang-format on */
}
