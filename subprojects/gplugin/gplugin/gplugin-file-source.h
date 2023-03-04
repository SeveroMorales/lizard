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

#ifndef GPLUGIN_FILE_SOURCE_H
#define GPLUGIN_FILE_SOURCE_H

#include <glib.h>
#include <glib-object.h>

#include <gplugin/gplugin-manager.h>
#include <gplugin/gplugin-source.h>

G_BEGIN_DECLS

#define GPLUGIN_TYPE_FILE_SOURCE (gplugin_file_source_get_type())
G_DECLARE_FINAL_TYPE(
	GPluginFileSource,
	gplugin_file_source,
	GPLUGIN,
	FILE_SOURCE,
	GObject)

GPluginSource *gplugin_file_source_new(GPluginManager *manager);

G_END_DECLS

#endif /* GPLUGIN_FILE_SOURCE_H */
