/*
 * Copyright (C) 2011-2022 Gary Kramlich <grim@reaperworld.com>
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

#ifndef GPLUGIN_SOURCE_H
#define GPLUGIN_SOURCE_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GPLUGIN_TYPE_SOURCE (gplugin_source_get_type())
G_DECLARE_INTERFACE(GPluginSource, gplugin_source, GPLUGIN, SOURCE, GObject)

struct _GPluginSourceInterface {
	/*< private >*/
	GTypeInterface parent;

	/*< public >*/
	gboolean (*scan)(GPluginSource *source);

	/*< private >*/
	gpointer reserved[4];
};

gboolean gplugin_source_scan(GPluginSource *source);

G_END_DECLS

#endif /* GPLUGIN_SOURCE_H */
