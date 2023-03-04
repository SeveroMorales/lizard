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

#if !defined(GNT_GLOBAL_HEADER_INSIDE) && !defined(GNT_COMPILATION)
# error "only <gnt.h> may be included directly"
#endif

#ifndef GNTWM_H
#define GNTWM_H

#include "gntwidget.h"
#include "gntmenu.h"
#include "gntws.h"

#include <panel.h>
#include <time.h>

#define GNT_TYPE_WM gnt_wm_get_type()

/**
 * GntNode:
 * @me: The widget handled by this node.
 * @window: The ncurses window that backs this node.
 * @scroll: The scroll position of the node.
 * @panel: The ncurses panel that backs this node.
 * @ws: The workspace of this node.
 *
 * A node in the window management tree.
 */
typedef struct {
	GntWidget *me;

	WINDOW *window;
	int scroll;
	PANEL *panel;
	GntWS *ws;
} GntNode;

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(GntWM, gnt_wm, GNT, WM, GntBindable)

/**
 * GntWMClass:
 * @new_window: The class closure for the #GntWM::new-win signal. This is called
 *              when a new window is shown.
 * @decorate_window: The class closure for the #GntWM::decorate-win signal.
 * @close_window: The class closure for the #GntWM::close-win signal. This is
 *                called when a window is being closed.
 * @window_resize_confirm: The class closure for the #GntWM::confirm-resize
 *                         signal. The WM may want to confirm a size for a
 *                         window first.
 * @window_resized: The class closure for the #GntWM::window-resized signal.
 * @window_move_confirm: The class closure for the #GntWM::confirm-move signal.
 *                       The WM may want to confirm the position of a window.
 * @window_moved: The class closure for the #GntWM::window-moved signal.
 * @window_update: The class closure for the #GntWM::window-update signal. This
 *                 gets called when:
 *                 - the title of the window changes
 *                 - the 'urgency' of the window changes
 * @key_pressed: This should usually return %NULL if the keys were processed by
 *               the WM. If not, the WM can simply return the original string,
 *               which will be processed by the default WM. The custom WM can
 *               also return a different static string for the default WM to
 *               process.
 * @mouse_clicked: The class closure for the #GntWM::mouse-clicked signal.
 * @give_focus: The class closure for the #GntWM::give-focus signal. Whatever
 *              the WM wants to do when a window is given focus.
 * @terminal_refresh: The class closure for the #GntWM::terminal-refresh signal.
 *                    This is invoked whenever the terminal window is resized,
 *                    or the screen session is attached to a new terminal.
 *                    (i.e., from the SIGWINCH callback)
 *                    Since: 2.1.0
 *
 * The class structure for #GntWM.
 */
struct _GntWMClass
{
	/*< private >*/
	GntBindableClass parent;

	/*< public >*/

	void (*new_window)(GntWM *wm, GntWidget *widget);
	void (*decorate_window)(GntWM *wm, GntWidget *win);
	gboolean (*close_window)(GntWM *wm, GntWidget *win);

	gboolean (*window_resize_confirm)(GntWM *wm, GntWidget *win, int *w, int *h);
	void (*window_resized)(GntWM *wm, GntNode *node);

	gboolean (*window_move_confirm)(GntWM *wm, GntWidget *win, int *x, int *y);
	void (*window_moved)(GntWM *wm, GntNode *node);

	void (*window_update)(GntWM *wm, GntNode *node);

	gboolean (*key_pressed)(GntWM *wm, const char *key);
	gboolean (*mouse_clicked)(GntWM *wm, GntMouseEvent event, int x, int y, GntWidget *widget);

	void (*give_focus)(GntWM *wm, GntWidget *widget);

	void (*terminal_refresh)(GntWM *wm);

	/*< private >*/
	gpointer reserved[4];
};

/**
 * gnt_wm_get_current_workspace:
 * @wm: The window-manager.
 *
 * Gets the current workspace.
 *
 * Returns: (transfer none): The @GntWS that is currently active.
 *
 * Since: 3.0.0
 */
GntWS *gnt_wm_get_current_workspace(GntWM *wm);

/**
 * gnt_wm_add_workspace:
 * @wm:   The window-manager.
 * @ws:   The workspace to add.
 *
 * Add a workspace.
 */
void gnt_wm_add_workspace(GntWM *wm, GntWS *ws);

/**
 * gnt_wm_switch_workspace:
 * @wm:   The window-manager.
 * @n:    Index of the workspace to switch to.
 *
 * Switch to a workspace.
 *
 * Returns:   %TRUE if the switch was successful.
 */
gboolean gnt_wm_switch_workspace(GntWM *wm, gint n);

/**
 * gnt_wm_switch_workspace_prev:
 * @wm:  The window-manager.
 *
 * Switch to the previous workspace from the current one.
 */
gboolean gnt_wm_switch_workspace_prev(GntWM *wm);

/**
 * gnt_wm_switch_workspace_next:
 * @wm:  The window-manager.
 *
 * Switch to the next workspace from the current one.
 */
gboolean gnt_wm_switch_workspace_next(GntWM *wm);

/**
 * gnt_wm_widget_move_workspace:
 * @wm:     The window manager.
 * @neww:   The new workspace.
 * @widget: The widget to move.
 *
 * Move a window to a specific workspace.
 */
void gnt_wm_widget_move_workspace(GntWM *wm, GntWS *neww, GntWidget *widget);

/**
 * gnt_wm_set_workspaces:
 * @wm:         The window manager.
 * @workspaces: (element-type Gnt.WS): The list of workspaces.
 *
 * Set the list of workspaces .
 */
void gnt_wm_set_workspaces(GntWM *wm, GList *workspaces);

/**
 * gnt_wm_widget_find_workspace:
 * @wm:       The window-manager.
 * @widget:   The widget to find.
 *
 * Find the workspace that contains a specific widget.
 *
 * Returns: (transfer none): The workspace that has the widget.
 */
GntWS *gnt_wm_widget_find_workspace(GntWM *wm, GntWidget *widget);

/**
 * gnt_wm_new_window:
 * @wm:       The window-manager.
 * @widget:   The new window.
 *
 * Process a new window.
 */
void gnt_wm_new_window(GntWM *wm, GntWidget *widget);

/**
 * gnt_wm_window_decorate:
 * @wm:       The window-manager.
 * @widget:   The widget to decorate.
 *
 * Decorate a window.
 */
void gnt_wm_window_decorate(GntWM *wm, GntWidget *widget);

/**
 * gnt_wm_window_close:
 * @wm:       The window-manager.
 * @widget:   The window to close.
 *
 * Close a window.
 */
void gnt_wm_window_close(GntWM *wm, GntWidget *widget);

/**
 * gnt_wm_foreach:
 * @wm:                 The window-manager.
 * @func: (scope call): The function to call for each key/value pair.
 * @user_data:          User data to pass to the function.
 *
 * Calls the given function for each of the #GntWidget / #GntNode pairs in the
 * #GntWM. The function is passed the widget and node of each pair, and the
 * given @user_data parameter. The window manager may not be modified while
 * iterating over it (you can't add/remove widgets).
 *
 * Since: 3.0.0
 */
void gnt_wm_foreach(GntWM *wm, GHFunc func, gpointer user_data);

/**
 * gnt_wm_process_input:
 * @wm:      The window-manager.
 * @string:  The input string to process.
 *
 * Process input.
 *
 * Returns: %TRUE of the string was processed, %FALSE otherwise.
 */
gboolean gnt_wm_process_input(GntWM *wm, const char *string);

/**
 * gnt_wm_process_click:
 * @wm:      The window manager.
 * @event:   The mouse event.
 * @x:       The x-coordinate of the mouse.
 * @y:       The y-coordinate of the mouse.
 * @widget:  The widget under the mouse.
 *
 * Process a click event.
 *
 * Returns:  %TRUE if the event was handled, %FALSE otherwise.
 */
gboolean gnt_wm_process_click(GntWM *wm, GntMouseEvent event, int x, int y, GntWidget *widget);

/**
 * gnt_wm_resize_window:
 * @wm:        The window manager.
 * @widget:    The window to resize.
 * @width:     The desired width of the window.
 * @height:    The desired height of the window.
 *
 * Resize a window.
 */
void gnt_wm_resize_window(GntWM *wm, GntWidget *widget, int width, int height);

/**
 * gnt_wm_move_window:
 * @wm:      The window manager.
 * @widget:  The window to move.
 * @x:       The desired x-coordinate of the window.
 * @y:       The desired y-coordinate of the window.
 *
 * Move a window.
 */
void gnt_wm_move_window(GntWM *wm, GntWidget *widget, int x, int y);

/**
 * gnt_wm_update_window:
 * @wm:      The window-manager.
 * @widget:  The window to update.
 *
 * Update a window.
 */
void gnt_wm_update_window(GntWM *wm, GntWidget *widget);

/**
 * gnt_wm_raise_window:
 * @wm:      The window-manager.
 * @widget:  The window to raise.
 *
 * Raise a window.
 */
void gnt_wm_raise_window(GntWM *wm, GntWidget *widget);

/**
 * gnt_wm_get_idle_time:
 *
 * Returns:  The idle time of the user.
 */
time_t gnt_wm_get_idle_time(void);

/**
 * gnt_wm_has_window_position:
 * @wm:    The window-manager.
 * @title: The title of the window.
 *
 * Checks whether the window manager has a recorded window position for a
 * window with the given @title.
 *
 * Returns: #TRUE if a position is recorded for the window, %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean gnt_wm_has_window_position(GntWM *wm, const gchar *title);

G_END_DECLS

#endif
