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

#ifndef PURPLE_DEBUG_H
#define PURPLE_DEBUG_H

#include <glib.h>
#include <glib-object.h>

#include <stdarg.h>

G_BEGIN_DECLS

/**
 * PurpleDebugLevel:
 * @PURPLE_DEBUG_ALL: All debug levels.
 * @PURPLE_DEBUG_MISC: General chatter.
 * @PURPLE_DEBUG_INFO: General operation Information.
 * @PURPLE_DEBUG_WARNING: Warnings.
 * @PURPLE_DEBUG_ERROR: Errors.
 * @PURPLE_DEBUG_FATAL: Fatal errors.
 *
 * Available debug levels.
 */
typedef enum {
	PURPLE_DEBUG_ALL = 0,
	PURPLE_DEBUG_MISC,
	PURPLE_DEBUG_INFO,
	PURPLE_DEBUG_WARNING,
	PURPLE_DEBUG_ERROR,
	PURPLE_DEBUG_FATAL

} PurpleDebugLevel;

#include "purpledebugui.h"

/**
 * purple_debug:
 * @level: The debug level.
 * @category: The category (or %NULL).
 * @format: The format string.
 * @...:  The parameters to insert into the format string.
 *
 * Outputs debug information.
 */
void purple_debug(PurpleDebugLevel level, const gchar *category, const gchar *format, ...) G_GNUC_PRINTF(3, 4);

/**
 * purple_debug_misc:
 * @category: The category or %NULL.
 * @format: The format string.
 * @...:  The parameters to insert into the format string.
 *
 * Outputs misc. level debug information.
 *
 * This is a wrapper for purple_debug(), and uses #PURPLE_DEBUG_MISC as the
 * level.
 *
 * See purple_debug().
 */
void purple_debug_misc(const gchar *category, const gchar *format, ...) G_GNUC_PRINTF(2, 3);

/**
 * purple_debug_info:
 * @category: The category (or %NULL).
 * @format: The format string.
 * @...: The parameters to insert into the format string.
 *
 * Outputs info level debug information.
 *
 * This is a wrapper for purple_debug(), and uses #PURPLE_DEBUG_INFO as
 * the level.
 *
 * See purple_debug().
 */
void purple_debug_info(const gchar *category, const gchar *format, ...) G_GNUC_PRINTF(2, 3);

/**
 * purple_debug_warning:
 * @category: The category or %NULL.
 * @format: The format string.
 * @...: The parameters to insert into the format string.
 *
 * Outputs warning level debug information.
 *
 * This is a wrapper for purple_debug(), and uses #PURPLE_DEBUG_WARNING as the
 * level.
 *
 * See purple_debug().
 */
void purple_debug_warning(const gchar *category, const gchar *format, ...) G_GNUC_PRINTF(2, 3);

/**
 * purple_debug_error:
 * @category: The category or %NULL.
 * @format: The format string.
 * @...: The parameters to insert into the format string.
 *
 * Outputs error level debug information.
 *
 * This is a wrapper for purple_debug(), and uses #PURPLE_DEBUG_ERROR as the
 * level.
 *
 * See purple_debug().
 */
void purple_debug_error(const gchar *category, const gchar *format, ...) G_GNUC_PRINTF(2, 3);

/**
 * purple_debug_fatal:
 * @category: The category (or %NULL).
 * @format: The format string.
 * @...: The parameters to insert into the format string.
 *
 * Outputs fatal error level debug information.
 *
 * This is a wrapper for purple_debug(), and uses #PURPLE_DEBUG_ERROR as the
 * level.
 *
 * See purple_debug().
 */
void purple_debug_fatal(const gchar *category, const gchar *format, ...) G_GNUC_PRINTF(2, 3);

/**
 * purple_debug_set_verbose:
 * @verbose: %TRUE to enable verbose debugging or %FALSE to disable it.
 *
 * Enable or disable verbose debugging.  This ordinarily should only be called
 * by purple_debug_init(), but there are cases where this can be useful for
 * plugins.
 */
void purple_debug_set_verbose(gboolean verbose);

/**
 * purple_debug_is_verbose:
 *
 * Check if verbose logging is enabled.
 *
 * Returns: %TRUE if verbose debugging is enabled, %FALSE if it is not.
 */
gboolean purple_debug_is_verbose(void);

/**
 * purple_debug_set_unsafe:
 * @unsafe: %TRUE to enable debug logging of messages that could potentially
 *          contain passwords and other sensitive information. %FALSE to
 *          disable it.
 *
 * Enable or disable unsafe debugging. This ordinarily should only be called by
 * purple_debug_init(), but there are cases where this can be useful for
 * plugins.
 */
void purple_debug_set_unsafe(gboolean unsafe);

/**
 * purple_debug_is_unsafe:
 *
 * Check if unsafe debugging is enabled. Defaults to %FALSE.
 *
 * Returns: %TRUE if the debug logging of all messages is enabled, %FALSE if
 *          messages that could potentially contain passwords and other
 *          sensitive information are not logged.
 */
gboolean purple_debug_is_unsafe(void);

/******************************************************************************
 * Debug Subsystem
 *****************************************************************************/

/**
 * purple_debug_init:
 *
 * Initializes the debug subsystem.
 */
void purple_debug_init(void);

G_END_DECLS

#endif /* PURPLE_DEBUG_H */
