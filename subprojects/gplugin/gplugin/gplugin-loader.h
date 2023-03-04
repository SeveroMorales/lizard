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

#ifndef GPLUGIN_LOADER_H
#define GPLUGIN_LOADER_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GPLUGIN_TYPE_LOADER (gplugin_loader_get_type())
G_DECLARE_DERIVABLE_TYPE(
	GPluginLoader,
	gplugin_loader,
	GPLUGIN,
	LOADER,
	GObject)

/* circular include so this needs to be after the G_DECLARE_* marcro */
#include <gplugin/gplugin-plugin.h>

struct _GPluginLoaderClass {
	/*< private >*/
	GObjectClass gparent;

	/*< public >*/
	GSList *(*supported_extensions)(GPluginLoader *loader);

	GPluginPlugin *(
		*query)(GPluginLoader *loader, const gchar *filename, GError **error);

	gboolean (
		*load)(GPluginLoader *loader, GPluginPlugin *plugin, GError **error);
	gboolean (*unload)(
		GPluginLoader *loader,
		GPluginPlugin *plugin,
		gboolean shutdown,
		GError **error);

	/*< private >*/
	gpointer reserved[4];
};

const gchar *gplugin_loader_get_id(GPluginLoader *loader);

GSList *gplugin_loader_get_supported_extensions(GPluginLoader *loader);

GPluginPlugin *gplugin_loader_query_plugin(
	GPluginLoader *loader,
	const gchar *filename,
	GError **error);

gboolean gplugin_loader_load_plugin(
	GPluginLoader *loader,
	GPluginPlugin *plugin,
	GError **error);
gboolean gplugin_loader_unload_plugin(
	GPluginLoader *loader,
	GPluginPlugin *plugin,
	gboolean shutdown,
	GError **error);

G_END_DECLS

#endif /* GPLUGIN_LOADER_H */
