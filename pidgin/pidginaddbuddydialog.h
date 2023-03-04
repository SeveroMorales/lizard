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

#ifndef PIDGIN_ADD_BUDDY_DIALOG_H
#define PIDGIN_ADD_BUDDY_DIALOG_H

#include <gtk/gtk.h>

#include <purple.h>

G_BEGIN_DECLS

/**
 * PidginAddBuddyDialog:
 *
 * A dialog for adding buddies to your contact list.
 *
 * Since: 3.0.0
 */

#define PIDGIN_TYPE_ADD_BUDDY_DIALOG (pidgin_add_buddy_dialog_get_type())
G_DECLARE_FINAL_TYPE(PidginAddBuddyDialog, pidgin_add_buddy_dialog, PIDGIN,
                     ADD_BUDDY_DIALOG, GtkDialog)

/**
 * pidgin_add_buddy_dialog_new:
 * @account: (nullable): The [class@Purple.Account] to pre-select.
 * @username: (nullable): The username to pre-fill.
 * @alias: (nullable): The alias to pre-fill.
 * @message: (nullable): The invite message to pre-fill.
 * @group: (nullable): The group anem to pre-fill.
 *
 * Creates a add buddy dialog with the pre-filled optional values.
 *
 * Returns: (transfer full): The widget.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_add_buddy_dialog_new(PurpleAccount *account, const gchar *username, const gchar *alias, const gchar *message, const gchar *group);

G_END_DECLS

#endif /* PIDGIN_ADD_BUDDY_DIALOG_H */
