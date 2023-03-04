/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2020 Gary Kramlich <grim@reaperworld.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>

#include <talkatu/talkatueditor.h>

#include <talkatu/talkatubuffer.h>
#include <talkatu/talkatuinput.h>
#include <talkatu/talkatutoolbar.h>

/**
 * TalkatuEditor:
 *
 * A composite widget that contains a #TalkatuToolbar, #TalkatuInput, and an
 * optional send button in the common instant messaging input layout.
 */
struct _TalkatuEditor {
	GtkBox parent;

	GtkWidget *input;
	GtkWidget *toolbar;
	GtkWidget *send_button;
};

enum {
	PROP_0,
	PROP_TOOLBAR,
	PROP_SHOW_TOOLBAR,
	PROP_INPUT,
	PROP_BUFFER,
	PROP_SHOW_SEND_BUTTON,
	N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = {NULL, };

G_DEFINE_TYPE(TalkatuEditor, talkatu_editor, GTK_TYPE_BOX)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
talkatu_editor_buffer_changed(TalkatuEditor *editor) {
	GActionGroup *action_group = NULL;
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->input));

	if(TALKATU_IS_BUFFER(buffer)) {
		action_group = G_ACTION_GROUP(talkatu_buffer_get_action_group(TALKATU_BUFFER(buffer)));
	}

	gtk_widget_insert_action_group(editor->toolbar, "talkatu", action_group);
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
talkatu_editor_view_buffer_changed_handler(G_GNUC_UNUSED GObject *obj,
                                           G_GNUC_UNUSED GParamSpec *pspec,
                                           gpointer data) {
	talkatu_editor_buffer_changed(TALKATU_EDITOR(data));
}

static void
talkatu_editor_send_button_clicked_cb(G_GNUC_UNUSED GtkButton *button,
                                      gpointer data) {
	TalkatuEditor *editor = TALKATU_EDITOR(data);

	talkatu_input_send_message(TALKATU_INPUT(editor->input));

	gtk_widget_grab_focus(editor->input);
}

/******************************************************************************
 * GObject Stuff
 *****************************************************************************/
static void
talkatu_editor_get_property(GObject *obj, guint param_id, GValue *value, GParamSpec *pspec) {
	TalkatuEditor *editor = TALKATU_EDITOR(obj);

	switch(param_id) {
		case PROP_TOOLBAR:
			g_value_set_object(value, talkatu_editor_get_toolbar(editor));
			break;
		case PROP_BUFFER:
			g_value_set_object(value, talkatu_editor_get_buffer(editor));
			break;
		case PROP_SHOW_TOOLBAR:
			g_value_set_boolean(value, talkatu_editor_get_toolbar_visible(editor));
			break;
		case PROP_INPUT:
			g_value_set_object(value, talkatu_editor_get_input(editor));
			break;
		case PROP_SHOW_SEND_BUTTON:
			g_value_set_boolean(value, talkatu_editor_get_send_button_visible(editor));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
talkatu_editor_set_property(GObject *obj, guint param_id, const GValue *value, GParamSpec *pspec) {
	TalkatuEditor *editor = TALKATU_EDITOR(obj);

	switch(param_id) {
		case PROP_SHOW_TOOLBAR:
			talkatu_editor_set_toolbar_visible(editor, g_value_get_boolean(value));
			break;
		case PROP_BUFFER:
			talkatu_editor_set_buffer(editor, g_value_get_object(value));
			break;
		case PROP_SHOW_SEND_BUTTON:
			talkatu_editor_set_send_button_visible(editor, g_value_get_boolean(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
talkatu_editor_init(TalkatuEditor *editor) {
	gtk_widget_init_template(GTK_WIDGET(editor));

	/* bind the visibility of the toolbar to the editor property */
	g_object_bind_property(
		editor, "show-toolbar",
		editor->toolbar, "visible",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL
	);

	/* bind the visibility of the send button to the editor property */
	g_object_bind_property(
		editor, "show-send-button",
		editor->send_button, "visible",
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL
	);
}

static void
talkatu_editor_class_init(TalkatuEditorClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->get_property = talkatu_editor_get_property;
	obj_class->set_property = talkatu_editor_set_property;

	properties[PROP_TOOLBAR] = g_param_spec_object(
		"toolbar", "toolbar",
		"The toolbar widget",
		TALKATU_TYPE_TOOLBAR,
		G_PARAM_READABLE
	);

	properties[PROP_SHOW_TOOLBAR] = g_param_spec_boolean(
		"show-toolbar", "show-toolbar",
		"Whether or not to show the toolbar",
		TRUE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT
	);

	properties[PROP_INPUT] = g_param_spec_object(
		"input", "input",
		"The input widget",
		TALKATU_TYPE_VIEW,
		G_PARAM_READABLE
	);

	properties[PROP_BUFFER] = g_param_spec_object(
		"buffer", "buffer",
		"The GtkTextBuffer for the view",
		GTK_TYPE_TEXT_BUFFER,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT
	);

	properties[PROP_SHOW_SEND_BUTTON] = g_param_spec_boolean(
		"show-send-button", "show-send-button",
		"Whether or not to show the send button",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT
	);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	/* load our template */
	gtk_widget_class_set_template_from_resource(
		GTK_WIDGET_CLASS(klass),
		"/org/imfreedom/keep/talkatu/talkatu/ui/editor.ui"
	);

	gtk_widget_class_bind_template_child_internal(widget_class, TalkatuEditor, input);
	gtk_widget_class_bind_template_child_internal(widget_class, TalkatuEditor, toolbar);
	gtk_widget_class_bind_template_child_internal(widget_class, TalkatuEditor, send_button);

	gtk_widget_class_bind_template_callback(widget_class, talkatu_editor_view_buffer_changed_handler);

	gtk_widget_class_bind_template_callback(widget_class, talkatu_editor_send_button_clicked_cb);
}

/******************************************************************************
 * Public API
 *****************************************************************************/

/**
 * talkatu_editor_new:
 *
 * Creates a new #TalkatuEditor with a normal #GtkTextBuffer.
 *
 * Returns: (transfer full): The new #TalkatuEditor instance.
 */
GtkWidget *
talkatu_editor_new(void) {
	return GTK_WIDGET(g_object_new(TALKATU_TYPE_EDITOR, NULL));
}

/**
 * talkatu_editor_get_toolbar:
 * @editor: The #TalkatuEditor instance.
 *
 * Gets the #TalkatuToolbar that @editor is using.
 *
 * Returns: (transfer none): The #TalkatuToolbar that @editor is using.
 */
GtkWidget *
talkatu_editor_get_toolbar(TalkatuEditor *editor) {
	g_return_val_if_fail(TALKATU_IS_EDITOR(editor), NULL);

	return editor->toolbar;
}

/**
 * talkatu_editor_set_toolbar_visible:
 * @editor: The #TalkatuEditor instance.
 * @visible: Whether or not the toolbar should be visible.
 *
 * Makes the toolbar visible if @visible is %TRUE otherwise hides it.
 */
void
talkatu_editor_set_toolbar_visible(TalkatuEditor *editor, gboolean visible) {
	g_return_if_fail(TALKATU_IS_EDITOR(editor));

	gtk_widget_set_visible(editor->toolbar, visible);
}

/**
 * talkatu_editor_get_toolbar_visible:
 * @editor: The #TalkatuEditor instance.
 *
 * Returns whether or not the toolbar is visible.
 *
 * Returns: %TRUE if the toolbar is visible, %FALSE otherwise.
 */
gboolean
talkatu_editor_get_toolbar_visible(TalkatuEditor *editor) {
	g_return_val_if_fail(TALKATU_IS_EDITOR(editor), FALSE);

	return gtk_widget_get_visible(editor->toolbar);
}

/**
 * talkatu_editor_get_input:
 * @editor: The #TalkatuEditor instance.
 *
 * Gets the #TalkatuInput that @editor is using.
 *
 * Returns: (transfer none): The #TalkatuInput that @editor is using.
 */
GtkWidget *
talkatu_editor_get_input(TalkatuEditor *editor) {
	g_return_val_if_fail(TALKATU_IS_EDITOR(editor), NULL);

	return editor->input;
}

/**
 * talkatu_editor_get_buffer:
 * @editor: The #TalkatuEditor instance.
 *
 * Gets the #GtkTextBuffer for the internal input.
 *
 * Returns: (transfer none): The #GtkTextBuffer for the internal input.
 */
GtkTextBuffer *
talkatu_editor_get_buffer(TalkatuEditor *editor) {
	g_return_val_if_fail(TALKATU_IS_EDITOR(editor), NULL);

	return gtk_text_view_get_buffer(GTK_TEXT_VIEW(editor->input));
}

/**
 * talkatu_editor_set_buffer:
 * @editor: The #TalkatuEditor instance.
 * @buffer: A new #GtkTextBuffer to use.
 *
 * Sets the #GtkTextBuffer for the internal input to @buffer.
 */
void
talkatu_editor_set_buffer(TalkatuEditor *editor, GtkTextBuffer *buffer) {
	g_return_if_fail(TALKATU_IS_EDITOR(editor));

	gtk_text_view_set_buffer(GTK_TEXT_VIEW(editor->input), buffer);
}

/**
 * talkatu_editor_get_send_button_visible:
 * @editor: The #TalkatuEditor instance.
 *
 * Returns whether or not the send button is visible.
 *
 * Returns: %TRUE if the send button is visible, %FALSE otherwise.
 */
gboolean
talkatu_editor_get_send_button_visible(TalkatuEditor *editor) {
	g_return_val_if_fail(TALKATU_IS_EDITOR(editor), FALSE);

	return gtk_widget_get_visible(editor->send_button);
}

/**
 * talkatu_editor_set_send_button_visible:
 * @editor: The #TalkatuEditor instance.
 * @visible: Whether or not the send button should be visible.
 *
 * Sets the visibility of the send button depending on @visible.
 */
void
talkatu_editor_set_send_button_visible(TalkatuEditor *editor, gboolean visible) {
	g_return_if_fail(TALKATU_IS_EDITOR(editor));

	gtk_widget_set_visible(editor->send_button, visible);
}
