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

#ifndef GNTWS_H
#define GNTWS_H

#include "gntwidget.h"

#include <panel.h>

G_BEGIN_DECLS

#define GNT_TYPE_WS gnt_ws_get_type()

G_DECLARE_DERIVABLE_TYPE(GntWS, gnt_ws, GNT, WS, GntBindable)

/**
 * GntWSClass:
 * @draw_taskbar: This is never called?
 *
 * The class structure for #GntWS.
 *
 * Since: 2.1.0
 */
struct _GntWSClass
{
	/*< private >*/
	GntBindableClass parent;

	/*< public >*/
	void (*draw_taskbar)(GntWS *ws, gboolean reposition);

	/*< private >*/
	gpointer reserved[4];
};

/**
 * gnt_ws_new:
 * @name:  The desired name of the workspace, or %NULL.
 *
 * Create a new workspace with the specified name.
 *
 * Returns: The newly created workspace.
 *
 * Since: 2.1.0
 */
GntWS *gnt_ws_new(const char *name);

/**
 * gnt_ws_set_name:
 * @ws:    The workspace to rename.
 * @name:  The new name of the workspace.
 *
 * Set the name of a workspace.
 *
 * Since: 2.1.0
 */
void gnt_ws_set_name(GntWS *ws, const gchar *name);

/**
 * gnt_ws_is_empty:
 * @ws: The workspace.
 *
 * Gets whether the workspace contains no widgets.
 *
 * Returns: %TRUE if the workspace is empty, %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean gnt_ws_is_empty(GntWS *ws);

/**
 * gnt_ws_add_widget:
 * @ws:     The workspace.
 * @widget: The widget to add.
 *
 * Add a widget to a workspace.
 *
 * Since: 2.1.0
 */
void gnt_ws_add_widget(GntWS *ws, GntWidget *widget);

/**
 * gnt_ws_remove_widget:
 * @ws:      The workspace
 * @widget:  The widget to remove from the workspace.
 *
 * Remove a widget from a workspace.
 *
 * Since: 2.1.0
 */
void gnt_ws_remove_widget(GntWS *ws, GntWidget *widget);

/**
 * gnt_ws_get_top_widget:
 * @ws: The workspace.
 *
 * Gets the widget that is at the top of the workspace.
 *
 * Returns: (transfer none): The widget at the top of the workspace.
 *
 * Since: 3.0.0
 */
GntWidget *gnt_ws_get_top_widget(GntWS *ws);

/**
 * gnt_ws_get_widgets:
 * @ws: The workspace.
 *
 * Gets all widgets contained in the workspace. This is probably only useful
 * for implementing #GntWM subclasses.
 *
 * Returns: (transfer none) (element-type GntWidget): The list of widgets in
 *          the workspace.
 *
 * Since: 3.0.0
 */
GList *gnt_ws_get_widgets(GntWS *ws);

/**
 * gnt_ws_widget_hide:
 * @widget:  The widget to hide.
 * @nodes:   A hashtable containing information about the widgets.
 *
 * Hide a widget in a workspace.
 *
 * Since: 2.1.0
 */
void gnt_ws_widget_hide(GntWidget *widget, GHashTable *nodes);

/**
 * gnt_ws_widget_show:
 * @widget:   The widget to show.
 * @nodes:   A hashtable containing information about the widgets.
 *
 * Show a widget in a workspace.
 *
 * Since: 2.1.0
 */
void gnt_ws_widget_show(GntWidget *widget, GHashTable *nodes);

/**
 * gnt_ws_draw_taskbar:
 * @ws:         The workspace.
 * @reposition: Whether the workspace should reposition the taskbar.
 *
 * Draw the taskbar in a workspace.
 *
 * Since: 2.1.0
 */
void gnt_ws_draw_taskbar(GntWS *ws, gboolean reposition);

/**
 * gnt_ws_hide:
 * @ws:      The workspace to hide.
 * @table:   A hashtable containing information about the widgets.
 *
 * Hide a workspace.
 *
 * Since: 2.1.0
 */
void gnt_ws_hide(GntWS *ws, GHashTable *table);

/**
 * gnt_ws_show:
 * @ws:      The workspace to hide.
 * @table:   A hashtable containing information about the widgets.
 *
 * Show a workspace.
 *
 * Since: 2.1.0
 */
void gnt_ws_show(GntWS *ws, GHashTable *table);

/**
 * gnt_ws_get_name:
 * @ws:   The workspace.
 *
 * Get the name of a workspace.
 *
 * Returns:  The name of the workspace (can be %NULL).
 *
 * Since: 2.1.0
 */
const char * gnt_ws_get_name(GntWS *ws);

#endif
