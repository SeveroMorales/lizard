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

#include "gplugin-file-tree.h"

#include <glib/gi18n-lib.h>

/******************************************************************************
 * FileTreeEntry API
 *****************************************************************************/
GPluginFileTreeEntry *
gplugin_file_tree_entry_new(const gchar *filename)
{
	GPluginFileTreeEntry *entry = NULL;

	g_return_val_if_fail(filename, NULL);

	entry = g_slice_new(GPluginFileTreeEntry);

	entry->filename = g_filename_to_utf8(filename, -1, NULL, NULL, NULL);

	/* we have to use e->filename, because g_utf8_strrchr returns a pointer
	 * in the string given too it, and not a new copy.
	 */
	entry->extension =
		g_utf8_strrchr(entry->filename, -1, g_utf8_get_char("."));

	/* if we have an extension, we need to iterate past the . */
	if(entry->extension)
		entry->extension = g_utf8_next_char(entry->extension);

	return entry;
}

void
gplugin_file_tree_entry_free(GPluginFileTreeEntry *entry)
{
	g_return_if_fail(entry);

	g_free(entry->filename);

	g_slice_free(GPluginFileTreeEntry, entry);

	entry = NULL;
}

/******************************************************************************
 * Helpers
 *****************************************************************************/
static gboolean
gplugin_file_tree_free_helper(GNode *n, G_GNUC_UNUSED gpointer d)
{
	GPluginFileTreeEntry *e = n->data;

	if(e)
		gplugin_file_tree_entry_free(e);

	return FALSE;
}

/******************************************************************************
 * FileTree API
 *****************************************************************************/
/**
 * gplugin_file_tree_new:
 * @paths: A #GList containing a list of paths to search.
 *
 * Builds a GNode tree consisting of a root node, whose children contain an
 * allocated string of a plugin directory.  The directory node's children are
 * GPluginFileTreeEntry instance for the files in that directory.
 */
GNode *
gplugin_file_tree_new(GList *paths)
{
	GList *iter = NULL;
	GNode *root = NULL;

	root = g_node_new(NULL);

	for(iter = paths; iter; iter = iter->next) {
		GDir *dir = NULL;
		GError *error = NULL;
		GNode *node_dir = NULL;
		GPluginFileTreeEntry *entry = NULL;
		const gchar *path = (const gchar *)iter->data;
		const gchar *filename = NULL;

		dir = g_dir_open(path, 0, &error);
		if(error) {
			g_debug(
				_("Failed to open %s: %s"),
				path,
				(error->message) ? error->message : _("unknown failure"));

			g_error_free(error);
			error = NULL;

			continue;
		}

		/* we have a usable directory, so prepend it into the tree */
		entry = gplugin_file_tree_entry_new(path);
		node_dir = g_node_new(entry);
		g_node_prepend(root, node_dir);

		/* now run through all of the files and add then to the dir node */
		while((filename = g_dir_read_name(dir)) != NULL) {
			GNode *file = NULL;
			gchar *test_filename = g_build_filename(path, filename, NULL);

			if(g_file_test(test_filename, G_FILE_TEST_IS_REGULAR)) {
				entry = gplugin_file_tree_entry_new(filename);
				file = g_node_new(entry);

				g_node_prepend(node_dir, file);
			}

			g_free(test_filename);
		}

		/* close the dir */
		g_dir_close(dir);
	}

	return root;
}

/**
 * gplugin_file_tree_free:
 * @root: The root of the file tree
 *
 * Free's a previously created file tree.
 */
void
gplugin_file_tree_free(GNode *root)
{
	g_return_if_fail(root);

	g_node_traverse(
		root,
		G_POST_ORDER,
		G_TRAVERSE_LEAVES | G_TRAVERSE_NON_LEAVES,
		-1,
		gplugin_file_tree_free_helper,
		NULL);

	g_node_destroy(root);
}
