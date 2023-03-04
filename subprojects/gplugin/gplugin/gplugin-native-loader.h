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
#error "only <gplugin.h> or <gplugin-native.h> may be included directly"
#endif

#ifndef GPLUGIN_NATIVE_LOADER_H
#define GPLUGIN_NATIVE_LOADER_H

#include <glib.h>
#include <glib-object.h>

#include <gplugin/gplugin-loader.h>

#define GPLUGIN_NATIVE_PLUGIN_ABI_VERSION 0x01000000

G_BEGIN_DECLS

#define GPLUGIN_TYPE_NATIVE_LOADER (gplugin_native_loader_get_type())
G_DECLARE_FINAL_TYPE(
	GPluginNativeLoader,
	gplugin_native_loader,
	GPLUGIN,
	NATIVE_LOADER,
	GPluginLoader)

GPluginLoader *gplugin_native_loader_new(void);

G_END_DECLS

#endif /* GPLUGIN_NATIVE_LOADER_H */
