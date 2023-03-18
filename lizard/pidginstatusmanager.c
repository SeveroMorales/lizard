/*
 * Pidgin - Internet Messenger
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "pidginstatusmanager.h"

#include "pidginiconname.h"
#include "pidginstatuseditor.h"

enum {
	RESPONSE_USE,
	RESPONSE_ADD,
	RESPONSE_MODIFY,
	RESPONSE_REMOVE
};

enum {
	COLUMN_TITLE,
	COLUMN_ICON_NAME,
	COLUMN_TYPE,
	COLUMN_MESSAGE,
	COLUMN_STATUS,
	COLUMN_EDITOR
};

struct _PidginStatusManager {
	GtkDialog parent;

	GListStore *model;
	GtkSingleSelection *selection;

	GtkWidget *use_button;
	GtkWidget *modify_button;
	GtkWidget *remove_button;
};

G_DEFINE_TYPE(PidginStatusManager, pidgin_status_manager, GTK_TYPE_DIALOG)

/* Ugh, prototypes :,( */
static void pidgin_status_editor_destroy_cb(GtkWidget *widget, gpointer data);

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_status_manager_show_editor(PidginStatusManager *manager) {
	GObject *wrapper = NULL;
	PurpleSavedStatus *status = NULL;
	GtkWidget *editor = NULL;

	wrapper = gtk_single_selection_get_selected_item(manager->selection);
	status = g_object_get_data(wrapper, "savedstatus");
	editor = g_object_get_data(wrapper, "editor");

	if(status == NULL) {
		return;
	}

	if(!PIDGIN_IS_STATUS_EDITOR(editor)) {
		editor = pidgin_status_editor_new(status);

		gtk_window_set_transient_for(GTK_WINDOW(editor), GTK_WINDOW(manager));

		g_object_set_data(wrapper, "editor", editor);
		g_signal_connect_object(editor, "destroy",
		                        G_CALLBACK(pidgin_status_editor_destroy_cb),
		                        manager, 0);

		gtk_widget_show(editor);
	} else {
		gtk_window_present_with_time(GTK_WINDOW(editor), GDK_CURRENT_TIME);
	}
}

static void
pidgin_status_manager_remove_selected(PidginStatusManager *manager) {
	GObject *wrapper = NULL;
	PurpleSavedStatus *status = NULL;
	GtkWidget *editor = NULL;

	wrapper = gtk_single_selection_get_selected_item(manager->selection);
	status = g_object_get_data(wrapper, "savedstatus");
	editor = g_object_get_data(wrapper, "editor");

	if(GTK_IS_WIDGET(editor)) {
		gtk_window_destroy(GTK_WINDOW(editor));
	}

	purple_savedstatus_delete_by_status(status);
}

static PurpleSavedStatus *
pidgin_status_manager_get_selected_status(PidginStatusManager *manager) {
	GObject *wrapper = NULL;
	PurpleSavedStatus *status = NULL;

	wrapper = gtk_single_selection_get_selected_item(manager->selection);
	status = g_object_get_data(wrapper, "savedstatus");

	return status;
}

static void
pidgin_status_manager_add(PidginStatusManager *manager,
                          PurpleSavedStatus *status)
{
	GObject *wrapper = NULL;
	PurpleStatusPrimitive primitive;
	gchar *message = NULL;
	const gchar *icon_name = NULL, *type = NULL;

	message = purple_markup_strip_html(purple_savedstatus_get_message(status));

	primitive = purple_savedstatus_get_primitive_type(status);
	icon_name = pidgin_icon_name_from_status_primitive(primitive, NULL);
	type = purple_primitive_get_name_from_type(primitive);

	/* PurpleSavedStatus is a boxed type, so it can't be put in a GListModel;
	 * instead create a wrapper GObject instance to hold its information. */
	wrapper = g_object_new(G_TYPE_OBJECT, NULL);
	g_object_set_data(wrapper, "savedstatus", status);
	g_object_set_data_full(wrapper, "title",
	                       g_strdup(purple_savedstatus_get_title(status)),
	                       g_free);
	g_object_set_data_full(wrapper, "icon-name", g_strdup(icon_name), g_free);
	g_object_set_data_full(wrapper, "type", g_strdup(type), g_free);
	g_object_set_data_full(wrapper, "message", g_strdup(message), g_free);

	g_list_store_append(manager->model, wrapper);

	g_free(message);
	g_object_unref(wrapper);
}

static void
pidgin_status_manager_populate_helper(gpointer data, gpointer user_data) {
	PidginStatusManager *manager = user_data;
	PurpleSavedStatus *status = data;

	if(!purple_savedstatus_is_transient(status)) {
		pidgin_status_manager_add(manager, status);
	}
}

static void
pidgin_status_manager_refresh(PidginStatusManager *manager) {
	GList *statuses = NULL;

	g_list_store_remove_all(manager->model);

	statuses = purple_savedstatuses_get_all();
	g_list_foreach(statuses, pidgin_status_manager_populate_helper, manager);
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_status_manager_response_cb(GtkDialog *dialog, gint response_id,
                                  gpointer data)
{
	PidginStatusManager *manager = data;
	PurpleSavedStatus *status = NULL;
	GtkWidget *editor = NULL;

	switch(response_id) {
		case RESPONSE_USE:
			status = pidgin_status_manager_get_selected_status(manager);

			purple_savedstatus_activate(status);

			break;
		case RESPONSE_ADD:
			editor = pidgin_status_editor_new(NULL);
			gtk_window_set_transient_for(GTK_WINDOW(editor),
			                             GTK_WINDOW(manager));
			gtk_widget_show(editor);
			break;
		case RESPONSE_MODIFY:
			pidgin_status_manager_show_editor(manager);

			break;
		case RESPONSE_REMOVE:
			pidgin_status_manager_remove_selected(manager);
			break;
		case GTK_RESPONSE_CLOSE:
		case GTK_RESPONSE_DELETE_EVENT:
			gtk_window_destroy(GTK_WINDOW(dialog));
			break;
	}
}

static char *
pidgin_status_manager_sort_data_cb(GObject *wrapper, const char *name,
                                   G_GNUC_UNUSED gpointer data)
{
	const char *value = NULL;

	if(G_IS_OBJECT(wrapper)) {
		value = g_object_get_data(wrapper, name);
	}

	/* NOTE: Most GTK widget properties don't care if you return NULL, but the
	 * GtkStringSorter does some string comparisons without checking for NULL,
	 * so we need to ensure that non-NULL is returned to prevent runtime
	 * warnings. */
	return g_strdup(value ? value : "");
}

/* A closure from within a GtkBuilderListItemFactory passes an extra first
 * argument, so we need to drop that to re-use the above callback. */
static char *
pidgin_status_manager_lookup_text_data_cb(G_GNUC_UNUSED GObject *self,
                                          GObject *wrapper, const char *name,
                                          gpointer data)
{
	return pidgin_status_manager_sort_data_cb(wrapper, name, data);
}

static void
pidgin_status_manager_row_activated_cb(G_GNUC_UNUSED GtkColumnView *self,
                                       guint position, gpointer data)
{
	PidginStatusManager *manager = data;

	gtk_single_selection_set_selected(manager->selection, position);
	pidgin_status_manager_show_editor(manager);
}

static void
pidgin_status_manager_selection_changed_cb(G_GNUC_UNUSED GObject *object,
                                           G_GNUC_UNUSED GParamSpec *pspec,
                                           gpointer data)
{
	PidginStatusManager *manager = data;
	gboolean sensitive = TRUE;

	if(g_list_model_get_n_items(G_LIST_MODEL(manager->model)) == 0) {
		sensitive = FALSE;
	}

	gtk_widget_set_sensitive(manager->use_button, sensitive);
	gtk_widget_set_sensitive(manager->modify_button, sensitive);

	/* Only enable the remove button if the currently selected row is not the
	 * currently active status.
	 */
	if(sensitive) {
		PurpleSavedStatus *status = NULL;

		status = pidgin_status_manager_get_selected_status(manager);

		sensitive = status != purple_savedstatus_get_current();
	}

	gtk_widget_set_sensitive(manager->remove_button, sensitive);
}

static void
pidgin_status_manager_savedstatus_changed_cb(PurpleSavedStatus *new_status,
                                             G_GNUC_UNUSED PurpleSavedStatus *old_status,
                                             gpointer data)
{
	PidginStatusManager *manager = data;
	PurpleSavedStatus *selected = NULL;

	/* Disable the remove button if the selected status is the currently active
	 * status.
	 */
	selected = pidgin_status_manager_get_selected_status(manager);
	if(selected != NULL) {
		gboolean sensitive = selected != new_status;

		gtk_widget_set_sensitive(manager->remove_button, sensitive);
	}
}

static void
pidgin_status_manager_savedstatus_updated_cb(G_GNUC_UNUSED PurpleSavedStatus *status,
                                             gpointer data)
{
	PidginStatusManager *manager = data;

	pidgin_status_manager_refresh(manager);
}

static void
pidgin_status_editor_destroy_cb(GtkWidget *widget, gpointer data) {
	PidginStatusManager *manager = data;
	GListModel *model = G_LIST_MODEL(manager->model);
	guint n_items = 0;

	n_items = g_list_model_get_n_items(model);
	for(guint index = 0; index < n_items; index++) {
		GObject *wrapper = NULL;
		GtkWidget *editor = NULL;

		wrapper = g_list_model_get_item(model, index);
		editor = g_object_get_data(wrapper, "editor");

		/* Check if editor is the widget being destroyed. */
		if(editor == widget) {
			/* It is, so set it back to NULL to remove it from the wrapper. */
			g_object_set_data(wrapper, "editor", NULL);
			g_object_unref(wrapper);

			break;
		}

		g_object_unref(wrapper);
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_status_manager_finalize(GObject *obj) {
	purple_signals_disconnect_by_handle(obj);

	G_OBJECT_CLASS(pidgin_status_manager_parent_class)->finalize(obj);
}

static void
pidgin_status_manager_init(PidginStatusManager *manager) {
	gpointer handle = NULL;

	gtk_widget_init_template(GTK_WIDGET(manager));

	pidgin_status_manager_refresh(manager);

	handle = purple_savedstatuses_get_handle();
	purple_signal_connect(handle, "savedstatus-changed", manager,
	                      G_CALLBACK(pidgin_status_manager_savedstatus_changed_cb),
	                      manager);
	purple_signal_connect(handle, "savedstatus-added", manager,
	                      G_CALLBACK(pidgin_status_manager_savedstatus_updated_cb),
	                      manager);
	purple_signal_connect(handle, "savedstatus-deleted", manager,
	                      G_CALLBACK(pidgin_status_manager_savedstatus_updated_cb),
	                      manager);
	purple_signal_connect(handle, "savedstatus-modified", manager,
	                      G_CALLBACK(pidgin_status_manager_savedstatus_updated_cb),
	                      manager);
}

static void
pidgin_status_manager_class_init(PidginStatusManagerClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->finalize = pidgin_status_manager_finalize;

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/im/pidgin/Pidgin3/Status/manager.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginStatusManager,
	                                     model);
	gtk_widget_class_bind_template_child(widget_class, PidginStatusManager,
	                                     selection);
	gtk_widget_class_bind_template_child(widget_class, PidginStatusManager,
	                                     use_button);
	gtk_widget_class_bind_template_child(widget_class, PidginStatusManager,
	                                     modify_button);
	gtk_widget_class_bind_template_child(widget_class, PidginStatusManager,
	                                     remove_button);

	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_status_manager_response_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_status_manager_lookup_text_data_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_status_manager_sort_data_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_status_manager_row_activated_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_status_manager_selection_changed_cb);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_status_manager_new(void) {
	return g_object_new(PIDGIN_TYPE_STATUS_MANAGER, NULL);
}
