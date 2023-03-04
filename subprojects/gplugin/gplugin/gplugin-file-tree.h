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

#ifndef GPLUGIN_FILE_TREE_H
#define GPLUGIN_FILE_TREE_H

#include <glib.h>

typedef struct {
	gchar *filename;
	gchar *extension;
} GPluginFileTreeEntry;

G_BEGIN_DECLS

GNode *gplugin_file_tree_new(GList *paths);
void gplugin_file_tree_free(GNode *root);

G_END_DECLS

#endif /* GPLUGIN_FILE_TREE_H */
