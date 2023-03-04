/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PURPLE_CONTACT_MANAGER_H
#define PURPLE_CONTACT_MANAGER_H

#include <glib.h>
#include <glib-object.h>

#include <libpurple/account.h>
#include <libpurple/purplecontact.h>

G_BEGIN_DECLS

#define PURPLE_TYPE_CONTACT_MANAGER (purple_contact_manager_get_type())
G_DECLARE_FINAL_TYPE(PurpleContactManager, purple_contact_manager, PURPLE,
                     CONTACT_MANAGER, GObject)

/**
 * PurpleContactManager:
 *
 * A manager for [class@Purple.Contact]s.
 *
 * Since: 3.0.0
 */

/**
 * purple_contact_manager_get_default:
 *
 * Gets the default instance of [class@Purple.ContactManager].
 *
 * Typically this will be the main way for everyone to access the contact
 * manager.
 *
 * Returns: (transfer none): The default [class@Purple.ContactManager] instance.
 *
 * Since: 3.0.0
 */
PurpleContactManager *purple_contact_manager_get_default(void);

/**
 * purple_contact_manager_add:
 * @manager: The instance.
 * @contact: (transfer none): The [class@Purple.Contact] to add.
 *
 * Adds @contact to @manager. If a contact with a matching account and id
 * already exists, no action will be taken.
 *
 * Since: 3.0.0
 */
void purple_contact_manager_add(PurpleContactManager *manager, PurpleContact *contact);

/**
 * purple_contact_manager_remove:
 * @manager: The instance.
 * @contact: (transfer none): The [class@Purple.Contact] to remove.
 *
 * Attempts to remove @contact from @manager.
 *
 * Returns: If @contact is found and removed %TRUE will be returned otherwise
 *          %FALSE will be returned.
 *
 * Since: 3.0.0
 */
gboolean purple_contact_manager_remove(PurpleContactManager *manager, PurpleContact *contact);

/**
 * purple_contact_manager_remove_all:
 * @manager: The instance.
 * @account: The [class@Purple.Account] whose contacts to remove.
 *
 * Removes all of the contacts from @manager that belong to @account.
 *
 * Returns: %TRUE if anything was removed, %FALSE if nothing was removed.
 *
 * Since: 3.0.0
 */
gboolean purple_contact_manager_remove_all(PurpleContactManager *manager, PurpleAccount *account);

/**
 * purple_contact_manager_get_all:
 * @manager: The instance.
 * @account: The [class@Purple.Account] whose contacts to get.
 *
 * Gets a [iface@Gio.ListModel] of all contacts that belong to @account.
 *
 * Returns: (transfer none) (nullable): A [iface@Gio.ListModel] of all the
 *          contacts belonging to @account.
 *
 * Since: 3.0.0
 */
GListModel *purple_contact_manager_get_all(PurpleContactManager *manager, PurpleAccount *account);

/**
 * purple_contact_manager_find_with_username:
 * @manager: The instance.
 * @account: The [class@Purple.Account] whose contact to find.
 * @username: The username of the contact to find.
 *
 * Looks for a [class@Purple.Contact] that belongs to @account with a username
 * of @username.
 *
 * Returns: (transfer none): The [class@Purple.Contact] if found, otherwise
 *          %NULL.
 *
 * Since: 3.0.0
 */
PurpleContact *purple_contact_manager_find_with_username(PurpleContactManager *manager, PurpleAccount *account, const gchar *username);

/**
 * purple_contact_manager_find_with_id:
 * @manager: The instance.
 * @account: The [class@Purple.Account] whose contact to find.
 * @id: The id of the contact to find.
 *
 * Looks for a [class@Purple.Contact] that belongs to @account with a id of @id.
 *
 * Returns: (transfer none): The [class@Purple.Contact] if found, otherwise
 *          %NULL.
 *
 * Since: 3.0.0
 */
PurpleContact *purple_contact_manager_find_with_id(PurpleContactManager *manager, PurpleAccount *account, const gchar *id);

/**
 * purple_contact_manager_add_buddy:
 * @manager: The instance.
 * @buddy: A [class@Purple.Buddy] instance.
 *
 * Creates a new [class@Purple.Contact] and binds its properties to @buddy and
 * then adds the new [class@Purple.Contact] via
 * [method@Purple.ContactManager.add].
 *
 * This method is meant to help us transition to the new API and this method
 * shouldn't be used elsewhere.
 *
 * Since: 3.0.0
 */
G_DEPRECATED
void purple_contact_manager_add_buddy(PurpleContactManager *manager, PurpleBuddy *buddy);

/**
 * purple_contact_manager_add_person:
 * @manager: The instance.
 * @person: The [class@Purple.Person to add].
 *
 * Adds all of the contacts contained in @person to @manager.
 *
 * This function is mostly intended for unit testing and importing. You
 * typically you won't need to call this directly as @manager will
 * automatically add the [class@Purple.Person] instance when
 * [method@Purple.ContactManager.add] is called.
 *
 * Since: 3.0.0
 */
void purple_contact_manager_add_person(PurpleContactManager *manager, PurplePerson *person);

/**
 * purple_contact_manager_remove_person:
 * @manager: The instance.
 * @person: The [class@Purple.Person] to remove.
 * @remove_contacts: Whether or not the contacts should be removed from
 *                   @manager.
 *
 * Removes @person from @manager optionally removing all of the contacts
 * contained in @person as well if @remove_contacts is %TRUE.
 *
 * Since: 3.0.0
 */
void purple_contact_manager_remove_person(PurpleContactManager *manager, PurplePerson *person, gboolean remove_contacts);

G_END_DECLS

#endif /* PURPLE_CONTACT_MANAGER_H */
