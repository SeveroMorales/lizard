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

#ifndef PURPLE_WHITEBOARD_OPS_H
#define PURPLE_WHITEBOARD_OPS_H

#include <glib.h>

typedef struct _PurpleWhiteboardOps PurpleWhiteboardOps;

#include "purplewhiteboard.h"

G_BEGIN_DECLS

/**
 * PurpleWhiteboardOps:
 * @start:          start function
 * @end:            end function
 * @get_dimensions: get whiteboard dimensions
 * @set_dimensions: set whiteboard dimensions
 * @get_brush:      get the brush size and color
 * @set_brush:      set the brush size and color
 * @send_draw_list: send_draw_list function
 * @clear:          clear whiteboard
 *
 * Whiteboard protocol operations
 */
struct _PurpleWhiteboardOps
{
	void (*start)(PurpleWhiteboard *wb);
	void (*end)(PurpleWhiteboard *wb);
	void (*get_dimensions)(const PurpleWhiteboard *wb, int *width, int *height);
	void (*set_dimensions)(PurpleWhiteboard *wb, int width, int height);
	void (*get_brush) (const PurpleWhiteboard *wb, int *size, int *color);
	void (*set_brush) (PurpleWhiteboard *wb, int size, int color);
	void (*send_draw_list)(PurpleWhiteboard *wb, GList *draw_list);
	void (*clear)(PurpleWhiteboard *wb);

	/*< private >*/
	gpointer reserved[4];
};

G_END_DECLS

#endif /* PURPLE_WHITEBOARD_OPS_H */
