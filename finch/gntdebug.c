/*
 * finch
 *
 * Finch is the legal property of its developers, whose names are too numerous
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

#include <config.h>

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <glib/gstdio.h>

#include <purple.h>

#include <gnt.h>

#include "gntdebug.h"

#include <stdio.h>
#include <string.h>

#define PREF_ROOT "/finch/debug"

struct _FinchDebugUi
{
	GObject parent;

	/* Other members, including private data. */
};

static gboolean
handle_fprintf_stderr_cb(GIOChannel *source, G_GNUC_UNUSED GIOCondition cond,
                         G_GNUC_UNUSED gpointer data)
{
	gssize size;
	char message[1024];

	size = read(g_io_channel_unix_get_fd(source), message, sizeof(message) - 1);
	if (size <= 0) {
		/* Something bad probably happened elsewhere ... let's ignore */
	} else {
		message[size] = '\0';
		g_log("stderr", G_LOG_LEVEL_WARNING, "%s", message);
	}

	return TRUE;
}

static void
handle_fprintf_stderr(gboolean stop)
{
	GIOChannel *stderrch;
	static int readhandle = -1;
	int pipes[2];

	if (stop) {
		if (readhandle >= 0) {
			g_source_remove(readhandle);
			readhandle = -1;
		}
		return;
	}
	if (purple_input_pipe(pipes)) {
		readhandle = -1;
		return;
	};
	dup2(pipes[1], STDERR_FILENO);

	stderrch = g_io_channel_unix_new(pipes[0]);
	g_io_channel_set_close_on_unref(stderrch, TRUE);
	readhandle = g_io_add_watch_full(stderrch, G_PRIORITY_HIGH,
			G_IO_IN | G_IO_ERR | G_IO_PRI,
			handle_fprintf_stderr_cb, NULL, NULL);
	g_io_channel_unref(stderrch);
}

static struct
{
	GntWidget *window;
	GntWidget *tview;
	GntWidget *search;
	gboolean paused;
} debug;

static void
reset_debug_win(G_GNUC_UNUSED GntWidget *w, G_GNUC_UNUSED gpointer data)
{
	debug.window = debug.tview = debug.search = NULL;
}

static void
clear_debug_win(G_GNUC_UNUSED GntWidget *w, GntTextView *tv)
{
	gnt_text_view_clear(tv);
}

static void
print_stderr(const char *string)
{
	g_printerr("%s", string);
}

static void
toggle_pause(G_GNUC_UNUSED GntWidget *w, G_GNUC_UNUSED gpointer n)
{
	debug.paused = !debug.paused;
}

static GLogWriterOutput
finch_debug_g_log_handler(GLogLevelFlags log_level, const GLogField *fields,
                          gsize n_fields, G_GNUC_UNUSED gpointer user_data)
{
	const gchar *domain = NULL;
	const gchar *msg = NULL;
	const gchar *search_str = NULL;
	gint pos = 0;
	GntTextFormatFlags flag = 0;
	GDateTime *local_date_time = NULL;
	gchar *local_time = NULL;
	gsize i;

	if (debug.window == NULL || debug.paused) {
		return G_LOG_WRITER_UNHANDLED;
	}

	for (i = 0; i < n_fields; i++) {
		if (purple_strequal(fields[i].key, "GLIB_DOMAIN")) {
			domain = fields[i].value;
		} else if (purple_strequal(fields[i].key, "MESSAGE")) {
			msg = fields[i].value;
		}
	}

	if (msg == NULL) {
		return G_LOG_WRITER_UNHANDLED;
	}
	if (domain == NULL) {
		domain = "g_log";
	}

	/* Filter out log line if we have a search term, and it doesn't match
	 * the domain or message. */
	search_str = gnt_entry_get_text(GNT_ENTRY(debug.search));
	if (search_str != NULL && *search_str != '\0') {
		if (g_strrstr(domain, search_str) == NULL &&
		    g_strrstr(msg, search_str) == NULL)
		{
			return G_LOG_WRITER_UNHANDLED;
		}
	}

	pos = gnt_text_view_get_lines_below(GNT_TEXT_VIEW(debug.tview));

	local_date_time = g_date_time_new_now_local();
	local_time = g_date_time_format(local_date_time, "%H:%M:%S ");
	g_date_time_unref(local_date_time);

	gnt_text_view_append_text_with_flags(GNT_TEXT_VIEW(debug.tview), local_time,
	                                     GNT_TEXT_FLAG_NORMAL);
	g_free(local_time);

	gnt_text_view_append_text_with_flags(GNT_TEXT_VIEW(debug.tview), domain,
	                                     GNT_TEXT_FLAG_BOLD);
	gnt_text_view_append_text_with_flags(GNT_TEXT_VIEW(debug.tview), ": ",
	                                     GNT_TEXT_FLAG_BOLD);

	flag = GNT_TEXT_FLAG_NORMAL;
	switch (log_level & G_LOG_LEVEL_MASK) {
		case G_LOG_LEVEL_WARNING:
			flag |= GNT_TEXT_FLAG_UNDERLINE;
			/* fallthrough */
		case G_LOG_LEVEL_ERROR:
			flag |= GNT_TEXT_FLAG_BOLD;
			break;
		default:
			break;
	}

	gnt_text_view_append_text_with_flags(GNT_TEXT_VIEW(debug.tview), msg,
	                                     flag);
	gnt_text_view_append_text_with_flags(GNT_TEXT_VIEW(debug.tview), "\n",
	                                     GNT_TEXT_FLAG_NORMAL);
	if (pos <= 1) {
		gnt_text_view_scroll(GNT_TEXT_VIEW(debug.tview), 0);
	}

	return G_LOG_WRITER_HANDLED;
}

static void
size_changed_cb(GntWidget *widget, G_GNUC_UNUSED int oldw,
                G_GNUC_UNUSED int oldh)
{
	int w, h;
	gnt_widget_get_size(widget, &w, &h);
	purple_prefs_set_int(PREF_ROOT "/size/width", w);
	purple_prefs_set_int(PREF_ROOT "/size/height", h);
}

static gboolean
for_real(gpointer entry)
{
	purple_prefs_set_string(PREF_ROOT "/filter", gnt_entry_get_text(entry));
	return FALSE;
}

static void
update_filter_string(GntEntry *entry, G_GNUC_UNUSED gpointer data)
{
	int id = g_timeout_add_seconds(1, for_real, entry);
	g_object_set_data_full(G_OBJECT(entry), "update-filter", GINT_TO_POINTER(id),
					(GDestroyNotify)g_source_remove);
}

static void
file_save(GntFileSel *fs, const char *path, G_GNUC_UNUSED const char *file,
          GntTextView *tv)
{
	FILE *fp;
	GDateTime *date = NULL;
	gchar *date_str = NULL;

	if ((fp = g_fopen(path, "w+")) == NULL) {
		purple_notify_error(NULL, NULL, _("Unable to open file."), NULL, NULL);
		return;
	}

	date = g_date_time_new_now_local();
	date_str = g_date_time_format(date, "%c");

	fprintf(fp, "Finch Debug Log : %s\n", date_str);
	fprintf(fp, "%s", gnt_text_view_get_text(tv));
	fclose(fp);
	gnt_widget_destroy(GNT_WIDGET(fs));

	g_free(date_str);
	g_date_time_unref(date);
}

static void
save_debug_win(G_GNUC_UNUSED GntWidget *w, GntTextView *tv)
{
	GntWidget *window = gnt_file_sel_new();
	GntFileSel *sel = GNT_FILE_SEL(window);
	gnt_file_sel_set_current_location(sel, purple_home_dir());
	gnt_file_sel_set_suggested_filename(sel, "debug.txt");
	g_signal_connect(G_OBJECT(sel), "file_selected", G_CALLBACK(file_save), tv);
	g_signal_connect(G_OBJECT(sel), "cancelled", G_CALLBACK(gnt_widget_destroy), NULL);
	gnt_widget_show(window);
}

void
finch_debug_window_show(void)
{
	GntWidget *wid, *box, *label;

	debug.paused = FALSE;
	if (debug.window) {
		gnt_window_present(debug.window);
		return;
	}

	debug.window = gnt_vbox_new(FALSE);
	gnt_box_set_toplevel(GNT_BOX(debug.window), TRUE);
	gnt_box_set_title(GNT_BOX(debug.window), _("Debug Window"));
	gnt_box_set_pad(GNT_BOX(debug.window), 0);
	gnt_box_set_alignment(GNT_BOX(debug.window), GNT_ALIGN_MID);

	debug.tview = gnt_text_view_new();
	gnt_box_add_widget(GNT_BOX(debug.window), debug.tview);
	gnt_widget_set_size(debug.tview,
			purple_prefs_get_int(PREF_ROOT "/size/width"),
			purple_prefs_get_int(PREF_ROOT "/size/height"));
	g_signal_connect(G_OBJECT(debug.tview), "size_changed", G_CALLBACK(size_changed_cb), NULL);

	gnt_box_add_widget(GNT_BOX(debug.window), gnt_line_new(FALSE));

	box = gnt_hbox_new(FALSE);
	gnt_box_set_alignment(GNT_BOX(box), GNT_ALIGN_MID);
	gnt_box_set_fill(GNT_BOX(box), FALSE);

	/* XXX: Setting the GROW_Y for the following widgets don't make sense. But right now
	 * it's necessary to make the width of the debug window resizable ... like I said,
	 * it doesn't make sense. The bug is likely in the packing in gntbox.c.
	 */
	wid = gnt_button_new(_("Clear"));
	g_signal_connect(G_OBJECT(wid), "activate", G_CALLBACK(clear_debug_win), debug.tview);
	gnt_widget_set_grow_y(wid, TRUE);
	gnt_box_add_widget(GNT_BOX(box), wid);

	wid = gnt_button_new(_("Save"));
	g_signal_connect(G_OBJECT(wid), "activate", G_CALLBACK(save_debug_win), debug.tview);
	gnt_widget_set_grow_y(wid, TRUE);
	gnt_box_add_widget(GNT_BOX(box), wid);

	debug.search = gnt_entry_new(purple_prefs_get_string(PREF_ROOT "/filter"));
	label = gnt_label_new(_("Filter:"));
	gnt_widget_set_grow_x(label, FALSE);
	gnt_box_add_widget(GNT_BOX(box), label);
	gnt_box_add_widget(GNT_BOX(box), debug.search);
	g_signal_connect(G_OBJECT(debug.search), "text_changed", G_CALLBACK(update_filter_string), NULL);

	wid = gnt_check_box_new(_("Pause"));
	g_signal_connect(G_OBJECT(wid), "toggled", G_CALLBACK(toggle_pause), NULL);
	gnt_widget_set_grow_y(wid, TRUE);
	gnt_box_add_widget(GNT_BOX(box), wid);

	gnt_box_add_widget(GNT_BOX(debug.window), box);
	gnt_widget_set_grow_y(box, TRUE);

	gnt_widget_set_name(debug.window, "debug-window");

	g_signal_connect(G_OBJECT(debug.window), "destroy", G_CALLBACK(reset_debug_win), NULL);
	gnt_text_view_attach_scroll_widget(GNT_TEXT_VIEW(debug.tview), debug.window);
	gnt_text_view_attach_pager_widget(GNT_TEXT_VIEW(debug.tview), debug.window);

	gnt_widget_show(debug.window);
}

void
finch_debug_init_handler(void)
{
	g_log_set_writer_func(finch_debug_g_log_handler, NULL, NULL);
}

void
finch_debug_init(void)
{
	g_set_print_handler(print_stderr);   /* Redirect the debug messages to stderr */
	handle_fprintf_stderr(FALSE);

	purple_prefs_add_none(PREF_ROOT);
	purple_prefs_add_string(PREF_ROOT "/filter", "");
	purple_prefs_add_none(PREF_ROOT "/size");
	purple_prefs_add_int(PREF_ROOT "/size/width", 60);
	purple_prefs_add_int(PREF_ROOT "/size/height", 15);
}

void
finch_debug_uninit(void)
{
	handle_fprintf_stderr(TRUE);
}
