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

#ifndef PIDGIN_ACCOUNT_EDITOR_H
#define PIDGIN_ACCOUNT_EDITOR_H

#include <gtk/gtk.h>

#include <purple.h>

/**
 * PidginAccountEditor:
 *
 * #PidginAccountEditor is a dialog that allows you to edit an account.
 *
 * Since: 3.0.0
 */

G_BEGIN_DECLS

#define PIDGIN_TYPE_ACCOUNT_EDITOR pidgin_account_editor_get_type()
G_DECLARE_FINAL_TYPE(PidginAccountEditor, pidgin_account_editor, PIDGIN,
                     ACCOUNT_EDITOR, GtkDialog)

/**
 * pidgin_account_editor_new:
 * @account: (nullable): The [class@Purple.Account] to edit.
 *
 * Creates a new #PidginAccountEditor for @account. If @account is %NULL, the
 * editor will create a new account.
 *
 * Returns: (transfer full): The new instance.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_account_editor_new(PurpleAccount *account);

/**
 * pidgin_account_editor_get_account:
 * @editor: The instance.
 *
 * Gets the [class@Purple.Account] that @editor is modifying.
 *
 * Returns: (transfer none): The [class@Purple.Account] or %NULL.
 *
 * Since: 3.0.0
 */
PurpleAccount *pidgin_account_editor_get_account(PidginAccountEditor *editor);

G_END_DECLS

#endif /* PIDGIN_ACCOUNT_EDITOR_H */
