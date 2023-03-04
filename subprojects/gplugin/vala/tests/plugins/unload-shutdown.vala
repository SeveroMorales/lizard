/*
 * Copyright (C) 2011-2021 Gary Kramlich <grim@reaperworld.com>
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
using GPlugin;

namespace ValaShutdownPlugin {

public class Info : GPlugin.PluginInfo {
	public Info() {
		Object(
			id: "gplugin/vala-unload-shutdown"
		);
	}
}

}

public GPlugin.PluginInfo gplugin_query(out Error error) {
	error = null;

	return new ValaShutdownPlugin.Info();
}

public bool gplugin_load(GPlugin.Plugin plugin, out Error error) {
	error = null;

	return true;
}

public bool gplugin_unload(GPlugin.Plugin plugin, bool shutdown, out Error error) {
    if(!shutdown) {
        error = new Error(Quark.from_string("vala"), 0, "shutdown was false");

        return false;
    }

    error = null;

	return true;
}
