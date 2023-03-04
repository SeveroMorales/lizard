/* purple
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_IDLE_H
#define PURPLE_IDLE_H

#include <time.h>
#include <glib-object.h>

#include <purpleidleui.h>

G_BEGIN_DECLS

/**************************************************************************/
/* Idle API                                                               */
/**************************************************************************/

/**
 * purple_idle_touch:
 *
 * Touch our idle tracker.  This signifies that the user is
 * 'active'.  The conversation code calls this when the
 * user sends an IM, for example.
 */
void purple_idle_touch(void);

/**
 * purple_idle_set:
 *
 * Fake our idle time by setting the time at which our
 * accounts purportedly became idle.  This is used by
 * the I'dle Mak'er plugin.
 */
void purple_idle_set(time_t time);

/**************************************************************************/
/* Idle Subsystem                                                         */
/**************************************************************************/

/**
 * purple_idle_set_ui:
 * @ui: (transfer full): An instance of [iface@IdleUi].
 *
 * Sets the user interface idle reporter.
 *
 * Since: 3.0.0
 */
void purple_idle_set_ui(PurpleIdleUi *ui);

/**
 * purple_idle_get_ui:
 *
 * Gets the current idle reporter.
 *
 * Returns: (transfer none): The [iface@IdleUi] that is currently in use or
 *          %NULL if no idle reporter is available.
 *
 * Since: 3.0.0
 */
PurpleIdleUi *purple_idle_get_ui(void);

/**
 * purple_idle_init:
 *
 * Initializes the idle system.
 */
void purple_idle_init(void);

/**
 * purple_idle_uninit:
 *
 * Uninitializes the idle system.
 */
void purple_idle_uninit(void);

G_END_DECLS

#endif /* PURPLE_IDLE_H */
