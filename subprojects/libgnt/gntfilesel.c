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

#include "gntinternal.h"
#undef GNT_LOG_DOMAIN
#define GNT_LOG_DOMAIN "FileSel"

#include "gntbutton.h"
#include "gntentry.h"
#include "gntfilesel.h"
#include "gntlabel.h"
#include "gntstyle.h"
#include "gnttree.h"

#include "gntwidgetprivate.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glib/gstdio.h>

typedef struct
{
	GntWindow parent;

	GntWidget *dirs;     /* list of files */
	GntWidget *files;    /* list of directories */
	GntWidget *location; /* location entry */

	GntWidget *select; /* select button */
	GntWidget *cancel; /* cancel button */

	char *current; /* Full path of the current location */
	char *suggest; /* Suggested filename */
	/* XXX: someone should make these useful */
	gboolean must_exist; /* Make sure the selected file (the name entered in
	                        'location') exists */
	gboolean dirsonly;   /* Show only directories */
	gboolean multiselect;
	GList *tags; /* List of tagged files when multiselect is set */
} GntFileSelPrivate;

enum
{
	SIG_FILE_SELECTED,
	SIG_CANCELLED,
	SIGS
};

static guint signals[SIGS] = { 0 };
static void (*orig_map)(GntWidget *widget);
static void (*orig_size_request)(GntWidget *widget);

static void select_activated_cb(GntWidget *button, GntFileSel *sel);
static void cancel_activated_cb(GntWidget *button, GntFileSel *sel);

G_DEFINE_TYPE_WITH_PRIVATE(GntFileSel, gnt_file_sel, GNT_TYPE_WINDOW)

static void
gnt_file_sel_destroy(GntWidget *widget)
{
	GntFileSel *sel = GNT_FILE_SEL(widget);
	GntFileSelPrivate *priv = gnt_file_sel_get_instance_private(sel);

	g_clear_pointer(&priv->current, g_free);
	g_clear_pointer(&priv->suggest, g_free);
	if (priv->tags) {
		g_list_free_full(priv->tags, g_free);
		priv->tags = NULL;
	}
}

static char *
process_path(const char *path)
{
	char **splits = NULL;
	int i, j;
	char *str, *ret;

	splits = g_strsplit(path, G_DIR_SEPARATOR_S, -1);
	for (i = 0, j = 0; splits[i]; i++) {
		if (strcmp(splits[i], ".") == 0) {
			g_free(splits[i]);
			splits[i] = NULL;
		} else if (strcmp(splits[i], "..") == 0) {
			if (j)
				j--;
			g_free(splits[i]);
			splits[i] = NULL;
		} else {
			if (i != j) {
				g_free(splits[j]);
				splits[j] = splits[i];
				splits[i] = NULL;
			}
			j++;
		}
	}
	g_free(splits[j]);
	splits[j] = NULL;
	str = g_build_pathv(G_DIR_SEPARATOR_S, splits);
	ret = g_strdup_printf(G_DIR_SEPARATOR_S "%s", str);
	g_free(str);
	g_strfreev(splits);
	return ret;
}

static void
update_location(GntFileSel *sel)
{
	GntFileSelPrivate *priv = gnt_file_sel_get_instance_private(sel);
	char *old;
	const char *tmp;
	tmp = priv->suggest ? priv->suggest
	                    : (const char *)gnt_tree_get_selection_data(
	                              priv->dirsonly ? GNT_TREE(priv->dirs)
	                                             : GNT_TREE(priv->files));
	old = g_strdup_printf("%s%s%s", GNT_KEY_SAFE(priv->current),
	                      GNT_KEY_SAFE(priv->current)[1] ? G_DIR_SEPARATOR_S : "",
	                      tmp ? tmp : "");
	gnt_entry_set_text(GNT_ENTRY(priv->location), old);
	g_free(old);
}

static gboolean
is_tagged(GntFileSel *sel, const char *f)
{
	GntFileSelPrivate *priv = gnt_file_sel_get_instance_private(sel);
	char *ret =
	        g_strdup_printf("%s%s%s", priv->current,
	                        priv->current[1] ? G_DIR_SEPARATOR_S : "", f);
	gboolean find =
	        g_list_find_custom(priv->tags, ret,
	                           (GCompareFunc)g_utf8_collate) != NULL;
	g_free(ret);
	return find;
}

static gboolean
location_changed(GntFileSel *sel, GError **err)
{
	GntFileSelPrivate *priv = gnt_file_sel_get_instance_private(sel);
	GDir *dir;
	const gchar *str;

	if (!priv->dirs) {
		return TRUE;
	}

	gnt_tree_remove_all(GNT_TREE(priv->dirs));
	if (priv->files) {
		gnt_tree_remove_all(GNT_TREE(priv->files));
	}
	gnt_entry_set_text(GNT_ENTRY(priv->location), NULL);
	if (priv->current == NULL) {
		if (gnt_widget_get_mapped(GNT_WIDGET(sel))) {
			gnt_widget_draw(GNT_WIDGET(sel));
		}
		return TRUE;
	}

	/* XXX:\
	 * XXX: This is blocking.
	 * XXX:/
	 */
	dir = g_dir_open(priv->current, 0, err);
	if (dir == NULL || *err) {
		gnt_warning("error opening location %s (%s)", priv->current,
		            *err ? (*err)->message : "reason unknown");
		return FALSE;
	}

	if (*priv->current != '\0' &&
	    g_path_is_absolute(priv->current) &&
	    *g_path_skip_root(priv->current) != '\0') {
		gnt_tree_add_row_after(
		        GNT_TREE(priv->dirs), g_strdup(".."),
		        gnt_tree_create_row(GNT_TREE(priv->dirs), ".."), NULL,
		        NULL);
		if (priv->multiselect && priv->dirsonly &&
		    is_tagged(sel, "..")) {
			gnt_tree_set_row_flags(GNT_TREE(priv->dirs), "..",
			                       GNT_TEXT_FLAG_BOLD);
		}
	}

	while ((str = g_dir_read_name(dir)) != NULL) {
		gchar *fp = g_build_filename(priv->current, str, NULL);
		GStatBuf st;

		if (g_stat(fp, &st)) {
			gnt_warning("Error stating location %s", fp);
		} else {
			/* Just there to silence const warnings; the key is
			 * suitably copied when necessary. */
			gchar *key = (gchar *)str;

			if (S_ISDIR(st.st_mode)) {
				gnt_tree_add_row_after(
				        GNT_TREE(priv->dirs), g_strdup(str),
				        gnt_tree_create_row(
				                GNT_TREE(priv->dirs), key),
				        NULL, NULL);
				if (priv->multiselect && priv->dirsonly &&
				    is_tagged(sel, str)) {
					gnt_tree_set_row_flags(
					        GNT_TREE(priv->dirs), key,
					        GNT_TEXT_FLAG_BOLD);
				}
			} else {
				gchar *size;

				size = g_format_size((guint64)st.st_size);

				gnt_tree_add_row_after(
				        GNT_TREE(priv->files), g_strdup(str),
				        gnt_tree_create_row(
				                GNT_TREE(priv->files), key,
				                size, ""),
				        NULL, NULL);
				if (priv->multiselect && is_tagged(sel, str)) {
					gnt_tree_set_row_flags(
					        GNT_TREE(priv->files), key,
					        GNT_TEXT_FLAG_BOLD);
				}

				g_free(size);
			}
		}
		g_free(fp);
	}
	g_dir_close(dir);

	if (gnt_widget_get_mapped(GNT_WIDGET(sel))) {
		gnt_widget_draw(GNT_WIDGET(sel));
	}
	return TRUE;
}

static gboolean
dir_key_pressed(GntTree *tree, const char *key, GntFileSel *sel)
{
	GntFileSelPrivate *priv = gnt_file_sel_get_instance_private(sel);
	if (strcmp(key, "\r") == 0 || strcmp(key, "\n") == 0) {
		char *str = g_strdup(gnt_tree_get_selection_data(tree));
		char *path, *dir;

		if (!str)
			return TRUE;

		path = g_build_filename(priv->current, str, NULL);
		dir = g_path_get_basename(priv->current);
		if (!gnt_file_sel_set_current_location(sel, path)) {
			gnt_tree_set_selected(tree, str);
		} else if (strcmp(str, "..") == 0) {
			gnt_tree_set_selected(tree, dir);
		}
		gnt_bindable_perform_action_named(GNT_BINDABLE(tree), "end-search", NULL);
		g_free(dir);
		g_free(str);
		g_free(path);
		return TRUE;
	}
	return FALSE;
}

static gboolean
location_key_pressed(G_GNUC_UNUSED GntTree *tree, const char *key,
                     GntFileSel *sel)
{
	GntFileSelPrivate *priv = gnt_file_sel_get_instance_private(sel);
	char *path;
	char *str;

	if (strcmp(key, "\r") && strcmp(key, "\n"))
		return FALSE;

	str = (char *)gnt_entry_get_text(GNT_ENTRY(priv->location));
	if (*str == G_DIR_SEPARATOR)
		path = g_strdup(str);
	else
		path = g_strdup_printf("%s" G_DIR_SEPARATOR_S "%s",
		                       priv->current, str);
	str = process_path(path);
	g_free(path);
	path = str;

	if (gnt_file_sel_set_current_location(sel, path))
		goto success;

	path = g_path_get_dirname(str);
	g_free(str);

	if (!gnt_file_sel_set_current_location(sel, path)) {
		g_free(path);
		return FALSE;
	}

	/* XXX: Add support for globbing via g_pattern_spec_* */

success:
	g_free(path);
	return TRUE;
}

static void
file_sel_changed(GntWidget *widget, G_GNUC_UNUSED gpointer old,
                 G_GNUC_UNUSED gpointer current, GntFileSel *sel)
{
	GntFileSelPrivate *priv = gnt_file_sel_get_instance_private(sel);
	if (gnt_widget_get_has_focus(widget)) {
		g_free(priv->suggest);
		priv->suggest = NULL;
		update_location(sel);
	}
}

static void
gnt_file_sel_map(GntWidget *widget)
{
	GntFileSel *sel = GNT_FILE_SEL(widget);
	GntFileSelPrivate *priv = gnt_file_sel_get_instance_private(sel);
	GntWidget *hbox, *vbox;

	if (priv->current == NULL) {
		gnt_file_sel_set_current_location(sel, g_get_home_dir());
	}

	vbox = gnt_vbox_new(FALSE);
	gnt_box_set_pad(GNT_BOX(vbox), 0);
	gnt_box_set_alignment(GNT_BOX(vbox), GNT_ALIGN_MID);

	/* The dir. and files list */
	hbox = gnt_hbox_new(FALSE);
	gnt_box_set_pad(GNT_BOX(hbox), 0);

	gnt_box_add_widget(GNT_BOX(hbox), priv->dirs);

	if (!priv->dirsonly) {
		gnt_box_add_widget(GNT_BOX(hbox), priv->files);
	} else {
		g_signal_connect(G_OBJECT(priv->dirs), "selection_changed",
		                 G_CALLBACK(file_sel_changed), sel);
	}

	gnt_box_add_widget(GNT_BOX(vbox), hbox);
	gnt_box_add_widget(GNT_BOX(vbox), priv->location);

	/* The buttons */
	hbox = gnt_hbox_new(FALSE);
	gnt_box_add_widget(GNT_BOX(hbox), priv->cancel);
	gnt_box_add_widget(GNT_BOX(hbox), priv->select);
	gnt_box_add_widget(GNT_BOX(vbox), hbox);

	gnt_box_add_widget(GNT_BOX(sel), vbox);
	orig_map(widget);
	update_location(sel);
}

static gboolean
toggle_tag_selection(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntFileSel *sel = GNT_FILE_SEL(bind);
	GntFileSelPrivate *priv = gnt_file_sel_get_instance_private(sel);
	char *str;
	GList *find;
	char *file;
	GntWidget *tree;

	if (!priv->multiselect) {
		return FALSE;
	}
	tree = priv->dirsonly ? priv->dirs : priv->files;
	if (!gnt_widget_has_focus(tree) ||
			gnt_tree_is_searching(GNT_TREE(tree)))
		return FALSE;

	file = gnt_tree_get_selection_data(GNT_TREE(tree));

	str = gnt_file_sel_get_selected_file(sel);
	if ((find = g_list_find_custom(priv->tags, str,
	                               (GCompareFunc)g_utf8_collate)) != NULL) {
		g_free(find->data);
		priv->tags = g_list_delete_link(priv->tags, find);
		gnt_tree_set_row_flags(GNT_TREE(tree), file, GNT_TEXT_FLAG_NORMAL);
		g_free(str);
	} else {
		priv->tags = g_list_prepend(priv->tags, str);
		gnt_tree_set_row_flags(GNT_TREE(tree), file, GNT_TEXT_FLAG_BOLD);
	}

	gnt_bindable_perform_action_named(GNT_BINDABLE(tree), "move-down", NULL);

	return TRUE;
}

static gboolean
clear_tags(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	GntFileSel *sel = GNT_FILE_SEL(bind);
	GntFileSelPrivate *priv = gnt_file_sel_get_instance_private(sel);
	GntWidget *tree;
	GList *iter;

	if (!priv->multiselect) {
		return FALSE;
	}
	tree = priv->dirsonly ? priv->dirs : priv->files;
	if (!gnt_widget_has_focus(tree) ||
			gnt_tree_is_searching(GNT_TREE(tree)))
		return FALSE;

	g_list_free_full(priv->tags, g_free);
	priv->tags = NULL;

	for (iter = gnt_tree_get_rows(GNT_TREE(tree)); iter;
	     iter = iter->next) {
		gnt_tree_set_row_flags(GNT_TREE(tree), iter->data, GNT_TEXT_FLAG_NORMAL);
	}

	return TRUE;
}

static gboolean
up_directory(GntBindable *bind, G_GNUC_UNUSED GList *params)
{
	char *path, *dir;
	GntFileSel *sel = GNT_FILE_SEL(bind);
	GntFileSelPrivate *priv = gnt_file_sel_get_instance_private(sel);
	if (!gnt_widget_has_focus(priv->dirs) &&
	    !gnt_widget_has_focus(priv->files)) {
		return FALSE;
	}
	if (gnt_tree_is_searching(GNT_TREE(priv->dirs)) ||
	    gnt_tree_is_searching(GNT_TREE(priv->files))) {
		return FALSE;
	}

	path = g_build_filename(priv->current, "..", NULL);
	dir = g_path_get_basename(priv->current);
	if (gnt_file_sel_set_current_location(sel, path)) {
		gnt_tree_set_selected(GNT_TREE(priv->dirs), dir);
	}
	g_free(dir);
	g_free(path);
	return TRUE;
}

static void
gnt_file_sel_size_request(GntWidget *widget)
{
	GntFileSelPrivate *priv = NULL;
	gint width, height;

	gnt_widget_get_internal_size(widget, NULL, &height);
	if (height > 0) {
		return;
	}

	priv = gnt_file_sel_get_instance_private(GNT_FILE_SEL(widget));

	gnt_widget_get_internal_size(priv->dirs, &width, NULL);
	gnt_widget_set_internal_size(priv->dirs, width, 16);

	gnt_widget_get_internal_size(priv->files, &width, NULL);
	gnt_widget_set_internal_size(priv->files, width, 16);

	orig_size_request(widget);
}

static void
gnt_file_sel_class_init(GntFileSelClass *klass)
{
	GntBindableClass *bindable = GNT_BINDABLE_CLASS(klass);
	GntWidgetClass *widget_class = GNT_WIDGET_CLASS(klass);

	widget_class->destroy = gnt_file_sel_destroy;
	orig_map = widget_class->map;
	widget_class->map = gnt_file_sel_map;
	orig_size_request = widget_class->size_request;
	widget_class->size_request = gnt_file_sel_size_request;

	/**
	 * GntFileSel::file-selected:
	 * @widget: The file selection window that received the signal
	 * @path: The full path to the selected file
	 * @file: The name of the file only
	 *
	 * The ::file-selected signal is emitted when the Select button or the
	 * file tree on a #GntFileSel is activated.
	 *
	 * Since: 2.1.0
	 */
	signals[SIG_FILE_SELECTED] =
		g_signal_new("file_selected",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET(GntFileSelClass, file_selected),
					 NULL, NULL, NULL,
					 G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

	/**
	 * GntFileSel::cancelled:
	 * @widget: The file selection window that received the signal
	 *
	 * The ::cancelled signal is emitted when the Cancel button on a
	 * #GntFileSel is activated.
	 *
	 * Since: 2.14.0
	 */
	signals[SIG_CANCELLED] =
		g_signal_new("cancelled",
				G_TYPE_FROM_CLASS(klass),
				G_SIGNAL_RUN_LAST,
				G_STRUCT_OFFSET(GntFileSelClass, cancelled),
				NULL, NULL,
				g_cclosure_marshal_VOID__VOID,
				G_TYPE_NONE, 0);

	gnt_bindable_class_register_action(bindable, "toggle-tag", toggle_tag_selection, "t", NULL);
	gnt_bindable_class_register_action(bindable, "clear-tags", clear_tags, "c", NULL);
	gnt_bindable_class_register_action(bindable, "up-directory", up_directory, GNT_KEY_BACKSPACE, NULL);
	gnt_style_read_actions(G_OBJECT_CLASS_TYPE(klass), GNT_BINDABLE_CLASS(klass));
}

static void
gnt_file_sel_init(GntFileSel *sel)
{
	GntFileSelPrivate *priv = gnt_file_sel_get_instance_private(sel);
	priv->dirs = gnt_tree_new();
	gnt_tree_set_compare_func(GNT_TREE(priv->dirs),
	                          (GCompareFunc)g_utf8_collate);
	gnt_tree_set_hash_fns(GNT_TREE(priv->dirs), g_str_hash, g_str_equal,
	                      g_free);
	gnt_tree_set_column_titles(GNT_TREE(priv->dirs), "Directories");
	gnt_tree_set_show_title(GNT_TREE(priv->dirs), TRUE);
	gnt_tree_set_col_width(GNT_TREE(priv->dirs), 0, 20);
	g_signal_connect(G_OBJECT(priv->dirs), "key_pressed",
	                 G_CALLBACK(dir_key_pressed), sel);

	priv->files = gnt_tree_new_with_columns(2); /* Name, Size */
	gnt_tree_set_compare_func(GNT_TREE(priv->files),
	                          (GCompareFunc)g_utf8_collate);
	gnt_tree_set_hash_fns(GNT_TREE(priv->files), g_str_hash, g_str_equal,
	                      g_free);
	gnt_tree_set_column_titles(GNT_TREE(priv->files), "Filename", "Size");
	gnt_tree_set_show_title(GNT_TREE(priv->files), TRUE);
	gnt_tree_set_col_width(GNT_TREE(priv->files), 0, 25);
	gnt_tree_set_col_width(GNT_TREE(priv->files), 1, 10);
	gnt_tree_set_column_is_right_aligned(GNT_TREE(priv->files), 1, TRUE);
	g_signal_connect(G_OBJECT(priv->files), "selection_changed",
	                 G_CALLBACK(file_sel_changed), sel);

	/* The location entry */
	priv->location = gnt_entry_new(NULL);
	g_signal_connect(G_OBJECT(priv->location), "key_pressed",
	                 G_CALLBACK(location_key_pressed), sel);

	priv->cancel = gnt_button_new("Cancel");
	g_signal_connect(G_OBJECT(priv->cancel), "activate",
	                 G_CALLBACK(cancel_activated_cb), sel);

	priv->select = gnt_button_new("Select");

	g_signal_connect_swapped(G_OBJECT(priv->files), "activate",
	                         G_CALLBACK(gnt_widget_activate), priv->select);
	g_signal_connect(G_OBJECT(priv->select), "activate",
	                 G_CALLBACK(select_activated_cb), sel);
}

/******************************************************************************
 * GntFileSel API
 *****************************************************************************/
static void
select_activated_cb(G_GNUC_UNUSED GntWidget *button, GntFileSel *sel)
{
	char *path = gnt_file_sel_get_selected_file(sel);
	char *file = g_path_get_basename(path);
	g_signal_emit(sel, signals[SIG_FILE_SELECTED], 0, path, file);
	g_free(file);
	g_free(path);
}

static void
cancel_activated_cb(G_GNUC_UNUSED GntWidget *button, GntFileSel *sel)
{
	g_signal_emit(sel, signals[SIG_CANCELLED], 0);
}

GntWidget *gnt_file_sel_new(void)
{
	GntWidget *widget = g_object_new(GNT_TYPE_FILE_SEL, NULL);
	return widget;
}

gboolean gnt_file_sel_set_current_location(GntFileSel *sel, const char *path)
{
	GntFileSelPrivate *priv = NULL;
	char *old;
	GError *error = NULL;
	gboolean ret = TRUE;

	g_return_val_if_fail(GNT_IS_FILE_SEL(sel), FALSE);
	priv = gnt_file_sel_get_instance_private(sel);

	old = priv->current;
	priv->current = process_path(path);
	if (!location_changed(sel, &error)) {
		g_error_free(error);
		error = NULL;
		g_free(priv->current);
		priv->current = old;
		location_changed(sel, &error);
		ret = FALSE;
	} else
		g_free(old);

	update_location(sel);
	return ret;
}

void gnt_file_sel_set_dirs_only(GntFileSel *sel, gboolean dirs)
{
	GntFileSelPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_FILE_SEL(sel));
	priv = gnt_file_sel_get_instance_private(sel);

	priv->dirsonly = dirs;
}

gboolean gnt_file_sel_get_dirs_only(GntFileSel *sel)
{
	GntFileSelPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_FILE_SEL(sel), FALSE);
	priv = gnt_file_sel_get_instance_private(sel);

	return priv->dirsonly;
}

void gnt_file_sel_set_suggested_filename(GntFileSel *sel, const char *suggest)
{
	GntFileSelPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_FILE_SEL(sel));
	priv = gnt_file_sel_get_instance_private(sel);

	g_free(priv->suggest);
	priv->suggest = g_strdup(suggest);
}

char *gnt_file_sel_get_selected_file(GntFileSel *sel)
{
	GntFileSelPrivate *priv = NULL;
	char *ret;

	g_return_val_if_fail(GNT_IS_FILE_SEL(sel), NULL);
	priv = gnt_file_sel_get_instance_private(sel);

	if (priv->dirsonly) {
		ret = g_path_get_dirname(
		        gnt_entry_get_text(GNT_ENTRY(priv->location)));
	} else {
		ret = g_strdup(gnt_entry_get_text(GNT_ENTRY(priv->location)));
	}
	return ret;
}

void gnt_file_sel_set_must_exist(GntFileSel *sel, gboolean must)
{
	GntFileSelPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_FILE_SEL(sel));
	priv = gnt_file_sel_get_instance_private(sel);

	/*XXX: What do I do with this? */
	priv->must_exist = must;
}

gboolean gnt_file_sel_get_must_exist(GntFileSel *sel)
{
	GntFileSelPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_FILE_SEL(sel), FALSE);
	priv = gnt_file_sel_get_instance_private(sel);

	return priv->must_exist;
}

void gnt_file_sel_set_multi_select(GntFileSel *sel, gboolean set)
{
	GntFileSelPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_FILE_SEL(sel));
	priv = gnt_file_sel_get_instance_private(sel);

	priv->multiselect = set;
}

GList *gnt_file_sel_get_selected_multi_files(GntFileSel *sel)
{
	GntFileSelPrivate *priv = NULL;
	GList *list = NULL, *iter;
	char *str = NULL;

	g_return_val_if_fail(GNT_IS_FILE_SEL(sel), NULL);
	priv = gnt_file_sel_get_instance_private(sel);

	str = gnt_file_sel_get_selected_file(sel);

	for (iter = priv->tags; iter; iter = iter->next) {
		list = g_list_prepend(list, g_strdup(iter->data));
		if (g_utf8_collate(str, iter->data)) {
			g_free(str);
			str = NULL;
		}
	}
	if (str)
		list = g_list_prepend(list, str);
	list = g_list_reverse(list);
	return list;
}
