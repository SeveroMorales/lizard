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

#ifndef PURPLE_AVATAR_H
#define PURPLE_AVATAR_H

#include <glib.h>
#include <glib-object.h>

#include <gdk-pixbuf/gdk-pixbuf.h>

#include <libpurple/purpletags.h>

G_BEGIN_DECLS

#define PURPLE_TYPE_AVATAR (purple_avatar_get_type())
G_DECLARE_FINAL_TYPE(PurpleAvatar, purple_avatar, PURPLE, AVATAR, GObject)

/**
 * PurpleAvatar:
 *
 * A representation of an Avatar that is used for accounts, contacts, and
 * conversations.
 *
 * Since: 3.0.0
 */

/**
 * purple_avatar_new_from_filename:
 * @filename: The filename for the avatar.
 * @error: Return address for a #GError, or %NULL.
 *
 * Creates a new avatar from @filename.
 *
 * Returns: (transfer full): The new instance.
 *
 * Since: 3.0.0
 */
PurpleAvatar *purple_avatar_new_from_filename(const char *filename, GError **error);

/**
 * purple_avatar_new_from_resource:
 * @resource_path: The path of the resource file.
 * @error: Return address for a #GError, or %NULL.
 *
 * Creates a new avatar from the resource at @resource_path.
 *
 * Returns: (transfer full): The new instance.
 *
 * Since: 3.0.0
 */
PurpleAvatar *purple_avatar_new_from_resource(const char *resource_path, GError **error);

/**
 * purple_avatar_get_filename:
 * @avatar: The instance.
 *
 * Gets the base filename of @avatar.
 *
 * Returns: The base filename of @avatar.
 *
 * Since: 3.0.0
 */
const char *purple_avatar_get_filename(PurpleAvatar *avatar);

/**
 * purple_avatar_get_pixbuf:
 * @avatar: The instance.
 *
 * Gets the [class@GdkPixbuf.Pixbuf] of @avatar.
 *
 * If [property@Purple.Avatar:animated] is %TRUE, this returns a single frame
 * of the animation. To get the animation see
 * [method@Purple.Avatar.get_animation].
 *
 * Returns: (transfer none): The pixbuf of the avatar which could be %NULL.
 *
 * Since: 3.0.0
 */
GdkPixbuf *purple_avatar_get_pixbuf(PurpleAvatar *avatar);

/**
 * purple_avatar_get_animated:
 * @avatar: The instance.
 *
 * Gets whether or not @avatar is animated.
 *
 * Returns: %TRUE if @avatar is animated, %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_avatar_get_animated(PurpleAvatar *avatar);

/**
 * purple_avatar_get_animation:
 * @avatar: The instance.
 *
 * Gets the [class@GdkPixbuf.PixbufAnimation] if
 * [property@Purple.Avatar:animated] is %TRUE, otherwise %NULL.
 *
 * Returns: (transfer none): The animation or %NULL.
 *
 * Since: 3.0.0
 */
GdkPixbufAnimation *purple_avatar_get_animation(PurpleAvatar *avatar);

/**
 * purple_avatar_get_tags:
 * @avatar: The instance.
 *
 * Gets the [class@Purple.Tags] for @avatar.
 *
 * Returns: (transfer none): The [class@Purple.Tags] for @avatar.
 *
 * Since: 3.0.0
 */
PurpleTags *purple_avatar_get_tags(PurpleAvatar *avatar);

G_END_DECLS

#endif /* PURPLE_AVATAR_H */
