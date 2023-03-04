/*
 * purple
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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_ACCOUNT_PRESENCE_H
#define PURPLE_ACCOUNT_PRESENCE_H

#include "account.h"
#include "purplepresence.h"

G_BEGIN_DECLS

/**
 * PurpleAccountPresence:
 *
 * A #PurpleAccountPresence represent the #PurplePresence for a specific
 * #PurpleAccount.
 */

#define PURPLE_TYPE_ACCOUNT_PRESENCE (purple_account_presence_get_type())
G_DECLARE_FINAL_TYPE(PurpleAccountPresence, purple_account_presence, PURPLE,
                     ACCOUNT_PRESENCE, PurplePresence)

/**
 * purple_account_presence_new:
 * @account: The account to associate with the presence.
 *
 * Creates a presence for an account.
 *
 * Returns: The new presence.
 *
 * Since: 3.0.0
 */
PurpleAccountPresence *purple_account_presence_new(PurpleAccount *account);

/**
 * purple_account_presence_get_account:
 * @presence: The presence.
 *
 * Returns an account presence's account.
 *
 * Returns: (transfer none): The presence's account.
 */
PurpleAccount *purple_account_presence_get_account(PurpleAccountPresence *presence);

G_END_DECLS

#endif /* PURPLE_ACCOUNT_PRESENCE_H */
