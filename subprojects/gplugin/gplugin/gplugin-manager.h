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

#if !defined(GPLUGIN_GLOBAL_HEADER_INSIDE) && !defined(GPLUGIN_COMPILATION)
#error "only <gplugin.h> may be included directly"
#endif

#ifndef GPLUGIN_MANAGER_H
#define GPLUGIN_MANAGER_H

#include <glib.h>
#include <glib-object.h>

#include <gplugin/gplugin-plugin.h>

G_BEGIN_DECLS

#define GPLUGIN_TYPE_MANAGER (gplugin_manager_get_type())
G_DECLARE_FINAL_TYPE(GPluginManager, gplugin_manager, GPLUGIN, MANAGER, GObject)

typedef void (*GPluginManagerForeachFunc)(
	const gchar *id,
	GSList *plugins,
	gpointer data);

void gplugin_manager_append_path(GPluginManager *manager, const gchar *path);
void gplugin_manager_prepend_path(GPluginManager *manager, const gchar *path);
void gplugin_manager_remove_path(GPluginManager *manager, const gchar *path);
void gplugin_manager_remove_paths(GPluginManager *manager);

void gplugin_manager_add_default_paths(GPluginManager *manager);
void gplugin_manager_add_app_paths(
	GPluginManager *manager,
	const gchar *prefix,
	const gchar *appname);
void gplugin_manager_append_paths_from_environment(
	GPluginManager *manager,
	const gchar *name);
void gplugin_manager_prepend_paths_from_environment(
	GPluginManager *manager,
	const gchar *name);

GList *gplugin_manager_get_paths(GPluginManager *manager);

gboolean gplugin_manager_register_loader(
	GPluginManager *manager,
	GPluginLoader *loader,
	GError **error);
gboolean gplugin_manager_unregister_loader(
	GPluginManager *manager,
	GPluginLoader *loader,
	GError **error);
GList *gplugin_manager_get_loaders(GPluginManager *manager);

void gplugin_manager_refresh(GPluginManager *manager);

void gplugin_manager_foreach(
	GPluginManager *manager,
	GPluginManagerForeachFunc func,
	gpointer data);

GSList *gplugin_manager_find_plugins(GPluginManager *manager, const gchar *id);
GSList *gplugin_manager_find_plugins_with_version(
	GPluginManager *manager,
	const gchar *id,
	const gchar *op,
	const gchar *version);
GSList *gplugin_manager_find_plugins_with_state(
	GPluginManager *manager,
	GPluginPluginState state);

GPluginPlugin *gplugin_manager_find_plugin(
	GPluginManager *manager,
	const gchar *id);
GPluginPlugin *gplugin_manager_find_plugin_with_newest_version(
	GPluginManager *manager,
	const gchar *id);

GSList *gplugin_manager_get_plugin_dependencies(
	GPluginManager *manager,
	GPluginPlugin *plugin,
	GError **error);

gboolean gplugin_manager_load_plugin(
	GPluginManager *manager,
	GPluginPlugin *plugin,
	GError **error);
gboolean gplugin_manager_unload_plugin(
	GPluginManager *manager,
	GPluginPlugin *plugin,
	GError **error);

GList *gplugin_manager_list_plugins(GPluginManager *manager);

GPluginManager *gplugin_manager_get_default(void);

G_END_DECLS

#endif /* GPLUGIN_MANAGER_H */
