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

#ifndef GNT_WINDOW_H
#define GNT_WINDOW_H

#include "gntbox.h"
#include "gntcolors.h"
#include "gntkeys.h"
#include "gntmenu.h"

#define GNT_TYPE_WINDOW				(gnt_window_get_type())

/**
 * GntWindowFlags:
 * @GNT_WINDOW_MAXIMIZE_X: The window size will be maximized in the
 *                         x-direction.
 * @GNT_WINDOW_MAXIMIZE_Y: The window size will be maximized in the
 *                         y-direction.
 *
 * Flags indicating the maximization state of a #GntWindow.
 */
typedef enum
{
	GNT_WINDOW_MAXIMIZE_X = 1 << 0,
	GNT_WINDOW_MAXIMIZE_Y = 1 << 1,
} GntWindowFlags;

/**
 * GntWindowClass:
 *
 * The class structure for #GntWindow.
 */
struct _GntWindowClass
{
	/*< private >*/
	GntBoxClass parent;

	/*< private >*/
	gpointer reserved[4];
};

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(GntWindow, gnt_window, GNT, WINDOW, GntBox)

/**
 * gnt_hwindow_new:
 * @homogeneous: %TRUE if the widgets inside the window should have the same
 *               dimensions.
 *
 * Create a new window with widgets stacked horizontally.
 *
 * Returns:  The newly created window.
 */
#define gnt_hwindow_new(homogeneous) gnt_window_box_new(homogeneous, FALSE)

/**
 * gnt_vwindow_new:
 * @homogeneous: %TRUE if the widgets inside the window should have the same
 *               dimensions.
 *
 * Create a new window with widgets stacked vertically.
 *
 * Returns:  The newly created window.
 */
#define gnt_vwindow_new(homogeneous) gnt_window_box_new(homogeneous, TRUE)

/**
 * gnt_window_new:
 *
 * Create a new window.
 *
 * Returns: The newly created window.
 */
GntWidget * gnt_window_new(void);

/**
 * gnt_window_box_new:
 * @homogeneous: %TRUE if the widgets inside the window should have the same
 *               dimensions.
 * @vert: %TRUE if the widgets inside the window should be stacked vertically.
 *
 * Create a new window.
 *
 * Returns:  The newly created window.
 */
GntWidget *gnt_window_box_new(gboolean homogeneous, gboolean vert);

/**
 * gnt_window_set_menu:
 * @window:  The window.
 * @menu:    The menu for the window.
 *
 * Set the menu for a window.
 */
void gnt_window_set_menu(GntWindow *window, GntMenu *menu);

/**
 * gnt_window_get_menu:
 * @window:  The window.
 *
 * Get the menu for a window.
 *
 * Returns: (transfer none) (nullable):  The menu for the window.
 *
 * Since: 2.14.0
 */
GntMenu *gnt_window_get_menu(GntWindow *window);

/**
 * gnt_window_get_accel_item:
 * @window:    The window.
 * @key:       The keystroke.
 *
 * Return the id of a menuitem specified to a keystroke.
 *
 * Returns: The id of the menuitem bound to the keystroke, or %NULL.
 *
 * Since: 2.3.0
 */
const char * gnt_window_get_accel_item(GntWindow *window, const char *key);

/**
 * gnt_window_set_maximize:
 * @window:    The window to maximize.
 * @maximize:  The maximization state of the window.
 *
 * Maximize a window, either horizontally or vertically, or both.
 *
 * Since: 2.3.0
 */
void gnt_window_set_maximize(GntWindow *window, GntWindowFlags maximize);

/**
 * gnt_window_get_maximize:
 * @window:  The window.
 *
 * Get the maximization state of a window.
 *
 * Returns:  The maximization state of the window.
 *
 * Since: 2.3.0
 */
GntWindowFlags gnt_window_get_maximize(GntWindow *window);

void gnt_window_workspace_hiding(GntWindow *window);
void gnt_window_workspace_showing(GntWindow *window);

G_END_DECLS

#endif /* GNT_WINDOW_H */
