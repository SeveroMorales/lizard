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

#include <stdio.h>
#include <string.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

static void
_add_require_path(lua_State *L, const char *path)
{
	const char *pkg_path = NULL;
	char buff[255];

	lua_getglobal(L, "package");
	lua_getfield(
		L,
		-1,
		"path"); // get field "path" from table at top of stack (-1)
	pkg_path = lua_tostring(L, -1); // grab path string from top of stack

	memset(buff, 0, sizeof(buff));
	snprintf(buff, sizeof(buff), "%s;%s", pkg_path, path);

	lua_pop(L, 1);               // pop off the path field
	lua_pushstring(L, buff);     // push the new one
	lua_setfield(L, -2, "path"); // set the field "path" in table at -2 with
								 // value at top of stack
	lua_pop(L, 1);               // get rid of package table from top of stack
}

int
main(int argc, char *argv[])
{
	lua_State *L = NULL;
	int ret = 0;

	L = luaL_newstate();
	if(L == NULL) {
		return 134;
	}

	luaL_openlibs(L);

	/* add some additional paths to package.path */
	//_add_require_path(L, "/usr/local/lib/luarocks/rocks");

	/* now try to do the require */
	lua_getglobal(L, "require");
	lua_pushstring(L, "lgi");

	ret = lua_pcall(L, 1, 1, 0);
	if(ret != 0) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
	}

	return ret;
}
