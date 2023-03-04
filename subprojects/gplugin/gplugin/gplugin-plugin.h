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

#if !defined(GPLUGIN_GLOBAL_HEADER_INSIDE) && !defined(GPLUGIN_COMPILATION)
#error "only <gplugin.h> may be included directly"
#endif

#ifndef GPLUGIN_PLUGIN_H
#define GPLUGIN_PLUGIN_H

#include <glib.h>
#include <glib-object.h>

/* clang-format off */
typedef enum /*< prefix=GPLUGIN_PLUGIN_STATE,underscore_name=GPLUGIN_PLUGIN_STATE >*/ {
	GPLUGIN_PLUGIN_STATE_UNKNOWN = -1,
	GPLUGIN_PLUGIN_STATE_ERROR = 0,
	GPLUGIN_PLUGIN_STATE_QUERIED,
	GPLUGIN_PLUGIN_STATE_REQUERY,
	GPLUGIN_PLUGIN_STATE_LOADED,
	GPLUGIN_PLUGIN_STATE_LOAD_FAILED,
	GPLUGIN_PLUGIN_STATE_UNLOAD_FAILED,

	/*< private >*/
	GPLUGIN_PLUGIN_STATES, /*< skip >*/
} GPluginPluginState;
/* clang-format on */

G_BEGIN_DECLS

#define GPLUGIN_TYPE_PLUGIN (gplugin_plugin_get_type())
G_DECLARE_INTERFACE(GPluginPlugin, gplugin_plugin, GPLUGIN, PLUGIN, GObject)

/* circular dependencies suck... */
#include <gplugin/gplugin-loader.h>
#include <gplugin/gplugin-plugin-info.h>

struct _GPluginPluginInterface {
	/*< private >*/
	GTypeInterface parent;

	/*< public >*/
	void (*state_changed)(
		GPluginPlugin *plugin,
		GPluginPluginState oldstate,
		GPluginPluginState newstate);

	/*< private >*/
	gpointer reserved[8];
};

gchar *gplugin_plugin_get_filename(GPluginPlugin *plugin);
GPluginLoader *gplugin_plugin_get_loader(GPluginPlugin *plugin);
GPluginPluginInfo *gplugin_plugin_get_info(GPluginPlugin *plugin);

GPluginPluginState gplugin_plugin_get_state(GPluginPlugin *plugin);
void gplugin_plugin_set_state(GPluginPlugin *plugin, GPluginPluginState state);

GPluginPluginState gplugin_plugin_get_desired_state(GPluginPlugin *plugin);
void gplugin_plugin_set_desired_state(
	GPluginPlugin *plugin,
	GPluginPluginState state);

GError *gplugin_plugin_get_error(GPluginPlugin *plugin);

const gchar *gplugin_plugin_state_to_string(GPluginPluginState state);

G_END_DECLS

#endif /* GPLUGIN_PLUGIN_H */
