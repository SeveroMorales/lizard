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

#ifndef PIDGIN_CONTACT_LIST_H
#define PIDGIN_CONTACT_LIST_H

#include <glib.h>

#include <adwaita.h>

#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * PidginContactList:
 *
 * #PidginContactList is a widget that displays the [iface@Gio.ListModel] of
 * [class@Purple.Person] from [class@Purple.ContactManager].
 *
 * Since: 3.0.0
 */

#define PIDGIN_TYPE_CONTACT_LIST (pidgin_contact_list_get_type())
G_DECLARE_FINAL_TYPE(PidginContactList, pidgin_contact_list, PIDGIN,
                     CONTACT_LIST, GtkBox)

/**
 * pidgin_contact_list_new:
 *
 * Creates a new #PidginContactList instance that will display
 * [iface@Gio.ListModel] of [class@Purple.Person] from
 * [class@Purple.ContactManager].
 *
 * Returns: (transfer full): The new #PidginContactList instance.
 */
GtkWidget *pidgin_contact_list_new(void);

G_END_DECLS

#endif /* PIDGIN_CONTACT_LIST_H */
