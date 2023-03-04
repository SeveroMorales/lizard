/*
 * finch
 *
 * Finch is the legal property of its developers, whose names are too numerous
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

#if !defined(FINCH_GLOBAL_HEADER_INSIDE) && !defined(FINCH_COMPILATION)
# error "only <finch.h> may be included directly"
#endif

#ifndef FINCH_DEBUG_H
#define FINCH_DEBUG_H

#include <purple.h>

G_BEGIN_DECLS

/**********************************************************************
 * GNT Debug API
 **********************************************************************/

/**
 * finch_debug_init_handler:
 *
 * Initialize handler for GLib logging system.
 *
 * This must be called early if you want to capture logs at startup, and avoid
 * printing them out.
 *
 * Since: 3.0.0
 */
void finch_debug_init_handler(void);

/**
 * finch_debug_init:
 *
 * Perform necessary initializations.
 *
 * Since: 3.0.0
 */
void finch_debug_init(void);

/**
 * finch_debug_uninit:
 *
 * Perform necessary uninitialization.
 */
void finch_debug_uninit(void);

/**
 * finch_debug_window_show:
 *
 * Show the debug window.
 */
void finch_debug_window_show(void);

G_END_DECLS

#endif /* FINCH_DEBUG_H */

