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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib/gi18n-lib.h>
#include <glib/gstdio.h>

#include <gtk/gtk.h>

#include <purple.h>

#include "gtkdialogs.h"
#include "gtkutils.h"
#include "pidginapplication.h"
#include "pidgincore.h"
#include "pidgindebug.h"

#include <gdk/gdkkeysyms.h>


struct _PidginDebugWindow {
	GtkWindow parent;

	GtkWidget *textview;
	GtkTextBuffer *buffer;
	GtkTextMark *start_mark;
	GtkTextMark *end_mark;
	struct {
		GtkTextTag *level[PURPLE_DEBUG_FATAL + 1];
		GtkTextTag *category;
		GtkTextTag *filtered_invisible;
		GtkTextTag *filtered_visible;
		GtkTextTag *match;
		GtkTextTag *paused;
	} tags;
	GtkWidget *filter;
	GtkWidget *expression;
	GtkWidget *filterlevel;

	gboolean paused;

	GtkWidget *popover;
	GtkWidget *popover_invert;
	GtkWidget *popover_highlight;
	gboolean invert;
	gboolean highlight;
	GRegex *regex;
};

typedef struct {
	GDateTime *timestamp;
	PurpleDebugLevel level;
	gchar *domain;
	gchar *message;
} PidginDebugMessage;

static gboolean debug_print_enabled = FALSE;
static PidginDebugWindow *debug_win = NULL;
static guint pref_callback_id = 0;
static guint debug_enabled_timer = 0;

G_DEFINE_TYPE(PidginDebugWindow, pidgin_debug_window, GTK_TYPE_WINDOW);

static gboolean
save_default_size_cb(GObject *gobject, G_GNUC_UNUSED GParamSpec *pspec,
                     G_GNUC_UNUSED gpointer data)
{
	if (gtk_widget_get_visible(GTK_WIDGET(gobject))) {
		gint width, height;
		gtk_window_get_default_size(GTK_WINDOW(gobject), &width, &height);
		purple_prefs_set_int(PIDGIN_PREFS_ROOT "/debug/width",  width);
		purple_prefs_set_int(PIDGIN_PREFS_ROOT "/debug/height", height);
	}

	return FALSE;
}

static gboolean
view_near_bottom(PidginDebugWindow *win)
{
	GtkAdjustment *adj = gtk_scrollable_get_vadjustment(
			GTK_SCROLLABLE(win->textview));
	return (gtk_adjustment_get_value(adj) >=
			(gtk_adjustment_get_upper(adj) -
			 gtk_adjustment_get_page_size(adj) * 1.5));
}

static void
save_response_cb(GtkNativeDialog *self, gint response_id, gpointer data)
{
	PidginDebugWindow *win = (PidginDebugWindow *)data;

	if(response_id == GTK_RESPONSE_ACCEPT) {
		GFile *file = NULL;
		GFileOutputStream *output = NULL;
		GtkTextIter start, end;
		GDateTime *date = NULL;
		gchar *date_str = NULL;
		gchar *tmp = NULL;
		GError *error = NULL;

		file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(self));
		output = g_file_replace(file, NULL, FALSE, G_FILE_CREATE_NONE, NULL,
		                        &error);
		g_object_unref(file);

		if(output == NULL) {
			purple_debug_error("debug",
			                   "Unable to open file to save debug log: %s",
			                   error->message);
			g_error_free(error);
			g_object_unref(self);
			return;
		}

		date = g_date_time_new_now_local();
		date_str = g_date_time_format(date, "%c");
		g_date_time_unref(date);

		tmp = g_strdup_printf("Pidgin Debug Log : %s\n", date_str);
		g_output_stream_write_all(G_OUTPUT_STREAM(output), tmp, strlen(tmp),
		                          NULL, NULL, &error);
		g_free(tmp);
		g_free(date_str);

		if(error != NULL) {
			purple_debug_error("debug", "Unable to save debug log: %s",
			                   error->message);
			g_error_free(error);
			g_object_unref(output);
			g_object_unref(self);
			return;
		}

		gtk_text_buffer_get_bounds(win->buffer, &start, &end);
		tmp = gtk_text_buffer_get_text(win->buffer, &start, &end, TRUE);
		g_output_stream_write_all(G_OUTPUT_STREAM(output), tmp, strlen(tmp),
		                          NULL, NULL, &error);
		g_free(tmp);

		if(error != NULL) {
			purple_debug_error("debug", "Unable to save debug log: %s",
			                   error->message);
			g_error_free(error);
			g_object_unref(output);
			g_object_unref(self);
			return;
		}

		g_object_unref(output);
	}

	g_object_unref(self);
}

static void
save_cb(G_GNUC_UNUSED GtkWidget *w, PidginDebugWindow *win)
{
	GtkFileChooserNative *filesel;

	filesel = gtk_file_chooser_native_new(_("Save Debug Log"), GTK_WINDOW(win),
	                                      GTK_FILE_CHOOSER_ACTION_SAVE,
	                                      _("_Save"), _("_Cancel"));

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(filesel),
	                                  "purple-debug.log");

	g_signal_connect(filesel, "response", G_CALLBACK(save_response_cb), win);

	gtk_native_dialog_set_modal(GTK_NATIVE_DIALOG(filesel), TRUE);
	gtk_native_dialog_show(GTK_NATIVE_DIALOG(filesel));
}

static void
clear_cb(G_GNUC_UNUSED GtkWidget *w, PidginDebugWindow *win)
{
	gtk_text_buffer_set_text(win->buffer, "", 0);
}

static void
pause_cb(GtkWidget *w, PidginDebugWindow *win)
{
	win->paused = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w));

	if (!win->paused) {
		GtkTextIter start, end;
		gtk_text_buffer_get_bounds(win->buffer, &start, &end);
		gtk_text_buffer_remove_tag(win->buffer, win->tags.paused,
				&start, &end);
		gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(win->textview),
				win->end_mark, 0, TRUE, 0, 1);
	}
}

/******************************************************************************
 * regex stuff
 *****************************************************************************/
static void
regex_clear_color(GtkWidget *w) {
	gtk_widget_remove_css_class(w, "error");
	gtk_widget_remove_css_class(w, "success");
}

static void
regex_change_color(GtkWidget *w, gboolean success) {
	if (success) {
		gtk_widget_remove_css_class(w, "error");
		gtk_widget_add_css_class(w, "success");
	} else {
		gtk_widget_remove_css_class(w, "success");
		gtk_widget_add_css_class(w, "error");
	}
}

static void
do_regex(PidginDebugWindow *win, GtkTextIter *start, GtkTextIter *end)
{
	GError *error = NULL;
	GMatchInfo *match;
	gint initial_position;
	gint start_pos, end_pos;
	GtkTextIter match_start, match_end;
	gchar *text;

	if (!win->regex) {
		return;
	}

	initial_position = gtk_text_iter_get_offset(start);

	if (!win->invert) {
		/* First hide everything. */
		gtk_text_buffer_apply_tag(win->buffer,
				win->tags.filtered_invisible, start, end);
	}

	text = gtk_text_buffer_get_text(win->buffer, start, end, TRUE);
	g_regex_match(win->regex, text, 0, &match);
	while (g_match_info_matches(match)) {
		g_match_info_fetch_pos(match, 0, &start_pos, &end_pos);
		start_pos += initial_position;
		end_pos += initial_position;

		/* Expand match to full line of message. */
		gtk_text_buffer_get_iter_at_offset(win->buffer,
				&match_start, start_pos);
		gtk_text_iter_set_line_index(&match_start, 0);
		gtk_text_buffer_get_iter_at_offset(win->buffer,
				&match_end, end_pos);
		gtk_text_iter_forward_line(&match_end);

		if (win->invert) {
			/* Make invisible. */
			gtk_text_buffer_apply_tag(win->buffer,
					win->tags.filtered_invisible,
					&match_start, &match_end);
		} else {
			/* Make visible again (with higher priority.) */
			gtk_text_buffer_apply_tag(win->buffer,
					win->tags.filtered_visible,
					&match_start, &match_end);

			if (win->highlight) {
				gtk_text_buffer_get_iter_at_offset(
						win->buffer,
						&match_start,
						start_pos);
				gtk_text_buffer_get_iter_at_offset(
						win->buffer,
						&match_end,
						end_pos);
				gtk_text_buffer_apply_tag(win->buffer,
						win->tags.match,
						&match_start,
						&match_end);
			}
		}

		g_match_info_next(match, &error);
	}

	g_match_info_free(match);
	g_free(text);
}

static void
regex_toggle_filter(PidginDebugWindow *win, gboolean filter)
{
	GtkTextIter start, end;

	gtk_text_buffer_get_bounds(win->buffer, &start, &end);
	gtk_text_buffer_remove_tag(win->buffer, win->tags.match, &start, &end);
	gtk_text_buffer_remove_tag(win->buffer, win->tags.filtered_invisible,
			&start, &end);
	gtk_text_buffer_remove_tag(win->buffer, win->tags.filtered_visible,
			&start, &end);

	if (filter) {
		do_regex(win, &start, &end);
	}
}

static void
regex_pref_filter_cb(G_GNUC_UNUSED const gchar *name,
                     G_GNUC_UNUSED PurplePrefType type,
					 gconstpointer val, gpointer data)
{
	PidginDebugWindow *win = (PidginDebugWindow *)data;
	gboolean active = GPOINTER_TO_INT(val), current;

	if (!win) {
		return;
	}

	current = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(win->filter));
	if (active != current) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(win->filter), active);
	}
}

static void
regex_pref_invert_cb(G_GNUC_UNUSED const gchar *name,
                     G_GNUC_UNUSED PurplePrefType type,
					 gconstpointer val, gpointer data)
{
	PidginDebugWindow *win = (PidginDebugWindow *)data;
	gboolean active = GPOINTER_TO_INT(val);

	win->invert = active;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(win->filter))) {
		regex_toggle_filter(win, TRUE);
	}
}

static void
regex_pref_highlight_cb(G_GNUC_UNUSED const gchar *name,
                        G_GNUC_UNUSED PurplePrefType type,
						gconstpointer val, gpointer data)
{
	PidginDebugWindow *win = (PidginDebugWindow *)data;
	gboolean active = GPOINTER_TO_INT(val);

	win->highlight = active;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(win->filter))) {
		regex_toggle_filter(win, TRUE);
	}
}

static void
regex_changed_cb(G_GNUC_UNUSED GtkWidget *w, PidginDebugWindow *win) {
	const gchar *text;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(win->filter))) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(win->filter), FALSE);
	}

	text = gtk_editable_get_text(GTK_EDITABLE(win->expression));
	purple_prefs_set_string(PIDGIN_PREFS_ROOT "/debug/regex", text);

	if (text == NULL || *text == '\0') {
		regex_clear_color(win->expression);
		gtk_widget_set_sensitive(win->filter, FALSE);
		return;
	}

	g_clear_pointer(&win->regex, g_regex_unref);

	win->regex = g_regex_new(text, G_REGEX_CASELESS|G_REGEX_JAVASCRIPT_COMPAT, 0, NULL);

	if (win->regex == NULL) {
		/* failed to compile */
		regex_change_color(win->expression, FALSE);
		gtk_widget_set_sensitive(win->filter, FALSE);
	} else {
		/* compiled successfully */
		regex_change_color(win->expression, TRUE);
		gtk_widget_set_sensitive(win->filter, TRUE);
	}
}

static void
regex_key_released_cb(G_GNUC_UNUSED GtkEventControllerKey *controller,
                      guint keyval, G_GNUC_UNUSED guint keycode,
                      G_GNUC_UNUSED GdkModifierType state, gpointer data)
{
	PidginDebugWindow *win = data;

	if (gtk_widget_is_sensitive(win->filter)) {
		GtkToggleButton *tb = GTK_TOGGLE_BUTTON(win->filter);
		if ((keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter) &&
			!gtk_toggle_button_get_active(tb))
		{
			gtk_toggle_button_set_active(tb, TRUE);
		}
		if (keyval == GDK_KEY_Escape && gtk_toggle_button_get_active(tb)) {
			gtk_toggle_button_set_active(tb, FALSE);
		}
	}
}

static void
regex_menu_cb(GtkWidget *item, PidginDebugWindow *win)
{
	gboolean active;

	active = gtk_check_button_get_active(GTK_CHECK_BUTTON(item));

	if (item == win->popover_highlight) {
		purple_prefs_set_bool(PIDGIN_PREFS_ROOT "/debug/highlight", active);
	} else if (item == win->popover_invert) {
		purple_prefs_set_bool(PIDGIN_PREFS_ROOT "/debug/invert", active);
	}
}

static void
regex_popup_cb(G_GNUC_UNUSED GtkGestureClick* self, G_GNUC_UNUSED gint n_press,
               gdouble x, gdouble y, gpointer data)
{
	PidginDebugWindow *win = data;

	gtk_popover_set_pointing_to(GTK_POPOVER(win->popover),
	                            &(const GdkRectangle){(int)x, (int)y, 0, 0});
	gtk_popover_popup(GTK_POPOVER(win->popover));
}

static void
regex_filter_toggled_cb(GtkToggleButton *button, PidginDebugWindow *win)
{
	gboolean active;

	active = gtk_toggle_button_get_active(button);

	purple_prefs_set_bool(PIDGIN_PREFS_ROOT "/debug/filter", active);

	regex_toggle_filter(win, active);
}

static void
debug_window_set_filter_level(PidginDebugWindow *win, int level)
{
	gboolean scroll;
	int i;

	if (level != (int)gtk_drop_down_get_selected(GTK_DROP_DOWN(win->filterlevel))) {
		gtk_drop_down_set_selected(GTK_DROP_DOWN(win->filterlevel), level);
	}

	scroll = view_near_bottom(win);
	for (i = 0; i <= PURPLE_DEBUG_FATAL; i++) {
		g_object_set(G_OBJECT(win->tags.level[i]),
				"invisible", i < level,
				NULL);
	}
	if (scroll) {
		gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(win->textview),
				win->end_mark, 0, TRUE, 0, 1);
	}
}

static void
filter_level_pref_changed(G_GNUC_UNUSED const char *name,
                          G_GNUC_UNUSED PurplePrefType type,
                          gconstpointer value, gpointer data)
{
	PidginDebugWindow *win = data;
	int level = GPOINTER_TO_INT(value);

	debug_window_set_filter_level(win, level);
}

static void
filter_level_changed_cb(GObject *obj, G_GNUC_UNUSED GParamSpec *pspec)
{
	GtkDropDown *dropdown = GTK_DROP_DOWN(obj);

	purple_prefs_set_int(PIDGIN_PREFS_ROOT "/debug/filterlevel",
	                     gtk_drop_down_get_selected(dropdown));
}

static void
pidgin_debug_window_dispose(GObject *object)
{
	PidginDebugWindow *win = PIDGIN_DEBUG_WINDOW(object);

	gtk_widget_unparent(win->popover);

	G_OBJECT_CLASS(pidgin_debug_window_parent_class)->dispose(object);
}

static void
pidgin_debug_window_finalize(GObject *object)
{
	PidginDebugWindow *win = PIDGIN_DEBUG_WINDOW(object);

	purple_prefs_disconnect_by_handle(pidgin_debug_get_handle());

	g_clear_pointer(&win->regex, g_regex_unref);

	debug_win = NULL;
	purple_prefs_set_bool(PIDGIN_PREFS_ROOT "/debug/enabled", FALSE);

	G_OBJECT_CLASS(pidgin_debug_window_parent_class)->finalize(object);
}

static void
pidgin_debug_window_class_init(PidginDebugWindowClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->dispose = pidgin_debug_window_dispose;
	obj_class->finalize = pidgin_debug_window_finalize;

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/im/pidgin/Pidgin3/Debug/debug.ui"
	);

	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, textview);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, buffer);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, tags.category);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, tags.filtered_invisible);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, tags.filtered_visible);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, tags.level[0]);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, tags.level[1]);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, tags.level[2]);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, tags.level[3]);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, tags.level[4]);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, tags.level[5]);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, tags.paused);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, filter);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, filterlevel);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, expression);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, tags.match);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, popover);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, popover_invert);
	gtk_widget_class_bind_template_child(
			widget_class, PidginDebugWindow, popover_highlight);
	gtk_widget_class_bind_template_callback(widget_class, save_cb);
	gtk_widget_class_bind_template_callback(widget_class, clear_cb);
	gtk_widget_class_bind_template_callback(widget_class, pause_cb);
	gtk_widget_class_bind_template_callback(widget_class,
			regex_filter_toggled_cb);
	gtk_widget_class_bind_template_callback(widget_class,
			regex_changed_cb);
	gtk_widget_class_bind_template_callback(widget_class, regex_popup_cb);
	gtk_widget_class_bind_template_callback(widget_class, regex_menu_cb);
	gtk_widget_class_bind_template_callback(widget_class,
			regex_key_released_cb);
	gtk_widget_class_bind_template_callback(widget_class,
			filter_level_changed_cb);
}

static void
pidgin_debug_window_init(PidginDebugWindow *win)
{
	gint width, height;
	void *handle;
	GtkTextIter end;

	gtk_widget_init_template(GTK_WIDGET(win));

	gtk_widget_set_parent(win->popover, win->filter);

	width  = purple_prefs_get_int(PIDGIN_PREFS_ROOT "/debug/width");
	height = purple_prefs_get_int(PIDGIN_PREFS_ROOT "/debug/height");

	purple_debug_info("pidgindebug", "Setting dimensions to %d, %d\n",
					width, height);

	gtk_window_set_default_size(GTK_WINDOW(win), width, height);

	g_signal_connect(G_OBJECT(win), "notify::default-width",
	                 G_CALLBACK(save_default_size_cb), NULL);
	g_signal_connect(G_OBJECT(win), "notify::default-height",
	                 G_CALLBACK(save_default_size_cb), NULL);

	handle = pidgin_debug_get_handle();

	/* we purposely disable the toggle button here in case
	 * /purple/gtk/debug/expression has an empty string.  If it does not have
	 * an empty string, the change signal will get called and make the
	 * toggle button sensitive.
	 */
	gtk_widget_set_sensitive(win->filter, FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(win->filter),
	                             purple_prefs_get_bool(PIDGIN_PREFS_ROOT "/debug/filter"));
	purple_prefs_connect_callback(handle, PIDGIN_PREFS_ROOT "/debug/filter",
	                              regex_pref_filter_cb, win);

	/* regex entry */
	gtk_editable_set_text(GTK_EDITABLE(win->expression),
	                      purple_prefs_get_string(PIDGIN_PREFS_ROOT "/debug/regex"));

	/* connect the rest of our pref callbacks */
	win->invert = purple_prefs_get_bool(PIDGIN_PREFS_ROOT "/debug/invert");
	purple_prefs_connect_callback(handle, PIDGIN_PREFS_ROOT "/debug/invert",
	                              regex_pref_invert_cb, win);

	win->highlight = purple_prefs_get_bool(PIDGIN_PREFS_ROOT "/debug/highlight");
	purple_prefs_connect_callback(handle, PIDGIN_PREFS_ROOT "/debug/highlight",
	                              regex_pref_highlight_cb, win);

	gtk_drop_down_set_selected(GTK_DROP_DOWN(win->filterlevel),
	                           purple_prefs_get_int(PIDGIN_PREFS_ROOT "/debug/filterlevel"));

	purple_prefs_connect_callback(handle, PIDGIN_PREFS_ROOT "/debug/filterlevel",
	                              filter_level_pref_changed, win);

	gtk_check_button_set_active(GTK_CHECK_BUTTON(win->popover_invert),
	                            win->invert);
	gtk_check_button_set_active(GTK_CHECK_BUTTON(win->popover_highlight),
	                            win->highlight);

	/* The *start* and *end* marks bound the beginning and end of an
	   insertion, used for filtering. The *end* mark is also used for
	   auto-scrolling. */
	gtk_text_buffer_get_end_iter(win->buffer, &end);
	win->start_mark = gtk_text_buffer_create_mark(win->buffer,
			"start", &end, TRUE);
	win->end_mark = gtk_text_buffer_create_mark(win->buffer,
			"end", &end, FALSE);

	/* Set active filter level in textview */
	debug_window_set_filter_level(win,
			purple_prefs_get_int(PIDGIN_PREFS_ROOT "/debug/filterlevel"));

	clear_cb(NULL, win);
}

static gboolean
debug_enabled_timeout_cb(gpointer data)
{
	gboolean enabled = GPOINTER_TO_INT(data);

	debug_enabled_timer = 0;

	if (enabled) {
		pidgin_debug_window_show();
	} else {
		pidgin_debug_window_hide();
	}

	return FALSE;
}

static void
debug_enabled_cb(G_GNUC_UNUSED const gchar *name,
                 G_GNUC_UNUSED PurplePrefType type,
                 gconstpointer value,
                 G_GNUC_UNUSED gpointer data)
{
	debug_enabled_timer = g_timeout_add(0, debug_enabled_timeout_cb,
	                                    (gpointer)value);
}

static gboolean
pidgin_debug_g_log_handler_cb(gpointer data)
{
	PidginDebugMessage *message = data;
	GtkTextTag *level_tag = NULL;
	gchar *local_time = NULL;
	GtkTextIter end;
	gboolean scroll;

	if (debug_win == NULL ||
			!purple_prefs_get_bool(PIDGIN_PREFS_ROOT "/debug/enabled")) {
		/* The Debug Window may have been closed/disabled after the thread that
		 * sent this message. */
		g_date_time_unref(message->timestamp);
		g_free(message->domain);
		g_free(message->message);
		g_free(message);
		return FALSE;
	}

	scroll = view_near_bottom(debug_win);
	gtk_text_buffer_get_end_iter(debug_win->buffer, &end);
	gtk_text_buffer_move_mark(debug_win->buffer, debug_win->start_mark, &end);

	level_tag = debug_win->tags.level[message->level];
	local_time = g_date_time_format(message->timestamp, "(%H:%M:%S) ");

	gtk_text_buffer_insert_with_tags(
			debug_win->buffer,
			&end,
			local_time,
			-1,
			level_tag,
			debug_win->paused ? debug_win->tags.paused : NULL,
			NULL);

	if (message->domain != NULL && *message->domain != '\0') {
		gtk_text_buffer_insert_with_tags(
				debug_win->buffer,
				&end,
				message->domain,
				-1,
				level_tag,
				debug_win->tags.category,
				debug_win->paused ? debug_win->tags.paused : NULL,
				NULL);
		gtk_text_buffer_insert_with_tags(
				debug_win->buffer,
				&end,
				": ",
				2,
				level_tag,
				debug_win->tags.category,
				debug_win->paused ? debug_win->tags.paused : NULL,
				NULL);
	}

	gtk_text_buffer_insert_with_tags(
			debug_win->buffer,
			&end,
			message->message,
			-1,
			level_tag,
			debug_win->paused ? debug_win->tags.paused : NULL,
			NULL);
	gtk_text_buffer_insert_with_tags(
			debug_win->buffer,
			&end,
			"\n",
			1,
			level_tag,
			debug_win->paused ? debug_win->tags.paused : NULL,
			NULL);

	if (purple_prefs_get_bool(PIDGIN_PREFS_ROOT "/debug/filter") &&
			debug_win->regex) {
		/* Filter out any new messages. */
		GtkTextIter start;

		gtk_text_buffer_get_iter_at_mark(debug_win->buffer, &start,
		                                 debug_win->start_mark);
		gtk_text_buffer_get_iter_at_mark(debug_win->buffer, &end,
		                                 debug_win->end_mark);

		do_regex(debug_win, &start, &end);
	}

	if (scroll) {
		gtk_text_view_scroll_to_mark(
				GTK_TEXT_VIEW(debug_win->textview),
				debug_win->end_mark, 0, TRUE, 0, 1);
	}

	g_free(local_time);
	g_date_time_unref(message->timestamp);
	g_free(message->domain);
	g_free(message->message);
	g_free(message);

	return FALSE;
}

static GLogWriterOutput
pidgin_debug_g_log_handler(GLogLevelFlags log_level, const GLogField *fields,
                           gsize n_fields, G_GNUC_UNUSED gpointer user_data)
{
	PidginDebugMessage *message = NULL;
	gsize i;

	if (debug_win == NULL) {
		if (debug_print_enabled) {
			return g_log_writer_default(log_level, fields, n_fields, user_data);
		} else {
			return G_LOG_WRITER_UNHANDLED;
		}
	}

	message = g_new0(PidginDebugMessage, 1);
	message->timestamp = g_date_time_new_now_local();

	for (i = 0; i < n_fields; i++) {
		if (purple_strequal(fields[i].key, "GLIB_DOMAIN")) {
			message->domain = g_strdup(fields[i].value);
		} else if (purple_strequal(fields[i].key, "MESSAGE")) {
			message->message = g_strdup(fields[i].value);
		}
	}

	if((log_level & G_LOG_LEVEL_ERROR) != 0) {
		message->level = PURPLE_DEBUG_ERROR;
	} else if((log_level & G_LOG_LEVEL_CRITICAL) != 0) {
		message->level = PURPLE_DEBUG_FATAL;
	} else if((log_level & G_LOG_LEVEL_WARNING) != 0) {
		message->level = PURPLE_DEBUG_WARNING;
	} else if((log_level & G_LOG_LEVEL_MESSAGE) != 0) {
		message->level = PURPLE_DEBUG_INFO;
	} else if((log_level & G_LOG_LEVEL_INFO) != 0) {
		message->level = PURPLE_DEBUG_INFO;
	} else if((log_level & G_LOG_LEVEL_DEBUG) != 0) {
		message->level = PURPLE_DEBUG_MISC;
	} else {
		message->level = PURPLE_DEBUG_MISC;
	}

	g_timeout_add(0, pidgin_debug_g_log_handler_cb, message);

	if (debug_print_enabled) {
		return g_log_writer_default(log_level, fields, n_fields, user_data);
	} else {
		return G_LOG_WRITER_HANDLED;
	}
}

void
pidgin_debug_window_show(void)
{
	if (debug_win == NULL) {
		GApplication *application = NULL;
		PidginApplication *pidgin_application = NULL;
		GtkWindow *parent = NULL;

		application = g_application_get_default();
		pidgin_application = PIDGIN_APPLICATION(application);
		parent = pidgin_application_get_active_window(pidgin_application);

		debug_win = PIDGIN_DEBUG_WINDOW(
				g_object_new(PIDGIN_TYPE_DEBUG_WINDOW, NULL));

		gtk_window_set_transient_for(GTK_WINDOW(debug_win), parent);
	}

	gtk_window_present_with_time(GTK_WINDOW(debug_win), GDK_CURRENT_TIME);

	purple_prefs_set_bool(PIDGIN_PREFS_ROOT "/debug/enabled", TRUE);
}

void
pidgin_debug_window_hide(void)
{
	if (debug_win != NULL) {
		gtk_window_destroy(GTK_WINDOW(debug_win));
	}
}

void
pidgin_debug_init_handler(void)
{
	g_log_set_writer_func(pidgin_debug_g_log_handler, NULL, NULL);
}

void
pidgin_debug_set_print_enabled(gboolean enable)
{
	debug_print_enabled = enable;
}

void
pidgin_debug_init(void)
{
	/* Debug window preferences. */
	/*
	 * NOTE: This must be set before prefs are loaded, and the callbacks
	 *       set after they are loaded, since prefs sets the enabled
	 *       preference here and that loads the window, which calls the
	 *       configure event, which overrides the width and height! :P
	 */

	purple_prefs_add_none(PIDGIN_PREFS_ROOT "/debug");

	/* Controls printing to the debug window */
	purple_prefs_add_bool(PIDGIN_PREFS_ROOT "/debug/enabled", FALSE);
	purple_prefs_add_int(PIDGIN_PREFS_ROOT "/debug/filterlevel",
	                     PURPLE_DEBUG_ALL);

	purple_prefs_add_int(PIDGIN_PREFS_ROOT "/debug/width",  450);
	purple_prefs_add_int(PIDGIN_PREFS_ROOT "/debug/height", 250);

	purple_prefs_add_string(PIDGIN_PREFS_ROOT "/debug/regex", "");
	purple_prefs_add_bool(PIDGIN_PREFS_ROOT "/debug/filter", FALSE);
	purple_prefs_add_bool(PIDGIN_PREFS_ROOT "/debug/invert", FALSE);
	purple_prefs_add_bool(PIDGIN_PREFS_ROOT "/debug/case_insensitive", FALSE);
	purple_prefs_add_bool(PIDGIN_PREFS_ROOT "/debug/highlight", FALSE);

	pref_callback_id = purple_prefs_connect_callback(NULL,
	                                                 PIDGIN_PREFS_ROOT "/debug/enabled",
	                                                 debug_enabled_cb, NULL);
}

void
pidgin_debug_uninit(void)
{
	if(pref_callback_id != 0) {
		purple_prefs_disconnect_callback(pref_callback_id);
	}
	pref_callback_id = 0;
	if(debug_enabled_timer != 0) {
		g_source_remove(debug_enabled_timer);
	}
	debug_enabled_timer = 0;
}

void *
pidgin_debug_get_handle(void) {
	static int handle;

	return &handle;
}

