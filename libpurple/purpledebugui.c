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

#include "purpledebugui.h"

gboolean
purple_debug_ui_is_enabled(PurpleDebugUi *ui, PurpleDebugLevel level,
                           const gchar *category)
{
	PurpleDebugUiInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_DEBUG_UI(ui), FALSE);

	iface = PURPLE_DEBUG_UI_GET_IFACE(ui);
	if(iface != NULL && iface->is_enabled != NULL) {
		return iface->is_enabled(ui, level, category);
	}

	return FALSE;
}

void
purple_debug_ui_print(PurpleDebugUi *ui, PurpleDebugLevel level,
                      const gchar *category, const gchar *arg_s)
{
	PurpleDebugUiInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_DEBUG_UI(ui));

	if(!purple_debug_ui_is_enabled(ui, level, category)) {
		return;
	}

	iface = PURPLE_DEBUG_UI_GET_IFACE(ui);
	if(iface != NULL && iface->print != NULL) {
		iface->print(ui, level, category, arg_s);
	}
}

G_DEFINE_INTERFACE(PurpleDebugUi, purple_debug_ui, G_TYPE_OBJECT);

static void
purple_debug_ui_default_init(G_GNUC_UNUSED PurpleDebugUiInterface *iface) {
}

