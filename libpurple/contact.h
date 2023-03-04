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

#ifndef PURPLE_META_CONTACT_H
#define PURPLE_META_CONTACT_H

#define PURPLE_TYPE_META_CONTACT             (purple_meta_contact_get_type())
#define PURPLE_META_CONTACT(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), PURPLE_TYPE_META_CONTACT, PurpleMetaContact))
#define PURPLE_META_CONTACT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), PURPLE_TYPE_META_CONTACT, PurpleMetaContactClass))
#define PURPLE_IS_META_CONTACT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), PURPLE_TYPE_META_CONTACT))
#define PURPLE_IS_META_CONTACT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), PURPLE_TYPE_META_CONTACT))
#define PURPLE_META_CONTACT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), PURPLE_TYPE_META_CONTACT, PurpleMetaContactClass))

typedef struct _PurpleMetaContact PurpleMetaContact;
typedef struct _PurpleMetaContactClass PurpleMetaContactClass;

#include "countingnode.h"
#include "group.h"

/**
 * PurpleMetaContact:
 *
 * A contact on the buddy list.
 *
 * A contact is a counting node, which means it keeps track of the counts of
 * the buddies under this contact.
 */
struct _PurpleMetaContact {
	PurpleCountingNode counting;
};

struct _PurpleMetaContactClass {
	PurpleCountingNodeClass counting_class;

	/*< private >*/
	void (*_purple_reserved1)(void);
	void (*_purple_reserved2)(void);
	void (*_purple_reserved3)(void);
	void (*_purple_reserved4)(void);
};

G_BEGIN_DECLS

/**************************************************************************/
/* Contact API                                                            */
/**************************************************************************/

/**
 * purple_meta_contact_get_type:
 *
 * Returns: The #GType for the #PurpleMetaContact object.
 */
GType purple_meta_contact_get_type(void);

/**
 * purple_meta_contact_new:
 *
 * Creates a new contact
 *
 * Returns:       A new contact struct
 */
PurpleMetaContact *purple_meta_contact_new(void);

/**
 * purple_meta_contact_get_group:
 * @contact:  The contact
 *
 * Gets the PurpleGroup from a PurpleMetaContact
 *
 * Returns: (transfer none): The group.
 */
PurpleGroup *purple_meta_contact_get_group(const PurpleMetaContact *contact);

/**
 * purple_meta_contact_get_priority_buddy:
 * @contact:  The contact
 *
 * Returns the highest priority buddy for a given contact.
 *
 * Returns: (transfer none): The highest priority buddy.
 */
PurpleBuddy *purple_meta_contact_get_priority_buddy(PurpleMetaContact *contact);

/**
 * purple_meta_contact_set_alias:
 * @contact:  The contact
 * @alias:    The alias
 *
 * Sets the alias for a contact.
 */
void purple_meta_contact_set_alias(PurpleMetaContact *contact, const char *alias);

/**
 * purple_meta_contact_get_alias:
 * @contact:  The contact
 *
 * Gets the alias for a contact.
 *
 * Returns:  The alias, or NULL if it is not set.
 */
const char *purple_meta_contact_get_alias(PurpleMetaContact *contact);

/**
 * purple_meta_contact_on_account:
 * @contact:  The contact to search through.
 * @account:  The account.
 *
 * Determines whether an account owns any buddies in a given contact
 *
 * Returns: TRUE if there are any buddies from account in the contact, or FALSE otherwise.
 */
gboolean purple_meta_contact_on_account(PurpleMetaContact *contact, PurpleAccount *account);

/**
 * purple_meta_contact_invalidate_priority_buddy:
 * @contact:  The contact
 *
 * Invalidates the priority buddy so that the next call to
 * purple_meta_contact_get_priority_buddy recomputes it.
 */
void purple_meta_contact_invalidate_priority_buddy(PurpleMetaContact *contact);

/**
 * purple_meta_contact_merge:
 * @source:  The contact to merge
 * @node:    The place to merge to (a buddy or contact)
 *
 * Merges two contacts
 *
 * All of the buddies from source will be moved to target
 */
void purple_meta_contact_merge(PurpleMetaContact *source, PurpleBlistNode *node);


G_END_DECLS

#endif /* PURPLE_META_CONTACT_H */

