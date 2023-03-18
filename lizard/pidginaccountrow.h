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

#ifndef PIDGIN_ACCOUNT_ROW_H
#define PIDGIN_ACCOUNT_ROW_H

#include <gtk/gtk.h>

#include <purple.h>

/**
 * PidginAccountRow:
 *
 * A [class@Gtk.ListBoxRow] subclass to display a [class@Purple.Account].
 *
 * Since: 3.0.0
 */

#define PIDGIN_TYPE_ACCOUNT_ROW (pidgin_account_row_get_type())
G_DECLARE_FINAL_TYPE(PidginAccountRow, pidgin_account_row, PIDGIN, ACCOUNT_ROW,
                     GtkListBoxRow)

G_BEGIN_DECLS

/**
 * pidgin_account_row_new:
 * @account: (nullable): The account to represent.
 *
 * Creates a new #PidginAccountRow to display a [class@Purple.Account].
 *
 * Returns: (transfer full): The new account row.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_account_row_new(PurpleAccount *account);

/**
 * pidgin_account_row_get_account:
 * @row: The instance.
 *
 * Gets the [class@Purple.Account] that @row is displaying.
 *
 * Returns: (transfer none): The account if set otherwise %NULL.
 *
 * Since: 3.0.0
 */
PurpleAccount *pidgin_account_row_get_account(PidginAccountRow *row);

/**
 * pidgin_account_row_set_account:
 * @row: The instance.
 * @account: (nullable): The new account to set, or %NULL to unset.
 *
 * Sets the [class@Purple.Account] for @row to display.
 *
 * Since: 3.0.0
 */
void pidgin_account_row_set_account(PidginAccountRow *row, PurpleAccount *account);

G_END_DECLS

#endif /* PIDGIN_ACCOUNT_ROW_H */
