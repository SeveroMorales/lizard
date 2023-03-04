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
uses GPlugin;

namespace GenieDependentPlugin
	class Info : GPlugin.PluginInfo
		construct()
			dependencies : array of string = {"dependency1", "dependency2"}

			Object(
				id: "gplugin/genie-dependent-plugin",
				dependencies: dependencies
			)

def gplugin_query(out error : Error) : GPlugin.PluginInfo
	error = null

	return new GenieDependentPlugin.Info()

def gplugin_load(plugin : GPlugin.Plugin, out error : Error) : bool
	error = null

	return true

def gplugin_unload(plugin : GPlugin.Plugin, shutdown : bool, out error : Error) : bool
	error = null

	return false
