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

#ifndef PURPLE_CONTACT_INFO_H
#define PURPLE_CONTACT_INFO_H

#include <glib.h>
#include <glib-object.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <libpurple/purplepresence.h>
#include <libpurple/purpletags.h>

G_BEGIN_DECLS

#define PURPLE_TYPE_CONTACT_INFO (purple_contact_info_get_type())
G_DECLARE_DERIVABLE_TYPE(PurpleContactInfo, purple_contact_info, PURPLE,
                         CONTACT_INFO, GObject)

/**
 * PurpleContactInfoPermission:
 * @PURPLE_CONTACT_INFO_PERMISSION_UNSET: The value is unset.
 * @PURPLE_CONTACT_INFO_PERMISSION_ALLOW: The contact is allowed to contact the
 *                                        user.
 * @PURPLE_CONTACT_INFO_PERMISSION_DENY: The contact is not allowed to contact the
 *                                       user.
 *
 * A representation of whether or not a contact has permission to contact the
 * user.
 *
 * Since: 3.0.0
 */
typedef enum {
	PURPLE_CONTACT_INFO_PERMISSION_UNSET = 0,
	PURPLE_CONTACT_INFO_PERMISSION_ALLOW,
	PURPLE_CONTACT_INFO_PERMISSION_DENY,
} PurpleContactInfoPermission;

#include <libpurple/purpleperson.h>

/**
 * PurpleContactInfoClass:
 *
 * The class struct for [class@Purple.ContactInfo].
 *
 * Since: 3.0.0
 */
struct _PurpleContactInfoClass {
	/*< private >*/
	GObjectClass parent;

	/*< private >*/
	gpointer reserved[4];
};

/**
 * PurpleContactInfo:
 *
 * The information about a contact. This information is used everywhere you
 * need to refer to a user. Be it a chat, an direct message, a file transfer,
 * etc.
 */

/**
 * purple_contact_info_new:
 * @id: (nullable): The id of the contact.
 *
 * Creates a new [class@Purple.ContactInfo].
 *
 * If @id is %NULL, an ID will be randomly generated.
 *
 * Returns: (transfer full): The new instance.
 *
 * Since: 3.0.0
 */
PurpleContactInfo *purple_contact_info_new(const gchar *id);

/**
 * purple_contact_info_get_id:
 * @info: The instance.
 *
 * Gets the id of @info.
 *
 * If a protocol would like to set this, it should call
 * [ctor@GObject.Object.new] and pass in the id attribute manually.
 *
 * Returns: The id of the contact.
 *
 * Since: 3.0.0
 */
const gchar *purple_contact_info_get_id(PurpleContactInfo *info);

/**
 * purple_contact_info_set_id:
 * @info: The instance.
 * @id: The new identifier.
 *
 * Sets the identifier of @info to @id. Note, this should be used rarely if
 * at all. The main intent of this, is for protocols to update the id of an
 * an account when it is connected if the id is missing.
 *
 * Since: 3.0.0
 */
void purple_contact_info_set_id(PurpleContactInfo *info, const char *id);

/**
 * purple_contact_info_get_username:
 * @info: The instance.
 *
 * Gets the username of @info.
 *
 * Returns: The username of @info.
 *
 * Since: 3.0.0
 */
const gchar *purple_contact_info_get_username(PurpleContactInfo *info);

/**
 * purple_contact_info_set_username:
 * @info: The instance.
 * @username: The new username.
 *
 * Sets the username of @info to @username.
 *
 * This is primarily used by protocol plugins like IRC when a user changes
 * their "nick" which is their username.
 *
 * Since: 3.0.0
 */
void purple_contact_info_set_username(PurpleContactInfo *info, const gchar *username);

/**
 * purple_contact_info_get_display_name:
 * @info: The instance.
 *
 * Gets the display name for @info. The display name is typically set by the
 * contact and is handled by the protocol plugin.
 *
 * Returns: (nullable): The display name of @info if one is set, otherwise
 *          %NULL will be returned.
 *
 * Since: 3.0.0
 */
const gchar *purple_contact_info_get_display_name(PurpleContactInfo *info);

/**
 * purple_contact_info_set_display_name:
 * @info: The instance.
 * @display_name: (nullable): The new displayname.
 *
 * Sets the display name of @info to @display_name.
 *
 * This should primarily only be used by protocol plugins and everyone else
 * should be using [method@Purple.ContactInfo.set_alias].
 *
 * Since: 3.0.0
 */
void purple_contact_info_set_display_name(PurpleContactInfo *info, const gchar *display_name);

/**
 * purple_contact_info_get_alias:
 * @info: The instance.
 *
 * Gets the alias for @info.
 *
 * Returns: (nullable): The alias of @info if one is set, otherwise %NULL.
 *
 * Since: 3.0.0
 */
const gchar *purple_contact_info_get_alias(PurpleContactInfo *info);

/**
 * purple_contact_info_set_alias:
 * @info: The instance.
 * @alias: (nullable): The new alias.
 *
 * Sets the alias of @info to @alias.
 *
 * Protocol plugins may use this value to synchronize across instances.
 *
 * Since: 3.0.0
 */
void purple_contact_info_set_alias(PurpleContactInfo *info, const gchar *alias);

/**
 * purple_contact_info_get_color:
 * @info: The instance.
 *
 * Gets the color that should be used to render this contact info. This is a
 * RGB hex code in a string format.
 *
 * Returns: The RGB hex code.
 *
 * Since: 3.0.0
 */
const char *purple_contact_info_get_color(PurpleContactInfo *info);

/**
 * purple_contact_info_set_color:
 * @info: The instance.
 * @color: The RGB hex code to set.
 *
 * Sets the color to use when rendering @info to @color.
 *
 * @color should start with a `#` and have a valid number of hex digits
 * following it. Different user interfaces may be able to handle additional
 * precision, but using `#RRGGBB` will have the highest compatibility.
 *
 * Since: 3.0.0
 */
void purple_contact_info_set_color(PurpleContactInfo *info, const char *color);

/**
 * purple_contact_info_get_avatar:
 * @info: The instance.
 *
 * Gets the avatar for @info if one is set.
 *
 * Returns: (transfer none): The avatar if set, otherwise %NULL.
 *
 * Since: 3.0.0
 */
GdkPixbuf *purple_contact_info_get_avatar(PurpleContactInfo *info);

/**
 * purple_contact_info_set_avatar:
 * @info: The instance.
 * @avatar: (nullable): The new avatar to set.
 *
 * Sets the avatar for @info to @avatar. If @avatar is %NULL an existing
 * avatar will be removed.
 *
 * Typically this should only called by the protocol plugin.
 *
 * Since: 3.0.0
 */
void purple_contact_info_set_avatar(PurpleContactInfo *info, GdkPixbuf *avatar);

/**
 * purple_contact_info_get_presence:
 * @info: The instance.
 *
 * Gets the [class@Purple.Presence] for @info.
 *
 * Returns: (transfer none) (nullable): The presence for @info if one is
 *          set, otherwise %NULL.
 *
 * Since: 3.0.0
 */
PurplePresence *purple_contact_info_get_presence(PurpleContactInfo *info);

/**
 * purple_contact_info_get_tags:
 * @info: The instance.
 *
 * Gets the [class@Purple.Tags] instance for @info.
 *
 * Returns: (transfer none): The tags for @info.
 *
 * Since: 3.0.0
 */
PurpleTags *purple_contact_info_get_tags(PurpleContactInfo *info);

/**
 * purple_contact_info_set_person:
 * @info: The instance.
 * @person: (nullable): The new [class@Purple.Person] or %NULL.
 *
 * Sets the person that @info belongs to to @person.
 *
 * Since: 3.0.0
 */
void purple_contact_info_set_person(PurpleContactInfo *info, PurplePerson *person);

/**
 * purple_contact_info_get_person:
 * @info: The instance.
 *
 * Gets the [class@Purple.Person] that @info belongs to.
 *
 * Returns: (transfer none) (nullable): The [class@Purple.Person] that @info
 *          belongs to, or %NULL.
 *
 * Since: 3.0.0
 */
PurplePerson *purple_contact_info_get_person(PurpleContactInfo *info);

/**
 * purple_contact_info_get_permission:
 * @info: The instance.
 *
 * Gets the [enum@Purple.ContactInfoPermission] for @info.
 *
 * Returns: The permission for @info.
 *
 * Since: 3.0.0
 */
PurpleContactInfoPermission purple_contact_info_get_permission(PurpleContactInfo *info);

/**
 * purple_contact_info_set_permission:
 * @info: The instance.
 * @permission: The new permission of the contact.
 *
 * Sets the permission of @info to @permission.
 *
 * Since: 3.0.0
 */
void purple_contact_info_set_permission(PurpleContactInfo *info, PurpleContactInfoPermission permission);

/**
 * purple_contact_info_get_name_for_display:
 * @info: The instance.
 *
 * Gets the name that should be displayed for @info. See
 * [property@Purple.ContactInfo:name-for-display] for more information.
 *
 * Returns: (transfer none): The name to display for @info.
 *
 * Since: 3.0.0
 */
const char *purple_contact_info_get_name_for_display(PurpleContactInfo *info);

/**
 * purple_contact_info_compare:
 * @a: The first instance.
 * @b: The second instance.
 *
 * Compares contacts @a and @b
 *
 * Returns: less than 0 if @a should be sorted before @b, 0 if they sorted
 *          equally, and greater than 0 if @a should be sorted after @b.
 *
 * Since: 3.0.0
 */
int purple_contact_info_compare(PurpleContactInfo *a, PurpleContactInfo *b);

/**
 * purple_contact_info_matches:
 * @info: The instance.
 * @needle: (nullable): The string to match.
 *
 * This will determine if the alias, display name, or username matches @needle.
 * The id is ignored because generally it is a UUID or hex string which will
 * give very confusing results to end users.
 *
 * If @needle is %NULL or empty string, %TRUE will be returned.
 *
 * Returns: %TRUE if @needle matches any of the above properties, otherwise
 *          %FALSE.
 *
 * Since: 3.0.0
 */
gboolean purple_contact_info_matches(PurpleContactInfo *info, const char *needle);

G_END_DECLS

#endif /* PURPLE_CONTACT_INFO_H */
