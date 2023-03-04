/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#ifndef PURPLE_IDLE_UI_H
#define PURPLE_IDLE_UI_H

#include <glib.h>
#include <glib-object.h>

#include <time.h>

G_BEGIN_DECLS

#define PURPLE_TYPE_IDLE_UI (purple_idle_ui_get_type())
G_DECLARE_INTERFACE(PurpleIdleUi, purple_idle_ui, PURPLE, IDLE_UI, GObject)

/**
 * PurpleIdleUiInterface:
 * @get_idle_time: vfunc to get the time that the user interface has been idle.
 *
 * An interface that a user interface can implement to let the core determine
 * idle times.
 *
 * Since: 3.0.0
 */
struct _PurpleIdleUiInterface {
    /*< private >*/
    GTypeInterface parent;

    /*< public >*/
    time_t (*get_idle_time)(PurpleIdleUi *ui);

    /*< private >*/
    gpointer reserved[4];
};

/**
 * purple_idle_ui_get_idle_time:
 * @ui: The idle ui instance.
 *
 * Gets the idle time from the user interface.
 *
 * Returns: The time that the user interface went idle.
 *
 * Since: 3.0.0
 */
time_t purple_idle_ui_get_idle_time(PurpleIdleUi *ui);

G_END_DECLS

#endif /* PURPLE_IDLE_UI_H */
