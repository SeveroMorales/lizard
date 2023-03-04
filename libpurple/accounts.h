/* purple
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_ACCOUNTS_H
#define PURPLE_ACCOUNTS_H

#include "account.h"
#include "status.h"

G_BEGIN_DECLS

/**************************************************************************/
/* Accounts API                                                           */
/**************************************************************************/

/**
 * purple_accounts_delete:
 * @account: The account.
 *
 * Deletes an account.
 *
 * This will remove any buddies from the buddy list that belong to this
 * account, buddy pounces that belong to this account, and will also
 * destroy @account.
 */
void purple_accounts_delete(PurpleAccount *account);

/**
 * purple_accounts_restore_current_statuses:
 *
 * This is called by the core after all subsystems and what
 * not have been initialized.  It sets all enabled accounts
 * to their startup status by signing them on, setting them
 * away, etc.
 *
 * You probably shouldn't call this unless you really know
 * what you're doing.
 */
void purple_accounts_restore_current_statuses(void);

/**************************************************************************/
/* Accounts Subsystem                                                     */
/**************************************************************************/

/**
 * purple_accounts_get_handle:
 *
 * Returns the accounts subsystem handle.
 *
 * Returns: The accounts subsystem handle.
 */
void *purple_accounts_get_handle(void);

/**
 * purple_accounts_init:
 *
 * Initializes the accounts subsystem.
 */
void purple_accounts_init(void);

/**
 * purple_accounts_uninit:
 *
 * Uninitializes the accounts subsystem.
 */
void purple_accounts_uninit(void);

/**
 * purple_accounts_schedule_save:
 *
 * Schedules saving of accounts
 */
void purple_accounts_schedule_save(void);

G_END_DECLS

#endif /* PURPLE_ACCOUNTS_H */
