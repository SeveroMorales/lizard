/*
 * Finch - Universal Text Chat Client
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

#include <purple.h>

#include <gnt.h>

#include "gntidle.h"

struct _FinchIdle {
    GObject parent;
};

/******************************************************************************
 * PurpleIdleUi implementation
 *****************************************************************************/
static time_t
finch_idle_get_idle_time(G_GNUC_UNUSED PurpleIdleUi *ui) {
	return gnt_wm_get_idle_time();
}

static void
finch_idle_purple_idle_ui_init(PurpleIdleUiInterface *iface) {
    iface->get_idle_time = finch_idle_get_idle_time;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE_EXTENDED(
    FinchIdle,
    finch_idle,
    G_TYPE_OBJECT,
    0,
    G_IMPLEMENT_INTERFACE(
        PURPLE_TYPE_IDLE_UI,
        finch_idle_purple_idle_ui_init
    )
);

static void
finch_idle_init(G_GNUC_UNUSED FinchIdle *idle) {
}

static void
finch_idle_class_init(G_GNUC_UNUSED FinchIdleClass *klass) {
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleIdleUi *
finch_idle_new(void) {
    return g_object_new(FINCH_TYPE_IDLE, NULL);
}
