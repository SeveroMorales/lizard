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

#include <glib/gi18n-lib.h>

#include <gtk/gtk.h>

#include <purple.h>

#include "pidginstatusbox.h"
#include "pidginiconname.h"

#define PRIMITIVE_FORMAT "primitive_%d"
#define SAVEDSTATUS_FORMAT "savedstatus_%lu"

typedef enum {
	PIDGIN_STATUS_BOX_TYPE_SEPARATOR,
	PIDGIN_STATUS_BOX_TYPE_PRIMITIVE,
	PIDGIN_STATUS_BOX_TYPE_POPULAR,
	PIDGIN_STATUS_BOX_TYPE_ACTION,
	PIDGIN_STATUS_BOX_NUM_TYPES
} PidginStatusBoxItemType;

struct _PidginStatusBox {
	GtkBox parent;

	GtkListStore *model;
	GtkWidget *combo;

	/* This is used to flipback to the correct status when one of the actions
	 * items is selected.
	 */
	gchar *active_id;
};

enum {
	ID_COLUMN,
	TYPE_COLUMN, /* PidginStatusBoxItemType */
	ICON_NAME_COLUMN,
	PRIMITIVE_COLUMN,
	TEXT_COLUMN,
	/* This value depends on TYPE_COLUMN. For POPULAR types, this is the
	 * creation time.
	 */
	DATA_COLUMN,
	EMBLEM_VISIBLE_COLUMN,
	NUM_COLUMNS
};

G_DEFINE_TYPE(PidginStatusBox, pidgin_status_box, GTK_TYPE_BOX)

/* This prototype is necessary so we can block this signal handler when we are
 * manually updating the combobox.
 */
static void pidgin_status_box_combo_changed_cb(GtkComboBox *combo, gpointer user_data);

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_status_box_update_to_status(PidginStatusBox *status_box,
                                   PurpleSavedStatus *status)
{
	gchar *id = NULL;
	gboolean set = FALSE;
	time_t creation_time = 0;

	/* Try to set the combo box to the saved status. */
	creation_time = purple_savedstatus_get_creation_time(status);
	id = g_strdup_printf(SAVEDSTATUS_FORMAT, creation_time);

	set = gtk_combo_box_set_active_id(GTK_COMBO_BOX(status_box->combo), id);
	g_free(id);

	/* If we failed to set via the savedstatus, fallback to the primitive. */
	if(!set) {
		PurpleStatusPrimitive primitive;

		primitive = purple_savedstatus_get_primitive_type(status);
		id = g_strdup_printf(PRIMITIVE_FORMAT, primitive);

		gtk_combo_box_set_active_id(GTK_COMBO_BOX(status_box->combo), id);
		g_free(id);
	}
}

static void
pidgin_status_box_add(PidginStatusBox *status_box,
                      PidginStatusBoxItemType type,
                      PurpleStatusPrimitive primitive, const gchar *text,
                      gpointer data)
{
	GtkTreeIter iter;
	gchar *id = NULL, *escaped_text = NULL;
	const gchar *icon_name = NULL;
	gboolean emblem_visible = FALSE;

	escaped_text = g_markup_escape_text(text, -1);

	if(type == PIDGIN_STATUS_BOX_TYPE_POPULAR) {
		PurpleSavedStatus *saved_status = NULL;
		time_t creation_time = GPOINTER_TO_INT(data);

		saved_status = purple_savedstatus_find_by_creation_time(creation_time);

		if(saved_status != NULL) {
			id = g_strdup_printf(SAVEDSTATUS_FORMAT, creation_time);

			if(!purple_savedstatus_is_transient(saved_status)) {
				emblem_visible = TRUE;
			}
		}
	}

	if(id == NULL && primitive != PURPLE_STATUS_UNSET) {
		id = g_strdup_printf(PRIMITIVE_FORMAT, primitive);
	}

	icon_name = pidgin_icon_name_from_status_primitive(primitive, NULL);

	gtk_list_store_append(status_box->model, &iter);
	gtk_list_store_set(status_box->model, &iter,
	                   ID_COLUMN, id,
	                   TYPE_COLUMN, type,
	                   ICON_NAME_COLUMN, icon_name,
	                   PRIMITIVE_COLUMN, primitive,
	                   TEXT_COLUMN, escaped_text,
	                   DATA_COLUMN, data,
	                   EMBLEM_VISIBLE_COLUMN, emblem_visible,
	                   -1);

	g_free(escaped_text);
	g_free(id);
}

static gboolean
pidgin_status_box_row_separator_func(GtkTreeModel *model, GtkTreeIter *iter,
                                     G_GNUC_UNUSED gpointer data)
{
	PidginStatusBoxItemType type;

	gtk_tree_model_get(model, iter, TYPE_COLUMN, &type, -1);

	return type == PIDGIN_STATUS_BOX_TYPE_SEPARATOR;
}

static void
pidgin_status_box_add_separator(PidginStatusBox *status_box) {
	GtkTreeIter iter;

	gtk_list_store_append(status_box->model, &iter);
	gtk_list_store_set(status_box->model, &iter,
	                   TYPE_COLUMN, PIDGIN_STATUS_BOX_TYPE_SEPARATOR,
	                   -1);
}

static void
pidgin_status_box_update_saved_statuses(PidginStatusBox *status_box) {
	GList *list, *cur;

	list = purple_savedstatuses_get_popular(6);
	if (list == NULL) {
		/* Odd... oh well, nothing we can do about it. */
		return;
	}

	for(cur = list; cur != NULL; cur = cur->next) {
		PurpleSavedStatus *saved = cur->data;
		GString *text = NULL;
		time_t creation_time;

		text = g_string_new(purple_savedstatus_get_title(saved));

		if(!purple_savedstatus_is_transient(saved)) {
			/*
			 * Transient statuses do not have a title, so the savedstatus
			 * API returns the message when purple_savedstatus_get_title() is
			 * called, so we don't need to get the message a second time.
			 */
			const gchar *message = NULL;

			message = purple_savedstatus_get_message(saved);
			if(message != NULL) {
				gchar *stripped = purple_markup_strip_html(message);

				purple_util_chrreplace(stripped, '\n', ' ');
				g_string_append_printf(text, " - %s", stripped);
				g_free(stripped);
			}
		}

		creation_time = purple_savedstatus_get_creation_time(saved);
		pidgin_status_box_add(status_box, PIDGIN_STATUS_BOX_TYPE_POPULAR,
		                      purple_savedstatus_get_primitive_type(saved),
		                      text->str, GINT_TO_POINTER(creation_time));

		g_string_free(text, TRUE);
	}

	g_list_free(list);

	pidgin_status_box_add_separator(status_box);
}

static void
pidgin_status_box_populate(PidginStatusBox *status_box) {
	pidgin_status_box_add(status_box, PIDGIN_STATUS_BOX_TYPE_PRIMITIVE,
	                      PURPLE_STATUS_AVAILABLE, _("Available"), NULL);
	pidgin_status_box_add(status_box, PIDGIN_STATUS_BOX_TYPE_PRIMITIVE,
	                      PURPLE_STATUS_AWAY, _("Away"), NULL);
	pidgin_status_box_add(status_box, PIDGIN_STATUS_BOX_TYPE_PRIMITIVE,
	                      PURPLE_STATUS_UNAVAILABLE, _("Do not disturb"),
	                      NULL);
	pidgin_status_box_add(status_box, PIDGIN_STATUS_BOX_TYPE_PRIMITIVE,
	                      PURPLE_STATUS_INVISIBLE, _("Invisible"), NULL);
	pidgin_status_box_add(status_box, PIDGIN_STATUS_BOX_TYPE_PRIMITIVE,
	                      PURPLE_STATUS_OFFLINE, _("Offline"), NULL);

	pidgin_status_box_add_separator(status_box);

	pidgin_status_box_update_saved_statuses(status_box);

	pidgin_status_box_add(status_box, PIDGIN_STATUS_BOX_TYPE_ACTION,
	                      PURPLE_STATUS_UNSET, _("New Status..."),
	                      "new-status");
	pidgin_status_box_add(status_box, PIDGIN_STATUS_BOX_TYPE_ACTION,
	                      PURPLE_STATUS_UNSET, _("Saved Statuses..."),
	                      "status-manager");
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_status_box_combo_changed_cb(GtkComboBox *combo, gpointer user_data) {
	PidginStatusBox *status_box = user_data;
	PidginStatusBoxItemType type;
	PurpleSavedStatus *saved_status = NULL;
	PurpleStatusPrimitive primitive;
	GtkTreeIter iter;
	gchar *id = NULL;
	gpointer data;

	if(!gtk_combo_box_get_active_iter(combo, &iter)) {
		return;
	}

	gtk_tree_model_get(GTK_TREE_MODEL(status_box->model), &iter,
	                   ID_COLUMN, &id,
	                   TYPE_COLUMN, &type,
	                   PRIMITIVE_COLUMN, &primitive,
	                   DATA_COLUMN, &data,
	                   -1);

	if(type == PIDGIN_STATUS_BOX_TYPE_PRIMITIVE) {
		saved_status = purple_savedstatus_find_transient_by_type_and_message(primitive, NULL);
		if(saved_status == NULL) {
			saved_status = purple_savedstatus_new(NULL, primitive);
		}
	} else if(type == PIDGIN_STATUS_BOX_TYPE_POPULAR) {
		time_t creation_time = GPOINTER_TO_INT(data);

		saved_status = purple_savedstatus_find_by_creation_time(creation_time);
	} else if(type == PIDGIN_STATUS_BOX_TYPE_ACTION) {
		GApplication *application = NULL;
		const gchar *action_name = (const gchar *)data;

		application = g_application_get_default();

		g_action_group_activate_action(G_ACTION_GROUP(application),
		                               action_name, NULL);

		gtk_combo_box_set_active_id(combo, status_box->active_id);
	}

	if(saved_status != NULL) {
		if(saved_status != purple_savedstatus_get_current()) {
			purple_savedstatus_activate(saved_status);
		}

		g_free(status_box->active_id);
		status_box->active_id = id;
	} else {
		g_free(id);
	}
}

static void
pidgin_status_box_savedstatus_changed_cb(PurpleSavedStatus *now,
                                         G_GNUC_UNUSED PurpleSavedStatus *old,
                                         gpointer data)
{
	PidginStatusBox *status_box = data;

	/* If we don't have a status, we have to bail. */
	if(now == NULL) {
		return;
	}

	pidgin_status_box_update_to_status(status_box, now);
}

static void
pidgin_status_box_savedstatus_updated_cb(G_GNUC_UNUSED PurpleSavedStatus *status,
                                         gpointer data)
{
	PidginStatusBox *status_box = data;
	PurpleSavedStatus *current = NULL;
	static gboolean getting_current = FALSE;

	/* purple_status_get_current will create a new status if this is a brand
	 * new install or the setting wasn't found. This leads to this handler
	 * getting stuck in a loop until it segfaults because the stack smashed
	 * into the heap. Anyways, we use this static boolean to check when this
	 * function is called by purple_status_get_current so we can bail out and
	 * break the loop.
	 */
	if(getting_current) {
		return;
	}

	gtk_list_store_clear(status_box->model);
	pidgin_status_box_populate(status_box);

	getting_current = TRUE;
	current = purple_savedstatus_get_current();
	pidgin_status_box_update_to_status(status_box, current);
	getting_current = FALSE;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_status_box_finalize(GObject *obj) {
	PidginStatusBox *status_box = PIDGIN_STATUS_BOX(obj);

	purple_signals_disconnect_by_handle(status_box);

	g_free(status_box->active_id);

	G_OBJECT_CLASS(pidgin_status_box_parent_class)->finalize(obj);
}

static void
pidgin_status_box_init(PidginStatusBox *status_box) {
	gpointer handle;

	gtk_widget_init_template(GTK_WIDGET(status_box));

	gtk_combo_box_set_row_separator_func(GTK_COMBO_BOX(status_box->combo),
	                                     pidgin_status_box_row_separator_func,
	                                     NULL, NULL);

	pidgin_status_box_populate(status_box);

	handle = purple_savedstatuses_get_handle();
	purple_signal_connect(handle, "savedstatus-changed", status_box,
	                      G_CALLBACK(pidgin_status_box_savedstatus_changed_cb),
	                      status_box);
	purple_signal_connect(handle, "savedstatus-added", status_box,
	                      G_CALLBACK(pidgin_status_box_savedstatus_updated_cb),
	                      status_box);
	purple_signal_connect(handle, "savedstatus-deleted", status_box,
	                      G_CALLBACK(pidgin_status_box_savedstatus_updated_cb),
	                      status_box);
	purple_signal_connect(handle, "savedstatus-modified", status_box,
	                      G_CALLBACK(pidgin_status_box_savedstatus_updated_cb),
	                      status_box);
}

static void
pidgin_status_box_constructed(GObject *obj) {
	PurpleSavedStatus *status = NULL;

	G_OBJECT_CLASS(pidgin_status_box_parent_class)->constructed(obj);

	status = purple_savedstatus_get_current();

	pidgin_status_box_update_to_status(PIDGIN_STATUS_BOX(obj), status);
}

static void
pidgin_status_box_class_init(PidginStatusBoxClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->finalize = pidgin_status_box_finalize;
	obj_class->constructed = pidgin_status_box_constructed;

	gtk_widget_class_set_template_from_resource(
	    widget_class,
	    "/im/pidgin/Pidgin3/Status/box.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginStatusBox,
	                                     model);
	gtk_widget_class_bind_template_child(widget_class, PidginStatusBox,
	                                     combo);

	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_status_box_combo_changed_cb);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_status_box_new(void) {
	return g_object_new(PIDGIN_TYPE_STATUS_BOX, NULL);
}
