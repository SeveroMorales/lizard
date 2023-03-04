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

#ifndef GPLUGIN_LOADER_TESTS_H
#define GPLUGIN_LOADER_TESTS_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

void gplugin_loader_tests_add_tests(const gchar *short_name);
void gplugin_loader_tests_main(
	const gchar *loader_dir,
	const gchar *plugin_dir,
	const gchar *short_name);

G_END_DECLS

#endif /* GPLUGIN_OPTIONS_H */
