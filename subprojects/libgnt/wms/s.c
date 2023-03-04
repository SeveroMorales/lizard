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

#include <string.h>
#include <sys/types.h>

#include <gnt.h>

#define GNT_TYPE_S_WM gnt_s_wm_get_type()
G_DECLARE_FINAL_TYPE(GntSWM, gnt_s_wm, GNT, S_WM, GntWM)

struct _GntSWM
{
	GntWM parent;
};

G_DEFINE_TYPE(GntSWM, gnt_s_wm, GNT_TYPE_WM)
void gntwm_init(GntWM **wm);

static void
envelope_main_window(GntWidget *win)
{
	WINDOW *window;
	int w, h;
	window = gnt_widget_get_window(win);
	gnt_widget_get_size(win, &w, &h);
	wresize(window, h, w + 1);
	mvwvline(window, 0, w, ACS_VLINE | COLOR_PAIR(GNT_COLOR_NORMAL), h);
	touchwin(window);
}

static void
envelope_normal_window(GntWidget *win)
{
	WINDOW *window;
	int w, h;

	if (!gnt_widget_get_has_border(win) || gnt_widget_get_transient(win))
		return;

	window = gnt_widget_get_window(win);
	gnt_widget_get_size(win, &w, &h);
	wbkgdset(window, ' ' | COLOR_PAIR(GNT_COLOR_NORMAL));
	mvwprintw(window, 0, w - 4, "[X]");
}

static void
s_decorate_window(G_GNUC_UNUSED GntWM *wm, GntWidget *win)
{
	const char *name;

	name = gnt_widget_get_name(win);
	if (name && strcmp(name, "MainWindow") == 0) {
		envelope_main_window(win);
	} else {
		envelope_normal_window(win);
	}
}

static void
s_window_update(GntWM *wm, GntNode *node)
{
	s_decorate_window(wm, node->me);
}

static void
s_new_window(GntWM *wm, GntWidget *win)
{
	int x, y, w, h;
	int maxx, maxy;
	const char *name;
	gboolean main_window = FALSE;

	g_return_if_fail(win != NULL);
	g_return_if_fail(wm != NULL);

	if (!GNT_IS_MENU(win)) {
		getmaxyx(stdscr, maxy, maxx);

		gnt_widget_get_position(win, &x, &y);
		gnt_widget_get_size(win, &w, &h);

		name = gnt_widget_get_name(win);

		if (name && strcmp(name, "MainWindow") == 0) {
			/* The MainWindow doesn't have no border nor nothing! */
			x = 0;
			y = 0;
			h = maxy - 1;
			main_window = TRUE;

			gnt_box_set_toplevel(GNT_BOX(win), FALSE);
			gnt_widget_set_take_focus(win, TRUE);

			gnt_widget_set_position(win, x, y);
			mvwin(gnt_widget_get_window(win), y, x);

			gnt_widget_set_size(win, -1, h + 2);  /* XXX: Why is the +2 needed here? -- sadrul */
		} else if (!gnt_widget_get_transient(win)) {
			const gchar *title = gnt_box_get_title(GNT_BOX(win));
			if (title == NULL || !gnt_wm_has_window_position(wm, title)) {
				/* In the middle of the screen */
				x = (maxx - w) / 2;
				y = (maxy - h) / 2;

				gnt_widget_set_position(win, x, y);
				mvwin(gnt_widget_get_window(win), y, x);
			}
		}
	}
	GNT_WM_CLASS(gnt_s_wm_parent_class)->new_window(wm, win);

	if (main_window)
		gnt_wm_raise_window(wm, win);
}

static GntWidget *
find_widget(GntWM *wm, const char *wname)
{
	GList *iter = gnt_ws_get_widgets(gnt_wm_get_current_workspace(wm));
	for (; iter; iter = iter->next) {
		GntWidget *widget = iter->data;
		const char *name = gnt_widget_get_name(widget);
		if (name && strcmp(name, wname) == 0) {
			return widget;
		}
	}
	return NULL;
}

static gboolean
s_mouse_clicked(G_GNUC_UNUSED GntWM *wm, GntMouseEvent event, int cx, int cy,
                GntWidget *widget)
{
	int x, y, w, h;

	if (!widget)
		return FALSE;
		/* This might be a place to bring up a context menu */

	if (event != GNT_LEFT_MOUSE_DOWN || !gnt_widget_get_has_border(widget))
		return FALSE;

	gnt_widget_get_position(widget, &x, &y);
	gnt_widget_get_size(widget, &w, &h);

	if (cy == y && cx == x + w - 3) {
		gnt_widget_destroy(widget);
		return TRUE;
	}

	return FALSE;
}

static gboolean
raise_main_window(GntBindable *bindable, G_GNUC_UNUSED GList *params)
{
	GntWM *wm = GNT_WM(bindable);
	GntWidget *main_window = find_widget(wm, "MainWindow");
	if (main_window)
		gnt_wm_raise_window(wm, main_window);
	return TRUE;
}

static void
gnt_s_wm_class_init(GntSWMClass *klass)
{
	GntWMClass *pclass = GNT_WM_CLASS(klass);

	pclass->new_window = s_new_window;
	pclass->decorate_window = s_decorate_window;
	pclass->window_update = s_window_update;
	pclass->mouse_clicked = s_mouse_clicked;

	gnt_bindable_class_register_action(GNT_BINDABLE_CLASS(klass),
			"raise-main-window", raise_main_window,
			"\033" "b", NULL);
	gnt_style_read_actions(G_OBJECT_CLASS_TYPE(klass), GNT_BINDABLE_CLASS(klass));
}

static void
gnt_s_wm_init(G_GNUC_UNUSED GntSWM *self)
{
}

void
gntwm_init(GntWM **wm)
{
	*wm = g_object_new(GNT_TYPE_S_WM, NULL);
}
