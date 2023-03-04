/*
 * Pidgin - Internet Messenger
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PIDGIN_GLOBAL_HEADER_INSIDE) && !defined(PIDGIN_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PIDGIN_ACCOUNT_FILTER_CONNECTED_H
#define PIDGIN_ACCOUNT_FILTER_CONNECTED_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * PidginAccountFilterConnected:
 *
 * #PidginAccountFilterConnected is a [class@Gtk.Filter] that will only show
 * accounts that are connected. It's intended to be used with
 * [class@Pidgin.AccountChooser] or a [iface@Gio.ListModel] that contains
 * [class@Purple.Account].
 *
 * Since: 3.0.0
 */

#define PIDGIN_TYPE_ACCOUNT_FILTER_CONNECTED pidgin_account_filter_connected_get_type()
G_DECLARE_FINAL_TYPE(PidginAccountFilterConnected,
                     pidgin_account_filter_connected, PIDGIN,
                     ACCOUNT_FILTER_CONNECTED, GtkFilter)

/**
 * pidgin_account_filter_connected_new:
 *
 * Creates a new #PidginAccountFilterConnected that should be used to filter
 * only online accounts.
 *
 * Returns: (transfer full): The new #PidginAccountFilterConnected instance.
 *
 * Since: 3.0.0
 */
GtkFilter *pidgin_account_filter_connected_new(void);

G_END_DECLS

#endif /* PIDGIN_ACCOUNT_FILTER_CONNECTED_H */
