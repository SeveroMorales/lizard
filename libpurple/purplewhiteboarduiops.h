/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Purple is the legal property of its developers, whose names are too numerous
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_WHITEBOARD_UIOPS_H
#define PURPLE_WHITEBOARD_UIOPS_H

#include <glib.h>
#include <glib-object.h>

#define PURPLE_TYPE_WHITEBOARD_UI_OPS (purple_whiteboard_ui_ops_get_type())
typedef struct _PurpleWhiteboardUiOps PurpleWhiteboardUiOps;

#include <libpurple/purplewhiteboard.h>

G_BEGIN_DECLS

/**
 * PurpleWhiteboardUiOps:
 * @create:         create whiteboard
 * @destroy:        destroy whiteboard
 * @set_dimensions: set whiteboard dimensions
 * @set_brush:      set the size and color of the brush
 * @draw_point:     draw a point
 * @draw_line:      draw a line
 * @clear:          clear whiteboard
 *
 * The PurpleWhiteboard UI Operations
 */
struct _PurpleWhiteboardUiOps
{
	void (*create)(PurpleWhiteboard *wb);
	void (*destroy)(PurpleWhiteboard *wb);
	void (*set_dimensions)(PurpleWhiteboard *wb, int width, int height);
	void (*set_brush) (PurpleWhiteboard *wb, int size, int color);
	void (*draw_point)(PurpleWhiteboard *wb, int x, int y,
	                   int color, int size);
	void (*draw_line)(PurpleWhiteboard *wb, int x1, int y1,
	                  int x2, int y2,
	                  int color, int size);
	void (*clear)(PurpleWhiteboard *wb);

	/*< private >*/
	gpointer reserved[4];
};

GType purple_whiteboard_ui_ops_get_type(void);

/**
 * purple_whiteboard_set_ui_ops:
 * @ops: The UI operations to set
 *
 * Sets the UI operations
 */
void purple_whiteboard_set_ui_ops(PurpleWhiteboardUiOps *ops);

/**
 * purple_whiteboard_ui_ops_create:
 * @whiteboard: A #PurpleWhiteboard instance.
 *
 * Creates a user interface for @whiteboard.
 */
void purple_whiteboard_ui_ops_create(PurpleWhiteboard *whiteboard);

/**
 * purple_whiteboard_ui_ops_destroy:
 * @whiteboard: A #PurpleWhiteboard instance.
 *
 * Destroys the user interface for @whiteboard.
 */
void purple_whiteboard_ui_ops_destroy(PurpleWhiteboard *whiteboard);

/**
 * purple_whiteboard_ui_ops_set_dimensions:
 * @whiteboard: A #PurpleWhiteboard instance.
 * @width: The new width.
 * @height: The new height.
 *
 * Sets the user interface dimensions for @whiteboard.
 */
void purple_whiteboard_ui_ops_set_dimensions(PurpleWhiteboard *whiteboard, gint width, gint height);

/**
 * purple_whiteboard_ui_ops_set_brush:
 * @whiteboard: A #PurpleWhiteboard instance.
 * @size: The size of the brush.
 * @color: The color to use.
 *
 * Sets the size and color of the active brush for @whiteboard.
 */
void purple_whiteboard_ui_ops_set_brush(PurpleWhiteboard *whiteboard, gint size, gint color);

/**
 * purple_whiteboard_ui_ops_draw_point:
 * @whiteboard: A #PurpleWhiteboard instance.
 * @x: The x coordinate.
 * @y: The y coordinate.
 * @color: The color of the point.
 * @size: The size of the point.
 *
 * Draws a point on @whiteboard.
 */
void purple_whiteboard_ui_ops_draw_point(PurpleWhiteboard *whiteboard, gint x, gint y, gint color, gint size);

/**
 * purple_whiteboard_ui_ops_draw_line:
 * @whiteboard: A #PurpleWhiteboard instance.
 * @x1: The starting point's x coordinate.
 * @y1: The starting point's y coordinate.
 * @x2: The end point's x coordinate.
 * @y2: The end point's y coordinate.
 * @color: The color for the line.
 * @size: The size of the line.
 *
 * Draws a line on @whiteboard.
 */
void purple_whiteboard_ui_ops_draw_line(PurpleWhiteboard *whiteboard, gint x1, gint y1, gint x2, gint y2, gint color, gint size);

/**
 * purple_whiteboard_ui_ops_clear:
 * @whiteboard: A #PurpleWhiteboard instance.
 *
 * Clears all the contents of @whiteboard.
 */
void purple_whiteboard_ui_ops_clear(PurpleWhiteboard *whiteboard);

G_END_DECLS

#endif /* PURPLE_WHITEBOARD_UIOPS_H */
