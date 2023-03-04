/*
 * Talkatu - GTK widgets for chat applications
 * Copyright (C) 2017-2020 Gary Kramlich <grim@reaperworld.com>
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
 * along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "talkatudemowindow.h"

struct _TalkatuDemoWindow {
	GtkApplicationWindow parent;

	GtkWidget *history;
	GtkWidget *editor;
	GtkWidget *typing;

	GtkToggleButton *toggle_plain;
	GtkToggleButton *toggle_whole;
	GtkToggleButton *toggle_html;
	GtkToggleButton *toggle_markdown;

	GtkToggleButton *toggle_toolbar;
	GtkToggleButton *toggle_send_button;
	GtkToggleButton *toggle_edited;

	GtkTextBuffer *buffer_plain;
	GtkTextBuffer *buffer_whole;
	GtkTextBuffer *buffer_html;
	GtkTextBuffer *buffer_markdown;

	GtkWidget *author_button;
};

G_DEFINE_TYPE(TalkatuDemoWindow, talkatu_demo_window, GTK_TYPE_APPLICATION_WINDOW);

static void
talkatu_demo_window_insert_html_response_cb(GtkNativeDialog *dialog,
                                            gint response, gpointer data)
{
	TalkatuDemoWindow *window = TALKATU_DEMO_WINDOW(data);

	if(response == GTK_RESPONSE_ACCEPT) {
		GFile *file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
		gchar *contents = NULL;
		gsize len;

		if(g_file_load_contents(file, NULL, &contents, &len, NULL, NULL)) {
			GtkTextMark *mark = NULL;
			GtkTextIter iter;

			mark = gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(window->buffer_html));
			gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(window->buffer_html),
			                                 &iter, mark);

			talkatu_markup_insert_html(TALKATU_BUFFER(window->buffer_html),
			                           &iter, contents, len);
			g_free(contents);
		}

		g_object_unref(file);
	}

	gtk_native_dialog_destroy(dialog);
	g_object_unref(dialog);
}

static void
talkatu_demo_window_insert_html_cb(G_GNUC_UNUSED GtkButton *toggle,
                                   gpointer data) {
	TalkatuDemoWindow *window = TALKATU_DEMO_WINDOW(data);
	GtkFileChooserNative *dialog = NULL;
	GtkFileFilter *filter = NULL;

	dialog = gtk_file_chooser_native_new(
		_("Insert html..."),
		GTK_WINDOW(window),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		"OK",
		"Cancel"
	);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "HTML files");
	gtk_file_filter_add_pattern(filter, "*.html");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	g_signal_connect(dialog, "response",
	                 G_CALLBACK(talkatu_demo_window_insert_html_response_cb),
	                 window);
	gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(dialog), TRUE);
	gtk_native_dialog_show(GTK_NATIVE_DIALOG(dialog));
}

static void
talkatu_demo_window_insert_markdown_response_cb(GtkNativeDialog *dialog,
                                                gint response, gpointer data)
{
	TalkatuDemoWindow *window = TALKATU_DEMO_WINDOW(data);

	if(response == GTK_RESPONSE_ACCEPT) {
		GFile *file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
		gchar *contents = NULL;
		gsize len;

		if(g_file_load_contents(file, NULL, &contents, &len, NULL, NULL)) {
			GtkTextMark *mark = NULL;
			GtkTextIter iter;

			mark = gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(window->buffer_markdown));
			gtk_text_buffer_get_iter_at_mark(GTK_TEXT_BUFFER(window->buffer_markdown),
			                                 &iter, mark);

			talkatu_markdown_insert(TALKATU_BUFFER(window->buffer_markdown),
			                        &iter, contents, len);
			g_free(contents);
		}

		g_object_unref(file);
	}

	gtk_native_dialog_destroy(dialog);
	g_object_unref(dialog);
}

static void
talkatu_demo_window_insert_markdown_cb(G_GNUC_UNUSED GtkButton *toggle,
                                       gpointer data) {
	TalkatuDemoWindow *window = TALKATU_DEMO_WINDOW(data);
	GtkFileChooserNative *dialog = NULL;
	GtkFileFilter *filter = NULL;

	dialog = gtk_file_chooser_native_new(
		_("Insert markdown..."),
		GTK_WINDOW(window),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		"OK",
		"Cancel"
	);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Markdown files");
	gtk_file_filter_add_pattern(filter, "*.md");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	g_signal_connect(dialog, "response",
	                 G_CALLBACK(talkatu_demo_window_insert_markdown_response_cb),
	                 window);
	gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(dialog), TRUE);
	gtk_native_dialog_show(GTK_NATIVE_DIALOG(dialog));
}

static void
talkatu_demo_window_buffer_changed_cb(G_GNUC_UNUSED GtkToggleButton *toggle,
                                      gpointer data) {
	TalkatuDemoWindow *window = TALKATU_DEMO_WINDOW(data);
	GtkWidget *view = talkatu_editor_get_input(TALKATU_EDITOR(window->editor));

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(window->toggle_plain))) {
		g_message("switching to plain buffer");
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), window->buffer_plain);
	} else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(window->toggle_whole))) {
		g_message("switching to whole buffer");
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), window->buffer_whole);
	} else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(window->toggle_html))) {
		g_message("switching to html buffer");
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), window->buffer_html);
	} else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(window->toggle_markdown))) {
		g_message("switching to markdown buffer");
		gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), window->buffer_markdown);
	}
}

static void
talkatu_demo_window_buffer_modified_cb(GtkTextBuffer *buffer, gpointer data) {
	TalkatuDemoWindow *window = TALKATU_DEMO_WINDOW(data);
	GtkWidget *input = talkatu_editor_get_input(TALKATU_EDITOR(window->editor));
	gchar *author = NULL;

	author = talkatu_message_get_author(TALKATU_MESSAGE(input));

	if(gtk_text_buffer_get_char_count(buffer) > 0) {
		talkatu_typing_label_start_typing(TALKATU_TYPING_LABEL(window->typing),
		                                  author);
	} else {
		talkatu_typing_label_finish_typing(TALKATU_TYPING_LABEL(window->typing),
		                                   author);
	}

	g_free(author);
}

static void
talkatu_demo_window_view_open_url_cb(G_GNUC_UNUSED TalkatuView *view,
                                     const gchar *url, gpointer data)
{
	gtk_show_uri(GTK_WINDOW(data), url, GDK_CURRENT_TIME);
}

static void
talkatu_demo_window_view_send_message_cb(TalkatuInput *input, gpointer data) {
	TalkatuDemoWindow *window = TALKATU_DEMO_WINDOW(data);
	gchar *sid = NULL;
	static guint64 id = 0;

	sid = g_strdup_printf("%" G_GUINT64_FORMAT, id++);
	talkatu_message_set_id(TALKATU_MESSAGE(input), sid);
	g_free(sid);

	talkatu_history_write_message(
		TALKATU_HISTORY(window->history),
		TALKATU_MESSAGE(input)
	);

	talkatu_message_set_contents(TALKATU_MESSAGE(input), "");
	talkatu_message_clear_attachments(TALKATU_MESSAGE(input));
}

static void
talkatu_demo_window_author_changed(GSimpleAction *action, GVariant *parameter,
                                   gpointer data)
{
	TalkatuDemoWindow *window = TALKATU_DEMO_WINDOW(data);
	const gchar *author = NULL;
	TalkatuEditor *editor = NULL;
	GtkWidget *input = NULL;

	author = g_variant_get_string(parameter, NULL);
	g_message("Changing author to %s", author);

	editor = TALKATU_EDITOR(window->editor);
	input = talkatu_editor_get_input(editor);
	talkatu_message_set_author(TALKATU_MESSAGE(input), author);

	g_action_change_state(G_ACTION(action), parameter);
}

static void
talkatu_demo_window_author_name_color_changed(GSimpleAction *action,
                                              GVariant *parameter,
                                              gpointer data)
{
	TalkatuDemoWindow *window = TALKATU_DEMO_WINDOW(data);
	const gchar *color_str = NULL;
	TalkatuEditor *editor = NULL;
	GtkWidget *input = NULL;
	GdkRGBA color;

	color_str = g_variant_get_string(parameter, NULL);
	g_message("Changing author name colour to %s", color_str);

	editor = TALKATU_EDITOR(window->editor);
	input = talkatu_editor_get_input(editor);
	if(gdk_rgba_parse(&color, color_str)) {
		talkatu_message_set_author_name_color(TALKATU_MESSAGE(input), &color);
	} else {
		talkatu_message_set_author_name_color(TALKATU_MESSAGE(input), NULL);
	}

	g_action_change_state(G_ACTION(action), parameter);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
talkatu_demo_window_init(TalkatuDemoWindow *window) {
	const GActionEntry entries[] = {
		{
			.name = "author-name",
			.activate = talkatu_demo_window_author_changed,
			.parameter_type = "s",
			.state = "'Alice'",
		}, {
			.name = "author-name-color",
			.activate = talkatu_demo_window_author_name_color_changed,
			.parameter_type = "s",
			.state = "'black'",
		}
	};

	g_action_map_add_action_entries(G_ACTION_MAP(window), entries,
	                                G_N_ELEMENTS(entries), window);

	gtk_widget_init_template(GTK_WIDGET(window));
}

static void
talkatu_demo_window_class_init(TalkatuDemoWindowClass *klass) {
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/org/imfreedom/keep/talkatu/talkatu/ui/demo/demo.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, history);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, editor);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, typing);

	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, buffer_plain);
 	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, buffer_whole);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, buffer_html);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, buffer_markdown);

	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, toggle_plain);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, toggle_whole);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, toggle_html);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, toggle_markdown);

	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, author_button);

	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, toggle_toolbar);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, toggle_send_button);
	gtk_widget_class_bind_template_child(widget_class, TalkatuDemoWindow, toggle_edited);

	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_buffer_changed_cb);
	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_buffer_modified_cb);
	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_view_open_url_cb);
	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_view_send_message_cb);

	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_insert_html_cb);
	gtk_widget_class_bind_template_callback(widget_class, talkatu_demo_window_insert_markdown_cb);

	gtk_widget_class_add_binding_action(widget_class,
	                                    GDK_KEY_w, GDK_CONTROL_MASK,
	                                    "window.close", NULL);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
talkatu_demo_window_new(void) {
	return GTK_WIDGET(g_object_new(TALKATU_DEMO_TYPE_WINDOW, NULL));
}
