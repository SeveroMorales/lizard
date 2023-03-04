/*
 * GNT - The GLib Ncurses Toolkit
 *
 * GNT is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(GNT_GLOBAL_HEADER_INSIDE) && !defined(GNT_COMPILATION)
# error "only <gnt.h> may be included directly"
#endif

#ifndef GNT_FILE_SEL_H
#define GNT_FILE_SEL_H

#include "gntcolors.h"
#include "gntkeys.h"
#include "gntwindow.h"

#define GNT_TYPE_FILE_SEL gnt_file_sel_get_type()

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(GntFileSel, gnt_file_sel, GNT, FILE_SEL, GntWindow)

/**
 * GntFileSelClass:
 * @file_selected: The class closure for the #GntFileSel::file-selected signal.
 * @cancelled: The class closure for the #GntFileSel::cancelled signal.
 *
 * The class structure for #GntFileSel.
 */
struct _GntFileSelClass
{
	/*< private >*/
	GntWindowClass parent;

	/*< public >*/
	void (*file_selected)(GntFileSel *sel, const char *path,
	                      const char *filename);
	void (*cancelled)(GntFileSel *sel);

	/*< private >*/
	gpointer reserved[4];
};

/**
 * gnt_file_sel_new:
 *
 * Create a new file selector.
 *
 * Returns:  The newly created file selector.
 */
GntWidget * gnt_file_sel_new(void);

/**
 * gnt_file_sel_set_current_location:
 * @sel:   The file selector.
 * @path:  The current path of the selector.
 *
 * Set the current location of the file selector.
 *
 * Returns: %TRUE if the current location was successfully changed, %FALSE otherwise.
 */
gboolean gnt_file_sel_set_current_location(GntFileSel *sel, const char *path);

/**
 * gnt_file_sel_set_dirs_only:
 * @sel:    The file selector.
 * @dirs:   %TRUE if only directories can be selected, %FALSE if files
 *               can also be selected.
 *
 * Set whether to only allow selecting directories.
 */
void gnt_file_sel_set_dirs_only(GntFileSel *sel, gboolean dirs);

/**
 * gnt_file_sel_get_dirs_only:
 * @sel:  The file selector.
 *
 * Check whether the file selector allows only selecting directories.
 *
 * Returns:  %TRUE if only directories can be selected.
 */
gboolean gnt_file_sel_get_dirs_only(GntFileSel *sel);

/**
 * gnt_file_sel_set_must_exist:
 * @sel:   The file selector.
 * @must:  %TRUE if the selected file must exist.
 *
 * Set whether a selected file must exist.
 */
void gnt_file_sel_set_must_exist(GntFileSel *sel, gboolean must);

/**
 * gnt_file_sel_get_must_exist:
 * @sel:  The file selector.
 *
 * Check whether the selector allows selecting non-existent files.
 *
 * Returns:  %TRUE if the selected file must exist, %FALSE if a non-existent
 *          file can be selected.
 */
gboolean gnt_file_sel_get_must_exist(GntFileSel *sel);

/**
 * gnt_file_sel_get_selected_file:
 * @sel:  The file selector.
 *
 * Get the selected file in the selector.
 *
 * Returns: The path of the selected file. The caller should g_free the returned
 *         string.
 */
char * gnt_file_sel_get_selected_file(GntFileSel *sel);

/**
 * gnt_file_sel_get_selected_multi_files:
 * @sel:  The file selector.
 *
 * Get the list of selected files in the selector.
 *
 * Returns: (transfer full) (element-type filename): A list of paths for the
 *          selected files. The caller must g_free() the contents of the list,
 *          and g_list_free() the list.
 */
GList * gnt_file_sel_get_selected_multi_files(GntFileSel *sel);

/**
 * gnt_file_sel_set_multi_select:
 * @sel:  The file selector.
 * @set:  %TRUE if selecting multiple files should be allowed.
 *
 * Allow selecting multiple files.
 */
void gnt_file_sel_set_multi_select(GntFileSel *sel, gboolean set);

/**
 * gnt_file_sel_set_suggested_filename:
 * @sel:      The file selector.
 * @suggest:  The suggested filename.
 *
 * Set the suggested file to have selected at startup.
 */
void gnt_file_sel_set_suggested_filename(GntFileSel *sel, const char *suggest);

G_END_DECLS

#endif /* GNT_FILE_SEL_H */

