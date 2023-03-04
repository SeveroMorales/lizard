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

#ifndef PIDGIN_CONTACT_LIST_WINDOW_H
#define PIDGIN_CONTACT_LIST_WINDOW_H

#include <glib.h>

#include <gtk/gtk.h>

G_BEGIN_DECLS


/**
 * PidginContactListWindow:
 *
 * #PidginContactListWindow is a transitional widget as we slowly migrate it
 * into the conversation window to make a single window application.
 *
 * Since: 3.0.0
 */

#define PIDGIN_TYPE_CONTACT_LIST_WINDOW (pidgin_contact_list_window_get_type())
G_DECLARE_FINAL_TYPE(PidginContactListWindow, pidgin_contact_list_window,
                     PIDGIN, CONTACT_LIST_WINDOW, GtkApplicationWindow)

/**
 * pidgin_contact_list_window_new:
 *
 * Creates a new #PidginContactListWindow instance.
 *
 * Returns: (transfer full): The new #PidginContactListWindow instance.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_contact_list_window_new(void);

/**
 * pidgin_contact_list_window_get_vbox:
 * @window: The #PidginContactListWindow instance.
 *
 * Gets the main vbox for @contact_list.
 *
 * Returns: (transfer none): The main vbox of @window.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_contact_list_window_get_vbox(PidginContactListWindow *window);

/**
 * pidgin_contact_list_window_get_menu_tray:
 * @window: The #PidginContactListWindow instance.
 *
 * Gets the #PidginMenuTray instance from the menu of @window.
 *
 * Returns: (transfer none): The #PidginMenuTray from the menu of @window.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_contact_list_window_get_menu_tray(PidginContactListWindow *window);

G_END_DECLS

#endif /* PIDGIN_CONTACT_LIST_WINDOW_H */
