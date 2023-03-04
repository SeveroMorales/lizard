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
#define GNT_LOG_DOMAIN "WM"

#ifdef USE_PYTHON
#include <Python.h>
#endif

/* Python.h may define _GNU_SOURCE and _XOPEN_SOURCE_EXTENDED, so protect
 * these checks with #ifndef/!defined() */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#if !defined _XOPEN_SOURCE_EXTENDED && (defined(__APPLE__) || defined(__unix__)) && !defined(__FreeBSD__)
#define _XOPEN_SOURCE_EXTENDED
#endif

#include <glib.h>
#include <glib/gstdio.h>
#include <ctype.h>
#include <gmodule.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gntwm.h"
#include "gntstyle.h"
#include "gntbox.h"
#include "gntbutton.h"
#include "gntentry.h"
#include "gntfilesel.h"
#include "gntlabel.h"
#include "gntmenu.h"
#include "gnttextview.h"
#include "gnttree.h"
#include "gntutils.h"
#include "gntwindow.h"

#include "gntboxprivate.h"
#include "gntmenuprivate.h"
#include "gntstyleprivate.h"
#include "gntwidgetprivate.h"
#include "gntwmprivate.h"
#include "gntwsprivate.h"

#define IDLE_CHECK_INTERVAL 5 /* 5 seconds */

typedef struct
{
	int x;
	int y;
} GntPosition;

typedef struct
{
	GntWidget *window;
	GntWidget *tree;
} GntListWindow;

typedef struct
{
	GMainLoop *loop;

	GList *workspaces;
	GList *tagged; /* tagged windows */
	GntWS *cws;

	GntListWindow list;
	GntListWindow *windows; /* Window-list window */
	GntListWindow *actions; /* Action-list window */

	GHashTable *nodes;        /* GntWidget -> GntNode */
	GHashTable *name_places;  /* window name -> ws */
	GHashTable *title_places; /* window title -> ws */

	GList *acts; /* List of actions */

	/* Currently active menu. There can be at most one menu at a time on
	 * the screen. If there is a menu being displayed, then all the
	 * keystrokes will be sent to the menu until it is closed, either when
	 * the user activates a menuitem, or presses Escape to cancel the menu.
	 */
	GntMenu *menu;

	/* Will be set to %TRUE when a user-event, ie. a mouse-click or a
	 * key-press is being processed. This variable will be used to
	 * determine whether to give focus to a new window.
	 */
	gboolean event_stack;

	GntKeyPressMode mode;

	GHashTable *positions;
} GntWMPrivate;

enum
{
	SIG_NEW_WIN,
	SIG_DECORATE_WIN,
	SIG_CLOSE_WIN,
	SIG_CONFIRM_RESIZE,
	SIG_RESIZED,
	SIG_CONFIRM_MOVE,
	SIG_MOVED,
	SIG_UPDATE_WIN,
	SIG_GIVE_FOCUS,
	SIG_KEY_PRESS,
	SIG_MOUSE_CLICK,
	SIG_TERMINAL_REFRESH,
	SIGS
};

static guint signals[SIGS] = { 0 };
static void gnt_wm_new_window_real(GntWM *wm, GntWidget *widget);
static void gnt_wm_win_resized(GntWM *wm, GntNode *node);
static void gnt_wm_win_moved(GntWM *wm, GntNode *node);
static void gnt_wm_give_focus(GntWM *wm, GntWidget *widget);
static void update_window_in_list(GntWMPrivate *priv, GntWidget *wid);
static void shift_window(GntWMPrivate *priv, GntWidget *widget, int dir);
static gboolean workspace_next(GntBindable *wm, GList *n);
static gboolean workspace_prev(GntBindable *wm, GList *n);

#if NCURSES_WIDECHAR
static int widestringwidth(wchar_t *wide);
#endif

static void ensure_normal_mode(GntWMPrivate *priv);
static gboolean write_already(gpointer data);
static guint write_timeout;
static time_t last_active_time;
static gboolean idle_update;
static GList *act = NULL; /* list of WS with unseen activity */
static gboolean ignore_keys = FALSE;
#ifdef USE_PYTHON
static gboolean started_python = FALSE;
#endif

G_DEFINE_TYPE_WITH_PRIVATE(GntWM, gnt_wm, GNT_TYPE_BINDABLE)

static void
free_node(gpointer data)
{
	GntNode *node = data;
	hide_panel(node->panel);
	del_panel(node->panel);
	g_free(node);
}

/* Private. */
void
gnt_wm_copy_win(GntWidget *widget, GntNode *node)
{
	WINDOW *src, *dst;
	if (!node)
		return;
	src = gnt_widget_get_window(widget);
	dst = node->window;
	copywin(src, dst, node->scroll, 0, 0, 0, getmaxy(dst) - 1, getmaxx(dst) - 1, 0);

	/* Update the hardware cursor */
	if (GNT_IS_WINDOW(widget) || GNT_IS_BOX(widget)) {
		GntWidget *active = gnt_box_get_active(GNT_BOX(widget));
		if (active) {
			WINDOW *active_window = gnt_widget_get_window(active);
			gint curx, cury, widgetx, widgety;
			gnt_widget_get_position(active, &curx, &cury);
			gnt_widget_get_position(widget, &widgetx, &widgety);
			curx += getcurx(active_window) - widgetx;
			cury += getcury(active_window) - widgety;
			if (wmove(node->window, cury, curx) != OK) {
				(void)wmove(node->window, 0, 0);
			}
		}
	}
}

/*
 * The following is a workaround for a bug in most versions of ncursesw.
 * Read about it in: http://article.gmane.org/gmane.comp.lib.ncurses.bugs/2751
 *
 * In short, if a panel hides one cell of a multi-cell character, then the rest
 * of the characters in that line get screwed. The workaround here is to erase
 * any such character preemptively.
 *
 * Caveat: If a wide character is erased, and the panel above it is moved enough
 * to expose the entire character, it is not always redrawn.
 */
static void
work_around_for_ncurses_bug(void)
{
#if NCURSES_WIDECHAR
	PANEL *panel = NULL;
	while ((panel = panel_below(panel)) != NULL) {
		int sx, ex, sy, ey, w, y;
		PANEL *below = panel;

		sx = getbegx(panel_window(panel));
		ex = getmaxx(panel_window(panel)) + sx;
		sy = getbegy(panel_window(panel));
		ey = getmaxy(panel_window(panel)) + sy;

		while ((below = panel_below(below)) != NULL) {
			if (sy > getbegy(panel_window(below)) + getmaxy(panel_window(below)) ||
					ey < getbegy(panel_window(below)))
				continue;
			if (sx > getbegx(panel_window(below)) + getmaxx(panel_window(below)) ||
					ex < getbegx(panel_window(below)))
				continue;
			for (y = MAX(sy, getbegy(panel_window(below))); y <= MIN(ey, getbegy(panel_window(below)) + getmaxy(panel_window(below))); y++) {
				cchar_t ch;
				memset(&ch, 0, sizeof(ch));
				if (mvwin_wch(panel_window(below), y - getbegy(panel_window(below)), sx - 1 - getbegx(panel_window(below)), &ch) != OK)
					goto right;
				w = widestringwidth(ch.chars);
				if (w > 1 && (ch.attr & 1)) {
					ch.chars[0] = ' ';
					ch.attr &= ~ A_CHARTEXT;
					mvwadd_wch(panel_window(below), y - getbegy(panel_window(below)), sx - 1 - getbegx(panel_window(below)), &ch);
					touchline(panel_window(below), y - getbegy(panel_window(below)), 1);
				}
right:
				if (mvwin_wch(panel_window(below), y - getbegy(panel_window(below)), ex + 1 - getbegx(panel_window(below)), &ch) != OK)
					continue;
				w = widestringwidth(ch.chars);
				if (w > 1 && !(ch.attr & 1)) {
					ch.chars[0] = ' ';
					ch.attr &= ~ A_CHARTEXT;
					mvwadd_wch(panel_window(below), y - getbegy(panel_window(below)), ex + 1 - getbegx(panel_window(below)), &ch);
					touchline(panel_window(below), y - getbegy(panel_window(below)), 1);
				}
			}
		}
	}
#endif
}

static void
update_act_msg(void)
{
	GntWidget *label;
	GList *iter;
	static GntWidget *message = NULL;
	GString *text;
	if (message)
		gnt_widget_destroy(message);
	if (!act)
		return;
	text = g_string_new("act: ");
	for (iter = act; iter; iter = iter->next) {
		GntWS *ws = iter->data;
		g_string_append_printf(text, "%s, ", gnt_ws_get_name(ws));
	}
	g_string_erase(text, text->len - 2, 2);
	message = gnt_vbox_new(FALSE);
	label = gnt_label_new_with_format(text->str, GNT_TEXT_FLAG_BOLD | GNT_TEXT_FLAG_HIGHLIGHT);
	gnt_widget_set_take_focus(message, FALSE);
	gnt_widget_set_transient(message, TRUE);
	gnt_box_add_widget(GNT_BOX(message), label);
	gnt_widget_set_name(message, "wm-message");
	gnt_widget_set_position(message, 0, 0);
	gnt_widget_draw(message);
	g_string_free(text, TRUE);
}

static gboolean
update_screen(GntWMPrivate *priv)
{
	if (priv->mode == GNT_KP_MODE_WAIT_ON_CHILD) {
		return TRUE;
	}

	if (priv->menu) {
		GntMenu *top = priv->menu;
		while (top) {
			GntNode *node = g_hash_table_lookup(priv->nodes, top);
			if (node)
				top_panel(node->panel);
			top = gnt_menu_get_submenu(top);
		}
	}
	work_around_for_ncurses_bug();
	update_panels();
	doupdate();
	return TRUE;
}

static gboolean
sanitize_position(GntWidget *widget, int *x, int *y, gboolean m)
{
	int X_MAX = getmaxx(stdscr);
	int Y_MAX = getmaxy(stdscr) - 1;
	int w, h;
	int nx, ny;
	gboolean changed = FALSE;
	GntWindowFlags flags = GNT_IS_WINDOW(widget) ?
			gnt_window_get_maximize(GNT_WINDOW(widget)) : 0;

	gnt_widget_get_size(widget, &w, &h);
	if (x) {
		if (m && (flags & GNT_WINDOW_MAXIMIZE_X) && *x != 0) {
			*x = 0;
			changed = TRUE;
		} else if (*x + w > X_MAX) {
			nx = MAX(0, X_MAX - w);
			if (nx != *x) {
				*x = nx;
				changed = TRUE;
			}
		}
	}
	if (y) {
		if (m && (flags & GNT_WINDOW_MAXIMIZE_Y) && *y != 0) {
			*y = 0;
			changed = TRUE;
		} else if (*y + h > Y_MAX) {
			ny = MAX(0, Y_MAX - h);
			if (ny != *y) {
				*y = ny;
				changed = TRUE;
			}
		}
	}
	return changed;
}

static void
refresh_node(GntWidget *widget, G_GNUC_UNUSED GntNode *node, gpointer m)
{
	int x, y, w, h;
	int nw, nh;

	int X_MAX = getmaxx(stdscr);
	int Y_MAX = getmaxy(stdscr) - 1;

	GntWindowFlags flags = 0;

	if (m && GNT_IS_WINDOW(widget)) {
		flags = gnt_window_get_maximize(GNT_WINDOW(widget));
	}

	gnt_widget_get_position(widget, &x, &y);
	gnt_widget_get_size(widget, &w, &h);

	if (sanitize_position(widget, &x, &y, !!m))
		gnt_screen_move_widget(widget, x, y);

	if (flags & GNT_WINDOW_MAXIMIZE_X)
		nw = X_MAX;
	else
		nw = MIN(w, X_MAX);

	if (flags & GNT_WINDOW_MAXIMIZE_Y)
		nh = Y_MAX;
	else
		nh = MIN(h, Y_MAX);

	if (nw != w || nh != h)
		gnt_screen_resize_widget(widget, nw, nh);
}

static void
read_window_positions(GntWMPrivate *priv)
{
	GKeyFile *gfile = g_key_file_new();
	char *filename = g_build_filename(gnt_get_config_dir(), ".gntpositions", NULL);
	GError *error = NULL;
	char **keys;
	gsize nk;

	if (!g_key_file_load_from_file(gfile, filename, G_KEY_FILE_NONE, &error)) {
		gnt_warning("%s", error->message);
		g_error_free(error);
		g_free(filename);
		return;
	}

	keys = g_key_file_get_keys(gfile, "positions", &nk, &error);
	if (error) {
		gnt_warning("%s", error->message);
		g_error_free(error);
		error = NULL;
	} else {
		while (nk--) {
			char *title = keys[nk];
			gsize l;
			char **coords = g_key_file_get_string_list(gfile, "positions", title, &l, NULL);
			if (l == 2) {
				int x = atoi(coords[0]);
				int y = atoi(coords[1]);
				GntPosition *p = g_new0(GntPosition, 1);
				p->x = x;
				p->y = y;
				g_hash_table_replace(priv->positions, g_strdup(title + 1), p);
			} else {
				gnt_warning("Invalid number of arguments (%" G_GSIZE_FORMAT
						") for positioning a window.", l);
			}
			g_strfreev(coords);
		}
		g_strfreev(keys);
	}

	g_free(filename);
	g_key_file_free(gfile);
}

static gboolean
check_idle(G_GNUC_UNUSED gpointer n)
{
	if (idle_update) {
		time(&last_active_time);
		idle_update = FALSE;
	}
	return TRUE;
}

static void
gnt_wm_init(GntWM *wm)
{
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);

	priv->name_places =
	        g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
	priv->title_places =
	        g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
	gnt_style_read_workspaces(wm);
	if (priv->workspaces == NULL) {
		priv->cws = gnt_ws_new("default");
		gnt_wm_add_workspace(wm, priv->cws);
	} else {
		priv->cws = priv->workspaces->data;
	}
	priv->nodes = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL,
	                                    free_node);
	priv->positions = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
	if (gnt_style_get_bool(GNT_STYLE_REMPOS, TRUE)) {
		read_window_positions(priv);
	}
	g_timeout_add_seconds(IDLE_CHECK_INTERVAL, check_idle, NULL);
	time(&last_active_time);
	gnt_wm_switch_workspace(wm, 0);
}

static void
switch_window(GntWM *wm, int direction, gboolean urgent)
{
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	GntWidget *w = NULL, *wid = NULL;
	GList *list;
	int pos, orgpos;

	if (priv->list.window || priv->menu) {
		return;
	}

	if (gnt_ws_is_empty(priv->cws) || gnt_ws_is_single(priv->cws)) {
		return;
	}

	if (priv->mode != GNT_KP_MODE_NORMAL) {
		ensure_normal_mode(priv);
	}

	w = gnt_ws_get_top_widget(priv->cws);
	list = gnt_ws_get_widgets(priv->cws);
	orgpos = pos = g_list_index(list, w);
	g_return_if_fail(pos >= 0);

	do {
		pos += direction;

		if (pos < 0) {
			wid = g_list_last(list)->data;
			pos = g_list_length(list) - 1;
		} else if ((guint)pos >= g_list_length(list)) {
			wid = list->data;
			pos = 0;
		} else {
			wid = g_list_nth_data(list, pos);
		}
	} while (urgent && !gnt_widget_get_is_urgent(wid) && pos != orgpos);

	gnt_wm_raise_window(wm, wid);
}

static gboolean
window_next(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	switch_window(wm, 1, FALSE);
	return TRUE;
}

static gboolean
window_prev(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	switch_window(wm, -1, FALSE);
	return TRUE;
}

static gboolean
switch_window_n(GntBindable *bind, GList *list)
{
	GntWM *wm = GNT_WM(bind);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	GList *l;
	int n;

	if (gnt_ws_is_empty(priv->cws)) {
		return TRUE;
	}

	if (list)
		n = GPOINTER_TO_INT(list->data);
	else
		n = 0;

	if ((l = g_list_nth(gnt_ws_get_widgets(priv->cws), n)) != NULL) {
		gnt_wm_raise_window(wm, l->data);
	}

	return TRUE;
}

static gboolean
window_scroll_up(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	GntWidget *window;
	GntNode *node;

	if (gnt_ws_is_empty(priv->cws)) {
		return TRUE;
	}

	window = gnt_ws_get_top_widget(priv->cws);
	node = g_hash_table_lookup(priv->nodes, window);
	if (!node)
		return TRUE;

	if (node->scroll) {
		node->scroll--;
		gnt_wm_copy_win(window, node);
		update_screen(priv);
	}
	return TRUE;
}

static gboolean
window_scroll_down(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	GntWidget *window;
	GntNode *node;
	int w, h;

	if (gnt_ws_is_empty(priv->cws)) {
		return TRUE;
	}

	window = gnt_ws_get_top_widget(priv->cws);
	node = g_hash_table_lookup(priv->nodes, window);
	if (!node)
		return TRUE;

	gnt_widget_get_size(window, &w, &h);
	if (h - node->scroll > getmaxy(node->window)) {
		node->scroll++;
		gnt_wm_copy_win(window, node);
		update_screen(priv);
	}
	return TRUE;
}

static gboolean
window_close(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);

	if (priv->list.window) {
		return TRUE;
	}

	if (!gnt_ws_is_empty(priv->cws)) {
		gnt_widget_destroy(gnt_ws_get_top_widget(priv->cws));
		ensure_normal_mode(priv);
	}

	return TRUE;
}

static void
destroy__list(G_GNUC_UNUSED GntWidget *widget, GntWMPrivate *priv)
{
	priv->list.window = NULL;
	priv->list.tree = NULL;
	priv->windows = NULL;
	priv->actions = NULL;
	update_screen(priv);
}

static void
setup__list(GntWMPrivate *priv)
{
	GntWidget *tree, *win;
	ensure_normal_mode(priv);
	win = priv->list.window = gnt_box_new(FALSE, FALSE);
	gnt_box_set_toplevel(GNT_BOX(win), TRUE);
	gnt_box_set_pad(GNT_BOX(win), 0);
	gnt_widget_set_transient(win, TRUE);

	tree = priv->list.tree = gnt_tree_new();
	gnt_box_add_widget(GNT_BOX(win), tree);

	g_signal_connect(G_OBJECT(win), "destroy", G_CALLBACK(destroy__list), priv);
}

static void
window_list_activate(GntTree *tree, GntWM *wm)
{
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	GntBindable *sel = gnt_tree_get_selection_data(GNT_TREE(tree));

	gnt_widget_destroy(priv->list.window);

	if (!sel)
		return;

	if (GNT_IS_WS(sel)) {
		gnt_wm_switch_workspace(wm,
		                        g_list_index(priv->workspaces, sel));
	} else {
		gnt_wm_raise_window(wm, GNT_WIDGET(sel));
	}
}

static void
populate_window_list(GntWMPrivate *priv, gboolean workspace)
{
	GList *iter;
	GntTree *tree = GNT_TREE(priv->windows->tree);
	if (!workspace) {
		for (iter = gnt_ws_get_widgets(priv->cws); iter;
		     iter = iter->next) {
			GntBox *box = GNT_BOX(iter->data);

			gnt_tree_add_row_last(
			        tree, box,
			        gnt_tree_create_row(tree,
			                            gnt_box_get_title(box)),
			        NULL);
			update_window_in_list(priv, GNT_WIDGET(box));
		}
	} else {
		GList *ws = priv->workspaces;
		for (; ws; ws = ws->next) {
			gnt_tree_add_row_last(tree, ws->data,
					gnt_tree_create_row(tree, gnt_ws_get_name(GNT_WS(ws->data))), NULL);
			for (iter = gnt_ws_get_widgets(GNT_WS(ws->data)); iter;
			     iter = iter->next) {
				GntBox *box = GNT_BOX(iter->data);

				gnt_tree_add_row_last(
				        tree, box,
				        gnt_tree_create_row(
				                tree, gnt_box_get_title(box)),
				        ws->data);
				update_window_in_list(priv, GNT_WIDGET(box));
			}
		}
	}
}

static gboolean
window_list_key_pressed(GntWidget *widget, const char *text, GntWMPrivate *priv)
{
	if (text[1] == 0 && !gnt_ws_is_empty(priv->cws)) {
		GntBindable *sel = gnt_tree_get_selection_data(GNT_TREE(widget));
		switch (text[0]) {
			case '-':
			case ',':
				if (GNT_IS_WS(sel)) {
					/* reorder the workspace. */
				} else
					shift_window(priv, GNT_WIDGET(sel), -1);
				break;
			case '=':
			case '.':
				if (GNT_IS_WS(sel)) {
					/* reorder the workspace. */
				} else
					shift_window(priv, GNT_WIDGET(sel), 1);
				break;
			default:
				return FALSE;
		}
		gnt_tree_remove_all(GNT_TREE(widget));
		populate_window_list(priv, GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "workspace")));
		gnt_tree_set_selected(GNT_TREE(widget), sel);
		return TRUE;
	}
	return FALSE;
}

static void
list_of_windows(GntWM *wm, gboolean workspace)
{
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	GntWidget *tree, *win;
	setup__list(priv);
	priv->windows = &priv->list;

	win = priv->windows->window;
	tree = priv->windows->tree;

	gnt_box_set_title(GNT_BOX(win), workspace ? "Workspace List" : "Window List");

	populate_window_list(priv, workspace);

	if (!gnt_ws_is_empty(priv->cws)) {
		gnt_tree_set_selected(GNT_TREE(tree), gnt_ws_get_top_widget(priv->cws));
	} else if (workspace) {
		gnt_tree_set_selected(GNT_TREE(tree), priv->cws);
	}

	g_signal_connect(G_OBJECT(tree), "activate", G_CALLBACK(window_list_activate), wm);
	g_signal_connect(G_OBJECT(tree), "key_pressed", G_CALLBACK(window_list_key_pressed), priv);
	g_object_set_data(G_OBJECT(tree), "workspace", GINT_TO_POINTER(workspace));

	gnt_tree_set_col_width(GNT_TREE(tree), 0, getmaxx(stdscr) / 3);
	gnt_widget_set_size(tree, 0, getmaxy(stdscr) / 2);
	gnt_widget_set_position(win, getmaxx(stdscr) / 3, getmaxy(stdscr) / 4);

	gnt_widget_show(win);
}

static gboolean
window_list(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);

	if (priv->list.window || priv->menu) {
		return TRUE;
	}

	if (gnt_ws_is_empty(priv->cws)) {
		return TRUE;
	}

	list_of_windows(wm, FALSE);

	return TRUE;
}

static void
dump_file_save(GntFileSel *fs, const char *path, G_GNUC_UNUSED const char *f,
               G_GNUC_UNUSED gpointer n)
{
	FILE *file;
	int x, y;
	chtype old = 0, now = 0;
	struct {
		char ascii;
		char *unicode;
	} unis[] = {
		{'q', "&#x2500;"},
		{'t', "&#x251c;"},
		{'u', "&#x2524;"},
		{'x', "&#x2502;"},
		{'-', "&#x2191;"},
		{'.', "&#x2193;"},
		{'l', "&#x250c;"},
		{'k', "&#x2510;"},
		{'m', "&#x2514;"},
		{'j', "&#x2518;"},
		{'a', "&#x2592;"},
		{'n', "&#x253c;"},
		{'w', "&#x252c;"},
		{'v', "&#x2534;"},
		{'\0', NULL}
	};

	gnt_widget_destroy(GNT_WIDGET(fs));

	if ((file = g_fopen(path, "w+")) == NULL) {
		return;
	}

	fprintf(file, "<head>\n  <meta http-equiv='Content-Type' content='text/html; charset=utf-8' />\n</head>\n<body>\n");
	fprintf(file, "<pre>");
	for (y = 0; y < getmaxy(stdscr); y++) {
		for (x = 0; x < getmaxx(stdscr); x++) {
			char ch[2] = {0, 0}, *print;
#if !NCURSES_WIDECHAR
			now = mvwinch(curscr, y, x);
			ch[0] = now & A_CHARTEXT;
			now ^= ch[0];
#else
			cchar_t wch;
			char unicode[13];
			memset(&wch, 0, sizeof(wch));
			mvwin_wch(curscr, y, x, &wch);
			now = wch.attr;
			ch[0] = (char)(wch.chars[0] & 0xff);
#endif

#define CHECK(attr, start, end) \
			do \
			{  \
				if (now & attr)  \
				{  \
					if (!(old & attr))  \
						fprintf(file, "%s", start);  \
				}  \
				else if (old & attr)  \
				{  \
					fprintf(file, "%s", end);  \
				}  \
			} while (0)

			CHECK(A_BOLD, "<b>", "</b>");
			CHECK(A_UNDERLINE, "<u>", "</u>");
			CHECK(A_BLINK, "<blink>", "</blink>");

			if ((now & A_COLOR) != (old & A_COLOR) ||
				(now & A_REVERSE) != (old & A_REVERSE))
			{
				short fgp, bgp, r, g, b;
				struct
				{
					int r, g, b;
				} fg, bg;

				if (pair_content(PAIR_NUMBER(now & A_COLOR), &fgp, &bgp) != OK) {
					fgp = -1;
					bgp = -1;
				}
				if (fgp == -1)
					fgp = COLOR_BLACK;
				if (bgp == -1)
					bgp = COLOR_WHITE;
				if (now & A_REVERSE)
				{
					short tmp = fgp;
					fgp = bgp;
					bgp = tmp;
				}
				if (color_content(fgp, &r, &g, &b) != OK) {
					r = g = b = 0;
				}
				fg.r = r; fg.b = b; fg.g = g;
				if (color_content(bgp, &r, &g, &b) != OK) {
					r = g = b = 255;
				}
				bg.r = r; bg.b = b; bg.g = g;
#define ADJUST(x) (x = x * 255 / 1000)
				ADJUST(fg.r);
				ADJUST(fg.g);
				ADJUST(fg.b);
				ADJUST(bg.r);
				ADJUST(bg.b);
				ADJUST(bg.g);

				if (x) fprintf(file, "</span>");
				fprintf(file, "<span style=\"background:#%02x%02x%02x;color:#%02x%02x%02x\">",
						bg.r, bg.g, bg.b, fg.r, fg.g, fg.b);
			}
			print = ch;
#if NCURSES_WIDECHAR
			if (wch.chars[0] > 255) {
				snprintf(unicode, sizeof(unicode), "&#x%x;", (unsigned int)wch.chars[0]);
				print = unicode;
			}
#endif
			if (now & A_ALTCHARSET)
			{
				int u;
				for (u = 0; unis[u].ascii; u++) {
					if (ch[0] == unis[u].ascii) {
						print = unis[u].unicode;
						break;
					}
				}
				if (!unis[u].ascii)
					print = " ";
			}
			if (ch[0] == '&')
				fprintf(file, "&amp;");
			else if (ch[0] == '<')
				fprintf(file, "&lt;");
			else if (ch[0] == '>')
				fprintf(file, "&gt;");
			else
				fprintf(file, "%s", print);
			old = now;
		}
		fprintf(file, "</span>\n");
		old = 0;
	}
	fprintf(file, "</pre>\n</body>");
	fclose(file);
}

static gboolean
dump_screen(G_GNUC_UNUSED GntBindable *b, G_GNUC_UNUSED GList *params)
{
	GntWidget *window = gnt_file_sel_new();
	GntFileSel *sel = GNT_FILE_SEL(window);

	g_object_set(G_OBJECT(window), "vertical", TRUE, NULL);
	gnt_box_add_widget(GNT_BOX(window), gnt_label_new("Please enter the filename to save the screenshot."));
	gnt_box_set_title(GNT_BOX(window), "Save Screenshot...");

	gnt_file_sel_set_suggested_filename(sel, "dump.html");
	g_signal_connect(G_OBJECT(sel), "file_selected", G_CALLBACK(dump_file_save), NULL);
	g_signal_connect_swapped(G_OBJECT(sel), "cancelled",
	                         G_CALLBACK(gnt_widget_destroy), sel);
	gnt_widget_show(window);
	return TRUE;
}

static void
shift_window(GntWMPrivate *priv, GntWidget *widget, int dir)
{
	GList *all = gnt_ws_get_widgets(priv->cws);
	GList *list = g_list_find(all, widget);
	int length, pos;
	if (!list)
		return;

	length = g_list_length(all);
	pos = g_list_position(all, list);

	pos += dir;
	if (dir > 0)
		pos++;

	if (pos < 0)
		pos = length;
	else if (pos > length)
		pos = 0;

	all = g_list_insert(all, widget, pos);
	all = g_list_delete_link(all, list);
	gnt_ws_set_list(priv->cws, all);
	gnt_ws_draw_taskbar(priv->cws, FALSE);
	if (!gnt_ws_is_empty(priv->cws)) {
		GntWidget *w = gnt_ws_get_top_widget(priv->cws);
		GntNode *node = g_hash_table_lookup(priv->nodes, w);
		top_panel(node->panel);
		update_panels();
		doupdate();
	}
}

static gboolean
shift_left(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);

	if (priv->list.window) {
		return TRUE;
	}

	if (gnt_ws_is_empty(priv->cws)) {
		return TRUE;
	}

	shift_window(priv, gnt_ws_get_top_widget(priv->cws), -1);
	return TRUE;
}

static gboolean
shift_right(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);

	if (priv->list.window) {
		return TRUE;
	}

	if (gnt_ws_is_empty(priv->cws)) {
		return TRUE;
	}

	shift_window(priv, gnt_ws_get_top_widget(priv->cws), 1);
	return TRUE;
}

static void
action_list_activate(GntTree *tree, GntWMPrivate *priv)
{
	GntAction *action = gnt_tree_get_selection_data(tree);
	action->callback();
	gnt_widget_destroy(priv->list.window);
}

static int
compare_action(gconstpointer p1, gconstpointer p2)
{
	const GntAction *a1 = p1;
	const GntAction *a2 = p2;

	return g_utf8_collate(a1->label, a2->label);
}

static gboolean
list_actions(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWidget *tree, *win;
	GList *iter;
	GntWM *wm = GNT_WM(bindable);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	int n;

	if (priv->list.window || priv->menu) {
		return TRUE;
	}

	if (priv->acts == NULL) {
		return TRUE;
	}

	setup__list(priv);
	priv->actions = &priv->list;

	win = priv->actions->window;
	tree = priv->actions->tree;

	gnt_box_set_title(GNT_BOX(win), "Actions");
	gnt_widget_set_has_border(tree, FALSE);
	/* XXX: Do we really want this? */
	gnt_tree_set_compare_func(GNT_TREE(tree), compare_action);

	for (iter = priv->acts; iter; iter = iter->next) {
		GntAction *action = iter->data;
		gnt_tree_add_row_last(GNT_TREE(tree), action,
				gnt_tree_create_row(GNT_TREE(tree), action->label), NULL);
	}
	g_signal_connect(G_OBJECT(tree), "activate", G_CALLBACK(action_list_activate), priv);
	n = g_list_length(priv->acts);
	gnt_widget_set_size(tree, 0, n);
	gnt_widget_set_position(win, 0, getmaxy(stdscr) - 3 - n);

	gnt_widget_show(win);
	return TRUE;
}

#if NCURSES_WIDECHAR
static int
widestringwidth(wchar_t *wide)
{
	gint len;
	gchar *str;

	len = wcstombs(NULL, wide, 0) + 1;
	str = g_new0(char, len);
	if(str != NULL) {
		gint ret;
		wcstombs(str, wide, len);
		ret = gnt_util_onscreen_width(str, NULL);
		g_free(str);

		return ret;
	}

	return 1;
}
#endif

/* Returns the onscreen width of the character at the position */
static int
reverse_char(WINDOW *d, int y, int x, gboolean set)
{
#define DECIDE(ch) (set ? ((ch) | A_REVERSE) : ((ch) & ~A_REVERSE))

#if !NCURSES_WIDECHAR
	chtype ch;
	ch = mvwinch(d, y, x);
	mvwaddch(d, y, x, DECIDE(ch));
	return 1;
#else
	cchar_t ch;
	int wc = 1;
	if (mvwin_wch(d, y, x, &ch) == OK) {
		wc = widestringwidth(ch.chars);
		ch.attr = DECIDE(ch.attr);
		ch.attr &= WA_ATTRIBUTES;   /* XXX: This is a workaround for a bug */
		mvwadd_wch(d, y, x, &ch);
	}

	return wc;
#endif
}

static void
window_reverse(GntWidget *win, gboolean set, GntWMPrivate *priv)
{
	int i;
	int w, h;
	WINDOW *d;

	if (!gnt_widget_get_has_border(win))
		return;

	d = gnt_widget_get_window(win);
	gnt_widget_get_size(win, &w, &h);

	if (gnt_widget_has_shadow(win)) {
		--w;
		--h;
	}

	/* the top and bottom */
	for (i = 0; i < w; i += reverse_char(d, 0, i, set));
	for (i = 0; i < w; i += reverse_char(d, h-1, i, set));

	/* the left and right */
	for (i = 0; i < h; i += reverse_char(d, i, 0, set));
	for (i = 0; i < h; i += reverse_char(d, i, w-1, set));

	gnt_wm_copy_win(win, g_hash_table_lookup(priv->nodes, win));
	update_screen(priv);
}

static void
ensure_normal_mode(GntWMPrivate *priv)
{
	if (priv->mode != GNT_KP_MODE_NORMAL) {
		if (!gnt_ws_is_empty(priv->cws)) {
			window_reverse(gnt_ws_get_top_widget(priv->cws), FALSE, priv);
		}
		priv->mode = GNT_KP_MODE_NORMAL;
	}
}

static gboolean
start_move(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);

	if (priv->list.window || priv->menu) {
		return TRUE;
	}
	if (gnt_ws_is_empty(priv->cws)) {
		return TRUE;
	}

	priv->mode = GNT_KP_MODE_MOVE;
	window_reverse(gnt_ws_get_top_widget(priv->cws), TRUE, priv);

	return TRUE;
}

static gboolean
start_resize(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);

	if (priv->list.window || priv->menu) {
		return TRUE;
	}
	if (gnt_ws_is_empty(priv->cws)) {
		return TRUE;
	}

	priv->mode = GNT_KP_MODE_RESIZE;
	window_reverse(gnt_ws_get_top_widget(priv->cws), TRUE, priv);

	return TRUE;
}

static gboolean
wm_quit(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);

	if (write_timeout)
		write_already(wm);
	g_main_loop_quit(priv->loop);
	return TRUE;
}

static gboolean
return_true(G_GNUC_UNUSED GntWM *wm, G_GNUC_UNUSED GntWidget *w,
            G_GNUC_UNUSED int *a, G_GNUC_UNUSED int *b)
{
	return TRUE;
}

static gboolean
refresh_screen(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	GList *iter;

	endwin();
	refresh();

	g_hash_table_foreach(priv->nodes, (GHFunc)refresh_node,
	                     GINT_TO_POINTER(TRUE));
	g_signal_emit(wm, signals[SIG_TERMINAL_REFRESH], 0);

	for (iter = gnt_ws_get_last(priv->cws); iter; iter = iter->prev) {
		GntWidget *w = iter->data;
		GntNode *node = g_hash_table_lookup(priv->nodes, w);
		top_panel(node->panel);
	}

	gnt_ws_draw_taskbar(priv->cws, TRUE);
	update_screen(priv);

	curs_set(0);   /* endwin resets the cursor to normal */
	keypad(stdscr, 1);

	return TRUE;
}

static gboolean
toggle_clipboard(G_GNUC_UNUSED GntBindable *bindable,
                 G_GNUC_UNUSED GList *params)
{
	static GntWidget *clip;
	gchar *text;
	if (clip) {
		gnt_widget_destroy(clip);
		clip = NULL;
		return TRUE;
	}
	text = gnt_get_clipboard_string();
	clip = gnt_hwindow_new(FALSE);
	gnt_widget_set_transient(clip, TRUE);
	gnt_widget_set_has_border(clip, FALSE);
	gnt_box_set_pad(GNT_BOX(clip), 0);
	gnt_box_add_widget(GNT_BOX(clip), gnt_label_new(" "));
	gnt_box_add_widget(GNT_BOX(clip), gnt_label_new(text));
	gnt_box_add_widget(GNT_BOX(clip), gnt_label_new(" "));
	gnt_widget_set_position(clip, 0, 0);
	gnt_widget_draw(clip);
	g_free(text);
	return TRUE;
}

static void
remove_tag(GntWidget *widget, G_GNUC_UNUSED gpointer data)
{
	mvwhline(gnt_widget_get_window(widget), 0, 1,
	         ACS_HLINE | gnt_color_pair(GNT_COLOR_NORMAL), 3);
	gnt_widget_draw(widget);
}

static gboolean
tag_widget(GntBindable *b, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(b);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	GntWidget *widget;
	GList *link;

	if (gnt_ws_is_empty(priv->cws)) {
		return FALSE;
	}
	widget = gnt_ws_get_top_widget(priv->cws);

	link = g_list_find(priv->tagged, widget);
	if (link) {
		priv->tagged = g_list_delete_link(priv->tagged, link);
		remove_tag(widget, NULL);
		return TRUE;
	}

	priv->tagged = g_list_prepend(priv->tagged, widget);
	wbkgdset(gnt_widget_get_window(widget),
	         ' ' | gnt_color_pair(GNT_COLOR_HIGHLIGHT));
	mvwprintw(gnt_widget_get_window(widget), 0, 1, "[T]");
	gnt_widget_draw(widget);
	return TRUE;
}

static gboolean
place_tagged(GntBindable *b, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(b);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	GList *iter;

	for (iter = priv->tagged; iter; iter = g_list_delete_link(iter, iter)) {
		GntWidget *widget = GNT_WIDGET(iter->data);
		gnt_wm_widget_move_workspace(wm, priv->cws, widget);
		remove_tag(widget, NULL);
	}

	priv->tagged = NULL;
	return TRUE;
}

static gboolean
workspace_list(GntBindable *b, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(b);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);

	if (priv->list.window || priv->menu) {
		return TRUE;
	}

	list_of_windows(wm, TRUE);

	return TRUE;
}

static gboolean
workspace_new(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	GntWS *ws = gnt_ws_new(NULL);
	gnt_wm_add_workspace(wm, ws);
	gnt_wm_switch_workspace(wm, g_list_index(priv->workspaces, ws));
	return TRUE;
}

static gboolean
ignore_keys_start(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);

	if (!priv->menu && !priv->list.window &&
	    priv->mode == GNT_KP_MODE_NORMAL) {
		ignore_keys = TRUE;
		return TRUE;
	}
	return FALSE;
}

static gboolean
ignore_keys_end(G_GNUC_UNUSED GntBindable *bindable,
                G_GNUC_UNUSED GList *params)
{
	if (ignore_keys) {
		ignore_keys = FALSE;
		return TRUE;
	}
	return FALSE;
}

static gboolean
window_next_urgent(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	switch_window(wm, 1, TRUE);
	return TRUE;
}

static gboolean
window_prev_urgent(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	switch_window(wm, -1, TRUE);
	return TRUE;
}

#ifdef USE_PYTHON
static void
python_script_selected(GntFileSel *fs, const char *path,
                       G_GNUC_UNUSED const char *f, G_GNUC_UNUSED gpointer n)
{
	char *dir = g_path_get_dirname(path);
	FILE *file = fopen(path, "r");
	PyObject *pp = PySys_GetObject("path");
#if PY_MAJOR_VERSION >= 3
	PyObject *dirobj = PyUnicode_FromString(dir);
#else
	PyObject *dirobj = PyString_FromString(dir);
#endif

	PyList_Insert(pp, 0, dirobj);
	Py_DECREF(dirobj);
	PyRun_SimpleFile(file, path);
	fclose(file);

	if (PyErr_Occurred()) {
		PyErr_Print();
	}
	g_free(dir);

	gnt_widget_destroy(GNT_WIDGET(fs));
}

static gboolean
run_python(G_GNUC_UNUSED GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWidget *window = gnt_file_sel_new();
	GntFileSel *sel = GNT_FILE_SEL(window);

	g_object_set(G_OBJECT(window), "vertical", TRUE, NULL);
	gnt_box_add_widget(GNT_BOX(window), gnt_label_new("Please select the python script you want to run."));
	gnt_box_set_title(GNT_BOX(window), "Select Python Script...");

	g_signal_connect(G_OBJECT(sel), "file_selected", G_CALLBACK(python_script_selected), NULL);
	g_signal_connect_swapped(G_OBJECT(sel), "cancelled",
	                         G_CALLBACK(gnt_widget_destroy), sel);
	gnt_widget_show(window);
	return TRUE;
}
#endif  /* USE_PYTHON */

static gboolean
help_for_bindable(GntWM *wm, GntBindable *bindable)
{
	gboolean ret = TRUE;
	GntBindableClass *klass = GNT_BINDABLE_GET_CLASS(bindable);

	if (klass->help_window) {
		gnt_wm_raise_window(wm, GNT_WIDGET(klass->help_window));
	} else {
		ret =  gnt_bindable_build_help_window(bindable);
	}
	return ret;
}

static gboolean
help_for_wm(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	return help_for_bindable(GNT_WM(bindable),bindable);
}

static gboolean
help_for_window(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	GntWidget *widget;

	if (gnt_ws_is_empty(priv->cws)) {
		return FALSE;
	}

	widget = gnt_ws_get_top_widget(priv->cws);

	return help_for_bindable(wm,GNT_BINDABLE(widget));
}

static gboolean
help_for_widget(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	GntWidget *widget;

	if (gnt_ws_is_empty(priv->cws)) {
		return TRUE;
	}

	widget = gnt_ws_get_top_widget(priv->cws);
	if (!GNT_IS_BOX(widget))
		return TRUE;

	return help_for_bindable(
	        wm, GNT_BINDABLE(gnt_box_get_active(GNT_BOX(widget))));
}

static void
accumulate_windows(GntWidget *window, G_GNUC_UNUSED gpointer node, GList **p)
{
	GList *list = *p;
	list = g_list_prepend(list, window);
	*p = list;
}

static void
gnt_wm_destroy(GObject *obj)
{
	GntWM *wm = GNT_WM(obj);
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	GList *list;

	list = NULL;
	g_hash_table_foreach(priv->nodes, (GHFunc)accumulate_windows, &list);
	g_list_free_full(list, (GDestroyNotify)gnt_widget_destroy);
	g_hash_table_destroy(priv->nodes);
	priv->nodes = NULL;

	g_list_free_full(priv->workspaces, g_object_unref);
	priv->workspaces = NULL;

	g_clear_pointer(&priv->loop, g_main_loop_unref);

#ifdef USE_PYTHON
	if (started_python) {
		Py_Finalize();
		started_python = FALSE;
	}
#endif
}

static void
gnt_wm_class_init(GntWMClass *klass)
{
	int i;
	GObjectClass *gclass = G_OBJECT_CLASS(klass);
	char key[32];

	gclass->dispose = gnt_wm_destroy;

	klass->new_window = gnt_wm_new_window_real;
	klass->decorate_window = NULL;
	klass->close_window = NULL;
	klass->window_resize_confirm = return_true;
	klass->window_resized = gnt_wm_win_resized;
	klass->window_move_confirm = return_true;
	klass->window_moved = gnt_wm_win_moved;
	klass->window_update = NULL;
	klass->key_pressed  = NULL;
	klass->mouse_clicked = NULL;
	klass->give_focus = gnt_wm_give_focus;

	signals[SIG_NEW_WIN] =
		g_signal_new("new_win",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET(GntWMClass, new_window),
					 NULL, NULL, NULL,
					 G_TYPE_NONE, 1, G_TYPE_POINTER);
	signals[SIG_DECORATE_WIN] =
		g_signal_new("decorate_win",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET(GntWMClass, decorate_window),
					 NULL, NULL, NULL,
					 G_TYPE_NONE, 1, G_TYPE_POINTER);
	signals[SIG_CLOSE_WIN] =
		g_signal_new("close_win",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET(GntWMClass, close_window),
					 NULL, NULL, NULL,
					 G_TYPE_NONE, 1, G_TYPE_POINTER);
	signals[SIG_CONFIRM_RESIZE] =
		g_signal_new("confirm_resize",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET(GntWMClass, window_resize_confirm),
					 g_signal_accumulator_true_handled, NULL, NULL,
					 G_TYPE_BOOLEAN, 3, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_POINTER);

	signals[SIG_CONFIRM_MOVE] =
		g_signal_new("confirm_move",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET(GntWMClass, window_move_confirm),
					 g_signal_accumulator_true_handled, NULL, NULL,
					 G_TYPE_BOOLEAN, 3, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_POINTER);

	signals[SIG_RESIZED] =
		g_signal_new("window_resized",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET(GntWMClass, window_resized),
					 NULL, NULL, NULL,
					 G_TYPE_NONE, 1, G_TYPE_POINTER);
	signals[SIG_MOVED] =
		g_signal_new("window_moved",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET(GntWMClass, window_moved),
					 NULL, NULL, NULL,
					 G_TYPE_NONE, 1, G_TYPE_POINTER);
	signals[SIG_UPDATE_WIN] =
		g_signal_new("window_update",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET(GntWMClass, window_update),
					 NULL, NULL, NULL,
					 G_TYPE_NONE, 1, G_TYPE_POINTER);

	signals[SIG_GIVE_FOCUS] =
		g_signal_new("give_focus",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET(GntWMClass, give_focus),
					 NULL, NULL, NULL,
					 G_TYPE_NONE, 1, G_TYPE_POINTER);

	signals[SIG_MOUSE_CLICK] =
		g_signal_new("mouse_clicked",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET(GntWMClass, mouse_clicked),
					 g_signal_accumulator_true_handled, NULL, NULL,
					 G_TYPE_BOOLEAN, 4, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_POINTER);

	/**
	 * GntWM::terminal-refresh:
	 *
	 * Since: 2.1.0
	 */
	signals[SIG_TERMINAL_REFRESH] =
		g_signal_new("terminal-refresh",
					 G_TYPE_FROM_CLASS(klass),
					 G_SIGNAL_RUN_LAST,
					 G_STRUCT_OFFSET(GntWMClass, terminal_refresh),
					 NULL, NULL, NULL,
					 G_TYPE_NONE, 0);

	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "window-next", window_next,
				"\033" "n", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "window-prev", window_prev,
				"\033" "p", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "window-close", window_close,
				"\033" "c", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "window-list", window_list,
				"\033" "w", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "dump-screen", dump_screen,
				"\033" "D", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "shift-left", shift_left,
				"\033" ",", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "shift-right", shift_right,
				"\033" ".", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "action-list", list_actions,
				"\033" "a", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "start-move", start_move,
				"\033" "m", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "start-resize", start_resize,
				"\033" "r", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "wm-quit", wm_quit,
				"\033" "q", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "refresh-screen", refresh_screen,
				"\033" "l", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "switch-window-n", switch_window_n,
				NULL, NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "window-scroll-down", window_scroll_down,
				"\033" GNT_KEY_CTRL_J, NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "window-scroll-up", window_scroll_up,
				"\033" GNT_KEY_CTRL_K, NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "help-for-widget", help_for_widget,
				"\033" "/", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "workspace-new", workspace_new,
				GNT_KEY_F9, NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "workspace-next", workspace_next,
				"\033" ">", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "workspace-prev", workspace_prev,
				"\033" "<", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "window-tag", tag_widget,
				"\033" "t", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "place-tagged", place_tagged,
				"\033" "T", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "workspace-list", workspace_list,
				"\033" "s", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "toggle-clipboard", toggle_clipboard,
				"\033" "C", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "help-for-wm", help_for_wm,
				"\033" "\\", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "help-for-window", help_for_window,
				"\033" "|", NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "ignore-keys-start", ignore_keys_start,
				NULL, NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "ignore-keys-end", ignore_keys_end,
				"\033" GNT_KEY_CTRL_G, NULL);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "window-next-urgent", window_next_urgent,
				"\033" "\t", NULL);
	snprintf(key, sizeof(key), "\033%s", GNT_KEY_BACK_TAB);
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "window-prev-urgent", window_prev_urgent,
				key[1] ? key : NULL, NULL);
#ifdef USE_PYTHON
	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass), "run-python", run_python,
				GNT_KEY_F3, NULL);
	if (!Py_IsInitialized()) {
#if PY_MAJOR_VERSION >= 3
		Py_SetProgramName(L"gnt");
#else
		Py_SetProgramName("gnt");
#endif
		Py_Initialize();
		started_python = TRUE;
	}
#endif

	gnt_style_read_actions(G_OBJECT_CLASS_TYPE(klass), GNT_BINDABLE_CLASS(klass));

	/* Make sure Alt+x are detected properly. */
	for (i = '0'; i <= '9'; i++) {
		char str[] = "\033X";
		str[1] = i;
		gnt_keys_add_combination(str);
	}
}

/******************************************************************************
 * GntWM API
 *****************************************************************************/
GntWS *
gnt_wm_get_current_workspace(GntWM *wm)
{
	GntWMPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_WM(wm), NULL);

	priv = gnt_wm_get_instance_private(wm);
	return priv->cws;
}

void
gnt_wm_add_workspace(GntWM *wm, GntWS *ws)
{
	GntWMPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_WM(wm));
	g_return_if_fail(GNT_IS_WS(ws));

	priv = gnt_wm_get_instance_private(wm);
	priv->workspaces = g_list_append(priv->workspaces, ws);
}

gboolean
gnt_wm_switch_workspace(GntWM *wm, gint n)
{
	GntWMPrivate *priv = NULL;
	GntWS *s;

	g_return_val_if_fail(GNT_IS_WM(wm), FALSE);
	priv = gnt_wm_get_instance_private(wm);

	s = g_list_nth_data(priv->workspaces, n);
	if (!s)
		return FALSE;

	if (priv->list.window) {
		gnt_widget_destroy(priv->list.window);
	}
	ensure_normal_mode(priv);
	gnt_ws_hide(priv->cws, priv->nodes);
	priv->cws = s;
	gnt_ws_show(priv->cws, priv->nodes);

	gnt_ws_draw_taskbar(priv->cws, TRUE);
	update_screen(priv);
	if (!gnt_ws_is_empty(priv->cws)) {
		gnt_wm_raise_window(wm, gnt_ws_get_top_widget(priv->cws));
	}

	if (act && g_list_find(act, priv->cws)) {
		act = g_list_remove(act, priv->cws);
		update_act_msg();
	}
	return TRUE;
}

gboolean
gnt_wm_switch_workspace_prev(GntWM *wm)
{
	GntWMPrivate *priv = NULL;
	gint n;

	g_return_val_if_fail(GNT_IS_WM(wm), FALSE);
	priv = gnt_wm_get_instance_private(wm);

	n = g_list_index(priv->workspaces, priv->cws);
	return gnt_wm_switch_workspace(wm, --n);
}

gboolean
gnt_wm_switch_workspace_next(GntWM *wm)
{
	GntWMPrivate *priv = NULL;
	gint n;

	g_return_val_if_fail(GNT_IS_WM(wm), FALSE);
	priv = gnt_wm_get_instance_private(wm);

	n = g_list_index(priv->workspaces, priv->cws);
	return gnt_wm_switch_workspace(wm, ++n);
}

static gboolean
workspace_next(GntBindable *wm, G_GNUC_UNUSED GList *params)
{
	return gnt_wm_switch_workspace_next(GNT_WM(wm));
}

static gboolean
workspace_prev(GntBindable *wm, G_GNUC_UNUSED GList *params)
{
	return gnt_wm_switch_workspace_prev(GNT_WM(wm));
}

void
gnt_wm_widget_move_workspace(GntWM *wm, GntWS *neww, GntWidget *widget)
{
	GntWMPrivate *priv = NULL;
	GntWS *oldw = NULL;
	GntNode *node;

	g_return_if_fail(GNT_IS_WM(wm));
	priv = gnt_wm_get_instance_private(wm);

	oldw = gnt_wm_widget_find_workspace(wm, widget);
	if (!oldw || oldw == neww)
		return;
	node = g_hash_table_lookup(priv->nodes, widget);
	if (node && node->ws == neww)
		return;

	if (node)
		node->ws = neww;

	gnt_ws_remove_widget(oldw, widget);
	gnt_ws_add_widget(neww, widget);
	if (neww == priv->cws) {
		gnt_ws_widget_show(widget, priv->nodes);
	} else {
		gnt_ws_widget_hide(widget, priv->nodes);
	}
}

static gint widget_in_workspace(gconstpointer workspace, gconstpointer wid)
{
	GntWS *s = (GntWS *)workspace;
	GList *list = gnt_ws_get_widgets(s);
	if (list && g_list_find(list, wid)) {
		return 0;
	}
	return 1;
}

GntWS *gnt_wm_widget_find_workspace(GntWM *wm, GntWidget *widget)
{
	GntWMPrivate *priv = NULL;
	GList *l;

	g_return_val_if_fail(GNT_IS_WM(wm), NULL);
	priv = gnt_wm_get_instance_private(wm);

	l = g_list_find_custom(priv->workspaces, widget, widget_in_workspace);
	if (l)
		return l->data;
	return NULL;
}

void gnt_wm_set_workspaces(GntWM *wm, GList *workspaces)
{
	GntWMPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_WM(wm));
	priv = gnt_wm_get_instance_private(wm);

	priv->workspaces = workspaces;
	gnt_wm_switch_workspace(wm, 0);
}

static void
update_window_in_list(GntWMPrivate *priv, GntWidget *wid)
{
	GntTextFormatFlags flag = 0;

	if (priv->windows == NULL) {
		return;
	}

	if (gnt_ws_is_top_widget(priv->cws, wid)) {
		flag |= GNT_TEXT_FLAG_DIM;
	} else if (gnt_widget_get_is_urgent(wid)) {
		flag |= GNT_TEXT_FLAG_BOLD;
	}

	gnt_tree_set_row_flags(GNT_TREE(priv->windows->tree), wid, flag);
}

static gboolean
match_title(const gchar *title, G_GNUC_UNUSED gpointer n,
            const gchar *wid_title)
{
	/* XXX: do any regex magic here. */
	return g_strrstr(wid_title, title) != NULL;
}

static GntWS *
new_widget_find_workspace(GntWM *wm, GntWidget *widget)
{
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	GntWS *ret = NULL;
	const gchar *name, *title;
	title = gnt_box_get_title(GNT_BOX(widget));
	if (title) {
		ret = g_hash_table_find(priv->title_places,
		                        (GHRFunc)match_title, (gpointer)title);
	}
	if (ret)
		return ret;
	name = gnt_widget_get_name(widget);
	if (name) {
		ret = g_hash_table_find(priv->name_places, (GHRFunc)match_title,
		                        (gpointer)name);
	}
	return ret ? ret : priv->cws;
}

static void
gnt_wm_new_window_real(GntWM *wm, GntWidget *widget)
{
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	GntNode *node;
	gboolean transient = FALSE;

	if (gnt_widget_get_window(widget) == NULL) {
		return;
	}

	node = g_new0(GntNode, 1);
	node->me = widget;
	node->scroll = 0;

	g_hash_table_replace(priv->nodes, widget, node);

	refresh_node(widget, node, GINT_TO_POINTER(TRUE));

	transient = gnt_widget_get_transient(node->me);

#if 1
	{
		int x, y, w, h, maxx, maxy;

		gnt_widget_get_position(widget, &x, &y);
		gnt_widget_get_size(widget, &w, &h);

		maxx = getmaxx(stdscr);
		maxy = getmaxy(stdscr) - 1;              /* room for the taskbar */
		maxx = MAX(0, maxx);
		maxy = MAX(0, maxy);

		x = MAX(0, x);
		y = MAX(0, y);
		if (x + w >= maxx)
			x = MAX(0, maxx - w);
		if (y + h >= maxy)
			y = MAX(0, maxy - h);

		w = MIN(w, maxx);
		h = MIN(h, maxy);
		node->window = newwin(h, w, y, x);
		gnt_wm_copy_win(widget, node);
	}
#endif

	node->panel = new_panel(node->window);
	set_panel_userptr(node->panel, node);

	if (!transient) {
		GntWS *ws = priv->cws;
		if (!gnt_wm_is_list_window(wm, node->me)) {
			if (GNT_IS_BOX(widget)) {
				ws = new_widget_find_workspace(wm, widget);
			}
			node->ws = ws;
			gnt_ws_append_widget(ws, widget);
		}

		if (priv->event_stack || gnt_wm_is_list_window(wm, node->me) ||
		    gnt_ws_is_top_widget(ws, node->me)) {
			gnt_wm_raise_window(wm, node->me);
		} else {
			bottom_panel(node->panel);     /* New windows should not grab focus */
			gnt_widget_set_focus(node->me, FALSE);
			gnt_widget_set_urgent(node->me);
			if (priv->cws != ws) {
				gnt_ws_widget_hide(widget, priv->nodes);
			}
		}
	}
}

void gnt_wm_new_window(GntWM *wm, GntWidget *widget)
{
	GntWMPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_WM(wm));
	priv = gnt_wm_get_instance_private(wm);

	widget = gnt_widget_get_toplevel(widget);

	if (!gnt_widget_get_visible(widget) ||
	    g_hash_table_lookup(priv->nodes, widget)) {
		update_screen(priv);
		return;
	}

	if (GNT_IS_BOX(widget)) {
		const gchar *title = gnt_box_get_title(GNT_BOX(widget));
		GntPosition *p = NULL;
		if (title && (p = g_hash_table_lookup(priv->positions, title)) != NULL) {
			sanitize_position(widget, &p->x, &p->y, TRUE);
			gnt_widget_set_position(widget, p->x, p->y);
			mvwin(gnt_widget_get_window(widget), p->y, p->x);
		}
	}

	g_signal_emit(wm, signals[SIG_NEW_WIN], 0, widget);
	g_signal_emit(wm, signals[SIG_DECORATE_WIN], 0, widget);

	if (priv->windows && !gnt_widget_get_transient(widget)) {
		if ((GNT_IS_BOX(widget) &&
		     gnt_box_get_title(GNT_BOX(widget))) &&
		    !gnt_wm_is_list_window(wm, widget) &&
		    gnt_widget_get_take_focus(widget)) {
			gnt_tree_add_row_last(
			        GNT_TREE(priv->windows->tree), widget,
			        gnt_tree_create_row(
			                GNT_TREE(priv->windows->tree),
			                gnt_box_get_title(GNT_BOX(widget))),
			        g_object_get_data(G_OBJECT(priv->windows->tree),
			                          "workspace")
			                ? priv->cws
			                : NULL);
			update_window_in_list(priv, widget);
		}
	}

	gnt_ws_draw_taskbar(priv->cws, FALSE);
	update_screen(priv);
}

void gnt_wm_window_decorate(GntWM *wm, GntWidget *widget)
{
	g_return_if_fail(GNT_IS_WM(wm));

	g_signal_emit(wm, signals[SIG_DECORATE_WIN], 0, widget);
}

void gnt_wm_window_close(GntWM *wm, GntWidget *widget)
{
	GntWMPrivate *priv = NULL;
	GntWS *s;
	int pos;
	gboolean transient;

	g_return_if_fail(GNT_IS_WM(wm));
	priv = gnt_wm_get_instance_private(wm);

	transient = gnt_widget_get_transient(widget);

	s = gnt_wm_widget_find_workspace(wm, widget);

	if (g_hash_table_lookup(priv->nodes, widget) == NULL)
		return;

	g_signal_emit(wm, signals[SIG_CLOSE_WIN], 0, widget);
	g_hash_table_remove(priv->nodes, widget);

	if (priv->windows) {
		gnt_tree_remove(GNT_TREE(priv->windows->tree), widget);
	}

	if (s) {
		pos = g_list_index(gnt_ws_get_widgets(s), widget);

		if (pos != -1) {
			gnt_ws_remove_widget(s, widget);

			if (!gnt_ws_is_empty(s) && priv->cws == s) {
				gnt_wm_raise_window(wm, gnt_ws_get_top_widget(s));
			}
		}
	} else if (transient && priv->cws && !gnt_ws_is_empty(priv->cws)) {
		gnt_wm_update_window(wm, gnt_ws_get_top_widget(priv->cws));
	}

	gnt_ws_draw_taskbar(priv->cws, FALSE);
	update_screen(priv);
}

time_t
gnt_wm_get_idle_time(void)
{
	return time(NULL) - last_active_time;
}

gboolean gnt_wm_process_input(GntWM *wm, const char *keys)
{
	GntWMPrivate *priv = NULL;
	gboolean ret = FALSE;

	g_return_val_if_fail(GNT_IS_WM(wm), FALSE);
	priv = gnt_wm_get_instance_private(wm);

	keys = gnt_bindable_remap_keys(GNT_BINDABLE(wm), keys);

	idle_update = TRUE;
	if(ignore_keys){
		if(keys && !strcmp(keys, "\033" GNT_KEY_CTRL_G)){
			if(gnt_bindable_perform_action_key(GNT_BINDABLE(wm), keys)){
				return TRUE;
			}
		}
		if (gnt_ws_is_empty(priv->cws)) {
			return FALSE;
		}
		return gnt_widget_key_pressed(gnt_ws_get_top_widget(priv->cws), keys);
	}

	if (gnt_bindable_perform_action_key(GNT_BINDABLE(wm), keys)) {
		return TRUE;
	}

	/* Do some manual checking */
	if (!gnt_ws_is_empty(priv->cws) && priv->mode != GNT_KP_MODE_NORMAL) {
		int xmin = 0, ymin = 0, xmax = getmaxx(stdscr), ymax = getmaxy(stdscr) - 1;
		int x, y, w, h;
		GntWidget *widget = gnt_ws_get_top_widget(priv->cws);
		int ox, oy, ow, oh;

		gnt_widget_get_position(widget, &x, &y);
		gnt_widget_get_size(widget, &w, &h);
		ox = x;	oy = y;
		ow = w;	oh = h;

		if (priv->mode == GNT_KP_MODE_MOVE) {
			if (strcmp(keys, GNT_KEY_LEFT) == 0) {
				if (x > xmin)
					x--;
			} else if (strcmp(keys, GNT_KEY_RIGHT) == 0) {
				if (x + w < xmax)
					x++;
			} else if (strcmp(keys, GNT_KEY_UP) == 0) {
				if (y > ymin)
					y--;
			} else if (strcmp(keys, GNT_KEY_DOWN) == 0) {
				if (y + h < ymax)
					y++;
			}
			if (ox != x || oy != y) {
				gnt_screen_move_widget(widget, x, y);
				window_reverse(widget, TRUE, priv);
				return TRUE;
			}
		} else if (priv->mode == GNT_KP_MODE_RESIZE) {
			if (strcmp(keys, GNT_KEY_LEFT) == 0) {
				w--;
			} else if (strcmp(keys, GNT_KEY_RIGHT) == 0) {
				if (x + w < xmax)
					w++;
			} else if (strcmp(keys, GNT_KEY_UP) == 0) {
				h--;
			} else if (strcmp(keys, GNT_KEY_DOWN) == 0) {
				if (y + h < ymax)
					h++;
			}
			if (oh != h || ow != w) {
				gnt_screen_resize_widget(widget, w, h);
				window_reverse(widget, TRUE, priv);
				return TRUE;
			}
		}
		if (strcmp(keys, "\r") == 0 || strcmp(keys, "\033") == 0) {
			window_reverse(widget, FALSE, priv);
			priv->mode = GNT_KP_MODE_NORMAL;
		}
		return TRUE;
	}

	/* Escape to close the window-list or action-list window */
	if (strcmp(keys, "\033") == 0) {
		if (priv->list.window) {
			gnt_widget_destroy(priv->list.window);
			return TRUE;
		}
	} else if (keys[0] == GNT_ESCAPE && isdigit(keys[1]) && keys[2] == '\0') {
		/* Alt+x for quick switch */
		int n = *(keys + 1) - '0';
		GList *list = NULL;

		if (n == 0)
			n = 10;

		list = g_list_append(list, GINT_TO_POINTER(n - 1));
		switch_window_n(GNT_BINDABLE(wm), list);
		g_list_free(list);
		return TRUE;
	}

	if (priv->menu) {
		ret = gnt_widget_key_pressed(GNT_WIDGET(priv->menu), keys);
	} else if (priv->list.window) {
		ret = gnt_widget_key_pressed(priv->list.window, keys);
	} else if (!gnt_ws_is_empty(priv->cws)) {
		GntWidget *win = gnt_ws_get_top_widget(priv->cws);
		if (GNT_IS_WINDOW(win)) {
			GntMenu *menu = gnt_window_get_menu(GNT_WINDOW(win));
			if (menu) {
				const char *id = gnt_window_get_accel_item(GNT_WINDOW(win), keys);
				if (id) {
					GntMenuItem *item = gnt_menu_get_item(menu, id);
					if (item)
						ret = gnt_menuitem_activate(item);
				}
			}
		}
		if (!ret)
			ret = gnt_widget_key_pressed(win, keys);
	}
	return ret;
}

static void
gnt_wm_win_resized(G_GNUC_UNUSED GntWM *wm, G_GNUC_UNUSED GntNode *node)
{
	/*refresh_node(node->me, node, NULL);*/
}

static void
gnt_wm_win_moved(G_GNUC_UNUSED GntWM *wm, GntNode *node)
{
	refresh_node(node->me, node, NULL);
}

void gnt_wm_resize_window(GntWM *wm, GntWidget *widget, int width, int height)
{
	GntWMPrivate *priv = NULL;
	gboolean ret = TRUE;
	GntNode *node;
	int maxx, maxy;

	g_return_if_fail(GNT_IS_WM(wm));
	priv = gnt_wm_get_instance_private(wm);

	widget = gnt_widget_get_toplevel(widget);
	node = g_hash_table_lookup(priv->nodes, widget);
	if (!node)
		return;

	g_signal_emit(wm, signals[SIG_CONFIRM_RESIZE], 0, widget, &width, &height, &ret);
	if (!ret)
		return;    /* resize is not permitted */
	hide_panel(node->panel);
	gnt_widget_set_size(widget, width, height);
	gnt_widget_draw(widget);

	maxx = getmaxx(stdscr);
	maxy = getmaxy(stdscr) - 1;
	height = MIN(height, maxy);
	width = MIN(width, maxx);
	wresize(node->window, height, width);
	replace_panel(node->panel, node->window);

	g_signal_emit(wm, signals[SIG_RESIZED], 0, node);

	show_panel(node->panel);
	update_screen(priv);
}

static void
write_gdi(gpointer key, GntPosition *p, gpointer data)
{
	fprintf(data, ".%s = %d;%d\n", (char *)key, p->x, p->y);
}

static gboolean
write_already(gpointer data)
{
	GntWM *wm = data;
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	FILE *file;
	char *filename;

	filename = g_build_filename(gnt_get_config_dir(), ".gntpositions", NULL);

	file = fopen(filename, "wb");
	if (file == NULL) {
		gnt_warning("error opening file (%s) to save positions", filename);
	} else {
		fprintf(file, "[positions]\n");
		g_hash_table_foreach(priv->positions, (GHFunc)write_gdi, file);
		fclose(file);
	}

	g_free(filename);
	write_timeout = 0;
	return FALSE;
}

static void
write_positions_to_file(GntWM *wm)
{
	if (write_timeout) {
		g_source_remove(write_timeout);
	}
	write_timeout = g_timeout_add_seconds(10, write_already, wm);
}

void gnt_wm_move_window(GntWM *wm, GntWidget *widget, int x, int y)
{
	GntWMPrivate *priv = NULL;
	gboolean ret = TRUE;
	GntNode *node;

	g_return_if_fail(GNT_IS_WM(wm));
	priv = gnt_wm_get_instance_private(wm);

	widget = gnt_widget_get_toplevel(widget);
	node = g_hash_table_lookup(priv->nodes, widget);
	if (!node)
		return;

	g_signal_emit(wm, signals[SIG_CONFIRM_MOVE], 0, widget, &x, &y, &ret);
	if (!ret)
		return;    /* resize is not permitted */

	gnt_widget_set_position(widget, x, y);
	move_panel(node->panel, y, x);

	g_signal_emit(wm, signals[SIG_MOVED], 0, node);
	if (gnt_style_get_bool(GNT_STYLE_REMPOS, TRUE) && GNT_IS_BOX(widget) &&
	    !gnt_widget_get_transient(widget)) {
		const gchar *title = gnt_box_get_title(GNT_BOX(widget));
		if (title) {
			GntPosition *p = g_new0(GntPosition, 1);
			GntWidget *wid = node->me;
			gnt_widget_get_position(wid, &p->x, &p->y);
			g_hash_table_replace(priv->positions, g_strdup(title), p);
			write_positions_to_file(wm);
		}
	}

	update_screen(priv);
}

static void
gnt_wm_give_focus(GntWM *wm, GntWidget *widget)
{
	GntWMPrivate *priv = gnt_wm_get_instance_private(wm);
	GntNode *node = g_hash_table_lookup(priv->nodes, widget);

	if (!node)
		return;

	gnt_widget_set_is_urgent(widget, FALSE);
	if (!gnt_wm_is_list_window(wm, widget) && !GNT_IS_MENU(widget)) {
		gnt_ws_bring_to_front(priv->cws, widget);
	}

	gnt_widget_draw(widget);
	top_panel(node->panel);

	if (priv->list.window) {
		GntNode *nd =
		        g_hash_table_lookup(priv->nodes, priv->list.window);
		top_panel(nd->panel);
	}
	gnt_ws_draw_taskbar(priv->cws, FALSE);
	update_screen(priv);
}

void gnt_wm_update_window(GntWM *wm, GntWidget *widget)
{
	GntWMPrivate *priv = NULL;
	GntNode *node = NULL;
	GntWS *ws;

	g_return_if_fail(GNT_IS_WM(wm));
	priv = gnt_wm_get_instance_private(wm);

	widget = gnt_widget_get_toplevel(widget);
	if (!GNT_IS_MENU(widget)) {
		if (!GNT_IS_BOX(widget))
			return;
		gnt_box_sync_children(GNT_BOX(widget));
	}

	ws = gnt_wm_widget_find_workspace(wm, widget);
	node = g_hash_table_lookup(priv->nodes, widget);
	if (node == NULL) {
		gnt_wm_new_window(wm, widget);
	} else
		g_signal_emit(wm, signals[SIG_UPDATE_WIN], 0, node);

	if (ws == priv->cws || gnt_widget_get_transient(widget)) {
		gnt_wm_copy_win(widget, node);
		gnt_ws_draw_taskbar(priv->cws, FALSE);
		update_screen(priv);
	} else if (ws && ws != priv->cws && gnt_widget_get_is_urgent(widget)) {
		if (!act || !g_list_find(act, ws)) {
			act = g_list_prepend(act, ws);
		}
		update_act_msg();
	}
}

gboolean gnt_wm_process_click(GntWM *wm, GntMouseEvent event, int x, int y, GntWidget *widget)
{
	gboolean ret = TRUE;

	g_return_val_if_fail(GNT_IS_WM(wm), FALSE);

	idle_update = TRUE;
	g_signal_emit(wm, signals[SIG_MOUSE_CLICK], 0, event, x, y, widget, &ret);
	return ret;
}

void gnt_wm_raise_window(GntWM *wm, GntWidget *widget)
{
	GntWMPrivate *priv = NULL;
	GntWS *ws;

	g_return_if_fail(GNT_IS_WM(wm));
	priv = gnt_wm_get_instance_private(wm);

	ws = gnt_wm_widget_find_workspace(wm, widget);

	if (priv->cws != ws) {
		gnt_wm_switch_workspace(wm, g_list_index(priv->workspaces, ws));
	}

	g_return_if_fail(priv->cws != NULL);

	gnt_ws_bring_to_front(priv->cws, widget);
	g_signal_emit(wm, signals[SIG_GIVE_FOCUS], 0, widget);
}

void
gnt_wm_foreach(GntWM *wm, GHFunc func, gpointer user_data)
{
	GntWMPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_WM(wm));
	priv = gnt_wm_get_instance_private(wm);

	g_hash_table_foreach(priv->nodes, func, user_data);
}

gboolean
gnt_wm_has_window_position(GntWM *wm, const gchar *title)
{
	GntWMPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_WM(wm), FALSE);
	priv = gnt_wm_get_instance_private(wm);

	return g_hash_table_lookup(priv->positions, title) != NULL;
}

/* Private. */
void
gnt_wm_set_mainloop(GntWM *wm, GMainLoop *loop)
{
	GntWMPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_WM(wm));
	priv = gnt_wm_get_instance_private(wm);

	priv->loop = loop;
}

/* Private. */
gboolean
gnt_wm_is_list_window(GntWM *wm, GntWidget *widget)
{
	GntWMPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_WM(wm), FALSE);
	priv = gnt_wm_get_instance_private(wm);

	return priv->list.window == widget;
}

/* Private. */
void
gnt_wm_set_place_by_name(GntWM *wm, const gchar *name, GntWS *ws)
{
	GntWMPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_WM(wm));
	priv = gnt_wm_get_instance_private(wm);

	g_hash_table_replace(priv->name_places, g_strdup(name), ws);
}

/* Private. */
void
gnt_wm_set_place_by_title(GntWM *wm, const gchar *title, GntWS *ws)
{
	GntWMPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_WM(wm));
	priv = gnt_wm_get_instance_private(wm);

	g_hash_table_replace(priv->title_places, g_strdup(title), ws);
}

/* Private. */
void
gnt_wm_add_action(GntWM *wm, GntAction *action)
{
	GntWMPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_WM(wm));
	priv = gnt_wm_get_instance_private(wm);

	priv->acts = g_list_append(priv->acts, action);
}

/* Private. */
GntMenu *
gnt_wm_get_menu(GntWM *wm)
{
	GntWMPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_WM(wm), NULL);
	priv = gnt_wm_get_instance_private(wm);

	return priv->menu;
}

/* Private. */
void
gnt_wm_set_menu(GntWM *wm, GntMenu *menu)
{
	GntWMPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_WM(wm));
	priv = gnt_wm_get_instance_private(wm);

	priv->menu = menu;
}

/* Private. */
gboolean
gnt_wm_get_event_stack(GntWM *wm)
{
	GntWMPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_WM(wm), FALSE);
	priv = gnt_wm_get_instance_private(wm);

	return priv->event_stack;
}

/* Private. */
void
gnt_wm_set_event_stack(GntWM *wm, gboolean set)
{
	GntWMPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_WM(wm));
	priv = gnt_wm_get_instance_private(wm);

	priv->event_stack = set;
}

/* Private. */
GntKeyPressMode
gnt_wm_get_keypress_mode(GntWM *wm)
{
	GntWMPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_WM(wm), GNT_KP_MODE_NORMAL);
	priv = gnt_wm_get_instance_private(wm);

	return priv->mode;
}

/* Private. */
void
gnt_wm_set_keypress_mode(GntWM *wm, GntKeyPressMode mode)
{
	GntWMPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_WM(wm));
	priv = gnt_wm_get_instance_private(wm);

	priv->mode = mode;
}
