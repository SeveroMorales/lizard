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

#ifndef PIDGIN_ACCOUNTS_DISABLED_MENU_H
#define PIDGIN_ACCOUNTS_DISABLED_MENU_H

#include <glib.h>
#include <gio/gio.h>

#include <purple.h>

G_BEGIN_DECLS

#define PIDGIN_TYPE_ACCOUNTS_DISABLED_MENU (pidgin_accounts_disabled_menu_get_type())
G_DECLARE_FINAL_TYPE(PidginAccountsDisabledMenu, pidgin_accounts_disabled_menu,
                     PIDGIN, ACCOUNTS_DISABLED_MENU, GMenuModel)

/**
 * pidgin_accounts_disabled_menu_new:
 *
 * Creates a menu that will automatically update itself to include accounts
 * that are disabled in libpurple.
 *
 * Returns: (transfer full): The new menu instance.
 *
 * Since: 3.0.0
 */
GMenuModel *pidgin_accounts_disabled_menu_new(void);

G_END_DECLS

#endif /* PIDGIN_ACCOUNTS_DISABLED_MENU_H */