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

#ifndef GNT_BOX_H
#define GNT_BOX_H

#include "gntwidget.h"

#define GNT_TYPE_BOX gnt_box_get_type()

/**
 * GntAlignment:
 * @GNT_ALIGN_LEFT: Align to the left. Applicable to vertically-oriented boxes
 *                  only.
 * @GNT_ALIGN_RIGHT: Align to the right. Applicable to vertically-oriented
 *                   boxes only.
 * @GNT_ALIGN_MID: Align to the middle. Applies to either orientation.
 * @GNT_ALIGN_TOP: Align to the top. Applicable to horizontally-oriented boxes
 *                 only.
 * @GNT_ALIGN_BOTTOM: Align to the bottom. Applicable to horizontally-oriented
 *                    boxes only.
 *
 * Alignment of contents of #GntBox widgets.
 */
typedef enum
{
	/* These for vertical boxes */
	GNT_ALIGN_LEFT,
	GNT_ALIGN_RIGHT,

	GNT_ALIGN_MID,

	/* These for horizontal boxes */
	GNT_ALIGN_TOP,
	GNT_ALIGN_BOTTOM
} GntAlignment;

/**
 * GntBoxClass:
 *
 * The class structure for #GntBox.
 */
struct _GntBoxClass
{
	/*< private >*/
	GntWidgetClass parent;

	/*< private >*/
	gpointer reserved[4];
};

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(GntBox, gnt_box, GNT, BOX, GntWidget)

/**
 * gnt_hbox_new:
 * @homogeneous: If %TRUE, all the widgets in it will have the same width.
 *
 * Create a new horizontal box.
 *
 * Returns: The new box.
 */
#define gnt_hbox_new(homogeneous) gnt_box_new(homogeneous, FALSE)

/**
 * gnt_vbox_new:
 * @homogeneous: If %TRUE, all the widgets in it will have the same height.
 *
 * Create a new vertical box.
 *
 * Returns: The new box.
 */
#define gnt_vbox_new(homogeneous) gnt_box_new(homogeneous, TRUE)

/**
 * gnt_box_new:
 * @homogeneous: If %TRUE, all the widgets in it will have the same height (or
 *               width).
 * @vert: Whether the widgets in it should be stacked vertically (if %TRUE) or
 *        horizontally (if %FALSE).
 *
 * Create a new box.
 *
 * Returns: The new box.
 */
GntWidget *gnt_box_new(gboolean homogeneous, gboolean vert);

/**
 * gnt_box_get_children:
 * @box: The box
 *
 * Returns a list of the children of the widget.
 *
 * Returns: (element-type GntWidget) (transfer container): A new list
 *          containing the children of the box.
 *
 * Since: 2.14.0
 */
GList *gnt_box_get_children(GntBox *box);

/**
 * gnt_box_add_widget:
 * @box:     The box
 * @widget:  The widget to add
 *
 * Add a widget in the box.
 */
void gnt_box_add_widget(GntBox *box, GntWidget *widget);

/**
 * gnt_box_add_widget_in_front:
 * @box:     The box
 * @widget:  The widget to add
 *
 * Add a widget in the box at its front.
 *
 * Since: 3.0.0
 */
void gnt_box_add_widget_in_front(GntBox *box, GntWidget *widget);

/**
 * gnt_box_set_title:
 * @box:    The box
 * @title:	 The title to set
 *
 * Set a title for the box.
 */
void gnt_box_set_title(GntBox *box, const char *title);

/**
 * gnt_box_get_title:
 * @box: The box
 *
 * Get the title for a box.
 *
 * Returns: The title of the box
 *
 * Since: 3.0.0
 */
const gchar *gnt_box_get_title(GntBox *box);

/**
 * gnt_box_set_pad:
 * @box: The box
 * @pad: The padding to use
 *
 * Set the padding to use between the widgets in the box.
 */
void gnt_box_set_pad(GntBox *box, int pad);

/**
 * gnt_box_set_toplevel:
 * @box: The box
 * @set: %TRUE if it's a toplevel box, %FALSE otherwise.
 *
 * Set whether it's a toplevel box (ie, a window) or not. If a box is toplevel,
 * then it will show borders, the title (if set) and shadow (if enabled in
 * <filename>.gntrc</filename>)
 */
void gnt_box_set_toplevel(GntBox *box, gboolean set);

/**
 * gnt_box_sync_children:
 * @box: The box
 *
 * Reposition and refresh the widgets in the box.
 */
void gnt_box_sync_children(GntBox *box);

/**
 * gnt_box_set_alignment:
 * @box:       The box
 * @alignment: The alignment to use
 *
 * Set the alignment for the widgets in the box.
 */
void gnt_box_set_alignment(GntBox *box, GntAlignment alignment);

/**
 * gnt_box_remove:
 * @box:       The box
 * @widget:    The widget to remove
 *
 * Remove a widget from the box. Calling this does NOT destroy the removed widget.
 */
void gnt_box_remove(GntBox *box, GntWidget *widget);

/**
 * gnt_box_remove_all:
 * @box: The box
 *
 * Remove all widgets from the box. This DOES destroy all widgets in the box.
 */
void gnt_box_remove_all(GntBox *box);

/**
 * gnt_box_readjust:
 * @box:  The box
 *
 * Readjust the size of each child widget, reposition the child widgets and
 * recalculate the size of the box.
 */
void gnt_box_readjust(GntBox *box);

/**
 * gnt_box_set_fill:
 * @box:   The box
 * @fill:  Whether the child widgets should fill the empty space
 *
 * Set whether the widgets in the box should fill the empty spaces.
 */
void gnt_box_set_fill(GntBox *box, gboolean fill);

/**
 * gnt_box_move_focus:
 * @box: The box
 * @dir: The direction. If it's 1, then the focus is moved forwards, if it's
 *            -1, the focus is moved backwards.
 *
 * Move the focus from one widget to the other.
 */
void gnt_box_move_focus(GntBox *box, int dir);

/**
 * gnt_box_give_focus_to_child:
 * @box:       The box
 * @widget:    The child widget to give focus
 *
 * Give focus to a specific child widget.
 */
void gnt_box_give_focus_to_child(GntBox *box, GntWidget *widget);

G_END_DECLS

#endif /* GNT_BOX_H */

