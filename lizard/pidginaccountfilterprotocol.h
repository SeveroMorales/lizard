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

#ifndef PIDGIN_ACCOUNT_FILTER_PROTOCOL_H
#define PIDGIN_ACCOUNT_FILTER_PROTOCOL_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * PidginAccountFilterProtocol:
 *
 * #PidginAccountFilterProtocol is a [class@Gtk.Filter] that will only show
 * accounts for the given protocol.  It's intended to be used with
 * [class@Pidgin.AccountChooser] or a [iface@Gio.ListModel] that contains
 * [class@Purple.Account].
 *
 * Since: 3.0.0
 */

#define PIDGIN_TYPE_ACCOUNT_FILTER_PROTOCOL pidgin_account_filter_protocol_get_type()
G_DECLARE_FINAL_TYPE(PidginAccountFilterProtocol,
                     pidgin_account_filter_protocol, PIDGIN,
                     ACCOUNT_FILTER_PROTOCOL, GtkFilter)

/**
 * pidgin_account_filter_protocol_new:
 * @protocol_id: The ID of the protocol to filter for.
 *
 * Creates a new #PidginAccountFilterProtocol that should be used to filter
 * only protocols with the given @protocol_id.
 *
 * Returns: (transfer full): The new #PidginAccountFilterProtocol instance.
 *
 * Since: 3.0.0
 */
GtkFilter *pidgin_account_filter_protocol_new(const gchar *protocol_id);

/**
 * pidgin_account_filter_protocol_get_protocol_id:
 * @filter: The #PidginAccountFilterProtocol instance.
 *
 * Gets the ID of the protocol that @filter is filtering for.
 *
 * Returns: The Protocol ID that @filter is filtering for.
 *
 * Since: 3.0.0
 */
const gchar *pidgin_account_filter_protocol_get_protocol_id(PidginAccountFilterProtocol *filter);

G_END_DECLS

#endif /* PIDGIN_ACCOUNT_FILTER_PROTOCOL_H */
