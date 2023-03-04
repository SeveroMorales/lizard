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

#include "purpleidleui.h"

/******************************************************************************
 * GInterface Implementation
 *****************************************************************************/
G_DEFINE_INTERFACE(PurpleIdleUi, purple_idle_ui, G_TYPE_INVALID)

static void
purple_idle_ui_default_init(G_GNUC_UNUSED PurpleIdleUiInterface *iface) {
}

/******************************************************************************
 * Public API
 *****************************************************************************/
time_t
purple_idle_ui_get_idle_time(PurpleIdleUi *ui) {
    PurpleIdleUiInterface *iface = NULL;

    g_return_val_if_fail(PURPLE_IS_IDLE_UI(ui), 0);

    iface = PURPLE_IDLE_UI_GET_IFACE(ui);
    if(iface != NULL && iface->get_idle_time != NULL) {
        return iface->get_idle_time(ui);
    }

    return 0;
}
