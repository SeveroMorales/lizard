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

#ifndef PURPLE_PERSON_H
#define PURPLE_PERSON_H

#include <glib.h>
#include <glib-object.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <libpurple/purpletags.h>

G_BEGIN_DECLS

#define PURPLE_TYPE_PERSON (purple_person_get_type())
G_DECLARE_FINAL_TYPE(PurplePerson, purple_person, PURPLE, PERSON, GObject)

#include <libpurple/purplecontactinfo.h>

/**
 * PurplePerson:
 *
 * A collection of [class@Purple.ContactInfo] that contains a user selectable
 * custom avatar and alias.
 *
 * Since: 3.0.0
 */

/**
 * purple_person_new:
 *
 * Creates a new [class@Purple.Person].
 *
 * Returns: (transfer full): The new instance.
 *
 * Since: 3.0.0
 */
PurplePerson *purple_person_new(void);

/**
 * purple_person_get_id:
 * @person: The instance.
 *
 * Gets the id of @person. This is created internally and is read-only.
 *
 * Returns: The id of @person.
 *
 * Since: 3.0.0
 */
const gchar *purple_person_get_id(PurplePerson *person);

/**
 * purple_person_get_alias:
 * @person: The instance.
 *
 * Gets the alias of @person that was set by the libpurple user.
 *
 * This will be %NULL if no alias is set.
 *
 * Returns: (nullable): The alias of @person or %NULL.
 *
 * Since: 3.0.0
 */
const gchar *purple_person_get_alias(PurplePerson *person);

/**
 * purple_person_set_alias:
 * @person: The instance.
 * @alias: (nullable): The new alias.
 *
 * Sets the alias of @person to @alias.
 *
 * This should only be called in direct response to a user interaction to set a
 * custom alias. Protocol plugins should only be setting the alias of
 * [class@Purple.Contact].
 *
 * If @alias is %NULL, then the previous alias is cleared.
 *
 * Since: 3.0.0
 */
void purple_person_set_alias(PurplePerson *person, const gchar *alias);

/**
 * purple_person_get_avatar_for_display:
 * @person: The instance.
 *
 * Gets the avatar that should be displayed for @person. If @person has an
 * avatar set, it will be returned, otherwise this will fallback to the value
 * of [property@Purple.ContactInfo:avatar] for
 * [property@Purple.Person:priority-contact-info].
 *
 * Returns: (transfer none) (nullable): The avatar to display.
 *
 * Since: 3.0.0
 */
GdkPixbuf *purple_person_get_avatar_for_display(PurplePerson *person);

/**
 * purple_person_get_avatar:
 * @person: The instance.
 *
 * Gets the avatar of @person or %NULL if no avatar is set.
 *
 * Returns: (transfer none) (nullable): The avatar for @person.
 *
 * Since: 3.0.0
 */
GdkPixbuf *purple_person_get_avatar(PurplePerson *person);

/**
 * purple_person_set_avatar:
 * @person: The instance.
 * @avatar: (nullable): The new avatar.
 *
 * Sets the avatar of @person to @avatar. If @avatar is %NULL then the previous
 * avatar is cleared.
 *
 * This should only be called in direct response to a user interaction to set a
 * custom avatar. Protocol plugins should only be setting the avatars of
 * [class@Purple.Contact].
 *
 * Since: 3.0.0
 */
void purple_person_set_avatar(PurplePerson *person, GdkPixbuf *avatar);

/**
 * purple_person_get_tags:
 * @person: The instance.
 *
 * Gets the [class@Purple.Tags] instance for @person.
 *
 * Returns: (transfer none): The tags for @person.
 *
 * Since: 3.0.0
 */
PurpleTags *purple_person_get_tags(PurplePerson *person);

/**
 * purple_person_get_name_for_display:
 * @person: The instance.
 *
 * Gets the name that should be displayed for @person. See
 * [property@Purple.Person:name-for-display] for more information.
 *
 * Returns: (transfer none): The name to display for @person.
 *
 * Since: 3.0.0
 */
const char *purple_person_get_name_for_display(PurplePerson *person);

/**
 * purple_person_add_contact:
 * @person: The instance.
 * @info: The [class@Purple.ContactInfo] to add.
 *
 * Adds @info to @person.
 *
 * Duplicate contacts are currently allowed, but that may change at a later
 * time.
 *
 * Since: 3.0.0
 */
void purple_person_add_contact_info(PurplePerson *person, PurpleContactInfo *info);

/**
 * purple_person_remove_contact_info:
 * @person: The instance.
 * @info: The [class@Purple.ContactInfo] to remove.
 *
 * Removes @info from @person.
 *
 * Returns: %TRUE if @info was found and removed otherwise %FALSE.
 *
 * Since: 3.0.0
 */
gboolean purple_person_remove_contact_info(PurplePerson *person, PurpleContactInfo *info);

/**
 * purple_person_get_priority_contact_info:
 * @person: The instance.
 *
 * Gets the contact info for the most available contact info in @person.
 *
 * Returns: (transfer none) (nullable): The priority contact or %NULL if
 *          @person does not have any contacts.
 *
 * Since: 3.0.0
 */
PurpleContactInfo *purple_person_get_priority_contact_info(PurplePerson *person);

/*
 * purple_person_has_contacts:
 * @person: The instance.
 *
 * Gets whether or not @person has any contacts.
 *
 * Returns: %TRUE if @person has at least one contact, otherwise %FALSE.
 *
 * Since: 3.0.0
 */
gboolean purple_person_has_contacts(PurplePerson *person);

/**
 * purple_person_matches:
 * @person: The instance.
 * @needle: (nullable): The string to match on.
 *
 * Checks if the alias matches @needle. This also checks @needle against
 * [method@Purple.ContactInfo.matches] for each [class@Purple.ContactInfo] that
 * @person is tracking.
 *
 * If @needle is %NULL or empty string, %TRUE will be returned.
 *
 * Returns: %TRUE if @person matches @needle in any way.
 *
 * Since: 3.0.0
 */
gboolean purple_person_matches(PurplePerson *person, const char *needle);

G_END_DECLS

#endif /* PURPLE_PERSON_H */
