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

#include <talkatu.h>

#include "pidginstatuseditor.h"

#include "pidginstatusprimitivechooser.h"

enum {
	PROP_0,
	PROP_STATUS,
	N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

enum {
	RESPONSE_USE,
	RESPONSE_SAVE
};

struct _PidginStatusEditor {
	GtkDialog parent;

	PurpleSavedStatus *status;

	GtkTextBuffer *buffer;

	GtkWidget *title;
	GtkWidget *primitive;
	GtkWidget *message;

	GtkWidget *use;
	GtkWidget *save;
};

G_DEFINE_TYPE(PidginStatusEditor, pidgin_status_editor, GTK_TYPE_DIALOG)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
pidgin_status_editor_set_status(PidginStatusEditor *editor,
                                PurpleSavedStatus *status)
{
	PurpleStatusPrimitive primitive;
	PidginStatusPrimitiveChooser *chooser = NULL;
	const gchar *title = NULL, *message = NULL;

	editor->status = status;

	if(editor->status != NULL) {
		title = purple_savedstatus_get_title(editor->status);
		primitive = purple_savedstatus_get_primitive_type(editor->status);
		message = purple_savedstatus_get_message(editor->status);
	} else {
		primitive = PURPLE_STATUS_AWAY;
	}

	if(title == NULL) {
		title = "";
	}

	if(message == NULL) {
		message = "";
	}

	gtk_editable_set_text(GTK_EDITABLE(editor->title), title);
	chooser = PIDGIN_STATUS_PRIMITIVE_CHOOSER(editor->primitive);
	pidgin_status_primitive_chooser_set_selected(chooser, primitive);
	talkatu_markup_set_html(TALKATU_BUFFER(editor->buffer), message, -1);

	g_object_notify_by_pspec(G_OBJECT(editor), properties[PROP_STATUS]);
}

static void
pidgin_status_editor_save_status(PidginStatusEditor *editor) {
	PidginStatusPrimitiveChooser *chooser = NULL;
	PurpleStatusPrimitive primitive;
	gchar *message = NULL;
	const gchar *title = NULL;

	title = gtk_editable_get_text(GTK_EDITABLE(editor->title));

	chooser = PIDGIN_STATUS_PRIMITIVE_CHOOSER(editor->primitive);
	primitive = pidgin_status_primitive_chooser_get_selected(chooser);

	message = talkatu_markup_get_html(editor->buffer, NULL);

	if(editor->status == NULL) {
		editor->status = purple_savedstatus_new(title, primitive);
	} else {
		const gchar *current_title = NULL;

		/* purple_savedstatus_set_title throws a warning if you try to save a
		 * status with an existing title, which means we can't just save the
		 * title if it hasn't changed.
		 */
		current_title = purple_savedstatus_get_title(editor->status);
		if(!purple_strequal(title, current_title)) {
			purple_savedstatus_set_title(editor->status, title);
		}

		purple_savedstatus_set_primitive_type(editor->status, primitive);
	}

	purple_savedstatus_set_message(editor->status, message);

	g_free(message);
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_status_editor_response_cb(GtkDialog *dialog, gint response_id,
                                 gpointer data)
{
	PidginStatusEditor *editor = data;

	switch(response_id) {
		case RESPONSE_USE:
			pidgin_status_editor_save_status(editor);
			purple_savedstatus_activate(editor->status);
			break;
		case RESPONSE_SAVE:
			pidgin_status_editor_save_status(editor);
			break;
	}

	gtk_window_destroy(GTK_WINDOW(dialog));
}

static void
pidgin_status_editor_title_changed_cb(G_GNUC_UNUSED GtkEditable *editable,
                                      gpointer data)
{
	PidginStatusEditor *editor = data;
	gboolean title_changed = FALSE, sensitive = FALSE;
	const gchar *title = NULL;

	title = gtk_editable_get_text(GTK_EDITABLE(editor->title));

	if(editor->status != NULL) {
		/* If we're editing a status, check if the title is the same. */
		title_changed = !purple_strequal(title,
		                                 purple_savedstatus_get_title(editor->status));
	} else {
		/* If this is a new status, check if the title is empty or not. */
		title_changed = !purple_strequal(title, "");
	}

	if(title_changed) {
		gboolean duplicated = purple_savedstatus_find(title) != NULL;

		if(duplicated) {
			gtk_widget_set_sensitive(editor->use, FALSE);
			gtk_widget_set_sensitive(editor->save, FALSE);

			return;
		}
	}

	sensitive = !purple_strequal(title, "");

	gtk_widget_set_sensitive(editor->use, sensitive);
	gtk_widget_set_sensitive(editor->save, sensitive);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_status_editor_get_property(GObject *obj, guint param_id, GValue *value,
                                  GParamSpec *pspec)
{
	PidginStatusEditor *editor = PIDGIN_STATUS_EDITOR(obj);

	switch(param_id) {
		case PROP_STATUS:
			g_value_set_pointer(value,
			                    pidgin_status_editor_get_status(editor));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_status_editor_set_property(GObject *obj, guint param_id,
                                  const GValue *value, GParamSpec *pspec)
{
	PidginStatusEditor *editor = PIDGIN_STATUS_EDITOR(obj);

	switch(param_id) {
		case PROP_STATUS:
			pidgin_status_editor_set_status(editor,
			                                g_value_get_pointer(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
pidgin_status_editor_init(PidginStatusEditor *manager) {
	gtk_widget_init_template(GTK_WIDGET(manager));
}

static void
pidgin_status_editor_class_init(PidginStatusEditorClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->get_property = pidgin_status_editor_get_property;
	obj_class->set_property = pidgin_status_editor_set_property;

	/**
	 * PidginStatusEditor:status:
	 *
	 * The [type@Purple.SavedStatus] that this editor is responsible for.
	 * This may be %NULL if it is creating a new status.
	 *
	 * Since: 3.0.0.
	 */
	/* we don't used boxed here because of the way status are currently
	 * managed.
	 */
	properties[PROP_STATUS] = g_param_spec_pointer(
		"status", "status",
		"The saved status we're editing",
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/im/pidgin/Pidgin3/Status/editor.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginStatusEditor,
	                                     buffer);

	gtk_widget_class_bind_template_child(widget_class, PidginStatusEditor,
	                                     title);
	gtk_widget_class_bind_template_child(widget_class, PidginStatusEditor,
	                                     primitive);
	gtk_widget_class_bind_template_child(widget_class, PidginStatusEditor,
	                                     message);

	gtk_widget_class_bind_template_child(widget_class, PidginStatusEditor,
	                                     use);
	gtk_widget_class_bind_template_child(widget_class, PidginStatusEditor,
	                                     save);

	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_status_editor_response_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_status_editor_title_changed_cb);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_status_editor_new(PurpleSavedStatus *status) {
	return g_object_new(
		PIDGIN_TYPE_STATUS_EDITOR,
		"status", status,
		NULL);
}

PurpleSavedStatus *
pidgin_status_editor_get_status(PidginStatusEditor *editor) {
	g_return_val_if_fail(PIDGIN_IS_STATUS_EDITOR(editor), NULL);

	return editor->status;
}
