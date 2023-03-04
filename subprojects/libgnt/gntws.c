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

#include <gmodule.h>

#include "gntinternal.h"
#include "gntbox.h"
#include "gntwidget.h"
#include "gntwindow.h"
#include "gntwm.h"
#include "gntws.h"

#include "gntwmprivate.h"

typedef struct
{
	gchar *name;
	GList *list;
	GList *ordered;
} GntWSPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(GntWS, gnt_ws, GNT_TYPE_BINDABLE)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
widget_hide(GntWidget *widget, GHashTable *nodes)
{
	GntNode *node = g_hash_table_lookup(nodes, widget);
	if (GNT_IS_WINDOW(widget))
		gnt_window_workspace_hiding(GNT_WINDOW(widget));
	if (node)
		hide_panel(node->panel);
}

static void
widget_show(gpointer data, gpointer nodes)
{
	GntNode *node = g_hash_table_lookup(nodes, data);
	gnt_widget_set_visible(GNT_WIDGET(data), TRUE);
	if (node) {
		show_panel(node->panel);
		gnt_wm_copy_win(GNT_WIDGET(data), node);
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
gnt_ws_destroy(GObject *obj)
{
	GntWS *ws = GNT_WS(obj);
	GntWSPrivate *priv = gnt_ws_get_instance_private(ws);

	g_clear_pointer(&priv->name, g_free);
}

static void
gnt_ws_init(G_GNUC_UNUSED GntWS *ws)
{
}

static void
gnt_ws_class_init(GntWSClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = gnt_ws_destroy;
}

/******************************************************************************
 * GntWS API
 *****************************************************************************/
void
gnt_ws_draw_taskbar(GntWS *ws, gboolean reposition)
{
	static WINDOW *taskbar = NULL;
	GntWSPrivate *priv = NULL;
	GList *iter;
	int n, width = 0;
	int i;

	if (gnt_is_refugee())
		return;

	g_return_if_fail(GNT_IS_WS(ws));
	priv = gnt_ws_get_instance_private(ws);

	if (taskbar == NULL) {
		taskbar = newwin(1, getmaxx(stdscr), getmaxy(stdscr) - 1, 0);
	} else if (reposition) {
		int Y_MAX = getmaxy(stdscr) - 1;
		mvwin(taskbar, Y_MAX, 0);
	}

	wbkgdset(taskbar, '\0' | gnt_color_pair(GNT_COLOR_NORMAL));
	werase(taskbar);

	n = g_list_length(priv->list);
	if (n)
		width = getmaxx(stdscr) / n;

	for (i = 0, iter = priv->list; iter; iter = iter->next, i++) {
		GntWidget *w = iter->data;
		int color;
		const gchar *title;

		if (w == priv->ordered->data) {
			/* This is the current window in focus */
			color = GNT_COLOR_TITLE;
		} else if (gnt_widget_get_is_urgent(w)) {
			/* This is a window with the URGENT hint set */
			color = GNT_COLOR_URGENT;
		} else {
			color = GNT_COLOR_NORMAL;
		}
		wbkgdset(taskbar, '\0' | gnt_color_pair(color));
		if (iter->next)
			mvwhline(taskbar, 0, width * i, ' ' | gnt_color_pair(color), width);
		else
			mvwhline(taskbar, 0, width * i, ' ' | gnt_color_pair(color), getmaxx(stdscr) - width * i);
		title = gnt_box_get_title(GNT_BOX(w));
		mvwprintw(taskbar, 0, width * i, "%s", title ? C_(title) : "<gnt>");
		if (i)
			mvwaddch(taskbar, 0, width *i - 1, ACS_VLINE | A_STANDOUT | gnt_color_pair(GNT_COLOR_NORMAL));
	}
	wrefresh(taskbar);
}

gboolean
gnt_ws_is_empty(GntWS *ws)
{
	GntWSPrivate *priv = NULL;
	g_return_val_if_fail(GNT_IS_WS(ws), TRUE);
	priv = gnt_ws_get_instance_private(ws);
	return priv->ordered == NULL;
}

void gnt_ws_add_widget(GntWS *ws, GntWidget* wid)
{
	GntWSPrivate *priv = NULL;
	GntWidget *oldfocus;

	g_return_if_fail(GNT_IS_WS(ws));
	priv = gnt_ws_get_instance_private(ws);

	oldfocus = priv->ordered ? priv->ordered->data : NULL;
	priv->list = g_list_append(priv->list, wid);
	priv->ordered = g_list_prepend(priv->ordered, wid);
	if (oldfocus)
		gnt_widget_set_focus(oldfocus, FALSE);
}

void gnt_ws_remove_widget(GntWS *ws, GntWidget* wid)
{
	GntWSPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_WS(ws));
	priv = gnt_ws_get_instance_private(ws);

	priv->list = g_list_remove(priv->list, wid);
	priv->ordered = g_list_remove(priv->ordered, wid);
}

GntWidget *
gnt_ws_get_top_widget(GntWS *ws)
{
	GntWSPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_WS(ws), NULL);
	priv = gnt_ws_get_instance_private(ws);

	return priv->ordered ? priv->ordered->data : NULL;
}

GList *
gnt_ws_get_widgets(GntWS *ws)
{
	GntWSPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_WS(ws), NULL);
	priv = gnt_ws_get_instance_private(ws);

	return priv->list;
}

void
gnt_ws_set_name(GntWS *ws, const gchar *name)
{
	GntWSPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_WS(ws));
	priv = gnt_ws_get_instance_private(ws);

	g_free(priv->name);
	priv->name = g_strdup(name);
}

void
gnt_ws_hide(GntWS *ws, GHashTable *nodes)
{
	GntWSPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_WS(ws));
	priv = gnt_ws_get_instance_private(ws);

	g_list_foreach(priv->ordered, (GFunc)widget_hide, nodes);
}

void gnt_ws_widget_hide(GntWidget *widget, GHashTable *nodes)
{
	widget_hide(widget, nodes);
}

void gnt_ws_widget_show(GntWidget *widget, GHashTable *nodes)
{
	widget_show(widget, nodes);
}

void
gnt_ws_show(GntWS *ws, GHashTable *nodes)
{
	GntWSPrivate *priv = NULL;
	GList *l;

	g_return_if_fail(GNT_IS_WS(ws));
	priv = gnt_ws_get_instance_private(ws);

	for (l = g_list_last(priv->ordered); l; l = g_list_previous(l)) {
		widget_show(l->data, nodes);
	}
}

GntWS *
gnt_ws_new(const gchar *name)
{
	GntWS *ws = GNT_WS(g_object_new(GNT_TYPE_WS, NULL));
	GntWSPrivate *priv = gnt_ws_get_instance_private(ws);

	priv->name = g_strdup(name ? name : "(noname)");

	return ws;
}

const gchar *
gnt_ws_get_name(GntWS *ws)
{
	GntWSPrivate *priv = NULL;

	g_return_val_if_fail(GNT_IS_WS(ws), NULL);
	priv = gnt_ws_get_instance_private(ws);

	return priv->name;
}

/* Internal. */
void
gnt_ws_set_list(GntWS *ws, GList *list)
{
	GntWSPrivate *priv = NULL;
	g_return_if_fail(GNT_IS_WS(ws));
	priv = gnt_ws_get_instance_private(ws);
	priv->list = list;
}

/* Internal. */
gboolean
gnt_ws_is_single(GntWS *ws)
{
	GntWSPrivate *priv = NULL;
	g_return_val_if_fail(GNT_IS_WS(ws), FALSE);
	priv = gnt_ws_get_instance_private(ws);
	return priv->ordered != NULL && priv->ordered->next == NULL;
}

/* Internal. */
gboolean
gnt_ws_is_top_widget(GntWS *ws, GntWidget *widget)
{
	GntWSPrivate *priv = NULL;
	g_return_val_if_fail(GNT_IS_WS(ws), FALSE);
	priv = gnt_ws_get_instance_private(ws);
	return priv->ordered && priv->ordered->data == widget;
}

/* Internal. */
GList *
gnt_ws_get_last(GntWS *ws)
{
	GntWSPrivate *priv = NULL;
	g_return_val_if_fail(GNT_IS_WS(ws), NULL);
	priv = gnt_ws_get_instance_private(ws);
	return priv->ordered ? g_list_last(priv->ordered) : NULL;
}

/* Internal.
 * Different from gnt_ws_add_widget in that it doesn't modify focus. */
void
gnt_ws_append_widget(GntWS *ws, GntWidget *widget)
{
	GntWSPrivate *priv = NULL;
	g_return_if_fail(GNT_IS_WS(ws));
	priv = gnt_ws_get_instance_private(ws);
	priv->list = g_list_append(priv->list, widget);
	priv->ordered = g_list_append(priv->ordered, widget);
}

/* Internal. */
void
gnt_ws_bring_to_front(GntWS *ws, GntWidget *widget)
{
	GntWSPrivate *priv = NULL;

	g_return_if_fail(GNT_IS_WS(ws));
	priv = gnt_ws_get_instance_private(ws);

	if (widget != priv->ordered->data) {
		GntWidget *old_widget = priv->ordered->data;
		priv->ordered = g_list_remove(priv->ordered, widget);
		priv->ordered = g_list_prepend(priv->ordered, widget);
		gnt_widget_set_focus(old_widget, FALSE);
		gnt_widget_draw(old_widget);
	}
	gnt_widget_set_focus(widget, TRUE);
	gnt_widget_draw(widget);
}
