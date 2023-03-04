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

#ifndef GNT_WIDGET_H
#define GNT_WIDGET_H

#include <stdio.h>
#include <glib.h>

#include "gntbindable.h"

/* XXX: This will probably move elsewhere */
/**
 * GntMouseEvent:
 * @GNT_LEFT_MOUSE_DOWN: A press of the left (primary) button.
 * @GNT_MIDDLE_MOUSE_DOWN: A press of the middle (secondary) button.
 * @GNT_RIGHT_MOUSE_DOWN: A press of the right (tertiary) button.
 * @GNT_MOUSE_UP: A release of a mouse button.
 * @GNT_MOUSE_SCROLL_UP: A scroll of the mouse wheel up.
 * @GNT_MOUSE_SCROLL_DOWN: A scroll of the mouse wheel down.
 *
 * The type of a mouse event, used in gnt_widget_clicked() and the
 * #GntWidget::clicked signal.
 */
typedef enum
{
	GNT_LEFT_MOUSE_DOWN = 1,
	GNT_RIGHT_MOUSE_DOWN,
	GNT_MIDDLE_MOUSE_DOWN,
	GNT_MOUSE_UP,
	GNT_MOUSE_SCROLL_UP,
	GNT_MOUSE_SCROLL_DOWN
} GntMouseEvent;

G_BEGIN_DECLS

#define GNT_TYPE_WIDGET gnt_widget_get_type()

G_DECLARE_DERIVABLE_TYPE(GntWidget, gnt_widget, GNT, WIDGET, GntBindable)

/**
 * GntWidgetClass:
 * @map: The class closure for the #GntWidget::map signal.
 * @show: This will call draw() and take focus (if it can take focus).
 * @destroy: The class closure for the #GntWidget::destroy signal.
 * @draw: The class closure for the #GntWidget::draw signal. This will draw the
 *        widget.
 * @hide: The class closure for the #GntWidget::hide signal.
 * @expose: The class closure for the #GntWidget::expose signal.
 * @gained_focus: The class closure for the #GntWidget::gained-focus signal.
 * @lost_focus: The class closure for the #GntWidget::lost-focus signal.
 * @size_request: The class closure for the #GntWidget::size-request signal.
 * @confirm_size: The class closure for the #GntWidget::confirm-size signal.
 * @size_changed: The class closure for the #GntWidget::size-changed signal.
 * @set_position: The class closure for the #GntWidget::position-set signal.
 * @key_pressed: The class closure for the #GntWidget::key-pressed signal.
 * @activate: The class closure for the #GntWidget::activate signal.
 * @clicked: The class closure for the #GntWidget::clicked signal.
 *
 * The class structure for #GntWidget.
 */
struct _GntWidgetClass
{
	/*< private >*/
	GntBindableClass parent;

	/*< public >*/
	void (*map)(GntWidget *widget);
	void (*show)(GntWidget *widget);
	void (*destroy)(GntWidget *widget);
	void (*draw)(GntWidget *widget);
	void (*hide)(GntWidget *widget);
	void (*expose)(GntWidget *widget, int x, int y, int width, int height);
	void (*gained_focus)(GntWidget *widget);
	void (*lost_focus)(GntWidget *widget);

	void (*size_request)(GntWidget *widget);
	gboolean (*confirm_size)(GntWidget *widget, int width, int height);
	void (*size_changed)(GntWidget *widget, int w, int h);
	void (*set_position)(GntWidget *widget, int x, int y);
	gboolean (*key_pressed)(GntWidget *widget, const char *keys);
	void (*activate)(GntWidget *widget);
	gboolean (*clicked)(GntWidget *widget, GntMouseEvent event, int x, int y);

	/*< private >*/
	gpointer reserved[4];
};

/**
 * gnt_widget_destroy:
 * @widget: The widget to destroy.
 *
 * Destroy a widget.
 *
 * Emits the "destroy" signal notifying all reference holders that they
 * should release @widget.
 */
void gnt_widget_destroy(GntWidget *widget);

/**
 * gnt_widget_show:
 * @widget:  The widget to show.
 *
 * Show a widget. This should only be used for toplevel widgets. For the rest
 * of the widgets, use #gnt_widget_draw instead.
 */
void gnt_widget_show(GntWidget *widget);

/**
 * gnt_widget_draw:
 * @widget:   The widget to draw.
 *
 * Draw a widget.
 */
void gnt_widget_draw(GntWidget *widget);

/**
 * gnt_widget_hide:
 * @widget:   The widget to hide.
 *
 * Hide a widget.
 */
void gnt_widget_hide(GntWidget *widget);

/**
 * gnt_widget_get_window:
 * @widget:  The widget.
 *
 * Get the Ncurses window of a widget.
 *
 * Returns: (transfer none) (nullable): The widget's window, as a gpointer.
 *          Cast to WINDOW* for use.
 *
 * Since: 3.0.0
 */
gpointer gnt_widget_get_window(GntWidget *widget);

/**
 * gnt_widget_set_parent:
 * @widget:  The widget.
 * @parent:  The parent widget.
 *
 * Set the parent of a widget.
 *
 * This is generally only useful when implementing subclasses of #GntBox.
 *
 * Since: 3.0.0
 */
void gnt_widget_set_parent(GntWidget *widget, GntWidget *parent);

/**
 * gnt_widget_get_parent:
 * @widget:  The widget.
 *
 * Get the parent of a widget.
 *
 * Returns: (transfer none) (nullable): The parent widget.
 *
 * Since: 2.14.0
 */
GntWidget *gnt_widget_get_parent(GntWidget *widget);

/**
 * gnt_widget_get_toplevel:
 * @widget:  The widget.
 *
 * Get the toplevel parent of a widget in the container hierarchy. If widget
 * has no parent widgets, it will be returned as the topmost widget.
 *
 * Returns: (transfer none) (nullable): The toplevel parent widget.
 *
 * Since: 2.14.0
 */
GntWidget *gnt_widget_get_toplevel(GntWidget *widget);

/**
 * gnt_widget_get_position:
 * @widget:  The widget.
 * @x:       Location to store the x-coordinate of the widget.
 * @y:       Location to store the y-coordinate of the widget.
 *
 * Get the position of a widget.
 */
void gnt_widget_get_position(GntWidget *widget, int *x, int *y);

/**
 * gnt_widget_set_position:
 * @widget:   The widget to reposition.
 * @x:        The x-coordinate of the widget.
 * @y:        The x-coordinate of the widget.
 *
 * Set the position of a widget.
 */
void gnt_widget_set_position(GntWidget *widget, int x, int y);

/**
 * gnt_widget_size_request:
 * @widget:  The widget.
 *
 * Request a widget to calculate its desired size.
 */
void gnt_widget_size_request(GntWidget *widget);

/**
 * gnt_widget_get_size:
 * @widget:    The widget.
 * @width:     Location to store the width of the widget.
 * @height:    Location to store the height of the widget.
 *
 * Get the size of a widget.
 */
void gnt_widget_get_size(GntWidget *widget, int *width, int *height);

/**
 * gnt_widget_set_size:
 * @widget:  The widget to resize.
 * @width:   The width of the widget.
 * @height:  The height of the widget.
 *
 * Set the size of a widget.
 *
 * Returns:  If the widget was resized to the new size.
 */
gboolean gnt_widget_set_size(GntWidget *widget, int width, int height);

/**
 * gnt_widget_confirm_size:
 * @widget:   The widget.
 * @width:    The requested width.
 * @height:    The requested height.
 *
 * Confirm a requested a size for a widget.
 *
 * Returns:  %TRUE if the new size was confirmed, %FALSE otherwise.
 */
gboolean gnt_widget_confirm_size(GntWidget *widget, int width, int height);

/**
 * gnt_widget_key_pressed:
 * @widget:  The widget.
 * @keys:    The keypress on the widget.
 *
 * Trigger the key-press callbacks for a widget.
 *
 * Returns:  %TRUE if the key-press was handled, %FALSE otherwise.
 */
gboolean gnt_widget_key_pressed(GntWidget *widget, const char *keys);

/**
 * gnt_widget_clicked:
 * @widget:   The widget.
 * @event:    The mouseevent.
 * @x:        The x-coordinate of the mouse.
 * @y:        The y-coordinate of the mouse.
 *
 * Trigger the 'click' callback of a widget.
 *
 * Returns:  %TRUE if the event was handled, %FALSE otherwise.
 */
gboolean gnt_widget_clicked(GntWidget *widget, GntMouseEvent event, int x, int y);

/**
 * gnt_widget_set_focus:
 * @widget:  The widget.
 * @set:     %TRUE if focus should be given to the widget, %FALSE if focus
 *           should be removed.
 *
 * Give or remove focus to a widget.
 *
 * Returns: %TRUE if the focus has been changed, %FALSE otherwise.
 */
gboolean gnt_widget_set_focus(GntWidget *widget, gboolean set);

/**
 * gnt_widget_activate:
 * @widget:  The widget to activate.
 *
 * Activate a widget. This only applies to widgets that can be activated (eg. GntButton)
 */
void gnt_widget_activate(GntWidget *widget);

/**
 * gnt_widget_set_name:
 * @widget:   The widget.
 * @name:     A new name for the widget.
 *
 * Set the name of a widget.
 */
void gnt_widget_set_name(GntWidget *widget, const gchar *name);

/**
 * gnt_widget_get_name:
 * @widget:   The widget.
 *
 * Get the name of a widget.
 *
 * Returns: The name of the widget.
 */
const gchar *gnt_widget_get_name(GntWidget *widget);

/**
 * gnt_widget_set_take_focus:
 * @widget:   The widget.
 * @set:      %TRUE if the widget can take focus.
 *
 * Set whether a widget can take focus or not.
 */
void gnt_widget_set_take_focus(GntWidget *widget, gboolean set);

/**
 * gnt_widget_get_take_focus:
 * @widget:   The widget.
 *
 * Get whether a widget can take focus or not.
 *
 * Returns:   %TRUE if the widget can take focus.
 *
 * Since: 2.14.0
 */
gboolean gnt_widget_get_take_focus(GntWidget *widget);

/**
 * gnt_widget_set_visible:
 * @widget:  The widget.
 * @set:     Whether the widget is visible or not.
 *
 * Set the visibility of a widget.
 */
void gnt_widget_set_visible(GntWidget *widget, gboolean set);

/**
 * gnt_widget_get_visible:
 * @widget:  The widget.
 *
 * Get the visibility of a widget.
 *
 * Returns:  Whether the widget is visible or not.
 *
 * Since: 2.14.0
 */
gboolean gnt_widget_get_visible(GntWidget *widget);

/**
 * gnt_widget_has_shadow:
 * @widget:  The widget.
 *
 * Check whether the widget has shadows.
 *
 * Returns:  %TRUE if the widget has shadows. This checks both the user-setting
 *          and whether the widget can have shadows at all.
 */
gboolean gnt_widget_has_shadow(GntWidget *widget);

/**
 * gnt_widget_in_destruction:
 * @widget: The widget
 *
 * Returns whether the widget is currently being destroyed.
 *
 * This information can sometimes be used to avoid doing unnecessary work.
 *
 * Returns:  %TRUE if the widget is being destroyed.
 *
 * Since: 2.14.0
 */
gboolean gnt_widget_in_destruction(GntWidget *widget);

/**
 * gnt_widget_set_drawing:
 * @widget:  The widget
 * @drawing: Whether or not the widget is being drawn
 *
 * Marks the widget as being drawn (or not).
 *
 * Since: 2.14.0
 */
void gnt_widget_set_drawing(GntWidget *widget, gboolean drawing);

/**
 * gnt_widget_get_drawing:
 * @widget: The widget
 *
 * Returns whether the widget is currently being drawn.
 *
 * This information can sometimes be used to avoid doing unnecessary work.
 *
 * Returns:  %TRUE if the widget is being drawn.
 *
 * Since: 2.14.0
 */
gboolean gnt_widget_get_drawing(GntWidget *widget);

/**
 * gnt_widget_set_mapped:
 * @widget: The widget
 * @mapped: Whether or not the widget is mapped
 *
 * Marks the widget as being mapped (or not).
 *
 * This should generally only be called from the widget's "map" or "unmap"
 * implementation.
 *
 * Since: 2.14.0
 */
void gnt_widget_set_mapped(GntWidget *widget, gboolean mapped);

/**
 * gnt_widget_get_mapped:
 * @widget: The widget
 *
 * Whether widget is mapped or not.
 *
 * Returns: Whether the widget is mapped or not.
 *
 * Since: 2.14.0
 */
gboolean gnt_widget_get_mapped(GntWidget *widget);

/**
 * gnt_widget_set_has_border:
 * @widget:     The widget
 * @has_border: Whether or not the widget has a border
 *
 * Sets the has-border property on widget to @has_border.
 *
 * Since: 2.14.0
 */
void gnt_widget_set_has_border(GntWidget *widget, gboolean has_border);

/**
 * gnt_widget_get_has_border:
 * @widget: The widget
 *
 * Returns the has-border property on widget.
 *
 * Returns: Whether the widget has a border or not.
 *
 * Since: 2.14.0
 */
gboolean gnt_widget_get_has_border(GntWidget *widget);

/**
 * gnt_widget_set_has_shadow:
 * @widget:     The widget
 * @has_shadow: Whether or not the widget has a shadow
 *
 * Sets the has-shadow property on widget to has_shadow. Note, setting this
 * property does not necessarily mean the widget will have a shadow, depending
 * on its styling.
 *
 * Since: 2.14.0
 */
void gnt_widget_set_has_shadow(GntWidget *widget, gboolean has_shadow);

/**
 * gnt_widget_get_has_shadow:
 * @widget: The widget
 *
 * Returns the has-shadow property on widget. Note, this is a property of the
 * widget, but does not necessarily mean the widget will have a shadow as that
 * depends on its styling. Use gnt_widget_has_shadow() to determine whether the
 * widget will actually have a shadow.
 *
 * Returns: Whether the widget has a shadow set or not.
 *
 * Since: 2.14.0
 */
gboolean gnt_widget_get_has_shadow(GntWidget *widget);

/**
 * gnt_widget_set_has_focus:
 * @widget:    The widget
 * @has_focus: Whether or not the widget has focus
 *
 * Sets the has-focus flag on a widget. Note, setting this flag does not
 * necessarily mean the widget will have focus.
 *
 * This is mostly for internal use; you probably want to use
 * gnt_widget_set_focus() instead.
 *
 * Since: 2.14.0
 */
void gnt_widget_set_has_focus(GntWidget *widget, gboolean has_focus);

/**
 * gnt_widget_get_has_focus:
 * @widget:  The widget
 *
 * Check whether a widget has the focus flag.
 *
 * This is mostly for internal use; you probably want to use
 * gnt_widget_has_focus() instead.
 *
 * Returns:  %TRUE if the widget's focus flag is set, %FALSE otherwise.
 */
gboolean gnt_widget_get_has_focus(GntWidget *widget);

/**
 * gnt_widget_set_is_urgent:
 * @widget: The widget to set the URGENT hint for
 * @urgent: Whether the URGENT hint should be set or not
 *
 * Set the URGENT hint for a widget.
 *
 * Since: 2.14.0
 */
void gnt_widget_set_is_urgent(GntWidget *widget, gboolean urgent);

/**
 * gnt_widget_get_is_urgent:
 * @widget: The widget
 *
 * Returns whether the widget has the URGENT hint set.
 *
 * Returns: Whether the URGENT hint is set or not.
 *
 * Since: 2.14.0
 */
gboolean gnt_widget_get_is_urgent(GntWidget *widget);

/**
 * gnt_widget_set_grow_x:
 * @widget: The widget
 * @grow_x: Whether the widget should grow or not
 *
 * Whether widget should grow in the x direction.
 *
 * Since: 2.14.0
 */
void gnt_widget_set_grow_x(GntWidget *widget, gboolean grow_x);

/**
 * gnt_widget_get_grow_x:
 * @widget: The widget
 *
 * Returns whether the widget should grow in the x direction.
 *
 * Returns: Whether widget should grow in the x direction.
 *
 * Since: 2.14.0
 */
gboolean gnt_widget_get_grow_x(GntWidget *widget);

/**
 * gnt_widget_set_grow_y:
 * @widget: The widget
 * @grow_y: Whether the widget should grow or not
 *
 * Whether widget should grow in the y direction.
 *
 * Since: 2.14.0
 */
void gnt_widget_set_grow_y(GntWidget *widget, gboolean grow_y);

/**
 * gnt_widget_get_grow_y:
 * @widget: The widget
 *
 * Returns whether the widget should grow in the y direction.
 *
 * Returns: Whether widget should grow in the y direction.
 *
 * Since: 2.14.0
 */
gboolean gnt_widget_get_grow_y(GntWidget *widget);

/**
 * gnt_widget_set_transient:
 * @widget:    The widget
 * @transient: Whether the widget is transient or not
 *
 * Whether the widget should be transient.
 *
 * Since: 2.14.0
 */
void gnt_widget_set_transient(GntWidget *widget, gboolean transient);

/**
 * gnt_widget_get_transient:
 * @widget: The widget
 *
 * Returns whether the widget is transient.
 *
 * Returns: Whether the widget should be transient.
 *
 * Since: 2.14.0
 */
gboolean gnt_widget_get_transient(GntWidget *widget);

/**
 * gnt_widget_set_disable_actions:
 * @widget:          The widget
 * @disable_actions: Whether the widget actions should be disabled or not
 *
 * Whether widget actions should be disabled.
 *
 * Since: 2.14.0
 */
void gnt_widget_set_disable_actions(GntWidget *widget,
                                    gboolean disable_actions);

/**
 * gnt_widget_get_disable_actions:
 * @widget: The widget
 *
 * Returns whether the widget actions are disabled.
 *
 * Returns: Whether the widget actions are disabled.
 *
 * Since: 2.14.0
 */
gboolean gnt_widget_get_disable_actions(GntWidget *widget);

G_END_DECLS

#endif /* GNT_WIDGET_H */
