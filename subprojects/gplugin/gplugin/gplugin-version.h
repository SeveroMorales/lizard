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

#ifndef GPLUGIN_VERSION_H
#define GPLUGIN_VERSION_H

#include <gplugin/gplugin-version-defs.h>

#define GPLUGIN_VERSION_CHECK(major, minor, micro) \
	((major) == GPLUGIN_MAJOR_VERSION && \
	 ((minor) < GPLUGIN_MINOR_VERSION || \
	  ((minor) == GPLUGIN_MINOR_VERSION && (micro) <= GPLUGIN_MICRO_VERSION)))

#include <glib.h>

G_BEGIN_DECLS

const gchar *gplugin_version_check(guint major, guint minor, guint micro);

gint gplugin_version_compare(const gchar *v1, const gchar *v2);

G_END_DECLS

#endif /* GPLUGIN_VERSION_H */
