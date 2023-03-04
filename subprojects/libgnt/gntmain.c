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

#define _GNU_SOURCE
#if (defined(__APPLE__) || defined(__unix__)) && !defined(__FreeBSD__) && !defined(__OpenBSD__)
#define _XOPEN_SOURCE_EXTENDED
#endif

#include <gmodule.h>

#include <sys/types.h>
#ifndef _WIN32
#include <sys/wait.h>
#endif

#include "gntinternal.h"
#undef GNT_LOG_DOMAIN
#define GNT_LOG_DOMAIN "Main"

#include "gntbox.h"
#include "gntbutton.h"
#include "gntcolors.h"
#include "gntclipboard.h"
#include "gntkeys.h"
#include "gntlabel.h"
#include "gntmenu.h"
#include "gntstyle.h"
#include "gnttree.h"
#include "gntutils.h"
#include "gntwindow.h"
#include "gntwm.h"

#include "gntboxprivate.h"
#include "gntmenuprivate.h"
#include "gntwmprivate.h"
#include "gntwsprivate.h"

#include "gntwidgetprivate.h"

#include <panel.h>

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#ifdef _WIN32
#undef _getch
#undef getch
#include <windows.h>
#include <conio.h>
#endif

/*
 * Notes: Interesting functions to look at:
 * scr_dump, scr_init, scr_restore: for workspaces
 *
 * Need to wattrset for colors to use with PDCurses.
 */

static GIOChannel *channel = NULL;
static guint channel_read_callback = 0;
static guint channel_error_callback = 0;

static gboolean ascii_only;
static gboolean mouse_enabled;

static void setup_io(void);

static gboolean refresh_screen(void);

static GntWM *wm; // -V707
static GntClipboard *clipboard;

int gnt_need_conversation_to_locale;

static gchar *custom_config_dir = NULL;

#define HOLDING_ESCAPE  (escape_stuff.timer != 0)

static struct {
	guint timer;
} escape_stuff;

static void
endwin_cb(G_GNUC_UNUSED gpointer data) {
	endwin();
}

static gboolean
escape_timeout(G_GNUC_UNUSED gpointer data)
{
	gnt_wm_process_input(wm, "\033");
	escape_stuff.timer = 0;
	return FALSE;
}

void
gnt_set_config_dir(const gchar *config_dir)
{
	if (channel) {
		gnt_warning("gnt_set_config_dir failed: %s",
			"gnt already initialized");
	}
	g_free(custom_config_dir);
	custom_config_dir = g_strdup(config_dir);
}

const gchar *
gnt_get_config_dir(void)
{
	if (custom_config_dir == NULL) {
		custom_config_dir =
		        g_build_filename(g_get_user_config_dir(), "gnt", NULL);
	}
	return custom_config_dir;
}

#ifndef _WIN32
/**
 * detect_mouse_action:
 *
 * Mouse support:
 *
 *  - bring a window on top if you click on its taskbar
 *  - click on the top-bar of the active window and drag+drop to move a window
 *  - click on a window to bring it to focus
 *   - allow scrolling in tree/textview on wheel-scroll event
 *   - click to activate button or select a row in tree
 *  wishlist:
 *   - have a little [X] on the windows, and clicking it will close that window.
 */
static gboolean
detect_mouse_action(const char *buffer)
{
	int x, y;
	static enum {
		MOUSE_NONE,
		MOUSE_LEFT,
		MOUSE_RIGHT,
		MOUSE_MIDDLE
	} button = MOUSE_NONE;
	static GntWidget *remember = NULL;
	static int offset = 0;
	GntMouseEvent event;
	GntWidget *widget = NULL;
	PANEL *p = NULL;

	if (gnt_ws_is_empty(gnt_wm_get_current_workspace(wm)) ||
	    buffer[0] != 27) {
		return FALSE;
	}

	buffer++;
	if (strlen(buffer) < 5)
		return FALSE;

	x = buffer[3];
	y = buffer[4];
	if (x < 0)	x += 256;
	if (y < 0)	y += 256;
	x -= 33;
	y -= 33;

	while ((p = panel_below(p)) != NULL) {
		const GntNode *node = panel_userptr(p);
		GntWidget *wid;
		gint widx, widy, width, height;
		if (!node)
			continue;
		wid = node->me;
		gnt_widget_get_position(wid, &widx, &widy);
		gnt_widget_get_internal_size(wid, &width, &height);
		if (widx <= x && x < widx + width) {
			if (widy <= y && y < widy + height) {
				widget = wid;
				break;
			}
		}
	}

	if (strncmp(buffer, "[M ", 3) == 0) {
		/* left button down */
		/* Bring the window you clicked on to front */
		/* If you click on the topbar, then you can drag to move the window */
		event = GNT_LEFT_MOUSE_DOWN;
	} else if (strncmp(buffer, "[M\"", 3) == 0) {
		/* right button down */
		event = GNT_RIGHT_MOUSE_DOWN;
	} else if (strncmp(buffer, "[M!", 3) == 0) {
		/* middle button down */
		event = GNT_MIDDLE_MOUSE_DOWN;
	} else if (strncmp(buffer, "[M`", 3) == 0) {
		/* wheel up*/
		event = GNT_MOUSE_SCROLL_UP;
	} else if (strncmp(buffer, "[Ma", 3) == 0) {
		/* wheel down */
		event = GNT_MOUSE_SCROLL_DOWN;
	} else if (strncmp(buffer, "[M#", 3) == 0) {
		/* button up */
		event = GNT_MOUSE_UP;
	} else
		return FALSE;

	if (widget && gnt_wm_process_click(wm, event, x, y, widget))
		return TRUE;

	if (event == GNT_LEFT_MOUSE_DOWN && widget &&
	    !gnt_wm_is_list_window(wm, widget) &&
	    !gnt_widget_get_transient(widget)) {
		gint widgetx, widgety;
		if (!gnt_ws_is_top_widget(gnt_wm_get_current_workspace(wm),
		                          widget)) {
			gnt_wm_raise_window(wm, widget);
		}
		gnt_widget_get_position(widget, &widgetx, &widgety);
		if (y == widgety) {
			offset = x - widgetx;
			remember = widget;
			button = MOUSE_LEFT;
		}
	} else if (event == GNT_MOUSE_UP) {
		if (button == MOUSE_NONE && y == getmaxy(stdscr) - 1) {
			/* Clicked on the taskbar */
			int n = g_list_length(gnt_ws_get_widgets(
			        gnt_wm_get_current_workspace(wm)));
			if (n) {
				int width = getmaxx(stdscr) / n;
				gnt_bindable_perform_action_named(GNT_BINDABLE(wm), "switch-window-n", x/width, NULL);
			}
		} else if (button == MOUSE_LEFT && remember) {
			x -= offset;
			if (x < 0)	x = 0;
			if (y < 0)	y = 0;
			gnt_screen_move_widget(remember, x, y);
		}
		button = MOUSE_NONE;
		remember = NULL;
		offset = 0;
	}

	if (widget)
		gnt_widget_clicked(widget, event, x, y);
	return TRUE;
}
#endif

static gboolean
io_invoke_error(GIOChannel *source, G_GNUC_UNUSED GIOCondition cond,
                gpointer data)
{
	/* XXX: it throws an error after evey io_invoke, I have no idea why */
#ifndef _WIN32
	guint id = GPOINTER_TO_UINT(data);

	g_source_remove(id);
	g_io_channel_unref(source);

	channel = NULL;
	setup_io();
#endif

	return TRUE;
}

static gboolean
io_invoke(GIOChannel *source, G_GNUC_UNUSED GIOCondition cond,
          G_GNUC_UNUSED gpointer data)
{
#ifdef _WIN32
	/* We need:
	 * - 1 for escape prefix
	 * - 6 for gunichar-to-gchar conversion (see g_unichar_to_utf8)
	 * - 1 for the terminating NUL
	 * or:
	 * - 1 for escape prefix
	 * - 1 for special key prefix
	 * - 1 for the key
	 * - 1 for the terminating NUL
	 */
	gchar keys[8];
	gchar *k = keys;
	int ch;
	gboolean is_special = FALSE;
	gboolean is_escape = FALSE;

	if (gnt_wm_get_keypress_mode(wm) == GNT_KP_MODE_WAIT_ON_CHILD) {
		return FALSE;
	}

	if (HOLDING_ESCAPE) {
		is_escape = TRUE;
		g_source_remove(escape_stuff.timer);
		escape_stuff.timer = 0;
	} else if (GetAsyncKeyState(VK_LMENU)) { /* left-ALT key */
		is_escape = TRUE;
	}
	if (is_escape) {
		*k = GNT_ESCAPE;
		k++;
	}

	ch = _getwch(); /* we could use _getwch_nolock */

	/* a small hack - we don't want to put NUL anywhere */
	if (ch == 0x00)
		ch = 0xE1;

	if (ch == 0xE0 || ch == 0xE1) {
		is_special = TRUE;
		if (!is_escape) {
			*k = GNT_ESCAPE;
			k++;
		}
		*k = ch;
		k++;
		ch = _getwch();
	}

	if (ch == 0x1B && !is_special) { /* ESC key */
		escape_stuff.timer = g_timeout_add(250, escape_timeout, NULL);
		return TRUE;
	}

	if (wm)
		gnt_wm_set_event_stack(wm, TRUE);

	if (is_special) {
		if (ch > 0xFF) {
			gnt_warning("a special key out of gchar range (%d)", ch);
			return TRUE;
		}
		*k = ch;
		k++;
	} else {
		gint result_len;

		result_len = g_unichar_to_utf8(ch, k);
		k += result_len;
	}
	*k = '\0';

	/* TODO: we could call detect_mouse_action here, but no
	 * events are triggered (yet?) for mouse on win32.
	 */

	gnt_wm_process_input(wm, keys);

	if (wm)
		gnt_wm_set_event_stack(wm, FALSE);

	return TRUE;
#else
	char keys[256];
	gssize rd;
	char *k;
	char *cvrt = NULL;

	if (gnt_wm_get_keypress_mode(wm) == GNT_KP_MODE_WAIT_ON_CHILD) {
		return FALSE;
	}

	rd = read(STDIN_FILENO, keys + HOLDING_ESCAPE, sizeof(keys) - 1 - HOLDING_ESCAPE);
	if (rd < 0)
	{
		int ch = getch(); /* This should return ERR, but let's see what it really returns */
		endwin();
		printf("ERROR: %s\n", g_strerror(errno));
		printf("File descriptor is: %d\n\nGIOChannel is: %p\ngetch() = %d\n", STDIN_FILENO, source, ch);
		raise(SIGABRT);
	}
	else if (rd == 0)
	{
		endwin();
		printf("EOF\n");
		raise(SIGABRT);
	}

	rd += HOLDING_ESCAPE;
	if (HOLDING_ESCAPE) {
		keys[0] = GNT_ESCAPE;
		g_source_remove(escape_stuff.timer);
		escape_stuff.timer = 0;
	}
	keys[rd] = 0;
	gnt_wm_set_event_stack(wm, TRUE);

	cvrt = g_locale_to_utf8(keys, rd, (gsize*)&rd, NULL, NULL);
	k = cvrt ? cvrt : keys;
	if (mouse_enabled && detect_mouse_action(k))
		goto end;

	while (rd) {
		char back;
		int p;

		if (k[0] == GNT_ESCAPE && rd == 1) {
			escape_stuff.timer = g_timeout_add(250, escape_timeout, NULL);
			break;
		}

		gnt_keys_refine(k);
		p = MAX(1, gnt_keys_find_combination(k));
		back = k[p];
		k[p] = '\0';
		gnt_wm_process_input(wm, k);     /* XXX: */
		k[p] = back;
		rd -= p;
		k += p;
	}
end:
	if (wm)
		gnt_wm_set_event_stack(wm, FALSE);
	g_free(cvrt);
	return TRUE;
#endif
}

static void
setup_io(void)
{
	guint result;

#ifdef _WIN32
	channel = g_io_channel_win32_new_fd(STDIN_FILENO);
#else
	channel = g_io_channel_unix_new(STDIN_FILENO);
#endif

	if (channel == NULL) {
		gnt_warning("failed creating new channel%s", "");
		return;
	}

	g_io_channel_set_close_on_unref(channel, TRUE);

	channel_read_callback = result = g_io_add_watch_full(channel,  G_PRIORITY_HIGH,
					(G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_PRI),
					io_invoke, NULL, NULL);

	channel_error_callback = g_io_add_watch_full(
	        channel, G_PRIORITY_HIGH, (G_IO_NVAL), io_invoke_error,
	        GUINT_TO_POINTER(result), NULL);

	g_io_channel_unref(channel);
}

static gboolean
refresh_screen(void)
{
	gnt_bindable_perform_action_named(GNT_BINDABLE(wm), "refresh-screen", NULL);
	return FALSE;
}

static gboolean
refresh_screen_cb(G_GNUC_UNUSED gpointer data)
{
	return refresh_screen();
}

#ifndef _WIN32
/* Xerox */
static void
clean_pid(void)
{
	int status;
	pid_t pid;

	do {
		pid = waitpid(-1, &status, WNOHANG);
	} while (pid != 0 && pid != (pid_t)-1);

	if ((pid == (pid_t) - 1) && (errno != ECHILD)) {
		char errmsg[BUFSIZ];
		g_snprintf(errmsg, BUFSIZ, "Warning: waitpid() returned %d", pid);
		perror(errmsg);
	}
}
#endif

static void
exit_confirmed(G_GNUC_UNUSED gpointer data)
{
	gnt_bindable_perform_action_named(GNT_BINDABLE(wm), "wm-quit", NULL);
}

static void
exit_win_close(G_GNUC_UNUSED GntWidget *w, GntWidget **win)
{
	*win = NULL;
}

static void
ask_before_exit(void)
{
	static GntWidget *win = NULL;
	GntMenu *menu;
	GntWidget *bbox, *button;

	menu = gnt_wm_get_menu(wm);
	while (menu) {
		gnt_widget_hide(GNT_WIDGET(menu));
		menu = gnt_menu_get_parent_menu(menu);
	}
	gnt_wm_set_menu(wm, NULL);

	if (win)
		goto raise;

	win = gnt_vwindow_new(FALSE);
	gnt_box_add_widget(GNT_BOX(win), gnt_label_new("Are you sure you want to quit?"));
	gnt_box_set_title(GNT_BOX(win), "Quit?");
	gnt_box_set_alignment(GNT_BOX(win), GNT_ALIGN_MID);
	g_signal_connect(G_OBJECT(win), "destroy", G_CALLBACK(exit_win_close), &win);

	bbox = gnt_hbox_new(FALSE);
	gnt_box_add_widget(GNT_BOX(win), bbox);

	button = gnt_button_new("Quit");
	g_signal_connect(G_OBJECT(button), "activate", G_CALLBACK(exit_confirmed), NULL);
	gnt_box_add_widget(GNT_BOX(bbox), button);

	button = gnt_button_new("Cancel");
	g_signal_connect_swapped(G_OBJECT(button), "activate", G_CALLBACK(gnt_widget_destroy), win);
	gnt_box_add_widget(GNT_BOX(bbox), button);

	gnt_widget_show(win);
raise:
	gnt_wm_raise_window(wm, win);
}

#ifdef SIGWINCH
static void (*org_winch_handler)(int);
static void (*org_winch_handler_sa)(int, siginfo_t *, void *);
#endif

#ifdef _WIN32
static BOOL WINAPI
CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType) {
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
		ask_before_exit();
		return TRUE;

	default:
		return FALSE;
	}
}
#else
static void
sighandler(int sig, siginfo_t *info, void *data)
{
	switch (sig) {
#ifdef SIGWINCH
	case SIGWINCH:
		erase();
		g_idle_add(refresh_screen_cb, NULL);
		if (org_winch_handler)
			org_winch_handler(sig);
		if (org_winch_handler_sa)
			org_winch_handler_sa(sig, info, data);
		break;
#endif
#ifndef _WIN32
	case SIGCHLD:
		clean_pid();
		break;
#endif
	case SIGINT:
		ask_before_exit();
		break;
	}
}
#endif

static void
init_wm(void)
{
	const char *name = gnt_style_get(GNT_STYLE_WM);
	gpointer handle;

	if (name && *name) {
		handle = g_module_open(name, G_MODULE_BIND_LAZY);
		if (handle) {
			gboolean (*init)(GntWM **);
			if (g_module_symbol(handle, "gntwm_init", (gpointer)&init)) {
				init(&wm);
			}
		}
	}
	if (wm == NULL)
		wm = g_object_new(GNT_TYPE_WM, NULL);
}

void
gnt_init(void)
{
	char *filename;
	const char *locale;
#ifndef _WIN32
	struct sigaction act;
#ifdef SIGWINCH
	struct sigaction oact;
#endif
#endif

	if (channel)
		return;

#ifdef _WIN32
	/* UTF-8 for input */
	/* TODO: check it with NO_WIDECHAR. */
	SetConsoleCP(65001);
#endif

	locale = setlocale(LC_ALL, "");

	setup_io();

#if !NCURSES_WIDECHAR
	ascii_only = TRUE;
	(void)locale;
#else
	if (locale && (strstr(locale, "UTF") || strstr(locale, "utf"))) {
		ascii_only = FALSE;
	} else {
		ascii_only = TRUE;
		gnt_need_conversation_to_locale = TRUE;
	}
#endif

	initscr();
	typeahead(-1);
	noecho();

	gnt_init_keys();
	gnt_init_styles();

	filename = g_build_filename(gnt_get_config_dir(), "gntrc", NULL);
	gnt_style_read_configure_file(filename);
	g_free(filename);

	gnt_init_colors();

	wbkgdset(stdscr, '\0' | gnt_color_pair(GNT_COLOR_NORMAL));
	refresh();

#ifdef ALL_MOUSE_EVENTS
	if ((mouse_enabled = gnt_style_get_bool(GNT_STYLE_MOUSE, FALSE)))
		mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
#endif

	wbkgdset(stdscr, '\0' | gnt_color_pair(GNT_COLOR_NORMAL));
	werase(stdscr);
	wrefresh(stdscr);

#ifdef _WIN32
	SetConsoleCtrlHandler(CtrlHandler, TRUE);
#else
	act.sa_sigaction = sighandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;

#ifdef SIGWINCH
	org_winch_handler = NULL;
	org_winch_handler_sa = NULL;
	sigaction(SIGWINCH, &act, &oact);
	if (oact.sa_flags & SA_SIGINFO)
	{
		org_winch_handler_sa = oact.sa_sigaction;
	}
	else if (oact.sa_handler != SIG_DFL && oact.sa_handler != SIG_IGN)
	{
		org_winch_handler = oact.sa_handler;
	}
#endif
	sigaction(SIGCHLD, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	signal(SIGPIPE, SIG_IGN);
#endif

	init_wm();
	refresh_screen();

	clipboard = g_object_new(GNT_TYPE_CLIPBOARD, NULL);
}

void
gnt_main(void)
{
	GMainLoop *loop = g_main_loop_new(NULL, FALSE);
	gnt_wm_set_mainloop(wm, loop);
	g_main_loop_run(loop);
}

/*********************************
 * Stuff for 'window management' *
 *********************************/

void gnt_window_present(GntWidget *window)
{
	if (gnt_wm_get_event_stack(wm)) {
		gnt_wm_raise_window(wm, window);
	} else {
		gnt_widget_set_urgent(window);
	}
}

void gnt_screen_occupy(GntWidget *widget)
{
	gnt_wm_new_window(wm, widget);
}

void gnt_screen_release(GntWidget *widget)
{
	if (wm)
		gnt_wm_window_close(wm, widget);
}

void gnt_screen_update(GntWidget *widget)
{
	gnt_wm_update_window(wm, widget);
}

gboolean gnt_widget_has_focus(GntWidget *widget)
{
	GntWidget *w;
	if (!widget)
		return FALSE;

	if (GNT_IS_MENU(widget))
		return TRUE;

	w = widget;

	widget = gnt_widget_get_toplevel(widget);

	if (gnt_wm_is_list_window(wm, widget)) {
		return TRUE;
	}
	if (gnt_ws_is_top_widget(gnt_wm_get_current_workspace(wm), widget)) {
		if (GNT_IS_BOX(widget) &&
		    (gnt_box_get_active(GNT_BOX(widget)) == w || widget == w)) {
			return TRUE;
		}
	}
	return FALSE;
}

void gnt_widget_set_urgent(GntWidget *widget)
{
	widget = gnt_widget_get_toplevel(widget);

	if (gnt_ws_is_top_widget(gnt_wm_get_current_workspace(wm), widget)) {
		return;
	}

	gnt_widget_set_is_urgent(widget, TRUE);

	gnt_wm_update_window(wm, widget);
}

void
gnt_quit(void)
{
	/* Prevent io_invoke() from being called after wm is destroyed */
	if (channel_error_callback) {
		g_source_remove(channel_error_callback);
	}
	if (channel_read_callback) {
		g_source_remove(channel_read_callback);
	}

	channel_error_callback = 0;
	channel_read_callback = 0;

	g_object_unref(G_OBJECT(wm));
	wm = NULL;

	update_panels();
	doupdate();
	gnt_uninit_colors();
	gnt_uninit_styles();
	endwin();

	g_clear_pointer(&custom_config_dir, g_free);
}

gboolean
gnt_ascii_only(void)
{
	return ascii_only;
}

void gnt_screen_resize_widget(GntWidget *widget, int width, int height)
{
	gnt_wm_resize_window(wm, widget, width, height);
}

void gnt_screen_move_widget(GntWidget *widget, int x, int y)
{
	gnt_wm_move_window(wm, widget, x, y);
}

void gnt_screen_rename_widget(GntWidget *widget, const char *text)
{
	gnt_box_set_title(GNT_BOX(widget), text);
	gnt_widget_draw(widget);
	gnt_wm_update_window(wm, widget);
}

void gnt_register_action(const gchar *label, GCallback callback)
{
	GntAction *action = g_new0(GntAction, 1);
	action->label = g_strdup(label);
	action->callback = callback;

	gnt_wm_add_action(wm, action);
}

static void
reset_menu(G_GNUC_UNUSED GntWidget *widget, G_GNUC_UNUSED gpointer data)
{
	gnt_wm_set_menu(wm, NULL);
}

gboolean gnt_screen_menu_show(gpointer newmenu)
{
	if (gnt_wm_get_menu(wm)) {
		/* For now, if a menu is being displayed, then another menu
		 * can NOT take over. */
		return FALSE;
	}

	gnt_wm_set_menu(wm, GNT_MENU(newmenu));
	gnt_widget_set_visible(GNT_WIDGET(newmenu), TRUE);
	gnt_widget_draw(GNT_WIDGET(newmenu));

	g_signal_connect(G_OBJECT(newmenu), "hide", G_CALLBACK(reset_menu),
	                 NULL);
	g_signal_connect(G_OBJECT(newmenu), "destroy", G_CALLBACK(reset_menu),
	                 NULL);

	return TRUE;
}

void gnt_set_clipboard_string(const gchar *string)
{
	gnt_clipboard_set_string(clipboard, string);
}

GntClipboard *
gnt_get_clipboard(void)
{
	return clipboard;
}

gchar *
gnt_get_clipboard_string(void)
{
	return gnt_clipboard_get_string(clipboard);
}

typedef struct
{
	void (*callback)(int status, gpointer data);
	gpointer data;
} ChildProcess;

static void
reap_child(G_GNUC_UNUSED GPid pid, gint status, gpointer data)
{
	ChildProcess *cp = data;
	if (cp->callback) {
		cp->callback(status, cp->data);
	}
	g_free(cp);
#ifndef _WIN32
	clean_pid();
#endif
	gnt_wm_set_keypress_mode(wm, GNT_KP_MODE_NORMAL);
	endwin();
	setup_io();
	refresh();
	refresh_screen();
}

gboolean gnt_giveup_console(const char *wd, char **argv, char **envp,
		gint *stin, gint *stout, gint *sterr,
		void (*callback)(int status, gpointer data), gpointer data)
{
	GPid pid = 0;
	ChildProcess *cp = NULL;

	if (!g_spawn_async_with_pipes(wd, argv, envp,
			G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
			endwin_cb, NULL,
			&pid, stin, stout, sterr, NULL))
		return FALSE;

	cp = g_new0(ChildProcess, 1);
	cp->callback = callback;
	cp->data = data;
	g_source_remove(channel_read_callback);
	channel_read_callback = 0;
	gnt_wm_set_keypress_mode(wm, GNT_KP_MODE_WAIT_ON_CHILD);
	g_child_watch_add(pid, reap_child, cp);

	return TRUE;
}

gboolean
gnt_is_refugee(void)
{
	return (wm &&
	        gnt_wm_get_keypress_mode(wm) == GNT_KP_MODE_WAIT_ON_CHILD);
}

/* to save other's time... this ugly function converts the given string to the
 * locale if necessary and returns it as a const gchar *.  Since it needs to
 * return a const gchar * there's a bunch of messing around with a static
 * variable.  While this works, this makes this non-thread safe and who knows
 * what else.
 */
const char *C_(const char *x)
{
	static gchar *c = NULL;

	/* clear the old value we had for cm since it's no longer needed */
	g_free(c);

	if (gnt_need_conversation_to_locale) {
		GError *error = NULL;
		gchar *newc = NULL;

		newc = g_locale_from_utf8(x, -1, NULL, NULL, &error);
		if(error != NULL) {
			gnt_warning("Error: %s\n", error->message ? error->message : "(unknown)");

			g_error_free(error);

			return x;
		}

		if(newc != NULL) {
			c = newc;

			return c;
		}
	}

	return x;
}

