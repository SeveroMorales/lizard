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

#ifndef PURPLE_WHITEBOARD_H
#define PURPLE_WHITEBOARD_H

#define PURPLE_TYPE_WHITEBOARD (purple_whiteboard_get_type())
typedef struct _PurpleWhiteboard PurpleWhiteboard;

#include "account.h"

#include <libpurple/purplewhiteboardops.h>

/**
 * PurpleWhiteboard:
 *
 * A abstract whiteboard object.
 */

G_BEGIN_DECLS

/**
 * PurpleWhiteboardClass:
 *
 * The class methods for a PurpleWhiteboard.
 *
 * Right now this is empty but it will be filled out with the function from
 * PurpleWhiteboardOps in a future review request.
 */
struct _PurpleWhiteboardClass {
	/*< private >*/
	GObjectClass parent;

	/*< private >*/
	gpointer reserved[16];
};

/**
 * purple_whiteboard_get_type:
 * The standard _get_type function for #PurpleWhiteboard.
 *
 * Returns: The #GType for the #PurpleWhiteboard object.
 */
G_DECLARE_DERIVABLE_TYPE(PurpleWhiteboard, purple_whiteboard, PURPLE,
                         WHITEBOARD, GObject)

/**
 * purple_whiteboard_set_protocol_ops:
 * @whiteboard: The #PurpleWhiteboard instance.
 * @ops: The #PurpleWhiteboardOps to set.
 *
 * Sets the protocol operations for @whiteboard.
 */
void purple_whiteboard_set_protocol_ops(PurpleWhiteboard *whiteboard, PurpleWhiteboardOps *ops);

/**
 * purple_whiteboard_new:
 * @account: A #PurpleAccount instance.
 * @id: The identifier of the whiteboard.
 * @state: The state.
 *
 * Creates a new whiteboard.
 *
 * Returns: (transfer full): The new #PurpleWhiteboard instance.
 */
PurpleWhiteboard *purple_whiteboard_new(PurpleAccount *account, const gchar *id, gint state);

/**
 * purple_whiteboard_get_account:
 * @whiteboard: The #PurpleWhiteboard instance.
 *
 * Gets the #PurpleAccount that @whiteboard is tied to.
 *
 * Returns: (transfer none): The #PurpleAccount for @whiteboard.
 */
PurpleAccount *purple_whiteboard_get_account(PurpleWhiteboard *whiteboard);

/**
 * purple_whiteboard_get_id:
 * @whiteboard: The #PurpleWhiteboard instance.
 *
 * Gets the id of @whiteboard.
 *
 * Returns: The id of @whiteboard.
 *
 * Since: 3.0.0
 */
const gchar *purple_whiteboard_get_id(PurpleWhiteboard *whiteboard);

/**
 * purple_whiteboard_set_state:
 * @whiteboard: The whiteboard.
 * @state: The state
 *
 * Set the state of @whiteboard to @state.
 */
void purple_whiteboard_set_state(PurpleWhiteboard *whiteboard, gint state);

/**
 * purple_whiteboard_get_state:
 * @whiteboard: The #PurpleWhiteboard instance.
 *
 * Gets the state of @whiteboard.
 *
 * Returns: The state of the @whiteboard.
 */
gint purple_whiteboard_get_state(PurpleWhiteboard *whiteboard);

/**
 * purple_whiteboard_start:
 * @whiteboard: The #PurpleWhiteboard instance.
 *
 * Puts @whiteboard into the started state if it wasn't already.
 */
void purple_whiteboard_start(PurpleWhiteboard *whiteboard);

/**
 * purple_whiteboard_draw_list_destroy:
 * @draw_list: (element-type gint): The drawing list.
 *
 * Destroys a drawing list for a whiteboard
 */
void purple_whiteboard_draw_list_destroy(GList *draw_list);

/**
 * purple_whiteboard_get_dimensions:
 * @whiteboard: The #PurpleWhiteboard instance.
 * @width: (nullable) (out): A return address for the width.
 * @height: (nullable) (out): A return address for the height.
 *
 * Gets the dimension of a whiteboard.
 *
 * Returns: %TRUE if the values of @width and @height were set.
 */
gboolean purple_whiteboard_get_dimensions(PurpleWhiteboard *whiteboard, gint *width, gint *height);

/**
 * purple_whiteboard_set_dimensions:
 * @whiteboard: The #PurpleWhiteboard instance.
 * @width: The new width.
 * @height: The new height.
 *
 * Sets the dimensions for @whiteboard.
 */
void purple_whiteboard_set_dimensions(PurpleWhiteboard *whiteboard, gint width, gint height);

/**
 * purple_whiteboard_draw_point:
 * @whiteboard: The #PurpleWhiteboard instance.
 * @x: The x coordinate.
 * @y: The y coordinate.
 * @color: The color to use.
 * @size: The brush size.
 *
 * Draws a point on @whiteboard with the given parameters.
 */
void purple_whiteboard_draw_point(PurpleWhiteboard *whiteboard, gint x, gint y, gint color, gint size);

/**
 * purple_whiteboard_send_draw_list:
 * @whiteboard: The #PurpleWhiteboard instance.
 * @list: (element-type gint): A GList of points.
 *
 * Send a list of points to draw.
 */
void purple_whiteboard_send_draw_list(PurpleWhiteboard *whiteboard, GList *list);

/**
 * purple_whiteboard_draw_line:
 * @whiteboard: The #PurpleWhiteboard instance.
 * @x1: The top-left x coordinate.
 * @y1: The top-left y coordinate.
 * @x2: The bottom-right x coordinate.
 * @y2: The bottom-right y coordinate.
 * @color: The color to use.
 * @size: The brush size.
 *
 * Draws a line on @whiteboard with the given parameters.
 */
void purple_whiteboard_draw_line(PurpleWhiteboard *whiteboard, gint x1, gint y1, gint x2, gint y2, gint color, gint size);

/**
 * purple_whiteboard_clear:
 * @whiteboard: The #PurpleWhiteboard instance.
 *
 * Clears the contents of @whiteboard.
 */
void purple_whiteboard_clear(PurpleWhiteboard *whiteboard);

/**
 * purple_whiteboard_send_clear:
 * @whiteboard: The #PurpleWhiteboard instance.
 *
 * Sends a request to the buddy to clear @whiteboard.
 */
void purple_whiteboard_send_clear(PurpleWhiteboard *whiteboard);

/**
 * purple_whiteboard_send_brush:
 * @whiteboard: The #PurpleWhiteboard instance.
 * @size: The size of the brush.
 * @color: The color of the brush.
 *
 * Sends a request to change the size and color of the brush.
 */
void purple_whiteboard_send_brush(PurpleWhiteboard *whiteboard, gint size, gint color);

/**
 * purple_whiteboard_get_brush:
 * @whiteboard: The #PurpleWhiteboard instance.
 * @size: (nullable) (out): A return address for the size of the brush.
 * @color: (nullable) (out): A return address for the color of the brush.
 *
 * Gets the size and color of the brush.
 *
 * Returns:	%TRUE if the size and color were set.
 */
gboolean purple_whiteboard_get_brush(PurpleWhiteboard *whiteboard, gint *size, gint *color);

/**
 * purple_whiteboard_set_brush:
 * @whiteboard: The #PurpleWhiteboard instance.
 * @size: The size of the brush.
 * @color: The color of the brush.
 *
 * Sets the size and color of the brush.
 */
void purple_whiteboard_set_brush(PurpleWhiteboard *whiteboard, gint size, gint color);

/**
 * purple_whiteboard_get_draw_list:
 * @whiteboard: The #PurpleWhiteboard instance.
 *
 * Gets the drawing list.
 *
 * Returns: (transfer none) (element-type gint): The drawing list.
 */
GList *purple_whiteboard_get_draw_list(PurpleWhiteboard *whiteboard);

/**
 * purple_whiteboard_set_draw_list:
 * @whiteboard: The #PurpleWhiteboard instance.
 * @draw_list: (element-type gint): The drawing list.
 *
 * Sets the drawing list.
 */
void purple_whiteboard_set_draw_list(PurpleWhiteboard *whiteboard, GList* draw_list);

/**
 * purple_whiteboard_set_protocol_data:
 * @whiteboard: The #PurpleWhiteboard instance.
 * @proto_data: The protocol data to set for the whiteboard.
 *
 * Sets the protocol data for @whiteboard.
 */
void purple_whiteboard_set_protocol_data(PurpleWhiteboard *whiteboard, gpointer proto_data);

/**
 * purple_whiteboard_get_protocol_data:
 * @whiteboard: The #PurpleWhiteboard instance.
 *
 * Gets the protocol data for a whiteboard.
 *
 * Returns: The protocol data for the whiteboard.
 */
gpointer purple_whiteboard_get_protocol_data(PurpleWhiteboard *whiteboard);

/**
 * purple_whiteboard_equal:
 * @whiteboard1: The first #PurpleWhiteboard instance to check.
 * @whiteboard2: The second #PurpleWhiteboard instance to check.
 *
 * Checks the id's for @whiteboard1 and @whiteboard2 and return whether or not
 * they are equal.
 *
 * Returns: %TRUE if the id's of @whiteboard1 and @whiteboard2 are equal.
 *
 * Since: 3.0.0
 */
gboolean purple_whiteboard_equal(PurpleWhiteboard *whiteboard1, PurpleWhiteboard *whiteboard2);

G_END_DECLS

#endif /* PURPLE_WHITEBOARD_H */

