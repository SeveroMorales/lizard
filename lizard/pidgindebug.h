/* pidgin
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#if !defined(PIDGIN_GLOBAL_HEADER_INSIDE) && !defined(PIDGIN_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PIDGIN_DEBUG_H
#define PIDGIN_DEBUG_H

#include <purple.h>

G_BEGIN_DECLS

#define PIDGIN_TYPE_DEBUG_WINDOW (pidgin_debug_window_get_type())
G_DECLARE_FINAL_TYPE(PidginDebugWindow, pidgin_debug_window, PIDGIN, DEBUG_WINDOW, GtkWindow)

/**
 * pidgin_debug_init_handler:
 *
 * Initialize handler for GLib logging system.
 *
 * This must be called early if you want to capture logs at startup, and avoid
 * printing them out.
 *
 * Since: 3.0.0
 */
void pidgin_debug_init_handler(void);

/**
 * pidgin_debug_set_print_enabled:
 *
 * Set whether the debug logging messages are sent the default GLib logging handler.
 *
 * This will print to the console, if Pidgin is run from there.
 *
 * Since: 3.0.0
 */
void pidgin_debug_set_print_enabled(gboolean enable);

/**
 * pidgin_debug_init:
 *
 * Perform necessary initializations.
 *
 * Since: 3.0.0
 */
void pidgin_debug_init(void);

/**
 * pidgin_debug_uninit:
 *
 * Perform necessary uninitializations.
 *
 * Since: 3.0.0
 */
void pidgin_debug_uninit(void);

/**
 * pidgin_debug_get_handle:
 *
 * Get the handle for the GTK debug system.
 *
 * Returns: the handle to the debug system
 */
void *pidgin_debug_get_handle(void);

/**
 * pidgin_debug_window_show:
 *
 * Shows the debug window.
 */
void pidgin_debug_window_show(void);

/**
 * pidgin_debug_window_hide:
 *
 * Hides the debug window.
 */
void pidgin_debug_window_hide(void);

G_END_DECLS

#endif /* PIDGIN_DEBUG_H */
