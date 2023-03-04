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
		id="gplugin/lua-unload-failed",
	}
end

function gplugin_load(plugin)
	return true
end

function gplugin_unload(plugin, shutdown)
	return false
end
