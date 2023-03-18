/* pidgin
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include <glib/gi18n-lib.h>

#include <gtk/gtk.h>

#include <purple.h>

#include "gtksavedstatuses.h"
#include "pidginiconname.h"

enum {
	SS_MENU_ENTRY_TYPE_PRIMITIVE,
	SS_MENU_ENTRY_TYPE_SAVEDSTATUS
};

enum {
	/* _SSMenuEntryType */
	SS_MENU_TYPE_COLUMN,

	/*
	 * This is a GdkPixbuf (the other columns are strings).
	 * This column is visible.
	 */
	SS_MENU_ICON_COLUMN,

	/* The text displayed on the status box.  This column is visible. */
	SS_MENU_TEXT_COLUMN,

	/*
	 * This value depends on SS_MENU_TYPE_COLUMN.  For _SAVEDSTATUS types,
	 * this is the creation time.  For _PRIMITIVE types,
	 * this is the PurpleStatusPrimitive.
	 */
	SS_MENU_DATA_COLUMN,

	/*
	 * This is the emblem to use for this status
	 */
	SS_MENU_EMBLEM_COLUMN,

	/*
	 * And whether or not that emblem is visible
	 */
	SS_MENU_EMBLEM_VISIBLE_COLUMN,

	SS_MENU_NUM_COLUMNS
};

static void
status_menu_cb(GtkComboBox *widget, void(*callback)(PurpleSavedStatus*))
{
	GtkTreeIter iter;
	int type;
	gpointer data;
	PurpleSavedStatus *status = NULL;

	if (!gtk_combo_box_get_active_iter(widget, &iter))
		return;

	gtk_tree_model_get(gtk_combo_box_get_model(widget), &iter,
			   SS_MENU_TYPE_COLUMN, &type,
			   SS_MENU_DATA_COLUMN, &data,
			   -1);

	if (type == SS_MENU_ENTRY_TYPE_PRIMITIVE)
	{
		PurpleStatusPrimitive primitive = GPOINTER_TO_INT(data);
		status = purple_savedstatus_find_transient_by_type_and_message(primitive, NULL);
		if (status == NULL)
			status = purple_savedstatus_new(NULL, primitive);
	}
	else if (type == SS_MENU_ENTRY_TYPE_SAVEDSTATUS)
		status = purple_savedstatus_find_by_creation_time(GPOINTER_TO_INT(data));

	callback(status);
}

static gint
saved_status_sort_alphabetically_func(gconstpointer a, gconstpointer b)
{
	const PurpleSavedStatus *saved_status_a = a;
	const PurpleSavedStatus *saved_status_b = b;
	return g_utf8_collate(purple_savedstatus_get_title(saved_status_a),
				  purple_savedstatus_get_title(saved_status_b));
}

static gboolean
pidgin_status_menu_add_primitive(GtkListStore *model,
                                 G_GNUC_UNUSED GtkWidget *w,
                                 PurpleStatusPrimitive primitive,
                                 PurpleSavedStatus *current_status)
{
	GtkTreeIter iter;
	gboolean currently_selected = FALSE;

	gtk_list_store_append(model, &iter);
	gtk_list_store_set(model, &iter,
			   SS_MENU_TYPE_COLUMN, SS_MENU_ENTRY_TYPE_PRIMITIVE,
			   SS_MENU_ICON_COLUMN, pidgin_icon_name_from_status_primitive(primitive, NULL),
			   SS_MENU_TEXT_COLUMN, purple_primitive_get_name_from_type(primitive),
			   SS_MENU_DATA_COLUMN, GINT_TO_POINTER(primitive),
			   SS_MENU_EMBLEM_VISIBLE_COLUMN, FALSE,
			   -1);

	if (purple_savedstatus_is_transient(current_status)
			&& !purple_savedstatus_has_substatuses(current_status)
			&& purple_savedstatus_get_primitive_type(current_status) == primitive)
		currently_selected = TRUE;

	return currently_selected;
}

static void
pidgin_status_menu_update_iter(GtkWidget *combobox, GtkListStore *store, GtkTreeIter *iter,
		PurpleSavedStatus *status)
{
	PurpleStatusPrimitive primitive;

	if (store == NULL)
		store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(combobox)));

	primitive = purple_savedstatus_get_primitive_type(status);
	gtk_list_store_set(store, iter,
			SS_MENU_TYPE_COLUMN, SS_MENU_ENTRY_TYPE_SAVEDSTATUS,
			SS_MENU_ICON_COLUMN, pidgin_icon_name_from_status_primitive(primitive, NULL),
			SS_MENU_TEXT_COLUMN, purple_savedstatus_get_title(status),
			SS_MENU_DATA_COLUMN, GINT_TO_POINTER(purple_savedstatus_get_creation_time(status)),
			SS_MENU_EMBLEM_COLUMN, "document-save",
			SS_MENU_EMBLEM_VISIBLE_COLUMN, TRUE,
			-1);
}

static gboolean
pidgin_status_menu_find_iter(GtkListStore *store, GtkTreeIter *iter, PurpleSavedStatus *find)
{
	int type;
	gpointer data;
	time_t creation_time = purple_savedstatus_get_creation_time(find);
	GtkTreeModel *model = GTK_TREE_MODEL(store);

	if (!gtk_tree_model_get_iter_first(model, iter))
		return FALSE;

	do {
		gtk_tree_model_get(model, iter,
				SS_MENU_TYPE_COLUMN, &type,
				SS_MENU_DATA_COLUMN, &data,
				-1);
		if (type == SS_MENU_ENTRY_TYPE_PRIMITIVE)
			continue;
		if (GPOINTER_TO_INT(data) == creation_time)
			return TRUE;
	} while (gtk_tree_model_iter_next(model, iter));

	return FALSE;
}

static void
savedstatus_added_cb(PurpleSavedStatus *status, GtkWidget *combobox)
{
	GtkListStore *store;
	GtkTreeIter iter;

	if (purple_savedstatus_is_transient(status))
		return;

	store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(combobox)));
	gtk_list_store_append(store, &iter);
	pidgin_status_menu_update_iter(combobox, store, &iter, status);
}

static void
savedstatus_deleted_cb(PurpleSavedStatus *status, GtkWidget *combobox)
{
	GtkListStore *store;
	GtkTreeIter iter;

	if (purple_savedstatus_is_transient(status))
		return;

	store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(combobox)));
	if (pidgin_status_menu_find_iter(store, &iter, status))
		gtk_list_store_remove(store, &iter);
}

static void
savedstatus_modified_cb(PurpleSavedStatus *status, GtkWidget *combobox)
{
	GtkListStore *store;
	GtkTreeIter iter;

	if (purple_savedstatus_is_transient(status))
		return;

	store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(combobox)));
	if (pidgin_status_menu_find_iter(store, &iter, status))
		pidgin_status_menu_update_iter(combobox, store, &iter, status);
}

GtkWidget *pidgin_status_menu(PurpleSavedStatus *current_status, GCallback callback)
{
	GtkWidget *combobox;
	GtkListStore *model;
	GList *sorted, *cur;
	int i = 0;
	int index = -1;
	GtkTreeIter iter;
	GtkCellRenderer *text_rend;
	GtkCellRenderer *icon_rend;
	GtkCellRenderer *emblem_rend;

	model = gtk_list_store_new(SS_MENU_NUM_COLUMNS, G_TYPE_INT, G_TYPE_STRING,
				   G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_STRING, G_TYPE_BOOLEAN);

	combobox = gtk_combo_box_new();

	if (pidgin_status_menu_add_primitive(model, combobox, PURPLE_STATUS_AVAILABLE, current_status))
		index = i;
	i++;

	if (pidgin_status_menu_add_primitive(model, combobox, PURPLE_STATUS_AWAY, current_status))
		index = i;
	i++;

	if (pidgin_status_menu_add_primitive(model, combobox, PURPLE_STATUS_INVISIBLE, current_status))
		index = i;
	i++;

	if (pidgin_status_menu_add_primitive(model, combobox, PURPLE_STATUS_OFFLINE, current_status))
		index = i;
	i++;

	sorted = g_list_copy((GList *)purple_savedstatuses_get_all());
	sorted = g_list_sort(sorted, saved_status_sort_alphabetically_func);
	for (cur = sorted; cur; cur = cur->next)
	{
		PurpleSavedStatus *status = (PurpleSavedStatus *) cur->data;
		if (!purple_savedstatus_is_transient(status))
		{
			gtk_list_store_append(model, &iter);

			pidgin_status_menu_update_iter(combobox, model, &iter, status);

			if (status == current_status)
				index = i;
			i++;
		}
	}
	g_list_free(sorted);

	gtk_combo_box_set_model(GTK_COMBO_BOX(combobox), GTK_TREE_MODEL(model));

	text_rend = gtk_cell_renderer_text_new();
	icon_rend = gtk_cell_renderer_pixbuf_new();
	emblem_rend = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), icon_rend, FALSE);
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), text_rend, TRUE);
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), emblem_rend, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combobox), icon_rend, "icon-name", SS_MENU_ICON_COLUMN, NULL);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combobox), text_rend, "markup", SS_MENU_TEXT_COLUMN, NULL);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combobox), emblem_rend,
					"icon-name", SS_MENU_EMBLEM_COLUMN, "visible", SS_MENU_EMBLEM_VISIBLE_COLUMN, NULL);
	g_object_set(text_rend, "ellipsize", PANGO_ELLIPSIZE_END, NULL);

	gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), index);
	g_signal_connect(G_OBJECT(combobox), "changed", G_CALLBACK(status_menu_cb), callback);

	/* Make sure the list is updated dynamically when a substatus is changed/deleted
	 * or a new one is added. */
	purple_signal_connect(purple_savedstatuses_get_handle(), "savedstatus-added",
			combobox, G_CALLBACK(savedstatus_added_cb), combobox);
	purple_signal_connect(purple_savedstatuses_get_handle(), "savedstatus-deleted",
			combobox, G_CALLBACK(savedstatus_deleted_cb), combobox);
	purple_signal_connect(purple_savedstatuses_get_handle(), "savedstatus-modified",
			combobox, G_CALLBACK(savedstatus_modified_cb), combobox);
	g_signal_connect(G_OBJECT(combobox), "destroy",
			G_CALLBACK(purple_signals_disconnect_by_handle), NULL);

	return combobox;
}
