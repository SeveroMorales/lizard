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
#error "only <gplugin.h> or <gplugin-native.h> may be included directly"
#endif

#ifndef GPLUGIN_NATIVE_PLUGIN_H
#define GPLUGIN_NATIVE_PLUGIN_H

#include <gmodule.h>

#include <gplugin/gplugin-plugin-info.h>
#include <gplugin/gplugin-plugin.h>

G_BEGIN_DECLS

#define GPLUGIN_TYPE_NATIVE_PLUGIN (gplugin_native_plugin_get_type())
G_DECLARE_FINAL_TYPE(
	GPluginNativePlugin,
	gplugin_native_plugin,
	GPLUGIN,
	NATIVE_PLUGIN,
	GTypeModule)

/* clang-format formats these in a way that breaks gtk-doc, so we have to
 * leave them as is.
 */

/* clang-format off */
typedef GPluginPluginInfo *(*GPluginNativePluginQueryFunc)(GError **error);
typedef gboolean (*GPluginNativePluginLoadFunc)(GPluginPlugin *plugin, GError **error);
typedef gboolean (*GPluginNativePluginUnloadFunc)(GPluginPlugin *plugin, gboolean shutdown, GError **error);
/* clang-format on */

GModule *gplugin_native_plugin_get_module(GPluginNativePlugin *plugin);

#define GPLUGIN_NATIVE_PLUGIN_DECLARE(name) \
	G_MODULE_EXPORT GPluginPluginInfo *gplugin_query(GError **error); \
	G_MODULE_EXPORT GPluginPluginInfo *gplugin_query(GError **error) \
	{ \
		return name##_query(error); \
	} \
\
	G_MODULE_EXPORT gboolean gplugin_load( \
		GPluginPlugin *plugin, \
		GError **error); \
	G_MODULE_EXPORT gboolean gplugin_load( \
		GPluginPlugin *plugin, \
		GError **error) \
	{ \
		return name##_load(plugin, error); \
	} \
\
	G_MODULE_EXPORT gboolean \
	gplugin_unload(GPluginPlugin *plugin, gboolean shutdown, GError **error); \
	G_MODULE_EXPORT gboolean \
	gplugin_unload(GPluginPlugin *plugin, gboolean shutdown, GError **error) \
	{ \
		return name##_unload(plugin, shutdown, error); \
	}

G_END_DECLS

#endif /* GPLUGIN_NATIVE_PLUGIN_H */
