--[[
 Copyright (C) 2011-2020 Gary Kramlich <grim@reaperworld.com>

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, see <https://www.gnu.org/licenses/>.
--]]

local lgi = require 'lgi'
local GPlugin = lgi.require('GPlugin', '1.0')

function gplugin_query()
	return GPlugin.PluginInfo {
		id = "gplugin/lua-basic-plugin",
		abi_version = 0x01020304,
		name = "basic plugin (Lua)",
		category = "test",
		version = "version",
		license_id = "license",
		summary = "summary",
		description = "description",
		authors = { "author1" },
		website = "website"
	}
end

function gplugin_load(plugin)
	if plugin == nil then
		error('plugin is nil')
	end

	return true
end

function gplugin_unload(plugin, shutdown)
	if plugin == nil then
		error('plugin is nil')
	end

	if shutdown then
		error('shutdown is true')
	end

	return true
end
