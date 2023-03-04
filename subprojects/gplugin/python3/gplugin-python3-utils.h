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

#ifndef GPLUGIN_PYTHON3_UTILS_H
#define GPLUGIN_PYTHON3_UTILS_H

#include <glib.h>

G_BEGIN_DECLS

gchar *gplugin_python3_filename_to_module(const gchar *filename);

gboolean gplugin_python3_add_module_path(const gchar *module_path);

GError *gplugin_python3_exception_to_gerror(void);

G_END_DECLS

#endif /* GPLUGIN_PYTHON3_UTILS_H */
