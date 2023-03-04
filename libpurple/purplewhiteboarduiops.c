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

#include "purplewhiteboarduiops.h"

/******************************************************************************
 * Globals
 *****************************************************************************/
static PurpleWhiteboardUiOps *ui_ops = NULL;

/******************************************************************************
 * Helpers
 *****************************************************************************/
static PurpleWhiteboardUiOps *
purple_whiteboard_ui_ops_copy(PurpleWhiteboardUiOps *ops) {
	PurpleWhiteboardUiOps *ops_new = NULL;

	g_return_val_if_fail(ops != NULL, NULL);

	ops_new = g_new(PurpleWhiteboardUiOps, 1);
	*ops_new = *ops;

	return ops_new;
}

/******************************************************************************
 * Public API
 *****************************************************************************/
G_DEFINE_BOXED_TYPE(PurpleWhiteboardUiOps, purple_whiteboard_ui_ops,
                    purple_whiteboard_ui_ops_copy, g_free)

void
purple_whiteboard_set_ui_ops(PurpleWhiteboardUiOps *ops) {
	ui_ops = ops;
}

void
purple_whiteboard_ui_ops_create(PurpleWhiteboard *whiteboard) {
	if(ui_ops != NULL && ui_ops->create != NULL) {
		ui_ops->create(whiteboard);
	}
}

void
purple_whiteboard_ui_ops_destroy(PurpleWhiteboard *whiteboard) {
	if(ui_ops != NULL && ui_ops->destroy != NULL) {
		ui_ops->destroy(whiteboard);
	}
}

void
purple_whiteboard_ui_ops_set_dimensions(PurpleWhiteboard *whiteboard,
                                        gint width, gint height)
{
	if(ui_ops != NULL && ui_ops->set_dimensions != NULL) {
		ui_ops->set_dimensions(whiteboard, width, height);
	}
}

void
purple_whiteboard_ui_ops_set_brush(PurpleWhiteboard *whiteboard, gint size,
                                   gint color)
{
	if(ui_ops != NULL && ui_ops->set_brush != NULL) {
		ui_ops->set_brush(whiteboard, size, color);
	}
}

void
purple_whiteboard_ui_ops_draw_point(PurpleWhiteboard *whiteboard, gint x,
                                    gint y, gint color, gint size)
{
	if(ui_ops != NULL && ui_ops->draw_point != NULL) {
		ui_ops->draw_point(whiteboard, x, y, color, size);
	}
}

void
purple_whiteboard_ui_ops_draw_line(PurpleWhiteboard *whiteboard, gint x1,
                                   gint y1, gint x2, gint y2, gint color,
                                   gint size)
{
	if(ui_ops != NULL && ui_ops->draw_line != NULL) {
		ui_ops->draw_line(whiteboard, x1, y1, x2, y2, color, size);
	}
}

void
purple_whiteboard_ui_ops_clear(PurpleWhiteboard *whiteboard) {
	if(ui_ops != NULL && ui_ops->clear != NULL) {
		ui_ops->clear(whiteboard);
	}
}
