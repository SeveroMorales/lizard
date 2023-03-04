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

#ifndef GPLUGIN_CORE_H
#define GPLUGIN_CORE_H

#include <glib.h>
#include <glib-object.h>

#define GPLUGIN_DOMAIN (g_quark_from_static_string("gplugin"))

/* clang-format off */
typedef enum /*< flags,prefix=GPLUGIN_CORE_FLAGS,underscore_name=GPLUGIN_CORE >*/ {
	GPLUGIN_CORE_FLAGS_NONE = 0,
	GPLUGIN_CORE_FLAGS_DISABLE_NATIVE_LOADER = 1 << 0,
	GPLUGIN_CORE_FLAGS_LOG_PLUGIN_STATE_CHANGES = 1 << 1,
} GPluginCoreFlags;
/* clang-format on */

G_BEGIN_DECLS

void gplugin_init(GPluginCoreFlags flags);
void gplugin_uninit(void);

GPluginCoreFlags gplugin_get_flags(void);

G_END_DECLS

#endif /* GPLUGIN_CORE_H */
