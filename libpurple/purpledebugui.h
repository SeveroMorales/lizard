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

#ifndef PURPLE_DEBUG_UI_H
#define PURPLE_DEBUG_UI_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define PURPLE_TYPE_DEBUG_UI (purple_debug_ui_get_type())

/**
 * PurpleDebugUi:
 *
 * #PurpleDebugUiInterface defines the behavior that libpurple uses to
 * interface the debug API with the user interface.
 *
 * Since: 3.0.0
 */
G_DECLARE_INTERFACE(PurpleDebugUi, purple_debug_ui, PURPLE, DEBUG_UI, GObject)

#include "debug.h"

/**
 * PurpleDebugUiInterface:
 * @print: Called to output a debug string to the UI.
 * @is_enabled: Returns if debug printing is enabled in the UI for a @level and
 *              @category.
 *
 * Debug UI operations.
 */
struct _PurpleDebugUiInterface {
	/*< private >*/
	GTypeInterface parent_iface;

	/*< public >*/
	void (*print)(PurpleDebugUi *ui, PurpleDebugLevel level, const gchar *category, const gchar *arg_s);
	gboolean (*is_enabled)(PurpleDebugUi *ui, PurpleDebugLevel level, const gchar *category);

	/*< private >*/
	gpointer reserved[4];
};

/**
 * purple_debug_ui_is_enabled:
 * @ui: The #PurpleDebugUi instance.
 * @level: The #PurpleDebugLevel.
 * @category: An optional category.
 *
 * Checks if the ui should output messages at the given level and optional
 * category.
 *
 * Typically this function will not need to be called outside of libpurple.
 *
 * Returns: %TRUE if the given level and category will be output by @ui, %FALSE
 *          otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_debug_ui_is_enabled(PurpleDebugUi *ui, PurpleDebugLevel level, const gchar *category);

/**
 * purple_debug_ui_print:
 * @ui: The #PurpleDebugUi instance.
 * @level: The #PurpleDebugLevel.
 * @category: An optional category.
 * @arg_s: The debug string to output.
 *
 * Outputs @arg_s via @ui with the given @level and optional @category.
 *
 * Since: 3.0.0
 */
void purple_debug_ui_print(PurpleDebugUi *ui, PurpleDebugLevel level, const gchar *category, const gchar *arg_s);

G_END_DECLS

#endif /* PURPLE_DEBUG_UI_H */

