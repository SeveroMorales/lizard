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

#ifndef GPLUGIN_PLUGIN_INFO_H
#define GPLUGIN_PLUGIN_INFO_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GPLUGIN_TYPE_PLUGIN_INFO (gplugin_plugin_info_get_type())
G_DECLARE_DERIVABLE_TYPE(
	GPluginPluginInfo,
	gplugin_plugin_info,
	GPLUGIN,
	PLUGIN_INFO,
	GObject)

#include <gplugin/gplugin-loader.h>
#include <gplugin/gplugin-version.h>

struct _GPluginPluginInfoClass {
	/*< private >*/
	GObjectClass parent;

	gpointer reserved[4];
};

/* clang-format off */
#define gplugin_plugin_info_new(id, abi_version, ...) \
	GPLUGIN_PLUGIN_INFO(g_object_new( \
		GPLUGIN_TYPE_PLUGIN_INFO, \
		"id", (id), \
		"abi-version", (abi_version), \
		__VA_ARGS__))
/* clang-format on */

const gchar *gplugin_plugin_info_get_id(GPluginPluginInfo *info);
gchar *gplugin_plugin_info_get_id_normalized(GPluginPluginInfo *info);
const gchar *const *gplugin_plugin_info_get_provides(GPluginPluginInfo *info);
gint gplugin_plugin_info_get_priority(GPluginPluginInfo *info);
guint32 gplugin_plugin_info_get_abi_version(GPluginPluginInfo *info);
gboolean gplugin_plugin_info_get_internal(GPluginPluginInfo *info);
gboolean gplugin_plugin_info_get_auto_load(GPluginPluginInfo *info);

G_DEPRECATED_FOR(gplugin_plugin_info_get_auto_load)
gboolean gplugin_plugin_info_get_load_on_query(GPluginPluginInfo *info);

const gchar *gplugin_plugin_info_get_name(GPluginPluginInfo *info);
const gchar *gplugin_plugin_info_get_version(GPluginPluginInfo *info);
const gchar *gplugin_plugin_info_get_settings_schema(GPluginPluginInfo *info);
const gchar *gplugin_plugin_info_get_license_id(GPluginPluginInfo *info);
const gchar *gplugin_plugin_info_get_license_text(GPluginPluginInfo *info);
const gchar *gplugin_plugin_info_get_license_url(GPluginPluginInfo *info);
const gchar *gplugin_plugin_info_get_icon_name(GPluginPluginInfo *info);
const gchar *gplugin_plugin_info_get_summary(GPluginPluginInfo *info);
const gchar *gplugin_plugin_info_get_description(GPluginPluginInfo *info);
const gchar *gplugin_plugin_info_get_category(GPluginPluginInfo *info);
const gchar *const *gplugin_plugin_info_get_authors(GPluginPluginInfo *info);
const gchar *gplugin_plugin_info_get_website(GPluginPluginInfo *info);
const gchar *const *gplugin_plugin_info_get_dependencies(
	GPluginPluginInfo *info);
gboolean gplugin_plugin_info_get_bind_global(GPluginPluginInfo *info);
gboolean gplugin_plugin_info_get_unloadable(GPluginPluginInfo *info);

G_END_DECLS

#endif /* GPLUGIN_PLUGIN_INFO_H */
